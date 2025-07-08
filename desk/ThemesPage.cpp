#include "pch.h"
#include "ThemesPage.h"
#include "desk.h"
#include "uxtheme.h"
#include "helper.h"
#include "theme.h"
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;


BOOL CThemeDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// global variables
	hCombobox = GetDlgItem(1101);
	hPreview = GetDlgItem(1103);
	size = GetClientSIZE(hPreview);

	// initialize theme manager
	InitUxtheme();

	WCHAR ws[MAX_PATH] = { 0 };
	int count{};

	// add all themes to combobox
	pThemeManager->GetThemeCount(&count);
	for (auto i = 0; i < count; ++i)
	{
		// same across all w10
		IUnknown* the;
		pThemeManager->GetTheme(i, &the);
		ITheme10* them = (ITheme10*)the;
		LPWSTR str = nullptr;
		them->get_DisplayName(&str);

		ComboBox_AddString(hCombobox, str);
		them->Release();
	}

	// select current theme in combobox
	int currThem{};
	pThemeManager->GetCurrentTheme(&currThem);
	ComboBox_SetCurSel(hCombobox, currThem);

	pThemeManager->GetTheme(currThem, &currentITheme);

	ITheme* themeClass = new ITheme(currentITheme);
	LPWSTR path = nullptr;
	themeClass->get_VisualStyle(&path);
	StringCpy(selectedTheme->szMsstylePath, path);
	selectedTheme->fMsstyleChanged = true;

	// update THEMEINFO before setting bitmap for now
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
	UpdateThemeInfo(ws, currThem);

	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_THEMES, nullptr);

	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	Static_SetBitmap(hPreview, bmp);
	DeleteObject(bmp);

	return 0;
}

BOOL CThemeDlgProc::OnThemeComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int index = ComboBox_GetCurSel(hCombobox);

	currentITheme->Release();
	pThemeManager->GetTheme(index, &currentITheme);

	LPWSTR ws;
	ITheme* themeClass = new ITheme(currentITheme);
	themeClass->get_background(&ws);

	LPWSTR path = nullptr;
	themeClass->get_VisualStyle(&path);

	// update the string if different
	if (PathFileExists(path) 
		&& StrCmpI(selectedTheme->szMsstylePath, path) != 0)
	{
		FreeString(selectedTheme->szMsstylePath);
		StringCpy(selectedTheme->szMsstylePath, path);
		wprintf(selectedTheme->szMsstylePath);
		selectedTheme->fMsstyleChanged = true;
	}

	// update THEMEINFO
	UpdateThemeInfo(ws, index);

	// set the preview bitmap to the static control
	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(path), &ebmp);
	Static_SetBitmap(hPreview, ebmp);

	SetModified(TRUE);
	DeleteBitmap(ebmp);
	return 0;
}

BOOL CThemeDlgProc::OnApply()
{
	// default apply flag, when applied in windows (ignore nothing)
	ULONG apply_flags = 0;
	int index = ComboBox_GetCurSel(hCombobox);

	if (selectedTheme->customWallpaperSelection)
		apply_flags |= THEMETOOL_APPLY_FLAG_IGNORE_BACKGROUND;

	if (selectedTheme->newColor)
		apply_flags |= THEMETOOL_APPLY_FLAG_IGNORE_COLOR;

	// apply the selected theme
	pThemeManager->SetCurrentTheme(m_hWnd, index, TRUE, apply_flags, 0);

	SetModified(FALSE);
	return 0;
}

BOOL CThemeDlgProc::OnSetActive()
{
	selectionPicker = true;
	if (selectedTheme->customWallpaperSelection || selectedTheme->newColor || selectedTheme->posChanged != -1 || selectedTheme->updateWallThemesPg)
	{
		int index = ComboBox_GetCurSel(hCombobox);
		pThemeManager->GetTheme(index, &currentITheme);
		ITheme* themeClass = new ITheme(currentITheme);
		LPWSTR path = nullptr;
		themeClass->get_VisualStyle(&path);

		COLORREF clr;
		if (selectedTheme->useDesktopColor)
		{
			pDesktopWallpaper->GetBackgroundColor(&clr);
		}
		else
		{
			themeClass->GetBackgroundColor(&clr);
		}

		// set the preview bitmap to the static control
		HBITMAP ebmp;
		pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(path), &ebmp);
		Static_SetBitmap(hPreview, ebmp);
		
		DeleteBitmap(ebmp);
		selectedTheme->updateWallThemesPg = false;
	}
	_TerminateProcess(pi);
	return 0;
}

// set relevant info accordng to this page
// if u select a new theme, the settings set by background page
// becomes irrelevant, overwrite them
void CThemeDlgProc::UpdateThemeInfo(LPWSTR ws, int currThem)
{
	printf("\nupdate themeinfo\n");

	// free 
	FreeString(selectedTheme->wallpaperPath);

	// update THEMEINFO
	if (lstrlenW(ws) == 0 || PathFileExists(ws) == FALSE)
	{
		// no wallpaper applied
		selectedTheme->wallpaperType = WT_NOWALL;
	}
	else
	{
		selectedTheme->wallpaperType = WT_PICTURE;
		StringCpy(selectedTheme->wallpaperPath, ws);
	}

	// common properties
	selectedTheme->newColor = NULL;
	selectedTheme->customWallpaperSelection = false;
	selectedTheme->posChanged = -1;
	selectedTheme->useDesktopColor = false;
}
