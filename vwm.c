/******************************************************************************
*                                                                             *
*   Module:     VWM.c                                                         *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/08/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is the main module for the Virtual Windows Manager

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 06/08/97   1.00  Original                                       Goran Devic *
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

char * sVWMClass   = VWM_CLASS;
char * sVWMCaption = VWM_CAPTION;
char * sVWMMenu    = VWM_MENU_NAME;

TBack       Back[MAX_BACK];             // Background file names

HWND        hDesktop;                   // Desktop handle
HWND        hVWM;                       // VWM window handle
HANDLE      VWMInst;                    // VWM program instance handle

HWND        Win [MAX_WIN];              // Array of windows handles
int         nHash [MAX_WIN];            // Index in the hash table
int         nWinTop;                    // First unused element in Win array
struct TInfo Info [MAX_HASH_TABLE];     // Array of Info structures
TMetrics    Metrics;                    // Program metrics structure

BOOL        fAutohide;                  // Autohide feature
int         nAutohideDelay;             // Autohide delay time in WM_TIMER ticks
BOOL        fMouse;                     // Enable mouse
int         MouseParkTimeMax;           // Interval for mouse at the screen edge
BOOL        fKeyboard;                  // Enable keyboard hotkey
int         fIcons;                     // Icons drawing option
int         fBorder;                    // Border option
BOOL        fLightBack;                 // Light background flag
BOOL        fDoubleBuffer;              // Double buffering is used for client
BOOL        fBitmap;                    // Client area background bitmap
int         nBitmapTile;                // Bitmap tiling option
BOOL        fLocked;                    // Console is locked

POINT       VCur;                       // Current virtual window (ex 0,1)
POINT       VSize;                      // Size of the virtual desktop in screens ex 2,4
POINT       VStep;                      // Size of each virtual window in a client
POINT       VOrigin;                    // Client coords of a visible desktop
POINT       VClientSize;                // Size of the client area
RECT        Vrc;                        // Inclusive rectangle of a current desktop
POINT       Desktop;                    // Desktop size
XFORM       xToClient;                  // Transformation Win->Client coords
XFORM       xToWin;                     // Transformation Client->Win coords
BOOL        fNeedPaint;                 // Need paint client flag
int         RefreshTimeMax;             // Windows refresh time interval
char *      sIgnoreClasses[MAX_IGNORE_CLASSES];// Array of names of classes to ignore

/******************************************************************************
*                                                                             *
*   Local Defines, Variables and Macros                                       *
*                                                                             *
******************************************************************************/

static HMENU hMenu;                     // Handle of a menu
static HMENU hMenuPopup;                // Handle of the first item in a menu
static HWND  hToolTip;                  // Handle of the tooltip control
static TOOLINFO tTip;                   // Tooltip structure
static HICON hIcon;                     // Handle of VWM icon

static BOOL fMouseParked;               // Desktop changes via mouse
static int MouseParkTime;               // Interval counter for mouse park time
static int RefreshTime;                 // Interval counter for windows refresh
static int CleanupTime;                 // Time to clean up hash structure
static int nAutohideCount;              // Autohide count
static BOOL fDontHide;                  // Do not hide window

static POINT ptClientMouse;             // Cached mouse coordinates on client
static LPSTR lpLastTip;                 // Cached last tip string

static BOOL fCriticalSection = FALSE;   // Critical section semaphore
static TInfo * pDragWindow;             // Window to drag
static POINT ptDrag;                    // Dragging previous point coordinate

// Still implicitly exported variables:

int nTimer;                             // Timer handle

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

long _EXPORT FAR PASCAL WindowProc( HWND, unsigned, UINT, LONG );


/******************************************************************************
*                                                                             *
*   void SemaphoreShowWindow( fShow )                                         *
*                                                                             *
*******************************************************************************
*
*   Semaphore function
*
******************************************************************************/
void SemaphoreShowWindow( fShow )
{
    if( fShow==TRUE )
        fDontHide++;
    else
        fDontHide--;
}


/******************************************************************************
*                                                                             *
*   void EchoError()                                                          *
*                                                                             *
*******************************************************************************
*
*   This function posts a message box with the LastError code
*
******************************************************************************/
void EchoError()
{
    char sError[32];

    sprintf( sError, "Error %d", GetLastError() );

    MessageBox( NULL,
        sError,
        sVWMCaption,
        MB_ICONSTOP | MB_OK );
}


