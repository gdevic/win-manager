/******************************************************************************
*                                                                             *
*   Module:     Registry.c                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       6/10/97                                                       *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module deals with the registry settings

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 6/10/97   1.00  Original                                       Goran Devic *
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

#define VERSION_STR_LEN     32

// The global key that is used to store configuration info

static char * sAppKey  = "Software\\Goran Devic\\VWM";
static char * sBackKey = "Software\\Goran Devic\\VWM\\Backgrounds";
static HANDLE hkResult, hkBack;
static DWORD  dwAction;

// The values of a global key

static char * sClassDWORD = "DWORD";
static DWORD  dwTypeDWORD = REG_DWORD;
static DWORD  dwSizeDWORD = sizeof(DWORD);
static DWORD  dwTypeStr   = REG_SZ;
static DWORD  dwSizeStr   = VERSION_STR_LEN;
static DWORD  dwSizeFile  = MAX_PATH - 1;

static LPTSTR sVersion   = "Version";

static LPTSTR sRegLeft   = "Left";
static LPTSTR sRegTop    = "Top";
static LPTSTR sRegWidth  = "Width";
static LPTSTR sRegHeight = "Height";

static LPTSTR sRegSizeX  = "Desktops in X";
static LPTSTR sRegSizeY  = "Desktops in Y";

static LPTSTR sKeyboard  = "Enable keyboard";
static LPTSTR sMouse     = "Enable mouse";
static LPTSTR sMousePark = "Mouse park time";
static LPTSTR sRefresh   = "Windows refresh time";

static LPTSTR sAutohide       = "Autohide";
static LPTSTR sAutohideDelay  = "Autohide delay";

static LPTSTR sIcons     = "Icons";
static LPTSTR sBorder    = "Border";
static LPTSTR sLightBack = "Light background";
static LPTSTR sDB        = "Double buffer";
static LPTSTR sBitmapTile= "Bitmap tiling";
static LPTSTR sBitmap    = "Current bitmap";

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/

/******************************************************************************
*                                                                             *
*   void LoadRegistryInfo( LPRECT rcVWM )                                     *
*                                                                             *
*******************************************************************************
*
*   This function is called once when the VWM starts to read in all the
#   values that need to be stored between sessions.
*
******************************************************************************/
void LoadRegistryInfo( LPRECT rcVWM )
{
    char    sVersionValue[VERSION_STR_LEN];
    int     i, retCode;


    // Open the key that is assigned to VWM application.  If the key does
    // not exists, it will be created

    RegCreateKeyEx( HKEY_CURRENT_USER, sAppKey, 0, sClassDWORD,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkResult, &dwAction );

    // Query the version string - so far we dont do anything with it

    RegQueryValueEx( hkResult, sVersion, NULL, &dwTypeStr, (LPBYTE) sVersionValue, &dwSizeStr );

    // Read the values from the registry; if you cannot read, use default value

    if( RegQueryValueEx( hkResult, sRegLeft,   NULL, &dwTypeDWORD, (LPBYTE) &rcVWM->left, &dwSizeDWORD ) != ERROR_SUCCESS)
        rcVWM->left   = INIT_VWM_LEFT;

    if( RegQueryValueEx( hkResult, sRegTop,    NULL, &dwTypeDWORD, (LPBYTE) &rcVWM->top, &dwSizeDWORD ) != ERROR_SUCCESS)
        rcVWM->top    = INIT_VWM_TOP;

    if( RegQueryValueEx( hkResult, sRegWidth,  NULL, &dwTypeDWORD, (LPBYTE) &rcVWM->right, &dwSizeDWORD ) != ERROR_SUCCESS)
        rcVWM->right  = INIT_VWM_WIDTH;

    if( RegQueryValueEx( hkResult, sRegHeight, NULL, &dwTypeDWORD, (LPBYTE) &rcVWM->bottom, &dwSizeDWORD ) != ERROR_SUCCESS)
        rcVWM->bottom = INIT_VWM_HEIGHT;

    if( RegQueryValueEx( hkResult, sRegSizeX, NULL, &dwTypeDWORD, (LPBYTE) &VSize.x, &dwSizeDWORD ) != ERROR_SUCCESS)
        VSize.x = INIT_VWM_XSIZE;

    if( RegQueryValueEx( hkResult, sRegSizeY, NULL, &dwTypeDWORD, (LPBYTE) &VSize.y, &dwSizeDWORD ) != ERROR_SUCCESS)
        VSize.y = INIT_VWM_YSIZE;

    if( RegQueryValueEx( hkResult, sMouse, NULL, &dwTypeDWORD, (LPBYTE) &fMouse, &dwSizeDWORD ) != ERROR_SUCCESS)
        fMouse = INIT_VWM_MOUSE;

    if( RegQueryValueEx( hkResult, sKeyboard, NULL, &dwTypeDWORD, (LPBYTE) &fKeyboard, &dwSizeDWORD ) != ERROR_SUCCESS)
        fKeyboard = INIT_VWM_KEYBOARD;

    if( RegQueryValueEx( hkResult, sMousePark, NULL, &dwTypeDWORD, (LPBYTE) &MouseParkTimeMax, &dwSizeDWORD ) != ERROR_SUCCESS)
        MouseParkTimeMax = INIT_VWM_MS_DELAY;

    if( RegQueryValueEx( hkResult, sRefresh, NULL, &dwTypeDWORD, (LPBYTE) &RefreshTimeMax, &dwSizeDWORD ) != ERROR_SUCCESS)
        RefreshTimeMax = REFRESH_TIME;

    if( RegQueryValueEx( hkResult, sAutohide, NULL, &dwTypeDWORD, (LPBYTE) &fAutohide, &dwSizeDWORD ) != ERROR_SUCCESS)
        fAutohide = INIT_VWM_AUTOHIDE;

    if( RegQueryValueEx( hkResult, sAutohideDelay, NULL, &dwTypeDWORD, (LPBYTE) &nAutohideDelay, &dwSizeDWORD ) != ERROR_SUCCESS)
        nAutohideDelay = INIT_VWM_AH_DELAY;

    if( RegQueryValueEx( hkResult, sIcons, NULL, &dwTypeDWORD, (LPBYTE) &fIcons, &dwSizeDWORD ) != ERROR_SUCCESS)
        fIcons = INIT_VWM_ICONS;

    if( RegQueryValueEx( hkResult, sBorder, NULL, &dwTypeDWORD, (LPBYTE) &fBorder, &dwSizeDWORD ) != ERROR_SUCCESS)
        fBorder = INIT_VWM_BORDER;

    if( RegQueryValueEx( hkResult, sLightBack, NULL, &dwTypeDWORD, (LPBYTE) &fLightBack, &dwSizeDWORD ) != ERROR_SUCCESS)
        fLightBack = INIT_VWM_LIGHT_BACK;

    if( RegQueryValueEx( hkResult, sDB, NULL, &dwTypeDWORD, (LPBYTE) &fDoubleBuffer, &dwSizeDWORD ) != ERROR_SUCCESS)
        fDoubleBuffer = INIT_VWM_DOUBLE_BUFFER;

    if( RegQueryValueEx( hkResult, sBitmapTile, NULL, &dwTypeDWORD, (LPBYTE) &nBitmapTile, &dwSizeDWORD ) != ERROR_SUCCESS)
        nBitmapTile = INIT_VWM_BITMAP_TILE;

    if( RegQueryValueEx( hkResult, sBitmap, NULL, &dwTypeDWORD, (LPBYTE) &fBitmap, &dwSizeDWORD ) != ERROR_SUCCESS)
        fBitmap = 0;


    // Open the key that is assigned to VWM backgrounds.  If the key does
    // not exists, it will be created

    RegCreateKeyEx( HKEY_CURRENT_USER, sBackKey, 0, sClassDWORD,
        REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_ALL_ACCESS, NULL, &hkBack, &dwAction );

    // Clear Back array

    ZeroMemory( &Back[0], sizeof(Back) );

    // Loop and get all the subkeys

    for( i=0, retCode=ERROR_SUCCESS; retCode==ERROR_SUCCESS && i<MAX_BACK; i++ )
    {
        retCode = RegEnumValue( hkBack, i, Back[i].sFile, &dwSizeFile, NULL, &dwTypeDWORD, (LPBYTE) &Back[i].nName, &dwSizeDWORD );
    }
}


