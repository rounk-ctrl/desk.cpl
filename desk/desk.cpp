#include "desk.h"
#include "pch.h"
#include "ThemesPage.h"
#include "BackgroundPage.h"

namespace fs = std::filesystem;

HINSTANCE g_hinst;
IThemeManager2* pThemeManager = NULL;
IDesktopWallpaper* pDesktopWallpaper = NULL;
ULONG_PTR gdiplusToken;
LPWSTR wallpath{};
int lastpos{};
COLORREF newColor{};
BOOL noWall{};
BOOL firstSelect = TRUE;

const IID IID_IThemeManager2 = { 0xc1e8c83e, 0x845d, 0x4d95, {0x81, 0xdb, 0xe2, 0x83, 0xfd, 0xff, 0xc0, 0x00} };

LRESULT CALLBACK ScrSaverDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

LRESULT CALLBACK AppearanceDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

LRESULT CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}


void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
	switch (uMsg)
	{
		//called before the dialog is created, hwndPropSheet = NULL, lParam points to dialog resource
	case PSCB_PRECREATE:
	{
		LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;

		if (!(lpTemplate->style & WS_SYSMENU))
		{
			lpTemplate->style |= WS_SYSMENU;
		}
	}
	break;

	//called after the dialog is created
	case PSCB_INITIALIZED:
		break;

	}
}
void PropertySheetMoment()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HRESULT hr = CoCreateInstance(CLSID_ThemeManager2, NULL, CLSCTX_INPROC_SERVER, IID_IThemeManager2, (void**)&pThemeManager);
	pThemeManager->Init(ThemeInitNoFlags);

	hr = CoCreateInstance(CLSID_DesktopWallpaper, NULL, CLSCTX_ALL, IID_IDesktopWallpaper, (void**)&pDesktopWallpaper);

	PROPSHEETPAGE psp[5];
	PROPSHEETHEADER psh;

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = g_hinst;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_THEMEDLG);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = ThemeDlgProc;
	psp[0].pszTitle = TEXT("Themes");
	psp[0].lParam = 0;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = g_hinst;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_BACKGROUNDDLG);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = BackgroundDlgProc;
	psp[1].pszTitle = TEXT("Background");
	psp[1].lParam = 0;

	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE;
	psp[2].hInstance = g_hinst;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_SCRSVRDLG);
	psp[2].pszIcon = NULL;
	psp[2].pfnDlgProc = ScrSaverDlgProc;
	psp[2].pszTitle = TEXT("Screen Saver");
	psp[2].lParam = 0;

	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USETITLE;
	psp[3].hInstance = g_hinst;
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_APPEARANCEDLG);
	psp[3].pszIcon = NULL;
	psp[3].pfnDlgProc = AppearanceDlgProc;
	psp[3].pszTitle = TEXT("Appearance");
	psp[3].lParam = 0;

	psp[4].dwSize = sizeof(PROPSHEETPAGE);
	psp[4].dwFlags = PSP_USETITLE;
	psp[4].hInstance = g_hinst;
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_SETTINGSDLG);
	psp[4].pszIcon = NULL;
	psp[4].pfnDlgProc = SettingsDlgProc;
	psp[4].pszTitle = TEXT("Settings");
	psp[4].lParam = 0;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
	psh.hwndParent = 0;
	psh.hInstance = g_hinst;
	psh.pszIcon = 0;
	psh.pszCaption = TEXT("Display Properties");
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = (PFNPROPSHEETCALLBACK)PropSheetCallback;

	PropertySheet(&psh);

	// cleanup
	Gdiplus::GdiplusShutdown(gdiplusToken);
	pThemeManager->Release();
}

extern "C" LONG APIENTRY CPlApplet(
	HWND hwndCPL,       // handle of Control Panel window
	UINT uMsg,          // message
	LONG_PTR lParam1,       // first message parameter
	LONG_PTR lParam2        // second message parameter
)
{
	LPCPLINFO lpCPlInfo;
	LONG retCode = 0;

	switch (uMsg)
	{
	case CPL_INIT:
		return TRUE;

	case CPL_GETCOUNT:
		return 1L;

	case CPL_INQUIRE:
		lpCPlInfo = (LPCPLINFO)lParam2;
		lpCPlInfo->idIcon = IDI_ICON1;
		lpCPlInfo->idName = IDS_THEMESCPL;
		lpCPlInfo->idInfo = IDS_THEMESDESC;
		lpCPlInfo->lData = 0L;
		break;
	case CPL_DBLCLK:
		PropertySheetMoment();
		break;
	}
	return retCode;
}