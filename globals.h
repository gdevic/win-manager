/******************************************************************************
*                                                                             *
*   Module:     Globals.h                                                     *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       06/08/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This is a header file for the Virtual Windows Manager

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 06/08/97   1.00  Original                                       Goran Devic *
* --------   ----  ---------------------------------------------  ----------- *
*******************************************************************************
*   Important Defines                                                         *
******************************************************************************/
#ifndef _GLOBALS_H_
#define _GLOBALS_H_


/******************************************************************************
*                                                                             *
*   Include Files                                                             *
*                                                                             *
******************************************************************************/

#include "Resource.h"

/******************************************************************************
*                                                                             *
*   Global Defines, Variables and Macros                                      *
*                                                                             *
******************************************************************************/

#define TEST            0               // 1 or 0 for testing purposes

#define _EXPORT __export

#define NAME                "Virtual Windows Manager"
#define VERSION             "Release 1"
#define ABOUT               "VWM Copyright 1997 Goran Devic"
#define VWM_CLASS           "VWMClass"  // Class name of the VWM
#define VWM_CAPTION         "VWM"       // Caption on the main window
#define VWM_MENU_NAME       "Menu"      // Resource name of the menu
#define VWM_ICON_NAME       "Icon"      // Resource icon name
#define VWM_PROP_PAGE1_NAME "Page1"     // Resource name of the prop page 1
#define VWM_PROP_PAGE2_NAME "Page2"     // Resource name of the prop page 2
#define VWM_PROP_PAGE3_NAME "Page3"     // Resource name of the prop page 3
#define VWM_NO_TIP          ""          // No tip (empty string)

// Menu items

#define MENU_ABOUT          100
#define MENU_EXIT           101
#define MENU_PROPS          102
#define MENU_LOCK           103

#define MENU_APPS           0           // Position of the apps submenu


#define MYWM_NOTIFYICON     (WM_USER + 1)
#define VWM_WM_REFRESH      (WM_USER + 2)
#define VWM_WM_HOTKEY       (WM_USER + 3)
#define VWM_WM_NEWSIZE      (WM_USER + 4)
#define VWM_WM_INIT         (WM_USER + 5)

#define VWMID_TASKBAR       100

// Initial parameters that are used when VWM is started for the first time

#define INIT_VWM_LEFT       0           // Init X coordinate
#define INIT_VWM_TOP        0           // Init Y coordinate
#define INIT_VWM_WIDTH      100         // Init VWM width in pixels
#define INIT_VWM_HEIGHT     100         // Init VWM height in pixels
#define INIT_VWM_XSIZE      2           // Init number of desktops in X dir
#define INIT_VWM_YSIZE      3           // Init number of desktops in Y dir
#define INIT_VWM_AUTOHIDE   TRUE        // Init autohide enable
#define INIT_VWM_AH_DELAY   30          // Init autohide delay in ticks
#define INIT_VWM_MOUSE      TRUE        // Init mouse enable
#define INIT_VWM_KEYBOARD   TRUE        // Init keyboard hotkey
#define INIT_VWM_MS_DELAY   5           // Init mouse delay in ticks
#define INIT_VWM_ICONS  IDC_ICONS_FIXED // Init icons radio button state
#define INIT_VWM_BORDER IDC_BORDER_THIN // Init border radio button state
#define INIT_VWM_LIGHT_BACK TRUE        // Init light background color
#define INIT_VWM_DOUBLE_BUFFER TRUE     // Init double buffer mode
#define INIT_VWM_BITMAP_TILE IDC_BACK_VIRT // Init bitmap tiling mode

#define VWM_TIMER           100         // Set timer every 100 ms
#define VWM_TIMER_TICK      10          // 10 times per second
#define REFRESH_TIME        10          // Refresh records every second
#define CLEANUP_TIME        (60*5)      // Hash table cleanup every 5 minutes

#define MAX_WIN             100         // Maximum number of managed windows
#define MAX_HASH_TABLE      201         // Hash table array length
#define MAX_TEXTLEN         64          // Length of a window text string
#define MAX_VCUR_X          16          // Maximum virtual desks in X direction
#define MAX_VCUR_Y          16          // Maximum virtual desks in Y direction
#define MAX_IGNORE_CLASSES  10          // Maximum number of classes to ignore
#define MAX_BACK            10          // Number of files in a combo box

#define INFO_MOVED          0x0001      // Window just manually moved
#define INFO_DRAGGED        0x0002      // Window was dragged when mouse clicked
#define INFO_GET_POS        0x0004      // Window needs fresh size and position
#define INFO_MINIMIZED      0x0008      // Window is minimized
#define INFO_MAXIMIZED      0x0010      // Window is maximized

// The following operations are used with UseDC() function.  They affect the
// secondary buffer display context for double buffering

#define GETDC_CREATE        0           // Create secondary buffer display context
#define GETDC_DESTROY       1           // Destroy secondary buffer display context
#define GETDC_GETDC         2           // Return the display context to draw into
#define GETDC_BLT           3           // Blt the secondary buffer to the client
#define GETDC_RESIZE        4           // Resize secondary buffer display context

