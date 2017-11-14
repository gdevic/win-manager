/******************************************************************************
*                                                                             *
*   Module:     Property.c                                                    *
*                                                                             *
*   Revision:   1.00                                                          *
*                                                                             *
*   Date:       7/8/97                                                      *
*                                                                             *
*   Author:     Goran Devic                                                   *
*                                                                             *
*******************************************************************************

    Module Description:

        This module contains the code for the property sheet.

*******************************************************************************
*                                                                             *
*   Changes:                                                                  *
*                                                                             *
*   DATE     REV   DESCRIPTION OF CHANGES                         AUTHOR      *
* --------   ----  ---------------------------------------------  ----------- *
* 7/8/97   1.00  Original                                       Goran Devic *
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

static char sFile[MAX_PATH];            // Temporary file path name
static LPHELPINFO lpInfo;               // Help info structure

/******************************************************************************
*                                                                             *
*   Functions                                                                 *
*                                                                             *
******************************************************************************/
BOOL WINAPI Page1Proc( HWND, UINT, WPARAM, LPARAM );
BOOL WINAPI Page2Proc( HWND, UINT, WPARAM, LPARAM );
BOOL WINAPI Page3Proc( HWND, UINT, WPARAM, LPARAM );

/******************************************************************************
*                                                                             *
*   void DisplayProperties()                                                  *
*                                                                             *
*******************************************************************************
*
*   This function creates the property sheet page.
*
*   Where:
*
*
*   Returns:
*
*
******************************************************************************/
void DisplayProperties()
{
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hPsp[3];

    // Create the property pages

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = VWMInst;
    psp.pszTemplate = VWM_PROP_PAGE1_NAME;
    psp.pfnDlgProc = (DLGPROC) Page1Proc;

    hPsp[0] = CreatePropertySheetPage( &psp );


    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = VWMInst;
    psp.pszTemplate = VWM_PROP_PAGE2_NAME;
    psp.pfnDlgProc = (DLGPROC) Page2Proc;

    hPsp[1] = CreatePropertySheetPage( &psp );


    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = VWMInst;
    psp.pszTemplate = VWM_PROP_PAGE3_NAME;
    psp.pfnDlgProc = (DLGPROC) Page3Proc;

    hPsp[2] = CreatePropertySheetPage( &psp );


    // Initialize the property sheet structure

    ZeroMemory( &psh, sizeof(PROPSHEETHEADER) );
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID;
    psh.hInstance = VWMInst;
    psh.hwndParent = hVWM;
    psh.pszIcon = VWM_ICON_NAME;
    psh.nPages = 3;
    psh.phpage = hPsp;
    psh.pszCaption = "Properties";

    // Display the property sheet

    PropertySheet( &psh );
}



