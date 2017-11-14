/******************************************************************************
*                                                                             *
*   Module:     Services.c                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/07/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the basic services

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


/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/


/******************************************************************************
*                                                                             *
*   void DrawWindow( HDC hVWMdc, TInfo * pInfo, RECT * rc, DWORD dwEdge )     *
*                                                                             *
*******************************************************************************
*
*   Draws a rectangle on the hVWMdc display context, that is the VWM client dc.
*
*   Where:
*       hVWMdc is the display context for the client rectangle to paint on
#       pInfo points to the info structure of a window to be drawn
#       rc contains the window rectangle on the client
#       dwEdge is the edge flag for the window border
*
*   Returns:
*
*
******************************************************************************/
void DrawWindow( HDC hVWMdc, TInfo * pInfo, LPRECT rc, DWORD dwEdge )
{
    HICON hIcon;
    int nHalf;
    int nWidth, nHeight;

    // Calculate the size of the window to draw

    nWidth = rc->right - rc->left;
    nHeight = rc->bottom - rc->top;

    // Draw a solid window background

    Rectangle( hVWMdc,  rc->left, rc->top, rc->right, rc->bottom );

    // Draw a 3D looking border around the window

    DrawEdge( hVWMdc, rc, dwEdge, BF_RECT );

    // Draw an icon that represents a window on a client

    if( fIcons != IDC_ICONS_DONTDRAW )
    {
        if( nWidth>31 && nHeight>31 )
        {
            hIcon = pInfo->hIcon;
            nHalf = 16;
        }
        else
        {
            hIcon = pInfo->hIconSmall;
            nHalf = 8;
        }

        if( fIcons == IDC_ICONS_STRETCH )
        {
            // Stretch the icon over the window area

            DrawIconEx( hVWMdc, rc->left + 2,
                            rc->top + 2,
                            hIcon,
                            nWidth - 2,
                            nHeight - 2,
                            0,
                            NULL,
                            DI_NORMAL );
        }
        else
        {
            // Use a proportional, but fixed size icon

            DrawIconEx( hVWMdc, rc->left + nWidth/2 - nHalf,
                            rc->top + nHeight/2 - nHalf,
                            hIcon,
                            nHalf * 2,
                            nHalf * 2,
                            0, NULL, DI_NORMAL );
        }
    }
}


/******************************************************************************
*                                                                             *
*   HDC UseDC( int op )                                                       *
*                                                                             *
*******************************************************************************
*
*   This function performs multiple functions on the secondary buffer display
#   context.
*
*   Where:
*       op is the operation to be performed
*
*   Returns:
*       memory display context.  This context may be used for drawing.
*
******************************************************************************/
HDC UseDC( int op )
{
    // These must be static since they keep current values

    static HDC hDC, hMemDC = (HDC) NULL;
    static HBITMAP hBitmap;

    // Switch on the function code...

    switch( op )
    {
        case GETDC_GETDC:
                // Return the display context to draw into

                return( hMemDC );

            break;

        case GETDC_BLT:
                // Blt the secondary buffer to the client window

                BitBlt( hDC, 0, 0, VClientSize.x, VClientSize.y, hMemDC, 0, 0, SRCCOPY );

                return( hMemDC );

            break;

        case GETDC_CREATE:
                // Create the secondary buffer context

                hDC = GetDC( hVWM );
                hMemDC = CreateCompatibleDC( hDC );
                hBitmap = CreateCompatibleBitmap( hDC, VClientSize.x, VClientSize.y );
                SelectObject( hMemDC, hBitmap );

                return( hMemDC );

            break;

        case GETDC_DESTROY:
                // Destroy the secondary buffer context

                ReleaseDC( hVWM, hDC );
                DeleteDC( hMemDC );
                DeleteObject( hBitmap );

                return( (HDC) NULL );

            break;

        case GETDC_RESIZE:
                // Resize the secondary buffer context

                UseDC( GETDC_DESTROY );

                return( UseDC( GETDC_CREATE) );

            break;
    }

    return( (HDC) NULL );
}


