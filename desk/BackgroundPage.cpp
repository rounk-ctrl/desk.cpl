#include "BackgroundPage.h"
#include "desk.h"
#include "helper.h"
namespace fs = std::filesystem;
HIMAGELIST hml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
BOOL firstInit;
int selectedIndex;

HWND hListView;
HWND hBackPreview;
HWND hPosCombobox;
int backPreviewWidth{};
int backPreviewHeight{};

#pragma region ListView helpers
int AddItem(HWND hListView, int rowIndex, LPCSTR text)
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

int AddColumn(HWND hListView, int width)
{
	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.cx = width;
	lvc.pszText = (LPWSTR)L"";
	lvc.iSubItem = 0;
	return ListView_InsertColumn(hListView, 0, &lvc);
}

LPWSTR GetWallpaperPath(HWND hListView, int iIndex)
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

BOOL ColorPicker(HWND hWnd, CHOOSECOLOR* clrOut)
{
	COLORREF clr;
	ITheme* themeClass = new ITheme(currentITheme);
	themeClass->GetBackgroundColor(&clr);

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
	return out;
}

HBITMAP WallpaperAsBmp(int width, int height, WCHAR* path, HWND hWnd, COLORREF color)
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
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
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

		if (index == 0)
		{
			graphics.SetClip(prevrect);

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
		}
		graphics.DrawImage(bitmap, prevrect);
	}


	// create hbitmap
	HBITMAP hBitmap = NULL;
	resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

	delete bitmap;
	delete monitor;
	delete resized;
	return hBitmap;
}

void AddMissingWallpapers(IUnknown* th, HWND hWnd)
{
	hListView = GetDlgItem(hWnd, 1202);

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

void SelectCurrentWallpaper(IUnknown* th, HWND hWnd)
{
	int currThe;
	WCHAR ws[MAX_PATH] = { 0 };
	pThemeManager->GetCurrentTheme(&currThe);

	if (selectedTheme->wallpaperType == WT_PICTURE)
	{
		EnableWindow(hPosCombobox, true);

		LVFINDINFO findInfo = { 0 };
		findInfo.flags = LVFI_STRING;
		findInfo.psz = PathFindFileName(selectedTheme->wallpaperPath);
		int inde = ListView_FindItem(hListView, -1, &findInfo);

		ListView_SetItemState(hListView, inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hListView, inde, FALSE);
	}
	else if (selectedTheme->wallpaperType == WT_NOWALL)
	{
		EnableWindow(hPosCombobox, false);
		ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hListView, 0, FALSE);
	}

}

std::set<LPCSTR, NaturalComparator> wallpapers;
const COMDLG_FILTERSPEC file_types[] = {
	{L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)", L"*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png"},
};

