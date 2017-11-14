/******************************************************************************
*                                                                             *
*   Module:     VWMDll.c                                                      *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       6/24/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module implements the DLL portion of Virtual Windows Manager.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
*  6/24/97   1.00  Original                                       Goran Devic *
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
*   DLL Export Functoins                                                      *
*                                                                             *
******************************************************************************/

__declspec (dllexport) BOOL SetHookHandle( HHOOK, HHOOK );
__declspec (dllexport) LRESULT CALLBACK ShellHook( int, WPARAM, LPARAM );
__declspec (dllexport) LRESULT CALLBACK KbdHook( int, WPARAM, LPARAM );

/******************************************************************************
*                                                                             *
*   Global Variables                                                          *
*                                                                             *
******************************************************************************/

HWND  __far hVWMDll;
HHOOK __far hShHook = NULL;
HHOOK __far hKbdHook = NULL;

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   BOOL SetHookHandle( HHOOK hShHnd, HHOOK hKbdHnd )                         *
*                                                                             *
*******************************************************************************
*
*   Sets the hook handle so the hook can be nested in ShellHook and KbdHook.
*
*   Where:
*       hHookHnd is the shell hook handle.
#       hKbdHnd is the keyboard hook handle.
*
*   Returns:
*       TRUE if this is a first call to this function
#       FALSE if this is not the first call, handle is already registered
*
******************************************************************************/
__declspec (dllexport) BOOL SetHookHandle( HHOOK hShHnd, HHOOK hKbdHnd )
{
    // If the hook handle is already defined, return with error

    if( hShHook != NULL || hKbdHook != NULL )
        return( FALSE );

    hShHook = hShHnd;
    hKbdHook = hKbdHnd;

    // Find the handle of the parent window

    hVWMDll = FindWindow( VWM_CLASS, VWM_CAPTION );

    if( hVWMDll != NULL )
        return( TRUE );

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   ShellHook( int nCode, WPARAM wParam, LPARAM lParam )                      *
*                                                                             *
*******************************************************************************
*
*   This is a shell hook
*
*
******************************************************************************/
__declspec (dllexport) LRESULT ShellHook( int nCode, WPARAM wParam, LPARAM lParam )
{
    CWPSTRUCT * cwp;

    cwp = (CWPSTRUCT *) lParam;

    if( cwp->message==WM_ACTIVATE ||
        cwp->message==WM_WINDOWPOSCHANGED )
    {
        PostMessage( hVWMDll, VWM_WM_REFRESH, wParam, lParam );
    }

    return( CallNextHookEx( hShHook, nCode, wParam, lParam ) );
}


/******************************************************************************
*                                                                             *
*   KbdHook( int nCode, WPARAM wParam, LPARAM lParam )                        *
*                                                                             *
*******************************************************************************
*
*   This is a keyboard hook.
*
*
******************************************************************************/
__declspec (dllexport) LRESULT KbdHook( int nCode, WPARAM wParam, LPARAM lParam )
{
    if( nCode==HC_ACTION && (lParam & (KF_ALTDOWN << 16)) && (lParam & (KF_UP << 16)))
    {
        if( wParam==VK_LEFT || wParam==VK_RIGHT ||
            wParam==VK_UP   || wParam==VK_DOWN )
        {
            PostMessage( hVWMDll, VWM_WM_HOTKEY, wParam, 0 );
        }
    }

    // Allow other windows to get the message

    return( 0 );
}