// The following operations are used with UseBitmap() function

#define USEBITMAP_BLT       0
#define USEBITMAP_LOAD      1
#define USEBITMAP_DESTROY   2


#if TEST
# define ERROR_MSG(str)     MessageBox(NULL,(str),VWM_CAPTION,MB_OK)
#else
# define ERROR_MSG(str)     ;
#endif

typedef struct TInfo                    // Virtual Window Info Structure
{
    HWND    hWin;                       // Window handle
    DWORD   dwFlags;                    // Internal flags
    HINSTANCE hInstance;                // Handle of the application instance

    RECT    rcNormalPosition;           // Normal position on the screen
    RECT    rcClient;                   // Window placement on the client
    POINT   MaxDesk;                    // Maximized window' main desktop coords
    RECT    rcClientMax;                // Maximized window client rectangle

    WINDOWPLACEMENT pl;                 // Window placement structure
    UINT    showWin;                    // Window original show state
    HICON   hIcon;                      // Handle of window icon
    HICON   hIconSmall;                 // Handle of window small icon
    char    sName [MAX_TEXTLEN];        // Window name

} TInfo;

typedef struct TMetrics                 // Program metrics structure
{
    int     nBorderX;                   // Window border size
    int     nBorderY;                   // Window border size
    int     nFullX;                     // Maximized window size
    int     nFullY;                     // Maximized window size
    int     nCaption;                   // Y size of a caption bar

} TMetrics;

typedef struct TBack                    // Background bitmap structure
{
    char sFile[MAX_PATH];               // File path and name
    int  nName;                         // Index to a name portion
    int  nDlg;                          // Index to a dialog list (combo box)

} TBack;

extern TBack Back[MAX_BACK];            // Background file names

extern char   * sVWMCaption;            // Program caption text

extern HWND     hDesktop;               // Desktop handle
extern HWND     hVWM;                   // VWM window handle
extern HANDLE   VWMInst;                // VWM program instance handle

extern HWND     Win [MAX_WIN];          // Array of windows handles
extern int      nHash [MAX_WIN];        // Index in the hash table
extern int      nWinTop;                // First unused element in Win array
extern struct TInfo Info [MAX_HASH_TABLE]; // Array of Info structures
extern TMetrics Metrics;                // Program metrics structure

extern BOOL     fAutohide;              // Autohide feature
extern int      nAutohideDelay;         // Autohide delay time in miliseconds
extern BOOL     fMouse;                 // Enable mouse
extern int      MouseParkTimeMax;       // Interval for mouse at the screen edge
extern BOOL     fKeyboard;              // Enable keyboard hotkey
extern int      fIcons;                 // Icons drawing option
extern int      fBorder;                // Border option
extern BOOL     fLightBack;             // Light background flag
extern BOOL     fDoubleBuffer;          // Double buffering is used for client
extern int      fBitmap;                // Client area background bitmap
extern int      nBitmapTile;            // Bitmap tiling option
extern BOOL     fLocked;                // Colsole is locked

extern POINT    VCur;                   // Current virtual window (ex 0,1)
extern POINT    VSize;                  // Size of the virtual desktop in screens ex 2,4
extern POINT    VStep;                  // Size of each virtual window in a client
extern POINT    VOrigin;                // Client coords of a visible desktop
extern POINT    VClientSize;            // Size of the client area
extern RECT     Vrc;                    // Inclusive rectangle of a current desktop
extern POINT    Desktop;                // Desktop size
extern XFORM    xToClient;              // Transformation Win->Client coords
extern XFORM    xToWin;                 // Transformation Client->Win coords
extern BOOL     fNeedPaint;             // Need paint client flag
extern int      RefreshTimeMax;         // Windows refresh time interval
extern char *   sIgnoreClasses[MAX_IGNORE_CLASSES];// Array of names of classes to ignore

/******************************************************************************
*                                                                             *
*   External Functions                                                        *
*                                                                             *
******************************************************************************/

extern BOOL VWM_Create( HWND hWnd );
extern void VWM_Close();

extern TInfo * GetWinInfo( HWND hWnd );
extern BOOL CALLBACK WinEnumProc( HWND hWin, LPARAM lParam );
extern void RecalculateClientPos();
extern void SetDeltaWindowPos( TInfo * pInfo, int dx, int dy );
extern void PurgeInfoTable();
extern BOOL UseBitmap( HDC hDC, int op );
extern HDC  UseDC( int op );
extern void SetVWMStyle();
extern TInfo * FindWindowPt( POINT pt, BOOL fMax );
extern BOOL RestoreWindow( POINT pt, TInfo * pInfo );
extern LPSTR GetWindowCaption( POINT pt );
extern void RestoreAll();
extern void LoadRegistryInfo( LPRECT rcVWM );
extern void SaveRegistryInfo();
extern void RecalculateNewVW( POINT * pt, HWND hInvalidate );
extern void VWMPaint( void );
extern BOOL TrayMessage(HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, PSTR pszTip);
extern void DisplayProperties();
extern void FormatAppsMenu( HMENU hMenu );
extern void LockConsole();

#endif  //  _GLOBALS_H_
