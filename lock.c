/******************************************************************************
*                                                                             *
*   Module:     Lock.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       7/30/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the lock/unlock of the console.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 7/30/97    1.00  Original                                       Goran Devic *
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

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

#define SECURE              1           // Compile all the security stuff ?

#define ID_MYLOCK           103         // Id of the lock edit control
#define ID_MYACCEPT         104         // Id of the `Accept' button
#define ID_MYCANCEL         105         // Id of the `Cancel' button

#define LOCK_MAX_PASSWD     10          // Max password len


static union
{
    WORD wLen;
    char sPasswd[LOCK_MAX_PASSWD+1];

} Passwd;

static char sGoodPasswd[LOCK_MAX_PASSWD+1];

static RECT rcCursor;                   // Cursor confinement area

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

BOOL WINAPI DialogLockProc( HWND, UINT, WPARAM, LPARAM );

/******************************************************************************
*                                                                             *
*   void LockConsole()                                                        *
*                                                                             *
*******************************************************************************
*
*   This function pops up a dialog prompting for the password and then
#   locks the keyboard using the K_Lock.VxD that have to be installed.
#
#   It unlocks the keyboard only after the same password had been typed again.
*
******************************************************************************/
void LockConsole()
{
    HANDLE hDevice;
    int nRet;

    // Lock the console; first, use a help from VxD

    hDevice = CreateFile("\\\\.\\C:\\Windows\\System\\K_Lock.VxD", 0, 0,
            NULL, 0, FILE_FLAG_DELETE_ON_CLOSE, NULL );

    // If loading of dynamic VxD failed, see if a device is static

    if( hDevice==INVALID_HANDLE_VALUE )
    {
        hDevice = CreateFile("\\\\.\\K_Lock", 0, 0, NULL, 0, 0, NULL);

        // If it still fail, VxD cannot be used

        if( (int)hDevice <= 0 )
        {
            // Put a message if we can't get the VxD help

            MessageBox( NULL, "Cannot load VxD helper!", "Lock cannot be used", MB_OK );

            return;
        }
    }

    // Display the dialog asking for the password as a modal dialog

    nRet = DialogBoxParam( VWMInst, "Lock1", NULL, DialogLockProc, TRUE );

    if( nRet != 0 )
    {
        // The password is accepted. Copy it to the good password

        CopyMemory( sGoodPasswd, Passwd.sPasswd, LOCK_MAX_PASSWD+1 );

        // Set the global locked flag

        fLocked = TRUE;

        // Show a message box asking for a password

        nRet = DialogBoxParam( VWMInst, "Lock1", NULL, DialogLockProc, FALSE );

    }

    // Unlock the console; first, release a VxD

    CloseHandle( hDevice );

    // Reset the global locked flag

    fLocked = FALSE;
}


/******************************************************************************
*                                                                             *
*   DialogLockProc(HWND hPage1, UINT Msg, WPARAM wParam, LPARAM lParam)       *
*                                                                             *
*******************************************************************************
*
*   This is a dialog function for the lock dialog.
*
******************************************************************************/
BOOL WINAPI DialogLockProc(HWND hPage1, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fCanCancel;

    switch( Msg )
    {
        //---------------------------------------------------------------------
        case WM_INITDIALOG:
                // Dialog is being initialized

                SendDlgItemMessage( hPage1, ID_MYLOCK, EM_SETLIMITTEXT, LOCK_MAX_PASSWD, 0 );

                fCanCancel = (BOOL) lParam;

                // Confine the mouse cursor to the dialog area

                GetWindowRect( hPage1, &rcCursor );
                rcCursor.left   += Metrics.nBorderX;
                rcCursor.top    += Metrics.nCaption + Metrics.nBorderY;
                rcCursor.right  -= Metrics.nBorderX;
                rcCursor.bottom -= Metrics.nBorderY;
#if SECURE
                ClipCursor( &rcCursor );
#endif
                // Do everything to make it active on top :-)

                SetFocus( hPage1 );
                SetForegroundWindow( hPage1 );
                SetActiveWindow( hPage1 );

                return( TRUE );

            break;
        //---------------------------------------------------------------------
        case WM_CLOSE:
                // Release the mouse capture

                ClipCursor( NULL );

                // Destroy the modal dialog

                EndDialog( hPage1, 0 );

            break;
        //---------------------------------------------------------------------
        case WM_COMMAND:

            switch( LOWORD(wParam) )
            {
                case ID_MYACCEPT:
                        // Get the password

                        ZeroMemory( &Passwd.sPasswd[0], LOCK_MAX_PASSWD+1 );
                        Passwd.wLen = LOCK_MAX_PASSWD;
                        SendDlgItemMessage( hPage1, ID_MYLOCK, EM_GETLINE, 0, (LPARAM)(LPCSTR) Passwd.sPasswd );

                        // If we cannot cancel, we are in the real password state

                        if( fCanCancel==FALSE )
                        {
                            // Compare the entered password with the good one

                            if( lstrcmp( sGoodPasswd, Passwd.sPasswd )==0 )
                            {
                                // Now we can exit

                                SendMessage( hPage1, WM_CLOSE, 0, 0 );
                            }
                        }
                        else
                        {
                            // We can cancel, so exit with the accept code (1)

                            ClipCursor( NULL );

                            // Destroy the modal dialog

                            EndDialog( hPage1, 1 );
                        }

                    break;

                case ID_MYCANCEL:

                        if( fCanCancel )
                            SendMessage( hPage1, WM_CLOSE, 0, 0 );

                    break;
            }

            break;

        //---------------------------------------------------------------------
        default:
            return FALSE;
        //---------------------------------------------------------------------
    }

    return( FALSE );
}

