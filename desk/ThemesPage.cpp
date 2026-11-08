/* ---------------------------------------------------------
* Themes page
* 
* Responsible for the "Themes" tab
* 
------------------------------------------------------------*/

#include "pch.h"
#include "desk.h"
#include "helper.h"
#include "theme.h"
#include "ThemesPage.h"
#include "uxtheme.h"
#include "cscheme.h"
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;


BOOL CThemeDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// store HWNDs
	hCombobox = GetDlgItem(1101);
	hPreview = GetDlgItem(1103);
	size = GetClientSIZE(hPreview);

	int count = 0;

	// add all themes to combobox
	pThemeManager->GetThemeCount(&count);
	for (int i = 0; i < count; ++i)
	{
		ComPtr<ITheme10> pTheme;
		pThemeManager->GetTheme(i, &pTheme);

		LPWSTR str = nullptr;
		pTheme->get_DisplayName(&str);

		ComboBox_AddString(hCombobox, str);
	}

	// select current theme in combobox
	int currThem = 0;
	pThemeManager->GetCurrentTheme(&currThem);
	ComboBox_SetCurSel(hCombobox, currThem);

	pThemeManager->GetTheme(currThem, &currentITheme);

	// update THEMEINFO before setting bitmap for now
	WCHAR ws[MAX_PATH] = { 0 };
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
	UpdateThemeInfo(ws);

	if (selectedTheme->szMsstylePath.empty())
	{
		if (IsClassicThemeEnabled()) SelectVisualStyle(L"(classic)");
		else
		{
			auto themeClass = std::make_unique<CTheme>(currentITheme);
			LPWSTR path = nullptr;
			themeClass->get_VisualStyle(&path);
			if (!SelectVisualStyle(path, TRUE)) return FALSE;
		}
	}

	HANDLE theme = LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str(),
		selectedTheme->szColorName.c_str(), selectedTheme->szSizeName.c_str());
	if (!theme && selectedTheme->szMsstylePath != L"(classic)")
	{
		MessageBox(L"The selected visual style could not be loaded.", L"Themes", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_THEMES, theme, GetDpiForWindow(m_hWnd));

	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	SetBitmap(hPreview, bmp);

	return 0;
}

BOOL CThemeDlgProc::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	pWndPreview = nullptr;
	return 0;
}

BOOL CThemeDlgProc::OnThemeComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int index = ComboBox_GetCurSel(hCombobox);

	currentITheme->Release();
	pThemeManager->GetTheme(index, &currentITheme);

	LPWSTR ws = NULL;
	auto themeClass = std::make_unique<CTheme>(currentITheme);
	themeClass->get_background(&ws);

	LPWSTR path = nullptr;
	themeClass->get_VisualStyle(&path);

	if (!SelectVisualStyle(path && PathFileExists(path) ? path : L"(classic)"))
	{
		MessageBox(L"The selected visual style could not be loaded.", L"Themes", MB_OK | MB_ICONERROR);
		return 0;
	}

	HANDLE theme = LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str(),
		selectedTheme->szColorName.c_str(), selectedTheme->szSizeName.c_str());
	if (selectedTheme->szMsstylePath != L"(classic)"
		&& (!theme || !CreateThemedMetricsScheme(GetDpiForWindow(m_hWnd), theme)))
	{
		CloseThemeFile(theme);
		MessageBox(L"The selected visual style could not be loaded.", L"Themes", MB_OK | MB_ICONERROR);
		return 0;
	}
	selectedTheme->fMsstyleChanged = true;
	UpdateThemeInfo(ws);
	themeSelected = TRUE;

	// set the preview bitmap to the static control
	HBITMAP ebmp = nullptr;
	if (FAILED(pWndPreview->GetUpdatedPreviewImage(wnd, theme, &ebmp, UPDATE_ALL)))
	{
		MessageBox(L"The selected visual style could not be previewed.", L"Themes", MB_OK | MB_ICONERROR);
		return 0;
	}
	SetBitmap(hPreview, ebmp);

	SetModified(TRUE);
	return 0;
}