/******************************************************************************
*                                                                             *
*   Page1Proc                                                                 *
*                                                                             *
*******************************************************************************
*
*   This is the main function for the property page 1.
*
******************************************************************************/
BOOL WINAPI Page1Proc(HWND hPage1, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pNmhdr;
    int newX, newY, newAutohide, newDelay;
    BOOL fOK;

    switch( Msg )
    {
        //---------------------------------------------------------------------
        case WM_INITDIALOG:
                // Dialog is being initialized

                // Set the current desktop sizes

                SetDlgItemInt( hPage1, IDC_SIZE_X, VSize.x, FALSE );
                SetDlgItemInt( hPage1, IDC_SIZE_Y, VSize.y, FALSE );

                // Set the autohide state and delay

                SetDlgItemInt( hPage1, IDC_AUTOHIDE_DELAY, nAutohideDelay / VWM_TIMER_TICK, FALSE );

                CheckDlgButton( hPage1, IDC_ENABLE_AUTOHIDE, fAutohide? BST_CHECKED : BST_UNCHECKED );

                // Set the mouse/keyboard enable and mouse delay time

                CheckDlgButton( hPage1, IDC_ENABLE_MOUSE, fMouse? BST_CHECKED : BST_UNCHECKED );
                CheckDlgButton( hPage1, IDC_ENABLE_KEYBOARD, fKeyboard? BST_CHECKED : BST_UNCHECKED );
                SetDlgItemInt(  hPage1, IDC_MOUSE_DELAY, MouseParkTimeMax * VWM_TIMER, FALSE );

                return( TRUE );

            break;
        //---------------------------------------------------------------------
        case WM_NOTIFY:

                pNmhdr = (NMHDR *) lParam;

                switch( pNmhdr->code )
                {
                    case PSN_APPLY:
                        // The user pressed OK button

                        // Get the new desktop size

                        newX = GetDlgItemInt( hPage1, IDC_SIZE_X, &fOK, FALSE );
                        newY = GetDlgItemInt( hPage1, IDC_SIZE_Y, &fOK, FALSE );

                        if( newX != VSize.x || newY != VSize.y )
                        {
                            if( newX<1 || newX>MAX_VCUR_X ) newX = VSize.x;
                            if( newY<1 || newY>MAX_VCUR_Y ) newY = VSize.y;

                            SendMessage( hVWM, VWM_WM_NEWSIZE, newX, newY );
                        }

                        // Get the new autohide options

                        newAutohide = GetDlgItemInt( hPage1, IDC_AUTOHIDE_DELAY, &fOK, FALSE );

                        if( newAutohide>=1 && newAutohide<=120 )
                            nAutohideDelay = newAutohide * VWM_TIMER_TICK;

                        fAutohide = IsDlgButtonChecked( hPage1, IDC_ENABLE_AUTOHIDE );

                        // Get the new mouse/keyboard enable flags and mouse delay

                        fMouse    = IsDlgButtonChecked( hPage1, IDC_ENABLE_MOUSE );
                        fKeyboard = IsDlgButtonChecked( hPage1, IDC_ENABLE_KEYBOARD );
                        newDelay  = GetDlgItemInt( hPage1, IDC_MOUSE_DELAY, &fOK, FALSE );

                        if( newDelay < 10000 )
                            MouseParkTimeMax = newDelay / VWM_TIMER;

                        return( PSNRET_NOERROR );
                }

            break;
        //---------------------------------------------------------------------
        case WM_HELP:
                // Context sensitive help

                lpInfo = (LPHELPINFO) lParam;
                if( lpInfo->iCtrlId > 0 )
                    WinHelp( lpInfo->hItemHandle, HELP_FILE_PATH, HELP_CONTEXTPOPUP, lpInfo->iCtrlId );
            break;
        //---------------------------------------------------------------------
        default:
            return FALSE;
        //---------------------------------------------------------------------
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   Page2Proc                                                                 *
*                                                                             *
*******************************************************************************
*
*   This is the main function for the property page 2.
*
******************************************************************************/
BOOL WINAPI Page2Proc(HWND hPage2, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pNmhdr;
    BOOL fExitCode;
    OPENFILENAME File;
    int i, nCode, nCodeNone;

    switch( Msg )
    {
        //---------------------------------------------------------------------
        case WM_INITDIALOG:
                // Dialog is being initialized

                // Set the radio buttons

                CheckRadioButton( hPage2, IDC_ICONS_DONTDRAW, IDC_ICONS_FIXED, fIcons );
                CheckRadioButton( hPage2, IDC_BORDER_THIN, IDC_BORDER_NO, fBorder );
                CheckRadioButton( hPage2, IDC_BACK_VIRT, IDC_BACK_STRETCH, nBitmapTile );

                // Set the light background checkbox

                CheckDlgButton( hPage2, IDC_LIGHT_BACK, fLightBack );

                // Add files to a combo box containing the background images

                nCodeNone = SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_ADDSTRING, 0, (LPARAM) "(none)" );

                i=0;
                while( Back[i].sFile[0] != '\0' )
                {
                    Back[i].nDlg = SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_ADDSTRING, 0, (LPARAM) &Back[i].sFile[Back[i].nName] );
                    i++;
                }

                // Set the current selection

                if( fBitmap )
                    SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_SETCURSEL, Back[fBitmap-1].nDlg, 0 );
                else
                    SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_SETCURSEL, nCodeNone, 0 );

                return( TRUE );

            break;

        //---------------------------------------------------------------------
        case WM_NOTIFY:

                pNmhdr = (NMHDR *) lParam;

                switch( pNmhdr->code )
                {
                    case PSN_APPLY:
                        // The user pressed OK button

                        // Check the radio buttons for icons

                        if( IsDlgButtonChecked( hPage2, IDC_ICONS_DONTDRAW) )
                            fIcons = IDC_ICONS_DONTDRAW;
                        else
                            if( IsDlgButtonChecked( hPage2, IDC_ICONS_FIXED) )
                                fIcons = IDC_ICONS_FIXED;
                            else
                                fIcons = IDC_ICONS_STRETCH;

                        // Check the radio buttons for border

                        if( IsDlgButtonChecked( hPage2, IDC_BORDER_THIN) )
                            fBorder = IDC_BORDER_THIN;
                        else
                            if( IsDlgButtonChecked( hPage2, IDC_BORDER_THICK) )
                                fBorder = IDC_BORDER_THICK;
                            else
                                fBorder = IDC_BORDER_NO;

                        SetVWMStyle();

                        // Check the radio buttons for bitmap tiling

                        if( IsDlgButtonChecked( hPage2, IDC_BACK_VIRT) )
                            nBitmapTile = IDC_BACK_VIRT;
                        else
                            if( IsDlgButtonChecked( hPage2, IDC_BACK_TILE) )
                                nBitmapTile = IDC_BACK_TILE;
                            else
                                nBitmapTile = IDC_BACK_STRETCH;

                        // Check the light back checkbox

                        fLightBack = IsDlgButtonChecked( hPage2, IDC_LIGHT_BACK );

                        // Get the current background selected item

                        nCode = SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_GETCURSEL, 0, 0 );

                        // Find the index of a selected bitmap structure

                        fBitmap = 0;

                        for( i=0; i<MAX_BACK && Back[i].sFile[0] != '\0'; i++ )
                            if( Back[i].nDlg == nCode )
                                fBitmap = i + 1;

                        // Load a specified bitmap

                        if( fBitmap )
                            if( UseBitmap( 0, USEBITMAP_LOAD )==FALSE )
                            {
                                // Bitmap cannot be loaded

                                MessageBox( hPage2, "Bitmap cannot be loaded!",
                                            sVWMCaption, MB_OK );

                                fBitmap = FALSE;

                                return( PSNRET_INVALID_NOCHANGEPAGE );
                            }

                        return( PSNRET_NOERROR );
                }

            break;

        //---------------------------------------------------------------------
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_BROWSE:
                        // Browse for a bitmap

                        ZeroMemory( &File, sizeof(OPENFILENAME) );
                        File.lStructSize = sizeof(OPENFILENAME);
                        File.hwndOwner = hPage2;
                        File.lpstrFilter = "Bitmap files (*.bmp)\0*.bmp\0"\
                                           "All files (*.*)\0*.*\0\0";
                        File.lpstrFile = sFile;
                        File.nMaxFile = MAX_PATH;
                        File.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
                        File.lpstrDefExt = "bmp";
                        File.lpstrTitle = "Select bitmap";

                        fExitCode = GetOpenFileName( &File );

                        if( fExitCode )
                        {
                            // Add a file to a local structure at the end

                            for( i=0; i<MAX_BACK; i++ )
                                if( Back[i].sFile[0]=='\0' )
                                    break;

                            lstrcpy( Back[i].sFile, sFile );
                            Back[i].nName = File.nFileOffset;

                            // Add a file to the combo box

                            Back[i].nDlg = SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_ADDSTRING, 0,
                                (LPARAM) &Back[i].sFile[File.nFileOffset] );

                            // Set this item to the forefront

                            SendDlgItemMessage( hPage2, IDC_BACK_LIST, CB_SETCURSEL, Back[i].nDlg, 0 );

                            InvalidateRect( hPage2, NULL, TRUE );
                        }
                    break;
            }

            break;

        //---------------------------------------------------------------------
        case WM_HELP:
                // Context sensitive help

                lpInfo = (LPHELPINFO) lParam;
                if( lpInfo->iCtrlId > 0 )
                    WinHelp( lpInfo->hItemHandle, HELP_FILE_PATH, HELP_CONTEXTPOPUP, lpInfo->iCtrlId );
            break;
        //---------------------------------------------------------------------
        default:
            return FALSE;
        //---------------------------------------------------------------------
    }

    return( FALSE );
}


