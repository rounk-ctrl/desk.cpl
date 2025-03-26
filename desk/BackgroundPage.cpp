﻿#include "pch.h"
#include "BackgroundPage.h"
#include "desk.h"
#include "helper.h"
namespace fs = std::filesystem;

std::set<LPCSTR, NaturalComparator> wallpapers;
const COMDLG_FILTERSPEC file_types[] = {
	{L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)", L"*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png"},
};

BOOL CBackgroundDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	firstInit = TRUE;
	hListView = GetDlgItem(1202);
	hBackPreview = GetDlgItem(1200);
	hPosCombobox = GetDlgItem(1205);

	backPreviewSize = GetClientSIZE(hBackPreview);

	RECT rect;
	::GetClientRect(hListView, &rect);
	AddColumn(hListView, rect.right - rect.left - 30);

	AddItem(hListView, 0, "(none)");
	HICON barrierico = LoadIcon(LoadLibrary(L"imageres.dll"), MAKEINTRESOURCE(1027));
	ImageList_AddIcon(hml, barrierico);
	ListView_SetImageList(hListView, hml, LVSIL_SMALL);
	DestroyIcon(barrierico);

	WCHAR wallpaperdir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Web\\Wallpaper", wallpaperdir, MAX_PATH);
	for (const auto& entry : fs::recursive_directory_iterator(wallpaperdir))
	{
		if (entry.is_regular_file() && (entry.path().extension() == L".jpg"
			|| entry.path().extension() == L".png"
			|| entry.path().extension() == L".bmp"
			|| entry.path().extension() == L".jpeg"
			|| entry.path().extension() == L".dib"
			|| entry.path().extension() == L".gif"))
		{
			LPWSTR lpwstrPath = _wcsdup(entry.path().c_str());
			LPCSTR path = ConvertStr(lpwstrPath);
			wallpapers.insert(path);
			free(lpwstrPath);
		}
	}

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
	const wchar_t* items[] = { L"Centre", L"Tile", L"Stretch",  L"Fit",  L"Fill",  L"Span" };
	for (int i = 0; i < _countof(items); i++)
	{
		ComboBox_AddString(hPosCombobox, items[i]);
	}

	DESKTOP_WALLPAPER_POSITION pos;
	pDesktopWallpaper->GetPosition(&pos);
	ComboBox_SetCurSel(hPosCombobox, pos);

	AddMissingWallpapers(currentITheme);
	SelectCurrentWallpaper(currentITheme);
	firstInit = FALSE;
	return 0;
}

BOOL CBackgroundDlgProc::OnBgSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	selectedTheme->posChanged = ComboBox_GetCurSel(hPosCombobox);

	COLORREF clr;
	pDesktopWallpaper->GetBackgroundColor(&clr);

	HBITMAP bmp = WallpaperAsBmp(GETSIZE(backPreviewSize), selectedTheme->wallpaperPath, hWnd, clr);
	Static_SetBitmap(hBackPreview, bmp);
	DeleteObject(bmp);

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
			int inde = AddItem(hListView, ListView_GetItemCount(hListView), ConvertStr(pszFilePath));

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
	CHOOSECOLOR cc;
	if (ColorPicker(hWnd, &cc) == TRUE)
	{
		selectedTheme->newColor = cc.rgbResult;

		COLORREF clr;
		ITheme* themeClass = new ITheme(currentITheme);
		themeClass->GetBackgroundColor(&clr);

		HBITMAP bmp = WallpaperAsBmp(GETSIZE(backPreviewSize), selectedTheme->wallpaperPath, hWnd, clr);
		Static_SetBitmap(hBackPreview, bmp);
		DeleteObject(bmp);

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

		if (StrCmpW(path, L"(none)") == 0)
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
		HBITMAP bmp = WallpaperAsBmp(GETSIZE(backPreviewSize), selectedTheme->wallpaperPath, m_hWnd, clrlv);
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
		COLORREF clr;
		if (selectedTheme->useDesktopColor)
		{
			pDesktopWallpaper->GetBackgroundColor(&clr);
		}
		else
		{
			ITheme* themeClass = new ITheme(currentITheme);
			themeClass->GetBackgroundColor(&clr);
		}

		HBITMAP bmp = WallpaperAsBmp(GETSIZE(backPreviewSize), NULL, m_hWnd, clr);
		Static_SetBitmap(hBackPreview, bmp);
	}
	_TerminateProcess(pi);
	return 0;
}


