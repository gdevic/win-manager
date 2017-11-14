/******************************************************************************
*                                                                             *
*   Module:     Enum.c                                                        *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/07/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains functions for enumeration of client windows

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 06/07/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Include Files                                                             *
******************************************************************************/
#include <windows.h>                    // Include standard Windows header

#include "Globals.h"                    // Include project globals

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

static char sClassName[80];

#define RECALC_CLIENT(pInfoClient,pInfo)                                                 \
pInfoClient.left   = xToClient.eM11 * pInfo->pl.rcNormalPosition.left   + xToClient.eDx; \
pInfoClient.top    = xToClient.eM22 * pInfo->pl.rcNormalPosition.top    + xToClient.eDy; \
pInfoClient.right  = xToClient.eM11 * pInfo->pl.rcNormalPosition.right  + xToClient.eDx; \
pInfoClient.bottom = xToClient.eM22 * pInfo->pl.rcNormalPosition.bottom + xToClient.eDy

#define RECALC_CLIENT_NORMAL(pInfoClient,pInfo)                                                 \
pInfoClient.left   = xToClient.eM11 * pInfo->rcNormalPosition.left   + xToClient.eDx; \
pInfoClient.top    = xToClient.eM22 * pInfo->rcNormalPosition.top    + xToClient.eDy; \
pInfoClient.right  = xToClient.eM11 * pInfo->rcNormalPosition.right  + xToClient.eDx; \
pInfoClient.bottom = xToClient.eM22 * pInfo->rcNormalPosition.bottom + xToClient.eDy