LRESULT CALLBACK BackgroundDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		firstInit = TRUE;
		hListView = GetDlgItem(hWnd, 1202);
		hBackPreview = GetDlgItem(hWnd, 1200);
		hPosCombobox = GetDlgItem(hWnd, 1205);

		RECT rect;
		GetClientRect(hBackPreview, &rect);
		backPreviewWidth = rect.right - rect.left;
		backPreviewHeight = rect.bottom - rect.top;
		WCHAR ws[MAX_PATH] = { 0 };

		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);

		GetClientRect(hListView, &rect);
		AddColumn(hListView, rect.right - rect.left - 30);

		AddItem(hListView, 0, "(none)");
		HICON barrierico = LoadIcon(LoadLibrary(L"imageres.dll"), MAKEINTRESOURCE(1027));
		ImageList_AddIcon(hml, barrierico);
		ListView_SetImageList(GetDlgItem(hWnd, 1202), hml, LVSIL_SMALL);
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
		AddMissingWallpapers(thcr, hWnd);
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

		AddMissingWallpapers(currentITheme, hWnd);
		SelectCurrentWallpaper(currentITheme, hWnd);
		firstInit = FALSE;
	}
	else if (uMsg == WM_COMMAND)
	{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (LOWORD(wParam) == 1205)
			{
				selectedTheme->posChanged = true;

				COLORREF clr;
				ITheme* themeClass = new ITheme(currentITheme);
				themeClass->GetBackgroundColor(&clr);

				HBITMAP bmp = WallpaperAsBmp(backPreviewWidth, backPreviewHeight, selectedTheme->wallpaperPath, hWnd, clr);
				Static_SetBitmap(hBackPreview, bmp);
				DeleteObject(bmp);

				PropSheet_Changed(GetParent(hWnd), hWnd);
			}
		}
		else if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == 1203)
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
			}
			else if (LOWORD(wParam) == 1207)
			{
				CHOOSECOLOR cc;
				if (ColorPicker(hWnd, &cc) == TRUE)
				{
					selectedTheme->newColor = cc.rgbResult;

					COLORREF clr;
					ITheme* themeClass = new ITheme(currentITheme);
					themeClass->GetBackgroundColor(&clr);

					HBITMAP bmp = WallpaperAsBmp(backPreviewWidth, backPreviewHeight, selectedTheme->wallpaperPath, hWnd, clr);
					Static_SetBitmap(hBackPreview, bmp);
					DeleteObject(bmp);

					PropSheet_Changed(GetParent(hWnd), hWnd);
				}

			}
		}
	}
	else if (uMsg == WM_NOTIFY)
	{
		PSHNOTIFY* nhdr = (PSHNOTIFY*)lParam;
		if (nhdr->hdr.idFrom == 1202 && nhdr->hdr.code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uNewState & LVIS_SELECTED)
			{
				selectedIndex = pnmv->iItem;
				LPWSTR path = GetWallpaperPath(hListView, pnmv->iItem);

				if (StrCmpW(path, L"(none)") == 0)
				{
					EnableWindow(hPosCombobox, false);

					if (selectedTheme->wallpaperPath != nullptr)
					{
						delete[] selectedTheme->wallpaperPath;
					}

					selectedTheme->wallpaperPath = nullptr;
					selectedTheme->wallpaperType = WT_NOWALL;

				}
				else
				{
					EnableWindow(hPosCombobox, true);

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

				COLORREF clr;
				ITheme* themeClass = new ITheme(currentITheme);
				themeClass->GetBackgroundColor(&clr);

				HBITMAP bmp = WallpaperAsBmp(backPreviewWidth, backPreviewHeight, selectedTheme->wallpaperPath, hWnd,clr);
				Static_SetBitmap(hBackPreview, bmp);

				PropSheet_Changed(GetParent(hWnd), hWnd);
				DeleteObject(bmp);
			}
		}
		else if (nhdr->hdr.code == PSN_APPLY)
		{
			if (selectedTheme->posChanged)
			{
				int index = ComboBox_GetCurSel(hPosCombobox);
				pDesktopWallpaper->SetPosition((DESKTOP_WALLPAPER_POSITION)index);
				selectedTheme->posChanged = false;
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

			PropSheet_UnChanged(GetParent(hWnd), hWnd);
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}
		else if (nhdr->hdr.code == PSN_SETACTIVE)
		{
			selectionPicker = false;
			if (!selectedTheme->customWallpaperSelection && !firstInit)
			{
				AddMissingWallpapers(currentITheme, hWnd);
				SelectCurrentWallpaper(currentITheme, hWnd);
			}
			// special case where preview wont update if (none) 
			// and background color changes due to theme change
			// just update each time u activate
			if (!firstInit && selectedIndex == 0)
			{
				COLORREF clr;
				ITheme* themeClass = new ITheme(currentITheme);
				themeClass->GetBackgroundColor(&clr);

				HBITMAP bmp = WallpaperAsBmp(backPreviewWidth, backPreviewHeight, NULL, hWnd, clr);
				Static_SetBitmap(hBackPreview, bmp);
			}
			if (pi.hProcess != nullptr)
			{
				TerminateProcess(pi.hProcess, 0);
				CloseHandle(pi.hThread);
				CloseHandle(pi.hProcess);
				pi.hProcess = nullptr;
				pi.hThread = nullptr;
			}
		}
	}
	else if (uMsg == WM_SETTINGCHANGE)
	{
		printf("WM_SETTINGCHANGED:\n");
		AddMissingWallpapers(currentITheme, hWnd);
		SelectCurrentWallpaper(currentITheme, hWnd);
	}
	return FALSE;
}