/******************************************************************************
*                                                                             *
*   void SaveRegistryInfo()                                                   *
*                                                                             *
*******************************************************************************
*
*   This function saves the values into registry.
*
******************************************************************************/
void SaveRegistryInfo()
{
    RECT rcVWM;
    int  i;


    // Calculate the VWM window position and size

    GetWindowRect( hVWM, &rcVWM );
    rcVWM.right -= rcVWM.left;
    rcVWM.bottom -= rcVWM.top;

    // Save the values into the registry.  The key is held open from the init
    // time (function LoadRegistryInfo)

    RegSetValueEx( hkResult, sVersion, NULL, REG_SZ, (LPBYTE) VERSION, VERSION_STR_LEN );

    RegSetValueEx( hkResult, sRegLeft,   NULL, REG_DWORD, (LPBYTE) &rcVWM.left, sizeof(DWORD) );
    RegSetValueEx( hkResult, sRegTop,    NULL, REG_DWORD, (LPBYTE) &rcVWM.top, sizeof(DWORD) );
    RegSetValueEx( hkResult, sRegWidth,  NULL, REG_DWORD, (LPBYTE) &rcVWM.right, sizeof(DWORD) );
    RegSetValueEx( hkResult, sRegHeight, NULL, REG_DWORD, (LPBYTE) &rcVWM.bottom, sizeof(DWORD) );

    RegSetValueEx( hkResult, sRegSizeX, NULL, REG_DWORD, (LPBYTE) &VSize.x, sizeof(DWORD) );
    RegSetValueEx( hkResult, sRegSizeY, NULL, REG_DWORD, (LPBYTE) &VSize.y, sizeof(DWORD) );

    RegSetValueEx( hkResult, sMouse,     NULL, REG_DWORD, (LPBYTE) &fMouse, sizeof(DWORD) );
    RegSetValueEx( hkResult, sKeyboard,  NULL, REG_DWORD, (LPBYTE) &fKeyboard, sizeof(DWORD) );
    RegSetValueEx( hkResult, sMousePark, NULL, REG_DWORD, (LPBYTE) &MouseParkTimeMax, sizeof(DWORD) );
    RegSetValueEx( hkResult, sRefresh,   NULL, REG_DWORD, (LPBYTE) &RefreshTimeMax, sizeof(DWORD) );

    RegSetValueEx( hkResult, sAutohide,      NULL, REG_DWORD, (LPBYTE) &fAutohide, sizeof(DWORD) );
    RegSetValueEx( hkResult, sAutohideDelay, NULL, REG_DWORD, (LPBYTE) &nAutohideDelay, sizeof(DWORD) );

    RegSetValueEx( hkResult, sIcons, NULL, REG_DWORD, (LPBYTE) &fIcons, sizeof(DWORD) );
    RegSetValueEx( hkResult, sBorder, NULL, REG_DWORD, (LPBYTE) &fBorder, sizeof(DWORD) );
    RegSetValueEx( hkResult, sLightBack, NULL, REG_DWORD, (LPBYTE) &fLightBack, sizeof(DWORD) );
    RegSetValueEx( hkResult, sDB, NULL, REG_DWORD, (LPBYTE) &fDoubleBuffer, sizeof(DWORD) );
    RegSetValueEx( hkResult, sBitmapTile, NULL, REG_DWORD, (LPBYTE) &nBitmapTile, sizeof(DWORD) );
    RegSetValueEx( hkResult, sBitmap, NULL, REG_DWORD, (LPBYTE) &fBitmap, sizeof(DWORD) );


    // First, delete the key contatinig the background images

    RegDeleteKey( HKEY_CURRENT_USER, sBackKey );

    // Create the (empty) key

    RegCreateKeyEx( HKEY_CURRENT_USER, sBackKey, 0, sClassDWORD,
        REG_OPTION_NON_VOLATILE, KEY_ENUMERATE_SUB_KEYS | KEY_ALL_ACCESS, NULL, &hkBack, &dwAction );

    // Loop and store all the subkeys for backgrounds

    for( i=0; Back[i].sFile[0]!='\0' && i<MAX_BACK; i++ )
    {
        RegSetValueEx( hkBack, Back[i].sFile, NULL, REG_DWORD, (LPBYTE) &Back[i].nName, sizeof(DWORD) );
    }
}

