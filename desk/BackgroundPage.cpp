﻿#include "pch.h"
#include "BackgroundPage.h"
#include "desk.h"
#include "helper.h"
#include "wndprvw.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;

std::vector<LPWSTR> wallpapers;
const COMDLG_FILTERSPEC file_types[] = {
	{L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)",
	L"*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png"},
};

HRESULT CBackgroundDlgProc::GetSolidBtnBmp(SIZE size, HBITMAP* pbOut)
{
	int i = MulDiv(10, GetDpiForWindow(m_hWnd), 96);
	Gdiplus::Bitmap bmp(size.cx - i, size.cy - i);
	Gdiplus::Graphics g(&bmp);
	Gdiplus::SolidBrush brush(Gdiplus::Color(SPLIT_COLORREF(GetDeskopColor())));
	g.FillRectangle(&brush, 0, 0, bmp.GetWidth(), bmp.GetHeight());

	return bmp.GetHBITMAP(Gdiplus::Color(0, 0, 0), pbOut) == Gdiplus::Ok ? S_OK : E_FAIL;
}

BOOL CBackgroundDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	firstInit = TRUE;
	hListView = GetDlgItem(1202);
	hBackPreview = GetDlgItem(1200);
	hPosCombobox = GetDlgItem(1205);
	backPreviewSize = GetClientSIZE(hBackPreview);

	if (!currentITheme)
	{
		int cur;
		pThemeManager->GetCurrentTheme(&cur);
		pThemeManager->GetTheme(cur, &currentITheme);
	}

	RECT rect;
	::GetClientRect(hListView, &rect);
	AddColumn(hListView, rect.right - rect.left - 30);

	AddItem(hListView, 0, L"(None)");
	HICON barrierico = LoadIcon(LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_AS_DATAFILE), MAKEINTRESOURCE(1027));
	ImageList_AddIcon(hml, barrierico);
	ListView_SetImageList(hListView, hml, LVSIL_SMALL);
	DestroyIcon(barrierico);

	WCHAR wallpaperdir[MAX_PATH];
	WCHAR windowswaldir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Web\\Wallpaper", wallpaperdir, MAX_PATH);
	ExpandEnvironmentStrings(L"%windir%", windowswaldir, MAX_PATH);
	LPCWSTR extensions[] = { L".jpg",  L".png", L".bmp", L".jpeg", L".dib", L".gif"};

	// this basically combines wallpaperdir and windowswaldir
	std::vector<std::pair<std::wstring, BOOL>> searchdir;
	searchdir.emplace_back(wallpaperdir, TRUE);
	searchdir.emplace_back(windowswaldir, FALSE);

	for (const auto& [dir, recurse] : searchdir)
	{
		EnumDir(dir.c_str(), extensions, ARRAYSIZE(extensions), wallpapers, recurse);
	}

	// this basically alphabetically sorts the wallpaper file names, for example if you have
	// a few files in windir and a few in web it will put windir files at the end and the web files on top
	// this basically fixes that issue and makes it like how it was in windows xp
	std::sort(wallpapers.begin(), wallpapers.end(), [](LPWSTR a, LPWSTR b) {
		LPCWSTR fileA = PathFindFileNameW(a);
		LPCWSTR fileB = PathFindFileNameW(b);
		int cmpResult = StrCmpLogicalW(fileA, fileB);
		return cmpResult < 0;
	});
	
	// start with k=1, k=0 is (none)
	int k = 1;
	for (auto path : wallpapers)
	{
		AddItem(hListView, k, path);
		k++;
	}

	// first time do it with current theme, the other func will take care of missing
	IUnknown* thcr;
	int cu;
	pThemeManager->GetCurrentTheme(&cu);
	pThemeManager->GetTheme(cu, &thcr);
	AddMissingWallpapers(thcr);
	thcr->Release();

	// combobox with positions of wallpaper
	LPCWSTR items[] = { L"Centre", L"Tile", L"Stretch",  L"Fit",  L"Fill",  L"Span" };
	for (int i = 0; i < _countof(items); i++)
	{
		ComboBox_AddString(hPosCombobox, items[i]);
	}

	DESKTOP_WALLPAPER_POSITION pos;
	pDesktopWallpaper->GetPosition(&pos);
	ComboBox_SetCurSel(hPosCombobox, pos);

	AddMissingWallpapers(currentITheme);
	SelectCurrentWallpaper(currentITheme);

	HBITMAP hBmp;
	GetSolidBtnBmp(GetClientSIZE(GetDlgItem(1207)), &hBmp);
	::SendMessage(GetDlgItem(1207), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
	DeleteBitmap(hBmp);

	firstInit = FALSE;
	return 0;
}