#define RECALC_CLIENTMAX(pInfo)                                 \
pInfo->rcClientMax.left   = VCur.x * VStep.x;                   \
pInfo->rcClientMax.right  = pInfo->rcClientMax.left + VStep.x;  \
pInfo->rcClientMax.top    = VCur.y * VStep.y;                   \
pInfo->rcClientMax.bottom = pInfo->rcClientMax.top + VStep.y


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   BOOL CALLBACK WinEnumProc( HWND hWin, int * nIndexTop )                   *
*                                                                             *
*******************************************************************************
*
*   This callback function enumerates windows on a Windows desktop.
*
******************************************************************************/
BOOL CALLBACK WinEnumProc( HWND hWin, LPARAM nIndexTop )
{
    TInfo   NewInfo;
    TInfo * pInfo;
    int     nWinTop;
    int     i;


    // nWinTop is given via a pointer

    nWinTop = *(int *) nIndexTop;

    // Initialize temporary placement structure

    ZeroMemory( &NewInfo, sizeof(TInfo) );
    NewInfo.pl.length = sizeof(WINDOWPLACEMENT);

    // If we have reached maximum number of windows, stop enumeration

    if( nWinTop == MAX_WIN )
    {
        // Print a message box

        MessageBox( hVWM, "Too many client windows!", sVWMCaption,
                MB_ICONWARNING + MB_SETFOREGROUND + MB_OK );

        return( FALSE );
    }

    // Now we have to decide whether to accept a window as a valid one or not.
    // If a window is not visible, do not take it -

    if( IsWindowVisible( hWin ) != FALSE )
    {
        // Look up in the Info table if we already have this window handle stored

        pInfo = GetWinInfo( hWin );

        if( pInfo->hWin == hWin )
        {
            // The window already exists, get window placement

            GetWindowPlacement( hWin, &NewInfo.pl );

            // A window is already in the hash table, but since it may have moved or
            // changed show state, we need to recalculate its client location

            if( NewInfo.pl.rcNormalPosition.top != pInfo->pl.rcNormalPosition.top ||
                NewInfo.pl.rcNormalPosition.left != pInfo->pl.rcNormalPosition.left ||
                NewInfo.pl.rcNormalPosition.right != pInfo->pl.rcNormalPosition.right ||
                NewInfo.pl.rcNormalPosition.bottom != pInfo->pl.rcNormalPosition.bottom ||
                NewInfo.pl.showCmd != pInfo->showWin )
            {
                CopyMemory( &pInfo->pl, &NewInfo.pl, sizeof(WINDOWPLACEMENT) );

                // If the newly detected window just became maximized, store the current
                // virtual desktop as its main window

                if( (pInfo->showWin != NewInfo.pl.showCmd) &&
                    (NewInfo.pl.showCmd == SW_SHOWMAXIMIZED) )
                {
                    pInfo->MaxDesk.x = VCur.x;
                    pInfo->MaxDesk.y = VCur.y;

                    RECALC_CLIENTMAX(pInfo);
                }

                pInfo->showWin = NewInfo.pl.showCmd;

                if( (!(pInfo->dwFlags & INFO_MOVED)) &&
                    (pInfo->showWin != SW_SHOWMINIMIZED) )
                {
                    RECALC_CLIENT(pInfo->rcClient,pInfo);
                }

                // If a window is normal, store its positions values

                if( pInfo->showWin == SW_SHOWNORMAL )
                {
                    CopyMemory( &pInfo->rcNormalPosition, &NewInfo.pl.rcNormalPosition, sizeof(RECT) );
                }

                pInfo->dwFlags &= ~INFO_MOVED;

                // Set the repaint flag

                fNeedPaint = TRUE;
            }

            // Store the window handle and advance the index

            Win[nWinTop] = hWin;
            *(int *) nIndexTop = nWinTop + 1;
        }
        else

        // The current window is a new window to the VWM database:
        // Get the window placement

        if( GetWindowPlacement( hWin, &NewInfo.pl ) != FALSE )
        {
            // If the window does not have a caption, do not take it

            GetWindowText( hWin, NewInfo.sName, MAX_TEXTLEN );

            if( NewInfo.sName[0] != '\0' )
            {
                // Search through the list of windows that will not be managed
                // and look for the caption that matches
#if TEST
                // Do not manage windows that have "Watcom" in the caption

                if( strstr(&NewInfo.sName, "WATCOM") != NULL )
                    return( TRUE );
#endif

                // Get the name of the class to which the current window belongs

                GetClassName( hWin, &sClassName, 80 );

                // Ignore certain classes

                i = 0;
                while( sIgnoreClasses[i] != NULL )
                {
                    if( !strcmpi( &sClassName, sIgnoreClasses[i]) )
                        return( TRUE );
                    i++;
                }

                // Get a handle of the application instance

                NewInfo.hInstance = (HINSTANCE) GetWindowLong( hWin, GWL_HINSTANCE );

                // Map its coordinates to the virtual desktop

                RECALC_CLIENT((&NewInfo)->rcClient,(&NewInfo));

                // Store the window handle

                NewInfo.hWin = hWin;

                // Copy the window icons

                NewInfo.hIcon      = (HICON) GetClassLong( hWin, GCL_HICON );
                NewInfo.hIconSmall = (HICON) GetClassLong( hWin, GCL_HICONSM );

                // An alternate way to get the icon

                if( NewInfo.hIcon==NULL && NewInfo.hIconSmall==NULL )
                {
                    NewInfo.hIcon      = (HICON) SendMessage( hWin, WM_GETICON, (WPARAM) FALSE, 0 );
                    NewInfo.hIconSmall = (HICON) SendMessage( hWin, WM_GETICON, (WPARAM) TRUE, 0 );
                }

                // Set the possibly missing icon

                if( NewInfo.hIcon == NULL && NewInfo.hIconSmall != NULL )
                    NewInfo.hIcon = NewInfo.hIconSmall;
                else
                if( NewInfo.hIconSmall == NULL && NewInfo.hIcon != NULL )
                    NewInfo.hIconSmall = NewInfo.hIcon;

                // Store the window original show state

                NewInfo.showWin = NewInfo.pl.showCmd;

                if( NewInfo.pl.showCmd==SW_SHOWMAXIMIZED )
                {
                    // Maximized window

                    NewInfo.dwFlags |= INFO_MAXIMIZED;
                }
                else
                if( NewInfo.pl.showCmd==SW_SHOWMINIMIZED )
                {
                    // Window is minimized

                    NewInfo.dwFlags |= INFO_MINIMIZED;
                }

                // Store index in the hash array (for debugging)

                nHash[nWinTop] = pInfo - Info;

                // Copy the new window Info structure to the table

                CopyMemory( pInfo, &NewInfo, sizeof(TInfo) );

                // Set the current desktop as the one the window will appear on

                if( NewInfo.showWin==SW_SHOWMAXIMIZED )
                {
                    pInfo->MaxDesk.x = VCur.x;
                    pInfo->MaxDesk.y = VCur.y;

                    RECALC_CLIENTMAX(pInfo);
                }

                Win[nWinTop] = hWin;
                *(int *) nIndexTop = nWinTop + 1;

                // Set the repaint flag

                fNeedPaint = TRUE;
            }
        }
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void PurgeInfoTable()                                                     *
*                                                                             *
*******************************************************************************
*
*   This function cleans up the Info array.  All the windows handles that
#   exist there are are not in the Win array and marked as free.
*
******************************************************************************/
void PurgeInfoTable()
{
    int i, j;


    // Loop for all entries of Info array and try to find the matching
    // handle in the Win array

    for( i=0; i<MAX_HASH_TABLE; i++)
    {
        if( Info[i].hWin != 0 )
        {
            for( j=0; j<nWinTop; j++)
            {
                if( Win[nWinTop] == Info[i].hWin )
                    break;
            }

            // If the entry could not be matched, destroy the Info slot

            if( j == nWinTop )
            {
                ZeroMemory( &Info[i], sizeof(TInfo) );
            }
        }
    }
}


/******************************************************************************
*                                                                             *
*   TInfo *GetWinInfo( HWND hWnd )                                            *
*                                                                             *
*******************************************************************************
*
*   Returns a pointer to the hash table that is addressed by the hWnd
#   parameter via default hash function.
*
*   Where:
*       hWnd - window handle to be looked up
*
*   Returns:
#       If the hWnd has a record in Info hash table, returns its address
#       If the hWnd does not exist in Info hash table, returns the address
#           of the free Info structure that could be used to store it.  In that
#           case hWin field of a Info structure is 0.
*
******************************************************************************/
TInfo *GetWinInfo( HWND hWnd )
{
    int nIndex;
    int nWraps = 0;


    // Since the windows handle is always divisible by 4, scale it down

    nIndex = (int) hWnd >> 2;

    // Get the index into a hash array

    nIndex = nIndex % MAX_HASH_TABLE;

    // Try to find the window handle linearly starting from the nIndex.

    do
    {
        // Return if an empty Info slot was encountered or if hWnd
        // window handle was found

        if( (Info[nIndex].hWin == 0) || (Info[nIndex].hWin == hWnd) )
            return( &Info[nIndex] );

        // Increment the hash table index and wrap around

        if( ++nIndex == MAX_HASH_TABLE )
            nIndex = 0, nWraps++;

    }while( nWraps < 2 );

    // If the hash table appears to be full, clean it up and call itself
    // Notify about the hash table being full

    ERROR_MSG("GetWinInfo: Hash table full - Purging...");

    PurgeInfoTable();

    return( GetWinInfo( hWnd ) );
}


/******************************************************************************
*                                                                             *
*   static void CalculateMetrics()                                            *
*                                                                             *
*******************************************************************************
*
*   This function calculates the basic metrics dependency
*
******************************************************************************/
static void CalculateMetrics()
{
    // Dependencies:
    //  In:
    //       Desktop            - size of the desktop (screen) area in pix
    //       VClientSize        - size of the VWM client area in pix
    //       VSize              - virtual size that is managed in screens
    //       VCur               - coordinates in screens of a current desktop
    //
    //  Recalculated Out:
    //      VStep               - size of one desktop in a client area in pix
    //      VOrigin             - coodrinates in pix of a current desktop within a client
    //      Vrc                 - inclusive rectangle of a current desktop
    //      xToClient           - transformation from screen to client
    //      xToWin              - transformation from client to screen
    //

    VStep.x = VClientSize.x / VSize.x;
    VStep.y = VClientSize.y / VSize.y;

    VOrigin.x = VCur.x * VStep.x;
    VOrigin.y = VCur.y * VStep.y;

    Vrc.left   = VOrigin.x;
    Vrc.top    = VOrigin.y;
    Vrc.right  = VOrigin.x + VStep.x;
    Vrc.bottom = VOrigin.y + VStep.y;

    // Rectangle may need a fixup due to the integer arithmetic

    if( VCur.x == VSize.x-1 )
        Vrc.right = VClientSize.x;

    if( VCur.y == VSize.y-1 )
        Vrc.bottom = VClientSize.y;

    // Set the transform that will map world into the client current window

    xToClient.eM11 = (FLOAT) VStep.x / Desktop.x;           // Horizontal scale
    xToClient.eM22 = (FLOAT) VStep.y / Desktop.y;           // Vertical scale
    xToClient.eDx  = (FLOAT) VOrigin.x;                     // Horiz. translation
    xToClient.eDy  = (FLOAT) VOrigin.y;                     // Vert. translation

    xToWin.eDx     = (FLOAT) -VOrigin.x;                    // Horiz. translation
    xToWin.eDy     = (FLOAT) -VOrigin.y;                    // Vert. translation
    xToWin.eM11    = (FLOAT) Desktop.x / VStep.x;           // Horizontal scale
    xToWin.eM22    = (FLOAT) Desktop.y / VStep.y;           // Vertical scale
}


/******************************************************************************
*                                                                             *
*   void RecalculateClientPos()                                               #
*                                                                             *
*******************************************************************************
*
*   This function recalculates all the client positions after the client size
#   changed.
#
#   It also recalculates all the necessary transformations.
*
******************************************************************************/
void RecalculateClientPos()
{
    TInfo * pInfo;
    int i, delta;


    // Recalculate the basic metrics

    CalculateMetrics();

    // Loop for all entries in Win array, find the Info and recalculate
    // client position

    for( i=0; i<nWinTop; i++)
    {
        pInfo = GetWinInfo( Win[i] );

        // The entry should always be matched !

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("RecalculateClientPos: Warning");
            continue;
        }

        if( pInfo->showWin != SW_SHOWMINIMIZED )
        {
            RECALC_CLIENT(pInfo->rcClient,pInfo);

            if( pInfo->showWin == SW_SHOWMAXIMIZED )
            {
                RECALC_CLIENTMAX(pInfo);
            }
        }
        else
        {
            RECALC_CLIENT_NORMAL(pInfo->rcClient,pInfo);
        }

        // If a window ended up to the right or bottom of the virtual desktop,
        // adjust its position.  That may happen when the number of desks is
        // decreased and a window could be `lost'.

        if( pInfo->rcClient.right >= VClientSize.x )
        {
            delta = (pInfo->rcClient.left - VClientSize.x) / VStep.x;
            delta = VStep.x * (delta + 1);
            SetDeltaWindowPos( pInfo, -delta, 0 );
        }

        if( pInfo->rcClient.bottom >= VClientSize.y )
        {
            delta = (pInfo->rcClient.top - VClientSize.y) / VStep.y;
            delta = VStep.y * (delta + 1);
            SetDeltaWindowPos( pInfo, 0, -delta );
        }
    }
}


/******************************************************************************
*                                                                             *
*   void RecalculateNewVW( POINT * pt, HWND hInvalidate )                     *
*                                                                             *
*******************************************************************************
*
*   This function recalculates all the client positions after the client
#   changed the virtual window.  Then it sets the current desk to the one
#   of pt coordinates.
#
#   It also recalculates all the necessary transformations.
#
#   Lastly, it invalidates the window whose handle is given.
#
#   Assumption:
#       Here we copy a portion of placement info into the parking pl structure.
#       Assumption is made that the RECT rcNormalPosition is defined at the
#       end of the structure.
*
*   Where:
*       pt is the new virtual desk coordinate
#       hInvalidate is the handle of a window to send the invalidate message
*
******************************************************************************/
void RecalculateNewVW( POINT * pt, HWND hInvalidate )
{
    POINT Delta;
    HDWP  hDwp;
    TInfo * pInfo;
    int i;


    // Calculate the difference in screen coordinates to be applied to
    // all guest window positions

    Delta.x = -(pt->x - VCur.x) * Desktop.x;
    Delta.y = -(pt->y - VCur.y) * Desktop.y;

    // Set the new current virtual window

    VCur.x = pt->x;
    VCur.y = pt->y;

    // Recalculate the basic metrics

    CalculateMetrics();

    // Get the window position memory

    hDwp = BeginDeferWindowPos( nWinTop );

    if( hDwp==NULL )
    {
        ERROR_MSG("RecalculateNewVW: Cannot defer window pos!");
        return;
    }

    // Loop for all entries in Win array, find the Info, recalculate new
    // guest position on the screen

    for( i=0; i<nWinTop; i++)
    {
        pInfo = GetWinInfo( Win[i] );

        // The entry should always be matched !

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("RecalculateNewVW: Warning");
            continue;
        }

        // Add the change in screen coordinates

        pInfo->pl.rcNormalPosition.left   += Delta.x;
        pInfo->pl.rcNormalPosition.top    += Delta.y;
        pInfo->pl.rcNormalPosition.right  += Delta.x;
        pInfo->pl.rcNormalPosition.bottom += Delta.y;

        if( pInfo->showWin != SW_SHOWMINIMIZED )
        {
            if( pInfo->showWin == SW_SHOWMAXIMIZED )
            {
                // Window should be maximized - either on or off
                // depending on the coordinates of a current desk

                if( VCur.x==pInfo->MaxDesk.x && VCur.y==pInfo->MaxDesk.y )
                {
                    // Maximized window is visible

                    hDwp = DeferWindowPos( hDwp, pInfo->hWin, HWND_TOP,
                        -Metrics.nBorderX,
                        -Metrics.nBorderY,
                        Metrics.nFullX,
                        Metrics.nFullY,
                        SWP_SHOWWINDOW );
                }
                else
                {
                    // Maximized window is not visible, but move it
                    // so that the edges are hidden

                    hDwp = DeferWindowPos( hDwp, pInfo->hWin, HWND_TOP,
                        Desktop.x,
                        Desktop.y,
                        Metrics.nFullX,
                        Metrics.nFullY,
                        SWP_SHOWWINDOW );
                }
            }
            else
            {
                // Window is normal size

                hDwp = DeferWindowPos( hDwp, pInfo->hWin, HWND_TOP,
                    pInfo->pl.rcNormalPosition.left,
                    pInfo->pl.rcNormalPosition.top,
                    pInfo->pl.rcNormalPosition.right - pInfo->pl.rcNormalPosition.left,
                    pInfo->pl.rcNormalPosition.bottom - pInfo->pl.rcNormalPosition.top,
                    SWP_SHOWWINDOW | SWP_NOSENDCHANGING );
            }
        }
    }

    EndDeferWindowPos( hDwp );

    // Invalidate the given window

    InvalidateRect( hInvalidate, NULL, TRUE );
}