BOOL CThemeDlgProc::OnApply()
{
	// default apply flag, when applied in windows (ignore nothing)
	ULONG apply_flags = 0;
	int index = ComboBox_GetCurSel(hCombobox);

	if (selectedTheme->customWallpaperSelection)
		apply_flags |= THEMETOOL_APPLY_FLAG_IGNORE_BACKGROUND;

	if (selectedTheme->newColor != 0xB0000000)
		apply_flags |= THEMETOOL_APPLY_FLAG_IGNORE_COLOR;

	if (selectedTheme->szMsstylePath != L"(classic)")
	{
		HANDLE theme = nullptr;
		if (!selectedTheme->szColorName.empty() && !selectedTheme->szSizeName.empty())
			theme = LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str(),
				selectedTheme->szColorName.c_str(), selectedTheme->szSizeName.c_str());
		if (!theme)
		{
			MessageBox(L"The selected visual style could not be loaded.", L"Themes", MB_OK | MB_ICONERROR);
			return PSNRET_INVALID_NOCHANGEPAGE;
		}
		CloseThemeFile(theme);
	}

	if (FAILED(pThemeManager->SetCurrentTheme(m_hWnd, index, TRUE, apply_flags, 0))
		|| (selectedTheme->szMsstylePath != L"(classic)"
			&& (selectedTheme->szColorName.empty() || selectedTheme->szSizeName.empty()
				|| !SetSystemVisualStyle || FAILED(SetSystemVisualStyle(selectedTheme->szMsstylePath.c_str(),
					selectedTheme->szColorName.c_str(), selectedTheme->szSizeName.c_str(), AT_NONE)))))
	{
		MessageBox(L"The selected theme could not be applied.", L"Themes", MB_OK | MB_ICONERROR);
		return PSNRET_INVALID_NOCHANGEPAGE;
	}

	SetModified(FALSE);
	return 0;
}

BOOL CThemeDlgProc::OnSetActive()
{
	selectionPicker = true;
	if (selectedTheme->customWallpaperSelection 
		|| selectedTheme->newColor != 0xB0000000 
		|| selectedTheme->posChanged != -1 
		|| selectedTheme->updateWallThemesPg)
	{
		// set the preview bitmap to the static control
		HBITMAP ebmp;
		pWndPreview->GetUpdatedPreviewImage(wnd, nullptr, &ebmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
		SetBitmap(hPreview, ebmp);

		selectedTheme->updateWallThemesPg = false;
	}
	if (selectedTheme->fMsstyleChanged || selectedTheme->fThemePgMsstyleUpdate)
	{
		HANDLE theme = LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str(),
			selectedTheme->szColorName.c_str(), selectedTheme->szSizeName.c_str());
		if (!theme && selectedTheme->szMsstylePath != L"(classic)")
		{
			MessageBox(L"The selected visual style could not be loaded.", L"Themes", MB_OK | MB_ICONERROR);
			return 0;
		}
		HBITMAP ebmp = nullptr;
		if (SUCCEEDED(pWndPreview->GetUpdatedPreviewImage(wnd, theme, &ebmp, UPDATE_WINDOW)))
		{
			SetBitmap(hPreview, ebmp);
			selectedTheme->fThemePgMsstyleUpdate = false;
		}
	}

	_TerminateProcess(pi);
	return 0;
}

// set relevant info accordng to this page
// if u select a new theme, the settings set by background page
// becomes irrelevant, overwrite them
void CThemeDlgProc::UpdateThemeInfo(LPWSTR ws)
{
	// update THEMEINFO
	if (lstrlen(ws) == 0 || PathFileExists(ws) == FALSE)
	{
		// no wallpaper applied
		selectedTheme->wallpaperPath = L"";
		selectedTheme->wallpaperType = WT_NOWALL;
	}
	else
	{
		BOOL isEn = FALSE;
		auto themeClass = std::make_unique<CTheme>(currentITheme);
		themeClass->IsSlideshowEnabled(&isEn);
		if (isEn)
		{
			selectedTheme->wallpaperType = WT_SLIDESHOW;
			selectedTheme->fSlideshowSelection = true;

		}
		else
		{
			selectedTheme->wallpaperType = WT_PICTURE;
		}

		// wndprvw uses this to display the preview
		selectedTheme->wallpaperPath = ws;
	}

	// common properties
	selectedTheme->newColor = 0xB0000000;
	selectedTheme->customWallpaperSelection = false;
	selectedTheme->posChanged = -1;
	selectedTheme->useDesktopColor = false;
}