BOOL CBackgroundDlgProc::OnBgSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	selectedTheme->posChanged = ComboBox_GetCurSel(hPosCombobox);

	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	Static_SetBitmap(hBackPreview, bmp);
	DeleteBitmap(bmp);

	SetModified(TRUE);
	return 0;
}

BOOL CBackgroundDlgProc::OnBrowse(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	IFileDialog* pfd;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));

	// get options
	DWORD dwFlags;
	hr = pfd->GetOptions(&dwFlags);

	// set the file types
	hr = pfd->SetFileTypes(ARRAYSIZE(file_types), file_types);

	// the first element from the array
	hr = pfd->SetFileTypeIndex(1);

	pfd->SetTitle(L"Browse");

	// Show the dialog
	hr = pfd->Show(hWnd);

	IShellItem* psiResult;
	hr = pfd->GetResult(&psiResult);
	if (SUCCEEDED(hr)) {
		PWSTR pszFilePath = NULL;
		hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
		if (SUCCEEDED(hr)) {
			int inde = AddItem(hListView, ListView_GetItemCount(hListView), pszFilePath);

			ListView_SetItemState(hListView, inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(hListView, inde, FALSE);
			CoTaskMemFree(pszFilePath);
		}
	}
	pfd->Release();
	return 0;
}

BOOL CBackgroundDlgProc::OnColorPick(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CHOOSECOLOR cc = {0};
	if (ColorPicker(hWnd, &cc) == TRUE)
	{
		selectedTheme->newColor = cc.rgbResult;
		HBITMAP hBmp;
		GetSolidBtnBmp(GetClientSIZE(GetDlgItem(1207)), &hBmp);
		::SendMessage(GetDlgItem(1207), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
		DeleteBitmap(hBmp);

		HBITMAP bmp;
		pWndPreview->GetPreviewImage(&bmp);
		Static_SetBitmap(hBackPreview, bmp);
		DeleteBitmap(bmp);

		SetModified(TRUE);
	}
	return 0;
}

BOOL CBackgroundDlgProc::OnWallpaperSelection(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)nmhdr;
	if (pnmv->uNewState & LVIS_SELECTED)
	{
		selectedIndex = pnmv->iItem;
		LPWSTR path = GetWallpaperPath(hListView, pnmv->iItem);

		if (lstrcmp(path, L"(none)") == 0)
		{
			::EnableWindow(hPosCombobox, false);

			if (selectedTheme->wallpaperPath != nullptr)
			{
				delete[] selectedTheme->wallpaperPath;
			}

			selectedTheme->wallpaperPath = nullptr;
			selectedTheme->wallpaperType = WT_NOWALL;

		}
		else
		{
			::EnableWindow(hPosCombobox, true);

			// set new path
			size_t len = wcslen(path) + 1;
			selectedTheme->wallpaperPath = new wchar_t[len];
			wcscpy_s(selectedTheme->wallpaperPath, len, path);

			selectedTheme->wallpaperType = WT_PICTURE;
		}
		if (selectionPicker)
		{
			selectedTheme->customWallpaperSelection = false;
		}
		else
		{
			selectedTheme->customWallpaperSelection = true;
		}

		COLORREF clrlv;
		if (selectedTheme->useDesktopColor)
		{
			pDesktopWallpaper->GetBackgroundColor(&clrlv);
		}
		else
		{
			ITheme* themeClass = new ITheme(currentITheme);
			themeClass->GetBackgroundColor(&clrlv);
		}

		HBITMAP bmp;
		if (!pWndPreview)
		{
			pWndPreview = Make<CWindowPreview>(backPreviewSize, nullptr, 0, PAGETYPE::PT_BACKGROUND, nullptr, GetDpiForWindow(m_hWnd));
			pWndPreview->GetPreviewImage(&bmp);
		}
		else
		{
			pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &bmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
		}
		Static_SetBitmap(hBackPreview, bmp);

		SetModified(TRUE);
		DeleteObject(bmp);
	}
	return 0;
}