/******************************************************************************
*                                                                             *
*   SetDeltaWindowPos( TInfo * pInfo, int dx, int dy )                        *
*                                                                             *
*******************************************************************************
*
*   This function is used to move a window by the specified delta coordinates
#   expressed in VWM client space
*
*   Where:
*       pInfo is a window to be moved
#       dx, dy is the offset in pixels in a VWM client space
*
*   Returns:
*
*
******************************************************************************/
void SetDeltaWindowPos( TInfo * pInfo, int dx, int dy )
{
    // Offset the window client coordinates

    pInfo->rcClient.left   += dx;
    pInfo->rcClient.top    += dy;
    pInfo->rcClient.right  += dx;
    pInfo->rcClient.bottom += dy;

    // Calculate new window coordinates on the screen based on the
    // client coordinates

    pInfo->pl.rcNormalPosition.left   = xToWin.eM11 * (pInfo->rcClient.left   + xToWin.eDx);
    pInfo->pl.rcNormalPosition.top    = xToWin.eM22 * (pInfo->rcClient.top    + xToWin.eDy);
    pInfo->pl.rcNormalPosition.right  = xToWin.eM11 * (pInfo->rcClient.right  + xToWin.eDx);
    pInfo->pl.rcNormalPosition.bottom = xToWin.eM22 * (pInfo->rcClient.bottom + xToWin.eDy);

    // If the window being moved is minimized, move its normal position values

    if( pInfo->showWin == SW_SHOWMINIMIZED )
    {
        pInfo->rcNormalPosition.left   = xToWin.eM11 * (pInfo->rcClient.left   + xToWin.eDx);
        pInfo->rcNormalPosition.top    = xToWin.eM22 * (pInfo->rcClient.top    + xToWin.eDy);
        pInfo->rcNormalPosition.right  = xToWin.eM11 * (pInfo->rcClient.right  + xToWin.eDx);
        pInfo->rcNormalPosition.bottom = xToWin.eM22 * (pInfo->rcClient.bottom + xToWin.eDy);
    }

    pInfo->dwFlags |= INFO_MOVED;

    // Send a message to a window to move

    SetWindowPos( pInfo->hWin, HWND_TOP,
            pInfo->pl.rcNormalPosition.left,
            pInfo->pl.rcNormalPosition.top,
            0, 0,
            SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
}