/******************************************************************************
*                                                                             *
*   void UseBitmap( HDC hDC, int op )                                         *
*                                                                             *
*******************************************************************************
*
*   This function performs multiple operation on a background bitmap.
*
*   Where:
*       hDC is the destination context for bitmap blt
*       op is one of the operations
#
#   Returns:
#       For operation of
#       USBITMAP_LOAD, True if bitmap loaded, false if error
*
******************************************************************************/
BOOL UseBitmap( HDC hDC, int op )
{
    static HDC hMemDC = NULL;
    static HBITMAP hBitmap;
    static BITMAP bm;
    int x, y, x_start, y_start;


    switch( op )
    {
        case USEBITMAP_BLT:
                // Tile or stretch bitmap on the DC device (client area)

                if( hMemDC==NULL )
                    break;

                switch( nBitmapTile )
                {
                    case IDC_BACK_VIRT:
                        // Each virtual desk has a stretched bitmap

                        for( y_start=0; y_start<VClientSize.y; y_start += VStep.y )
                        {
                            for( x=0, x_start=0; x<VSize.x; x++, x_start+=VStep.x )
                            {
                                StretchBlt( hDC, x_start, y_start, VStep.x, VStep.y,
                                        hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );
                            }
                        }

                        break;

                    case IDC_BACK_TILE:
                        // Normal tiling

                        for( y=0; y<VClientSize.y; y+=bm.bmHeight )
                        {
                            for( x=0; x<VClientSize.x; x+=bm.bmWidth )
                            {
                                BitBlt( hDC, x, y, bm.bmWidth, bm.bmHeight,
                                        hMemDC, 0, 0, SRCCOPY );
                            }
                        }

                        break;

                    case IDC_BACK_STRETCH:
                        // Stretch a bitmap over the whole client area

                        StretchBlt( hDC, 0, 0, VClientSize.x, VClientSize.y,
                                hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

                        break;
                }
            break;

        case USEBITMAP_LOAD:
                // Load a bitmap from the Back array

                // Destroy a bitmap that was possibly active

                UseBitmap( 0, USEBITMAP_DESTROY );

                hBitmap = LoadImage( NULL,
                    Back[fBitmap-1].sFile,
                    IMAGE_BITMAP,
                    0, 0,
                    LR_DEFAULTCOLOR | LR_LOADFROMFILE );

                // Loading a bitmap may still fail

                if( hBitmap==NULL )
                    return( FALSE );

                if( (hMemDC = CreateCompatibleDC( hDC )) == NULL )
                    return( FALSE );

                SelectObject( hMemDC, hBitmap );
                GetObject( hBitmap, sizeof(BITMAP), &bm );

            break;

        case USEBITMAP_DESTROY:
                // Destroy a bitmap if it was created

                if( hMemDC != NULL )
                {
                    DeleteDC( hMemDC );
                    DeleteObject( hBitmap );

                    hMemDC = NULL;
                }

            break;
    }

    return( TRUE );
}


/******************************************************************************
*                                                                             *
*   void VWMPaint( void )                                                     *
*                                                                             *
*******************************************************************************
*
*   This function performs painting of the VWM client area
*
*
******************************************************************************/
void VWMPaint( void )
{
    HDC hVWMdc;
    HPEN hBackPen;
    HBRUSH hSolidBrush, hBackBrush;
    LOGBRUSH lgBrush;
    PAINTSTRUCT ps;
    TInfo * pInfo;
    int i;


    // Get the DC for the VWM client area

    hVWMdc = BeginPaint( hVWM, &ps );

    // If double buffer is used, get the secondary buffer context

    if( fDoubleBuffer )
        hVWMdc = UseDC( GETDC_GETDC );

    // If no display structure is available, return

    if( hVWMdc == NULL )
        return;

    // Get the color brush and pens for the client object

    lgBrush.lbStyle = BS_SOLID;
    lgBrush.lbColor = GetSysColor( COLOR_3DFACE );
    hSolidBrush     = CreateBrushIndirect( &lgBrush );

    // Choose the background brush and pen

    if( fLightBack )
        hBackBrush = GetSysColorBrush( COLOR_3DFACE ),
        hBackPen   = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_3DSHADOW ) );
    else
        hBackBrush = GetSysColorBrush( COLOR_3DSHADOW ),
        hBackPen   = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_3DDKSHADOW ) );

    // If the background needs erasing, erase it

    if( ps.fErase )
    {
        // If the bitmap is selected, copy it or tile

        if( fBitmap )
        {
            UseBitmap( hVWMdc, USEBITMAP_BLT );
        }
        else
        {
            // No bitmap in the background, just plain color

            SelectObject( hVWMdc, hBackBrush );
            Rectangle( hVWMdc,  0, 0, VClientSize.x, VClientSize.y );
        }
    }

    // Paint the window that is currently active using the solid brush

    SelectObject( hVWMdc, hSolidBrush );
    Rectangle( hVWMdc,  Vrc.left, Vrc.top, Vrc.right, Vrc.bottom );
    DrawEdge( hVWMdc, &Vrc, EDGE_SUNKEN, BF_RECT );


    // Loop for all registered windows and draw a rectangle on the client
    // area depending where they are on a desktop.

    for( i=nWinTop-1; i>=0; i-- )
    {
        // Find the Info structure that contains the windows info

        pInfo = GetWinInfo( Win[i] );

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("VWMPaint: Warning");
            continue;
        }

        if( pInfo->showWin == SW_SHOWMINIMIZED )
        {
            // Window is minimized

            DrawWindow( hVWMdc, pInfo, &pInfo->rcClient, EDGE_SUNKEN );
        }
        else
        if( pInfo->showWin == SW_SHOWMAXIMIZED )
        {
            // Window is maximized

            DrawWindow( hVWMdc, pInfo, &pInfo->rcClientMax, EDGE_SUNKEN );
        }
        else
        {
            // Normal size window

            DrawWindow( hVWMdc, pInfo, &pInfo->rcClient, EDGE_RAISED );
        }
    }

    // Draw vertical lines that show the virtual screens bounds

    SelectObject( hVWMdc, hBackPen );

    for( i=1; i<VSize.x; i++ )
    {
        MoveToEx( hVWMdc, i*VStep.x, 0, NULL );
        LineTo(   hVWMdc, i*VStep.x, VClientSize.y );
    }

    // Draw horizontal lines

    for( i=1; i<VSize.y; i++ )
    {
        MoveToEx( hVWMdc, 0,             i*VStep.y, NULL );
        LineTo(   hVWMdc, VClientSize.x, i*VStep.y );
    }

    // If we use double buffer, blt the secondary buffer to the client

    if( fDoubleBuffer )
        UseDC( GETDC_BLT );

    // Release drawing objects

    DeleteObject( hBackPen );
    DeleteObject( hSolidBrush );

    // Release display context handle

    EndPaint( hVWM, &ps );
}