BOOL CBackgroundDlgProc::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	printf("WM_SETTINGCHANGED:\n");
	AddMissingWallpapers(currentITheme);
	SelectCurrentWallpaper(currentITheme);
	return 0;
}

BOOL CBackgroundDlgProc::OnApply()
{
	if (selectedTheme->posChanged != -1)
	{
		int index = ComboBox_GetCurSel(hPosCombobox);
		pDesktopWallpaper->SetPosition((DESKTOP_WALLPAPER_POSITION)index);
		selectedTheme->posChanged = -1;
	}
	if (selectedTheme->newColor)
	{
		pDesktopWallpaper->SetBackgroundColor(selectedTheme->newColor);
		selectedTheme->newColor = NULL;
	}
	if (selectedTheme->customWallpaperSelection)
	{
		if (selectedTheme->wallpaperType == WT_PICTURE)
		{
			pDesktopWallpaper->Enable(true);
			pDesktopWallpaper->SetWallpaper(NULL, selectedTheme->wallpaperPath);
		}
		else if (selectedTheme->wallpaperType == WT_NOWALL)
		{
			pDesktopWallpaper->Enable(false);
		}
		selectedTheme->customWallpaperSelection = false;
	}

	selectedTheme->updateWallThemesPg = true;
	SetModified(FALSE);
	return 0;
}

