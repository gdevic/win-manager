/******************************************************************************
*                                                                             *
*   Module:     Init.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       7/25/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the initialization code for the Virtual Windows
        Manager.  It also contains the exit cleanup code.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 7/25/97    1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <windows.h>                    // Include Windows support
#include <stdio.h>                      // Include standard I/O header file
#include <malloc.h>                     // Include memory operation defines
#include <commctrl.h>                   // Include common controls

#include "Globals.h"                    // Include global definitions

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

extern int nTimer;                      // Timer handle

/******************************************************************************
*                                                                             *
*   Functions Imported from DLL                                               *
*                                                                             *
******************************************************************************/

__declspec (dllimport) BOOL SetHookHandle( HHOOK, HHOOK );
__declspec (dllimport) LRESULT CALLBACK ShellHook( int, WPARAM, LPARAM );
__declspec (dllimport) LRESULT CALLBACK KbdHook( int, WPARAM, LPARAM );

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static char * sTip = NAME;

static HHOOK hShHook;                   // Shell hook handle
static HHOOK hKbdHook;                  // Keyboard hook handle

static HINSTANCE hInstDll;              // Instance of VWMDll.dll

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   BOOL VWM_Create()                                                         *
*                                                                             *
*******************************************************************************
*
*   This is a global init function that is called on WM_CREATE message.
*
*   Where:
*       hWnd is the window handle
*
*   Returns:
*       True if init successful, False to abort initialization.
*
******************************************************************************/
BOOL VWM_Create( HWND hWnd )
{
    RECT rcRect;
    TInfo * pInfo;
    int i;


    // Set the global VWM window handle

    hVWM = hWnd;

    // Global initialization
    // ========================================================================

    // Get the desktop size

    hDesktop = GetDesktopWindow();
    GetClientRect( hDesktop, &rcRect );
    Desktop.x = rcRect.right;
    Desktop.y = rcRect.bottom;

    // Get the client size

    GetClientRect( hVWM, &rcRect );
    VClientSize.x = rcRect.right;
    VClientSize.y = rcRect.bottom;

    VCur.x = 0;         // Current virtual window
    VCur.y = 0;

    fLocked = FALSE;

    // Calculate all the transformations, but not the client windows
    // (nWinTop==0)

    RecalculateClientPos( 1.0, 1.0 );


    // Init hash table to empty

    for( i=0; i<MAX_HASH_TABLE; i++)
    {
        Info[i].hWin = 0;
    }

    // Set some initial classes to ignore

    sIgnoreClasses[0] = "tooltips_class32";
    sIgnoreClasses[1] = "Progman";
    sIgnoreClasses[2] = VWM_CLASS;
    sIgnoreClasses[3] = NULL;

    // ======================================================================
    //  Enumerate windows
    // ======================================================================

    nWinTop = 0;

    if( EnumWindows( WinEnumProc, (LPARAM)&nWinTop ) == FALSE )
    {
        // Enumeration had failed.  Cannot do anything then, so quit

        MessageBox( NULL, "Cannot enumerate windows!", sVWMCaption,
                MB_ICONERROR + MB_SETFOREGROUND + MB_OK );

        return( FALSE );
    }

    // If any of the windows is initially maximized or minimized, we need to
    // show it using their normal size, store their normal size and position,
    // and then restore them as they were

    for( i=0; i<nWinTop; i++ )
    {
        // Find the windows info

        pInfo = GetWinInfo( Win[i] );

        // The entry should always be matched !

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("Init: Warning");
            continue;
        }

        // If a window is minimized or maximized,

        if( (pInfo->dwFlags & INFO_MINIMIZED) ||
            (pInfo->dwFlags & INFO_MAXIMIZED) )
        {
            // Show it normal-sized

            pInfo->pl.showCmd = SW_SHOWNORMAL;

            SetWindowPlacement( pInfo->hWin, &pInfo->pl );
        }
    }


    // Set the timer that will update window placement

    nTimer = SetTimer( hVWM, 1, VWM_TIMER, NULL );

    if( nTimer==0 )
    {
        // Unable to register timer

        MessageBox( NULL, "Cannot register timer!", sVWMCaption,
                MB_ICONERROR + MB_SETFOREGROUND + MB_OK );

        return( -1 );
    }

    // Create the secondary buffer display context

    UseDC( GETDC_CREATE );

    // Load the background bitmap

    if( UseBitmap( 0, USEBITMAP_LOAD ) == FALSE )
    {
        // Loading of bitmap failed

        fBitmap = FALSE;
    }

    // Add an icon to the taskbar notification area

    TrayMessage( hVWM, NIM_ADD, VWMID_TASKBAR, LoadIcon( VWMInst, "Icon" ), sTip );

    SendMessage( hVWM, VWM_WM_INIT, 0, 0 );


    // Set the shell and keyboard hooks that are in the dll
    // =====================================================

    hInstDll = (HINSTANCE) GetModuleHandle("VWMDll.dll");

    hShHook = SetWindowsHookEx( WH_CALLWNDPROC, (HOOKPROC) ShellHook, hInstDll, 0 );
    hKbdHook = SetWindowsHookEx( WH_KEYBOARD, (HOOKPROC) KbdHook, hInstDll, 0 );

    if( hShHook==NULL || hKbdHook==NULL)
    {
        MessageBox( NULL, "Can't register the hook", VWM_CAPTION, MB_ICONSTOP | MB_OK );
    }

    if( !SetHookHandle(hShHook, hKbdHook) )
    {
//      MessageBox( NULL, "Another instance is running", VWM_CAPTION, MB_ICONSTOP | MB_OK );
    }


    // If any of the windows is initially maximized or minimized, they are now
    // normal size.  We will loop again and restore their original state since
    // we have installed hook to refresh the window data structures.

    for( i=0; i<nWinTop; i++ )
    {
        // Find the windows info

        pInfo = GetWinInfo( Win[i] );

        // The entry should always be matched !

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("Init(2): Warning");
            continue;
        }

        // If a window was minimized or maximized,

        if( (pInfo->dwFlags & INFO_MINIMIZED) ||
            (pInfo->dwFlags & INFO_MAXIMIZED) )
        {
            // Show it as it was before

            if( pInfo->dwFlags & INFO_MAXIMIZED )
                pInfo->pl.showCmd = SW_SHOWMAXIMIZED;
            else
                pInfo->pl.showCmd = SW_SHOWMINIMIZED;

            SetWindowPlacement( pInfo->hWin, &pInfo->pl );
        }
    }


    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void VWM_Close()                                                          *
*                                                                             *
*******************************************************************************
*
*   This is a global closing function that is called when VWM is exiting.
*
*
******************************************************************************/
void VWM_Close()
{
    // Remove the hooks

    UnhookWindowsHookEx( hShHook );
    UnhookWindowsHookEx( hKbdHook );
}

