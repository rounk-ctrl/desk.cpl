/* ---------------------------------------------------------
* Themes page
* 
* Responsible for the "Themes" tab
* 
------------------------------------------------------------*/

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

	// detect classic theme, to show preview properly
	selectedTheme->szMsstylePath = L"(classic)";
	if (!IsClassicThemeEnabled())
	{
		auto themeClass = std::make_unique<CTheme>(currentITheme);

		LPWSTR path = nullptr;
		themeClass->get_VisualStyle(&path);
		selectedTheme->szMsstylePath = path;

		/*
		ITheme24H2* pth = (ITheme24H2*)currentITheme;
		LPWSTR path2;
		pth->GetPath(currThem, &path2);
		wprintf(L"%s\n", path2);

		IShellItemArray* array;
		pDesktopWallpaper->GetSlideshow(&array);
		
		std::vector<PIDLIST_ABSOLUTE> pidlList;

		DWORD size;
		array->GetCount(&size);
		for (int i = 0; i < size; ++i)
		{
			IShellItem* item;
			array->GetItemAt(i, &item);

			LPWSTR yes;
			item->GetDisplayName(SIGDN_FILESYSPATH, &yes);

			PIDLIST_ABSOLUTE pidl = nullptr;
			HRESULT hr = SHGetIDListFromObject(item, &pidl);
			if (SUCCEEDED(hr))
			{
				pidlList.push_back(pidl);
			}

			item->Release();
		}
		IShellItem* pShellItem = NULL;
		PIDLIST_ABSOLUTE pidlNew = nullptr;
		SHCreateItemFromParsingName(L"C:\\Windows\\ASUS\\Wallpapers\\ASUS.JPG", NULL, IID_PPV_ARGS(&pShellItem));
		HRESULT hr = SHGetIDListFromObject(pShellItem, &pidlNew);
		if (SUCCEEDED(hr))
		{
			pidlList.push_back(pidlNew);
		}

		IShellItemArray* ppNewArray;

		SHCreateShellItemArrayFromIDLists(
			static_cast<UINT>(pidlList.size()),
			const_cast<LPCITEMIDLIST*>(pidlList.data()),
			&ppNewArray);

		pDesktopWallpaper->SetSlideshow(ppNewArray);
		*/
	}

	// update THEMEINFO before setting bitmap for now
	WCHAR ws[MAX_PATH] = { 0 };
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
	UpdateThemeInfo(ws, currThem);

	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_THEMES, nullptr, GetDpiForWindow(m_hWnd));

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

	// update the string if different
	if (PathFileExists(path) 
		&& !IsClassicThemeEnabled()
		&& selectedTheme->szMsstylePath.compare(path) != 0)
	{
		selectedTheme->szMsstylePath = path;
		selectedTheme->fMsstyleChanged = true;
	}


	// update THEMEINFO
	UpdateThemeInfo(ws, index);

	// set the preview bitmap to the static control
	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(path), &ebmp, UPDATE_ALL);
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

	// apply the selected theme
	pThemeManager->SetCurrentTheme(m_hWnd, index, TRUE, apply_flags, 0);

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
		int index = ComboBox_GetCurSel(hCombobox);
		pThemeManager->GetTheme(index, &currentITheme);

		// set the preview bitmap to the static control
		HBITMAP ebmp;
		pWndPreview->GetUpdatedPreviewImage(wnd, nullptr, &ebmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
		SetBitmap(hPreview, ebmp);

		selectedTheme->updateWallThemesPg = false;
	}
	if (selectedTheme->fMsstyleChanged || selectedTheme->fThemePgMsstyleUpdate)
	{
		HBITMAP ebmp;
		pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str()), &ebmp, UPDATE_WINDOW);
		SetBitmap(hPreview, ebmp);

		selectedTheme->fThemePgMsstyleUpdate = selectedTheme->fMsstyleChanged = false;
	}
	_TerminateProcess(pi);
	return 0;
}

// set relevant info accordng to this page
// if u select a new theme, the settings set by background page
// becomes irrelevant, overwrite them
void CThemeDlgProc::UpdateThemeInfo(LPWSTR ws, int currThem)
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
		selectedTheme->wallpaperType = WT_PICTURE;
		selectedTheme->wallpaperPath = ws;
	}

	// common properties
	selectedTheme->newColor = 0xB0000000;
	selectedTheme->customWallpaperSelection = false;
	selectedTheme->posChanged = -1;
	selectedTheme->useDesktopColor = false;
}