/******************************************************************************
*                                                                             *
*   Page3Proc                                                                 *
*                                                                             *
*******************************************************************************
*
*   This is the main function for the property page 3.
*
******************************************************************************/
BOOL WINAPI Page3Proc(HWND hPage3, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pNmhdr;
    int i;

    switch( Msg )
    {
        //---------------------------------------------------------------------
        case WM_INITDIALOG:
                // Dialog is being initialized

                // Add the classes that is to be ignored

                i = 0;
                while( sIgnoreClasses[i] != NULL )
                {
                    SendDlgItemMessage( hPage3, IDC_IGNORE_CLASSES, LB_ADDSTRING, 0, (LPARAM) sIgnoreClasses[i] );
                    i++;
                }

                // Set the double buffer checkbox

                CheckDlgButton( hPage3, IDC_DOUBLE_BUFFER, fDoubleBuffer );

                return( TRUE );

            break;

        //---------------------------------------------------------------------
        case WM_NOTIFY:

                pNmhdr = (NMHDR *) lParam;

                switch( pNmhdr->code )
                {
                    case PSN_APPLY:
                        // The user pressed OK button

                        // Check the double buffer checkbox

                        fDoubleBuffer = IsDlgButtonChecked( hPage3, IDC_DOUBLE_BUFFER );

                        return( PSNRET_NOERROR );
                }

            break;

        //---------------------------------------------------------------------
        case WM_HELP:
                // Context sensitive help

                lpInfo = (LPHELPINFO) lParam;
                if( lpInfo->iCtrlId > 0 )
                    WinHelp( lpInfo->hItemHandle, HELP_FILE_PATH, HELP_CONTEXTPOPUP, lpInfo->iCtrlId );
            break;
        //---------------------------------------------------------------------
        default:
            return FALSE;
        //---------------------------------------------------------------------
    }

    return( FALSE );
}