/******************************************************************************
*                                                                             *
*   TInfo * FindWindowPt( POINT pt, BOOL fMax )                               *
*                                                                             *
*******************************************************************************
*
*   This function finds a window whose location is (x,y) on the client.
*
*   Where:
*       x, y  coordinates on the client window
#       fMax if True, may return maximized windows also
*
*   Returns:
*       Address of a Info structure of the top underlying window
#       NULL if there was no window on these client coordinates
*
******************************************************************************/
TInfo * FindWindowPt( POINT pt, BOOL fMax )
{
    TInfo * pInfo;
    int i;


    for( i=nWinTop-1; i>=0; i-- )
    {
        // Find the Info structure that contains the windows info

        pInfo = GetWinInfo( Win[i] );

        // This should not be necessary, but bugs do happen...

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("FindWindow: Warning");
            return NULL;
        }

        // If the window is maximized, use rcClientMax coordinates instead

        if( pInfo->showWin == SW_SHOWMAXIMIZED )
        {
            // Only if maximized windows may be returned...

            if( fMax )
            {
                if( PtInRect( &pInfo->rcClientMax, pt ) )
                    return( pInfo );
            }
        }
        else
        {
            // If the point is in the window rectangle, return its address

            if( PtInRect( &pInfo->rcClient, pt ) )
                return( pInfo );
        }
    }

    // Failed to find any window at this coordinates

    return( NULL );
}