BOOL CBackgroundDlgProc::OnSetActive()
{
	selectionPicker = false;
	if (!selectedTheme->customWallpaperSelection && !firstInit)
	{
		AddMissingWallpapers(currentITheme);
		SelectCurrentWallpaper(currentITheme);

		HBITMAP hBmp;
		GetSolidBtnBmp(GetClientSIZE(GetDlgItem(1207)), &hBmp);
		::SendMessage(GetDlgItem(1207), BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
		DeleteBitmap(hBmp);

		if (selectedTheme->posChanged == -1)
		{
			// update wallpaper position 
			DESKTOP_WALLPAPER_POSITION pos;
			pDesktopWallpaper->GetPosition(&pos);
			ComboBox_SetCurSel(hPosCombobox, pos);
		}
	}
	// special case where preview wont update if (none) 
	// and background color changes due to theme change
	// just update each time u activate
	if (!firstInit && selectedIndex == 0)
	{
		HBITMAP bmp; 
		if (pWndPreview)
		{
			pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &bmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
			Static_SetBitmap(hBackPreview, bmp);
			DeleteBitmap(bmp);
		}
	}
	_TerminateProcess(pi);
	return 0;
}


#pragma region ListView helpers
int CBackgroundDlgProc::AddItem(HWND hListView, int rowIndex, LPCWSTR text)
{
	if (text)
	{
		// gimmick
		SHFILEINFO sh{};
		SHGetFileInfo(text, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
		ImageList_AddIcon(hml, sh.hIcon);
	}

	// why is winapi so ass
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvItem.iItem = rowIndex;
	lvItem.iSubItem = 0;
	lvItem.iImage = rowIndex;
	lvItem.pszText = (LPWSTR)PathFindFileName(text);
	lvItem.lParam = (LPARAM)StrDup(text);

	return ListView_InsertItem(hListView, &lvItem);
}

int CBackgroundDlgProc::AddColumn(HWND hListView, int width)
{
	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.cx = width;
	lvc.pszText = (LPWSTR)L"";
	lvc.iSubItem = 0;
	return ListView_InsertColumn(hListView, 0, &lvc);
}

LPWSTR CBackgroundDlgProc::GetWallpaperPath(HWND hListView, int iIndex)
{
	LVITEM item = { 0 };
	item.iItem = iIndex;
	item.iSubItem = 0;
	item.cchTextMax = 256;
	item.mask = LVIF_PARAM;
	ListView_GetItem(hListView, &item);
	return (LPWSTR)item.lParam;
}
#pragma endregion

BOOL CBackgroundDlgProc::ColorPicker(HWND hWnd, CHOOSECOLOR* clrOut)
{
	COLORREF clr = GetDeskopColor();

	static COLORREF acrCustClr[16];
	for (int i = 0; i < ARRAYSIZE(acrCustClr); i++)
	{
		acrCustClr[i] = RGB(255, 255, 255);
	}

	CHOOSECOLOR cc = { 0 };
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = acrCustClr;
	cc.rgbResult = clr;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;

	BOOL out = ChooseColor(&cc);
	*clrOut = cc;
	selectedTheme->useDesktopColor = true;
	return out;
}

void CBackgroundDlgProc::AddMissingWallpapers(IUnknown* th)
{
	hListView = GetDlgItem(1202);

	std::vector<LPWSTR> missingWall;
	// 1- enabled
	// 0- disabled
	int isEn = 0;
	ITheme* themeClass = new ITheme(th);
	themeClass->IsSlideshowEnabled(&isEn);

	if (isEn == 1)
	{
		ISlideshowSettings* st;
		themeClass->get_SlideshowSettings(&st);

		IWallpaperCollection* wlp;
		st->GetAllMatchingWallpapers(&wlp);

		int count = wlp->GetCount();
		for (int i = 0; i < count; i++)
		{
			LPWSTR path = { 0 };
			wlp->GetWallpaperAt(i, &path);
			missingWall.push_back(path);
		}
	}
	for (auto path : missingWall)
	{
		LVFINDINFO findInfo = { 0 };
		findInfo.flags = LVFI_STRING;
		findInfo.psz = PathFindFileName(path);
		int inde = ListView_FindItem(hListView, -1, &findInfo);
		int k = ListView_GetItemCount(hListView);
		if (inde == -1)
		{
			AddItem(hListView, k, path);
			k++;
		}
	}

	LVFINDINFO findInfo = { 0 };
	findInfo.flags = LVFI_STRING;
	findInfo.psz = PathFindFileName(selectedTheme->wallpaperPath);
	int inde = ListView_FindItem(hListView, -1, &findInfo);
	if (inde == -1 && selectedTheme->wallpaperPath != nullptr)
	{
		if (PathFileExists(selectedTheme->wallpaperPath))
		{
			AddItem(hListView, ListView_GetItemCount(hListView), selectedTheme->wallpaperPath);
		}
	}
}

void CBackgroundDlgProc::SelectCurrentWallpaper(IUnknown* th)
{
	int currThe;
	pThemeManager->GetCurrentTheme(&currThe);

	if (!selectedTheme->wallpaperPath)
	{
		WCHAR ws[MAX_PATH] = { 0 };
		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);

		selectedTheme->wallpaperType = WT_PICTURE;
		StringCpy(selectedTheme->wallpaperPath, ws);

		selectedTheme->useDesktopColor = true;
	}

	if (selectedTheme->wallpaperType == WT_PICTURE)
	{
		::EnableWindow(hPosCombobox, true);

		LVFINDINFO findInfo = { 0 };
		findInfo.flags = LVFI_STRING;
		findInfo.psz = PathFindFileName(selectedTheme->wallpaperPath);
		int inde = ListView_FindItem(hListView, -1, &findInfo);

		ListView_SetItemState(hListView, inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hListView, inde, FALSE);
	}
	else if (selectedTheme->wallpaperType == WT_NOWALL)
	{
		::EnableWindow(hPosCombobox, false);
		ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hListView, 0, FALSE);
	}

}