/******************************************************************************
*                                                                             *
*   WinMain                                                                   *
*                                                                             *
*******************************************************************************
*
*   Initialization and message loop
*
******************************************************************************/
int PASCAL WinMain( HANDLE this_inst, HANDLE prev_inst, LPSTR cmdline, int cmdshow )
{
    MSG         msg;
    RECT        rcVWM;
    WNDCLASS    wc;


    // There can be only one instance of this application in the system.
    // Look if the window with our class and caption is registered

    if( FindWindow( sVWMClass, sVWMCaption ) != NULL )
    {
        // Found a previous instance, inform the user and exit

        MessageBox( NULL,
            "Virtual Windows Manager already running!",
            sVWMCaption,
            MB_ICONSTOP | MB_OK );

        return( 0 );
    }

    VWMInst = this_inst;

    hIcon = LoadIcon( this_inst, VWM_ICON_NAME );

    // Register window class for the application

    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = (LPVOID) WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = this_inst;
    wc.hIcon         = hIcon;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = sVWMClass;

    if( RegisterClass( &wc ) == FALSE )
    {
        // Cannot register class, return

        EchoError();
        return( FALSE );
    }


    // Load the information from the registry

    LoadRegistryInfo( &rcVWM );

    // Load the menu

    hMenu = LoadMenu( VWMInst, sVWMMenu );

    if( hMenu == NULL )
    {
        // Cannot load menu

        EchoError();
        return( FALSE );
    }

    // Get the handle of the first item in a menu

    hMenuPopup = GetSubMenu( hMenu, 0 );

    // Make sure that the common control dll is loaded

    InitCommonControls();


    // Create the main window

    hVWM = CreateWindowEx(
        WS_EX_STATICEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        sVWMClass,                      // Window class
        sVWMCaption,                    // Window caption
        WS_BORDER | WS_CAPTION | WS_SIZEBOX | WS_SYSMENU,
        rcVWM.left, rcVWM.top, rcVWM.right, rcVWM.bottom,
        NULL,                           // Parent window
        NULL,                           // Menu handle
        VWMInst,                        // Program instance
        NULL );                         // Params

    if( hVWM == NULL )
    {
        // Unable to create window.

        EchoError();

        return( FALSE );
    }

    // Set the window style

    SetVWMStyle();

    // Create a tooltip class as a subclass of a main window

    hToolTip = CreateWindowEx( 0,
        TOOLTIPS_CLASS,                 // Tooltip class
        NULL,                           // Caption
        TTS_ALWAYSTIP,                  // Flafs
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hVWM,                           // Parent window
        NULL,                           // Menu handle
        VWMInst,                        // Program instance
        NULL );                         // Params

    // Set the tooltip structure

    tTip.cbSize   = sizeof(TOOLINFO);
    tTip.hwnd     = hVWM;
    tTip.hinst    = NULL;
    tTip.uFlags   = TTF_SUBCLASS;
    tTip.uId      = 0;
    tTip.lpszText = LPSTR_TEXTCALLBACK;
    CopyMemory( &tTip.rect, &rcVWM, sizeof(RECT) );

    SendMessage( hToolTip, TTM_ADDTOOL, 0, (LPARAM) &tTip );


    // Display window

    ShowWindow( hVWM, cmdshow );
    UpdateWindow( hVWM );

    // Message pump...

    while( GetMessage( &msg, NULL, NULL, NULL ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return( msg.wParam );
}


/******************************************************************************
*                                                                             *
*   AboutDialog                                                               *
*                                                                             *
*******************************************************************************
*
*   This is a standard about dialog function
*
******************************************************************************/
BOOL _EXPORT FAR PASCAL AboutDlgProc( HWND hWnd, unsigned Msg, UINT wParam, LONG lParam )
{
    lParam = lParam;                    // Turn off warning :-)

    // Process the dialog box messages

    if( Msg==WM_COMMAND )
    {
        if( LOWORD( wParam ) == IDOK )
        {
            // The dialog is finished when user clicks on OK button

            EndDialog( hWnd, TRUE );

            return( TRUE );
        }
    }

    switch( Msg )
    {
        case WM_INITDIALOG:
                // Process the message for initialize dialog

                return( TRUE );
            break;
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   WindowProc                                                                *
*                                                                             *
*******************************************************************************
*
*   The main window procedure
*
******************************************************************************/
LONG _EXPORT FAR PASCAL WindowProc( HWND hWnd, unsigned Msg, UINT wParam, LONG lParam )
{
    POINT       pt;
    BOOL        fFlag;
    TInfo     * pInfo;
    int         result;
    int         nTempTop;
    int         dx, dy;

    // Switch on a windows message

    if( Msg==WM_COMMAND )
    {
        // Menu IDs in the range (0, MAX_WIN-1) are the selected apps

        if( LOWORD(wParam) < MAX_WIN )
        {
            // Get the window structure for the selected window

            pInfo = GetWinInfo( Win[ LOWORD(wParam) ] );

            // This should not be necessary, but bugs do happen...

            if( pInfo->hWin != Win[ LOWORD(wParam) ] )
            {
                ERROR_MSG("MenuSelect: Warning");
            }
            else
            {
                // Get the client coordinates of that window.  If the
                // window is maximized, calculate fake coords

                if( pInfo->showWin == SW_SHOWMAXIMIZED )
                {
                    result = ((pInfo->MaxDesk.y * VStep.y) << 16)
                           |  (pInfo->MaxDesk.x * VStep.x);
                }
                else
                {
                    result = (pInfo->rcClient.top << 16) | pInfo->rcClient.left;
                }

                SendMessage( hVWM, WM_LBUTTONUP, MK_SHIFT, result );

                // Activate and show the window

                RestoreWindow( pt, pInfo );

                InvalidateRect( hVWM, NULL, TRUE );
            }

            return( 0L );
        }

        // Make sure the window is visible and that it wont go
        // away during the duration of a menu

        SemaphoreShowWindow( TRUE );

        // Switch on a command messages

        switch( LOWORD( wParam ) )
        {
            case MENU_PROPS:
                    // Display the property sheet pages

                    DisplayProperties();

                break;

            case MENU_ABOUT:
                    // User selected about menu item

                    ShellAbout( hVWM, NAME, ABOUT, hIcon );

                break;

            case MENU_LOCK:
                    // User want to lock the keyboard

                    LockConsole();

                break;

            case MENU_EXIT:
                    // Send a message to itself to exit

                    PostMessage( hVWM, WM_CLOSE, 0, 0 );

                break;
        };

        // Window can go away now if it wants

        SemaphoreShowWindow( FALSE );
    }

    // Switch on rest of windows messages

    switch( Msg )
    {
        // --------------------------------------------------------------------
        case WM_MOUSEMOVE:
                // Cache the mouse coordinates

                ptClientMouse.x = LOWORD(lParam);
                ptClientMouse.y = HIWORD(lParam);

                // If we are dragging the window, perform the window movement

                if( pDragWindow != NULL )
                {
                    // Calculate the distance that the mouse moved

                    dx = ptClientMouse.x - ptDrag.x;
                    dy = ptClientMouse.y - ptDrag.y;

                    // Actually move the selected window if the difference is not 0

                    if( dx || dy )
                    {
                        pDragWindow->dwFlags |= INFO_DRAGGED;

                        // Store new point as the old one for the next pass

                        ptDrag.x = ptClientMouse.x;
                        ptDrag.y = ptClientMouse.y;

                        SetDeltaWindowPos( pDragWindow, dx, dy );

                        // Invalidate the client area, WM_PAINT will be sent by Windows

                        InvalidateRect( hVWM, NULL, TRUE );
                    }
                }
                else
                {
                    // Mouse has simply moved over the client area.  Send a
                    // message to a tooltip control to ask for the new text

                    if( lpLastTip != GetWindowCaption( ptClientMouse ) )
                    {
                        SendMessage( hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM) &tTip );
                    }
                }

            break;

        // --------------------------------------------------------------------
        case WM_LBUTTONDOWN:
                // User has pressed left button.  If the mouse is over a window,
                // prepare that window to be dragged

                // Get the coordinates inside a virtual window that
                // the mouse pointer is on

                ptDrag.x = LOWORD(lParam);
                ptDrag.y = HIWORD(lParam);

                // Get the address of the window to drag

                pDragWindow = FindWindowPt( ptDrag, FALSE );

            break;

        // --------------------------------------------------------------------
        case WM_LBUTTONUP:
                // User has released left button.  The current desktop
                // should be changed

                if( pDragWindow != NULL )
                {
                    if( pDragWindow->dwFlags & INFO_DRAGGED )
                    {
                        pDragWindow->dwFlags &= ~INFO_DRAGGED;

                        pDragWindow = NULL;

                        break;
                    }

                    pDragWindow = NULL;
                }

                // Calculate the coordinates of a virtual screen that the
                // mouse was in at this time

                pt.x = LOWORD(lParam) / VStep.x;
                pt.y = HIWORD(lParam) / VStep.y;

                // Due to integer (im)precision, clamp the max values

                if( (pt.x >= VSize.x) )
                    pt.x = VSize.x - 1;

                if( (pt.y >= VSize.y) )
                    pt.y = VSize.y - 1;

                // Set the temp flag to False

                fFlag = FALSE;

                // If the coordinates are different than the current...
                // change the virtual desktop to the one selected.

                if( (pt.x != VCur.x) || (pt.y != VCur.y) )
                {
                    // Recalculate the position on the client of all guest windows
                    // and invalidate the VWM area

                    RecalculateNewVW( &pt, hVWM );

                    fFlag = TRUE;
                }

                if( (wParam != MK_SHIFT) &&
                    ((wParam == MK_CONTROL) || (fFlag==FALSE)) )
                {
                    // If the coordinates are the same as the virtual desktop,
                    // we should restore the application that the mouse cursor was

                    // Get the coordinates inside a virtual window that
                    // the mouse pointer is on

                    pt.x = LOWORD(lParam);
                    pt.y = HIWORD(lParam);

                    if( RestoreWindow( pt, NULL ) == TRUE )
                    {
                        // Need to repaint the client window

                        SendMessage( hVWM, VWM_WM_REFRESH, 0, 0 );
                    }
                }

            break;

        // --------------------------------------------------------------------
        case WM_SIZE:
                // Client size changed - need to recalculate some variables

                VClientSize.x = LOWORD(lParam);
                VClientSize.y = HIWORD(lParam);

                // Recalculate the position on the client of all guest windows

                RecalculateClientPos();

                // Adjust tooltip rectangle to be equal to the VWM client

                GetClientRect( hVWM, &tTip.rect );
                SendMessage( hToolTip, TTM_NEWTOOLRECT, 0, (LPARAM) &tTip );

                // Resize the secondary buffer display context surface

                UseDC( GETDC_RESIZE );

            break;

        // --------------------------------------------------------------------
        case WM_PAINT:
                // Paint message

                VWMPaint();

            break;

        // --------------------------------------------------------------------
        case VWM_WM_REFRESH:
                // A window has been activated or deactivated.  Need to update
                // the internal database and redraw the client area

                // If we are in the critical section already, just pass the
                // message

                if( fCriticalSection )
                    break;

                // Set the critical section semaphore

                fCriticalSection = TRUE;

                // Reset the interval counter

                RefreshTime = RefreshTimeMax;

                // We will use temporary top index while enumerating

                nTempTop = 0;
                fNeedPaint = FALSE;

                // Enumerate windows handles

                if( EnumWindows( WinEnumProc, (LPARAM)&nTempTop ) == FALSE )
                {
                    // Enumeration had failed.  Cannot do anything then, so quit

                    MessageBox( NULL, "Cannot enumerate windows (2)!", sVWMCaption,
                            MB_ICONERROR + MB_SETFOREGROUND + MB_OK );

                    return( -1 );
                }

                // Redraw the client window if the information changed (differnt
                // number of windows, their location/size changed)

                if( nTempTop != nWinTop || fNeedPaint==TRUE )
                {
                    InvalidateRect( hVWM, NULL, TRUE );

                    nWinTop = nTempTop;

                    // The hash table may get filled up with old window records that
                    // are no longer valid.  We clean it up every so often

                    if( CleanupTime-- == 0 )
                    {
                        // Set the new clean up time

                        CleanupTime = CLEANUP_TIME;

                        PurgeInfoTable();
                    }
                }

                // End the critical section

                fCriticalSection = FALSE;

            break;

        // --------------------------------------------------------------------
        case VWM_WM_HOTKEY:
                // User has pressed alt-direction key combination

                // Change the virtual desk if keyboard hotkey is enabled

                if( fKeyboard && !fLocked )
                {
                    pt.x = -1;

                    if( wParam==VK_LEFT && VCur.x>0 )
                    {
                        pt.x = VCur.x - 1;
                        pt.y = VCur.y;
                    }
                    else if( wParam==VK_UP && VCur.y>0 )
                    {
                        pt.x = VCur.x;
                        pt.y = VCur.y - 1;
                    }
                    else if( wParam==VK_RIGHT && VCur.x<VSize.x-1 )
                    {
                        pt.x = VCur.x + 1;
                        pt.y = VCur.y;
                    }
                    else if( wParam==VK_DOWN && VCur.y<VSize.y-1 )
                    {
                        pt.x = VCur.x;
                        pt.y = VCur.y + 1;
                    }

                    if( pt.x != -1 )
                    {
                        // Recalculate the position on the client of all guest windows
                        // and invalidate the whole screen area

                        RecalculateNewVW( &pt, NULL );
                    }
                }
            break;

        // --------------------------------------------------------------------
        case WM_TIMER:
                // Timer:
                //   o  enumerate guest windows
                //   o  change the virtual desktop
                //   o  autohide count
                //   o  autohide show/hide logic

                if( RefreshTime-- == 0 )
                {
                    // Reset the interval counter

                    RefreshTime = RefreshTimeMax;

                    PostMessage( hVWM, VWM_WM_REFRESH, 0, 0 );
                }


                if( MouseParkTime-- == 0 )
                {
                    // Reset the interval counter

                    MouseParkTime = MouseParkTimeMax;

                    // If the mouse was held on a screen border for the duration of two
                    // timer periods, change the virtual desktop

                    GetCursorPos( &pt );

                    if( pt.x==0 || pt.y==0 || pt.x==Desktop.x-1 || pt.y==Desktop.y-1 )
                    {
                        // If the autohide is on, show the VWM window

                        ShowWindow( hVWM, SW_SHOW );
                        nAutohideCount = 0;

                        // Change desktop only if mouse is enabled on the second pass

                        if( fMouse && fMouseParked==TRUE )
                        {
                            // This is a second occurrence of a mouse at a border

                            // Change the virtual desk

                            if( pt.x==0 && VCur.x>0 )
                            {
                                pt.x = VCur.x - 1;
                                pt.y = VCur.y;

                                fMouseParked = FALSE;
                            }
                            else if( pt.y==0 && VCur.y>0 )
                            {
                                pt.x = VCur.x;
                                pt.y = VCur.y - 1;

                                fMouseParked = FALSE;
                            }
                            else if( pt.x==Desktop.x-1 && VCur.x<VSize.x-1 )
                            {
                                pt.x = VCur.x + 1;
                                pt.y = VCur.y;

                                fMouseParked = FALSE;
                            }
                            else if( pt.y==Desktop.y-1 && VCur.y<VSize.y-1 )
                            {
                                pt.x = VCur.x;
                                pt.y = VCur.y + 1;

                                fMouseParked = FALSE;
                            }

                            if( fMouseParked == FALSE )
                            {
                                // Recalculate the position on the client of all guest windows
                                // and invalidate the whole screen area

                                RecalculateNewVW( &pt, NULL );
                            }
                            else
                                fMouseParked = FALSE;

                        }
                        else
                        {
                            // Mouse had just arrived at the screen border

                            fMouseParked = TRUE;
                        }
                    }
                    else
                        fMouseParked = FALSE;
                }

                if( fAutohide && fDontHide<=0 )
                {
                    if( nAutohideCount++ > nAutohideDelay )
                    {
                        // If the mouse pointer is still on the VWM window, do
                        // not hide it.  Otherwise, hide it.

                        GetCursorPos( &pt );

                        if( WindowFromPoint(pt) != hVWM )
                            ShowWindow( hVWM, SW_HIDE );
                        else
                            nAutohideCount = 0; // Defer hide
                    }
                }

            break;

        // --------------------------------------------------------------------
        case WM_RBUTTONDOWN:
                // Right mouse button activates the pop up menu

                // Get the mouse coordinates inside the client area
                // and convert them into the screen coordinates

                pt.x = LOWORD(lParam);  // horizontal position of cursor
                pt.y = HIWORD(lParam);  // vertical position of cursor

                ClientToScreen( hVWM, &pt );

                // Make sure the window is visible and that it wont go
                // away during the duration of a menu

                SemaphoreShowWindow( TRUE );

                // Add the menu items with the names of running apps

                FormatAppsMenu( hMenuPopup );

                // Display the pop up menu

                TrackPopupMenu( hMenuPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON,
                        pt.x, pt.y, 0, hVWM, NULL );

                // Window can go away now if it wants

                SemaphoreShowWindow( FALSE );

            break;

        // --------------------------------------------------------------------
        case MYWM_NOTIFYICON:
                // Mouse action occurred over the taskbar icon

                if( wParam == VWMID_TASKBAR )
                {
                    // The icon is ours

                    switch( lParam )
                    {
                        case WM_LBUTTONDOWN:
                            // Left mouse button activates the pop up menu

                            GetCursorPos( &pt );

                            // Set the foreground window

                            SetForegroundWindow( hVWM );

                            TrackPopupMenu( hMenuPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON,
                                pt.x, pt.y, 0, hVWM, NULL );

                            // Clear the popup menu

                            PostMessage( hVWM, WM_NULL, 0, 0 );

                            break;
                    }
                }
            break;

        // --------------------------------------------------------------------
        case WM_NOTIFY:
                // This message is used by the tooltip to ask for the tip text

                if( ((LPNMHDR) lParam)->code == TTN_NEEDTEXT )
                {
                    // Find the guest window that is on the top where mouse points
                    // to and set its window caption as a tooltip

                    lpLastTip = GetWindowCaption( ptClientMouse );

                    ((LPTOOLTIPTEXT)lParam)->lpszText = lpLastTip;
                }

            break;

        // --------------------------------------------------------------------
        case VWM_WM_NEWSIZE:
                // Virtual desktop size has changed

                VSize.x = wParam;
                VSize.y = lParam;

                // Recalculate client position

                RecalculateClientPos();

                // Recalculate the position on the client of all guest windows
                // and invalidate the VWM area

                RecalculateNewVW( &VCur, hVWM );

            break;

        // --------------------------------------------------------------------
        case WM_KILLFOCUS:
                // When using autohide and the VWM looses the focus, the hide count starts

                nAutohideCount = 0;

                return( 0 );

            break;

        // --------------------------------------------------------------------
        case WM_CREATE:
                // Window is about to be created.

                // Init some local variables that are not exported to other files

                fMouseParked = FALSE;
                MouseParkTime = MouseParkTimeMax;
                RefreshTime = RefreshTimeMax;
                CleanupTime = CLEANUP_TIME;
                nAutohideCount = 0;
                lpLastTip = VWM_NO_TIP;
                fCriticalSection = FALSE;
                pDragWindow = NULL;
                fDontHide = FALSE;

                // Call the global initialization

                VWM_Create( hWnd );

            break;

        // --------------------------------------------------------------------
        case VWM_WM_INIT:
                // Custom initialization on the startup and when screen size changed

                // Get some system metrics

                Metrics.nBorderX = GetSystemMetrics( SM_CXSIZEFRAME );
                Metrics.nBorderY = GetSystemMetrics( SM_CYSIZEFRAME );

                Metrics.nFullX   = GetSystemMetrics( SM_CXMAXIMIZED );
                Metrics.nFullY   = GetSystemMetrics( SM_CYMAXIMIZED );

                Metrics.nCaption = GetSystemMetrics( SM_CYCAPTION );

            break;

        // --------------------------------------------------------------------
        case WM_CLOSE:
                // User tries to abort the program.  Ask him if he really
                // wants to do so

                result = MessageBox(
                    hVWM,
                    "Really quit VWM ?",
                    sVWMCaption,
                    MB_APPLMODAL | MB_YESNO | MB_ICONQUESTION );

                if( result==IDYES )
                {
                    // Yes - really quit

                    // Save relevant info into registry

                    SaveRegistryInfo();

                    DestroyWindow( hVWM );

                    // If any guest window is being left virtual,
                    // reset its position on the screen

                    RestoreAll();

                    // Remove icon from the taskbar

                    TrayMessage( hVWM, NIM_DELETE, VWMID_TASKBAR, NULL, NULL );

                    // Call the global close function

                    VWM_Close();

                    // Release the secondary buffer display context

                    UseDC( GETDC_DESTROY );

                    // Destroy the background bitmap

                    UseBitmap( 0, USEBITMAP_DESTROY );
                }
            break;

        // --------------------------------------------------------------------
        case WM_DESTROY:
                PostQuitMessage( 0 );
            break;

        // --------------------------------------------------------------------
        default:
            return( DefWindowProc( hWnd, Msg, wParam, lParam ) );
    }

    // By default, return 0 for all messages that were handled but not
    // explicitly terminated

    return( 0L );
}