/******************************************************************************
*                                                                             *
*   BOOL RestoreWindow( POINT pt, TInfo * pInfo )                             *
*                                                                             *
*******************************************************************************
*
*   This function finds and restores a window whose location is (x,y) on
#   the client if pInfo is NULL.  Otherwise, it uses pInfo window.
#   The window is set to foreground.
*
*   Where:
*       x, y  coordinates on the client window
*
*   Returns:
*       TRUE if a window to be restored is found, FALSE otherwise.
*
******************************************************************************/
BOOL RestoreWindow( POINT pt, TInfo * pInfo )
{
    // If the window was not given, find it from the given coordinates

    if( pInfo == NULL )
        pInfo = FindWindowPt( pt, TRUE );

    // If the window was found, activate it

    if( pInfo != NULL )
    {
        // If a window was minimized, set its normal position since it may be moved

        if( pInfo->pl.showCmd == SW_SHOWMINIMIZED )
        {
            SetDeltaWindowPos( pInfo, 0, 0 );

            // Show the window

            pInfo->pl.showCmd = SW_SHOWNORMAL;

            SetWindowPlacement( pInfo->hWin, &pInfo->pl );
        }

        // Show the window

        SetForegroundWindow( pInfo->hWin );

        return( TRUE );
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   LPSTR GetWindowCaption( POINT pt )                                        *
*                                                                             *
*******************************************************************************
*
*   This function returns a pointer to a window caption whose location is (x,y)
#   on the client.
*
*   Where:
*       x, y  coordinates on the client window
*
*   Returns:
*       Pointer to a string
*
******************************************************************************/
LPSTR GetWindowCaption( POINT pt )
{
    TInfo * pInfo;

    pInfo = FindWindowPt( pt, TRUE );

    // If the window was found, return its caption

    if( pInfo != NULL )
        return( pInfo->sName );

    return VWM_NO_TIP;
}


/******************************************************************************
*                                                                             *
*   void RestoreAll()                                                         *
*                                                                             *
*******************************************************************************
*
*   This function is called at the VWM termination to restore the position
#   of all windows that it still manages to their real position on the first
#   desktop.
*
******************************************************************************/
void RestoreAll()
{
    TInfo * pInfo;
    int nSizeX, nSizeY;
    int nWidth, nHeight;
    int i;


    // nSize? is added to each window coordinate. It is the size in pixels of
    // the largest virtual span.  That effectively insures that the window
    // coordinate will end up positive.

    nSizeX = MAX_VCUR_X * Desktop.x;
    nSizeY = MAX_VCUR_Y * Desktop.y;

    for( i=0; i<nWinTop; i++ )
    {
        // Find the Info structure that contains a window info

        pInfo = GetWinInfo( Win[i] );

        // This should not be necessary, but bugs do happen...

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("RestoreAll: Warning");
            continue;
        }

        // If a window is maximized, reset its max position first

        if( pInfo->showWin == SW_SHOWMAXIMIZED )
        {
            pInfo->pl.ptMaxPosition.x = -Metrics.nBorderX;
            pInfo->pl.ptMaxPosition.y = -Metrics.nBorderY;

            pInfo->pl.showCmd = SW_SHOWMAXIMIZED | SW_HIDE;

            SetWindowPlacement( pInfo->hWin, &pInfo->pl );
        }

        // Set a window real coordinate on the screen. Also, ensure that
        // the location is positive because of the mod function

        nWidth  = pInfo->pl.rcNormalPosition.right - pInfo->pl.rcNormalPosition.left;
        nHeight = pInfo->pl.rcNormalPosition.bottom - pInfo->pl.rcNormalPosition.top;

        pInfo->pl.rcNormalPosition.left   = (pInfo->pl.rcNormalPosition.left   + nSizeX) % Desktop.x;
        pInfo->pl.rcNormalPosition.top    = (pInfo->pl.rcNormalPosition.top    + nSizeY) % Desktop.y;

        // Make sure that the whole window fits on the screen

        if( nWidth > Desktop.x )
            nWidth = Desktop.x;

        if( nHeight > Desktop.y )
            nHeight = Desktop.y;

        if( pInfo->pl.rcNormalPosition.left + nWidth >= Desktop.x )
            pInfo->pl.rcNormalPosition.left -= nWidth;
        if( pInfo->pl.rcNormalPosition.left < 0 )
            pInfo->pl.rcNormalPosition.left = 0;

        if( pInfo->pl.rcNormalPosition.top + nHeight >= Desktop.y )
            pInfo->pl.rcNormalPosition.top -= nHeight;
        if( pInfo->pl.rcNormalPosition.top < 0 )
            pInfo->pl.rcNormalPosition.top = 0;

        pInfo->pl.rcNormalPosition.right  = pInfo->pl.rcNormalPosition.left + nWidth;
        pInfo->pl.rcNormalPosition.bottom = pInfo->pl.rcNormalPosition.top + nHeight;


        // Set the guest window inside the visible region.

        pInfo->pl.showCmd = SW_RESTORE | SW_SHOW;

        SetWindowPlacement( pInfo->hWin, &pInfo->pl );
    }
}


/******************************************************************************
*                                                                             *
*   BOOL TrayMessage(HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip)
*                                                                             *
*******************************************************************************
*
*   Sends a message to the tray bar setting up the icon and tip.
*
*   Where:
*       hDlg is the handle of the dialog that is to be activated
#       dwMessage is the message for tray icon
#       uID
#       hIcon is the icon handle.  Icon is destroyed after this function.
#       pszTip is the tip string
*
*   Returns:
*       True if function succeeded
*
******************************************************************************/
BOOL TrayMessage(HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip)
{
    BOOL res;
    NOTIFYICONDATA tnd;


    tnd.cbSize      = sizeof(NOTIFYICONDATA);
    tnd.hWnd        = hDlg;
    tnd.uID         = uID;

    tnd.uFlags      = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnd.uCallbackMessage  = MYWM_NOTIFYICON;
    tnd.hIcon       = hIcon;

    // If tip is defined, copy it

    if( pszTip )
    {
        lstrcpyn(tnd.szTip, pszTip, sizeof(tnd.szTip));
    }
    else
    {
        tnd.szTip[0] = '\0';
    }

    res = Shell_NotifyIcon(dwMessage, &tnd);

    if (hIcon)
        DestroyIcon(hIcon);

    return res;
}


/******************************************************************************
*                                                                             *
*   void SetVWMStyle()                                                        *
*                                                                             *
*******************************************************************************
*
*   Sets the style of the VWM window depending on the fBorder style flag.
*
******************************************************************************/
void SetVWMStyle()
{
    LONG Style;

    if( fBorder==IDC_BORDER_THIN )
        Style = WS_EX_STATICEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
    else
    if( fBorder==IDC_BORDER_THICK )
        Style = WS_EX_STATICEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_DLGMODALFRAME;
    else
        Style = WS_EX_STATICEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_CLIENTEDGE;

    SetWindowLong( hVWM, GWL_EXSTYLE, Style );
    InvalidateRect( NULL, NULL, TRUE );
}



/******************************************************************************
*                                                                             *
*   void FormatAppsMenu( HMENU hMenu )                                        *
*                                                                             *
*******************************************************************************
*
*   This function is called before the popup menu gets activated.  It sets
#   up the list of application as a submenu of the given menu.
*
*   Where:
*       hMenu is the base menu on which to build the list of apps as a
#       submenu with the position MENU_APPS
*
******************************************************************************/
void FormatAppsMenu( HMENU hMenu )
{
    HMENU hAppMenu;
    BOOL  fAtLeastOne = FALSE;
    int   i, count;
    TInfo * pInfo;


    // Get the position (handle) of the apps submenu

    hAppMenu = GetSubMenu( hMenu, MENU_APPS );

    // We need to clean it up so we can build the fresh app list

    count = GetMenuItemCount( hAppMenu );

    while( count-- )
        DeleteMenu( hAppMenu, 0, MF_BYPOSITION );


    // Add the menu content

    for( i=0; i<nWinTop; i++ )
    {
        // Find the Info structure that contains a window info

        pInfo = GetWinInfo( Win[i] );

        // This should not be necessary, but bugs do happen...

        if( pInfo->hWin != Win[i] )
        {
            ERROR_MSG("FormatAppsMenu: Warning");
            continue;
        }

        // Append the appliction name to the menu

        AppendMenu( hAppMenu, MF_STRING, i, &pInfo->sName );

        fAtLeastOne = TRUE;
    }

    // Enable or disable the submenu

    EnableMenuItem( hMenu, MENU_APPS, fAtLeastOne? MF_BYPOSITION | MF_ENABLED : MF_BYPOSITION | MF_GRAYED );

    // Refresh the menu information

    DrawMenuBar( hVWM );
}