#pragma region ListView helpers
int CBackgroundDlgProc::AddItem(HWND hListView, int rowIndex, LPCSTR text)
{
	if (text)
	{
		// gimmick
		SHFILEINFO sh{};
		SHGetFileInfo(ConvertStr2(text), FILE_ATTRIBUTE_NORMAL, &sh, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
		ImageList_AddIcon(hml, sh.hIcon);
	}

	// why is winapi so ass
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvItem.iItem = rowIndex;
	lvItem.iSubItem = 0;
	lvItem.iImage = rowIndex;
	lvItem.pszText = (LPWSTR)PathFindFileName(ConvertStr2(text));
	lvItem.lParam = (LPARAM)ConvertStr2(text);

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
	COLORREF clr;
	if (selectedTheme->useDesktopColor)
	{
		pDesktopWallpaper->GetBackgroundColor(&clr);
	}
	else
	{
		ITheme* themeClass = new ITheme(currentITheme);
		themeClass->GetBackgroundColor(&clr);
	}

	CHOOSECOLOR cc;
	COLORREF acrCustClr[16];
	ZeroMemory(&cc, sizeof(cc));
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

HBITMAP CBackgroundDlgProc::WallpaperAsBmp(int width, int height, WCHAR* path, HWND hWnd, COLORREF color)
{
	Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (!resized)
	{
		return NULL;
	}

	Gdiplus::Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(IDB_BITMAP1));

	int monitorwidth = GetSystemMetrics(SM_CXSCREEN);
	int monitorheight = GetSystemMetrics(SM_CYSCREEN);

	// pink
	Gdiplus::Color transparentColor(255, 255, 0, 255);

	Gdiplus::ImageAttributes imgAttr;
	imgAttr.SetColorKey(transparentColor, transparentColor, Gdiplus::ColorAdjustTypeBitmap);

	Gdiplus::Graphics graphics(resized);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeInvalid);
	Gdiplus::Rect rect(0, 10, monitor->GetWidth(), monitor->GetHeight());
	// draw monitor
	graphics.DrawImage(monitor, rect, 0, 0, width, height, Gdiplus::UnitPixel, &imgAttr);

	COLORREF colorref;
	if (selectedTheme->newColor)
		colorref = selectedTheme->newColor;
	else
		colorref = color;

	Gdiplus::Color clr(255, GetRValue(colorref), GetGValue(colorref), GetBValue(colorref));
	Gdiplus::SolidBrush* br = new Gdiplus::SolidBrush(clr);
	graphics.FillRectangle(br, 15, 25, width - 37, height - 68);

	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path, FALSE);
	if (bitmap)
	{
		int index = ComboBox_GetCurSel(hPosCombobox);
		Gdiplus::Rect prevrect(15, 25, width - 37, height - 68);
		graphics.SetClip(prevrect);


		if (index == DWPOS_CENTER)
		{
			double sX = static_cast<double>(bitmap->GetWidth()) / monitorwidth;
			double sY = static_cast<double>(bitmap->GetHeight()) / monitorheight;

			int newwidth = sX * prevrect.Width;
			int newheight = sY * prevrect.Height;
			prevrect.Width = newwidth;
			prevrect.Height = newheight;

			int marX = ((width - 37) - newwidth) / 2;
			int marY = ((height - 68) - newheight) / 2;
			prevrect.X += marX;
			prevrect.Y += marY;
			graphics.DrawImage(bitmap, prevrect);
		}
		else if (index == DWPOS_TILE)
		{
			double sX = static_cast<double>(bitmap->GetWidth()) / monitorwidth;
			double sY = static_cast<double>(bitmap->GetHeight()) / monitorheight;

			int newprewidth = sX * prevrect.Width;
			int newpreheight = sY * prevrect.Height;
			prevrect.Width = newprewidth;
			prevrect.Height = newpreheight;

			int sideImages = ((width - 37) / newprewidth) + 1;
			int topImages = ((height - 68) / newpreheight) + 1;

			for (int i = 0; i < topImages; i++)
			{
				for (int j = 0; j < sideImages; j++)
				{
					graphics.DrawImage(bitmap, prevrect);
					prevrect.X += prevrect.Width;
				}
				prevrect.X = 15;
				prevrect.Y += prevrect.Height;
			}
		}
		else
		{
			graphics.DrawImage(bitmap, prevrect);
		}
	}


	// create hbitmap
	HBITMAP hBitmap = NULL;
	resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

	delete bitmap;
	delete monitor;
	delete resized;
	return hBitmap;
}

void CBackgroundDlgProc::AddMissingWallpapers(IUnknown* th)
{
	hListView = GetDlgItem(1202);

	std::set<LPCSTR, NaturalComparator> missingWall;
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
			LPCSTR lpcstrPath = ConvertStr(path);
			missingWall.insert(lpcstrPath);
		}
	}
	for (auto path : missingWall)
	{
		LVFINDINFO findInfo = { 0 };
		findInfo.flags = LVFI_STRING;
		findInfo.psz = PathFindFileName(ConvertStr2(path));
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
		if (fs::exists(fs::path(selectedTheme->wallpaperPath)))
		{
			AddItem(hListView, ListView_GetItemCount(hListView), ConvertStr(selectedTheme->wallpaperPath));
		}
	}
}

void CBackgroundDlgProc::SelectCurrentWallpaper(IUnknown* th)
{
	int currThe;
	WCHAR ws[MAX_PATH] = { 0 };
	pThemeManager->GetCurrentTheme(&currThe);

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
