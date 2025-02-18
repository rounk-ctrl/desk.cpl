#include "BackgroundPage.h"
#include "desk.h"
#include "helper.h"
namespace fs = std::filesystem;
HIMAGELIST hml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
int currentIndex = 0;

int AddItem(HWND hListView, int rowIndex, LPCSTR text)
{
	if (text)
	{
		SHFILEINFO sh;
		SHGetFileInfo(ConvertStr2(text), FILE_ATTRIBUTE_NORMAL, &sh, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON);
		ImageList_AddIcon(hml, sh.hIcon);
	}
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvItem.iItem = rowIndex;
	lvItem.iSubItem = 0;
	lvItem.iImage = rowIndex;
	lvItem.pszText = (LPWSTR)PathFindFileName(ConvertStr2(text));
	lvItem.lParam = (LPARAM)ConvertStr2(text);

	return ListView_InsertItem(hListView, &lvItem);
}

HBITMAP WallpaperAsBmp(int width, int height, WCHAR* path, HWND hWnd)
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
	if (newColor)
		colorref = newColor;
	else
		colorref = GetSysColor(COLOR_BACKGROUND);

	Gdiplus::Color clr(255, GetRValue(colorref), GetGValue(colorref), GetBValue(colorref));
	Gdiplus::SolidBrush* br = new Gdiplus::SolidBrush(clr);
	graphics.FillRectangle(br, 15, 25, width - 37, height - 68);

	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path, FALSE);
	if (bitmap)
	{
		int index = (int)SendMessage(GetDlgItem(hWnd, 1205), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
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
	delete resized;
	return hBitmap;
}

std::set<LPCSTR, NaturalComparator> wallpapers;
const COMDLG_FILTERSPEC file_types[] = {
	{L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)", L"*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png"},
};

LRESULT CALLBACK BackgroundDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int width{};
	int height{};
	if (uMsg == WM_INITDIALOG)
	{
		ListView_DeleteAllItems(GetDlgItem(hWnd, 1202));
		WCHAR ws[MAX_PATH] = { 0 };
		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
		RECT rect;
		GetClientRect(GetDlgItem(hWnd, 1200), &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		IUnknown* th;
		int currThem{};
		pThemeManager->GetCurrentTheme(&currThem);
		pThemeManager->GetTheme(currThem, &th);

		// 1- enabled
		// 0- disabled
		int isEn = 0;
		if (g_osVersion.BuildNumber() >= 18362)
		{
			ITheme1903* th1903 = (ITheme1903*)th;
			th1903->IsSlideshowEnabled(&isEn);
		}
		else if (g_osVersion.BuildNumber() >= 17763)
		{
			ITheme1809* th1809 = (ITheme1809*)th;
			th1809->IsSlideshowEnabled(&isEn);
		}
		else
		{
			ITheme10* th10 = (ITheme10*)th;
			th10->IsSlideshowEnabled(&isEn);
		}


		GetClientRect(GetDlgItem(hWnd, 1202), &rect);
		width = rect.right - rect.left - 30;
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvc.cx = width;
		lvc.pszText = (LPWSTR)L"";
		lvc.iSubItem = 0;
		SendMessage(GetDlgItem(hWnd, 1202), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);


		AddItem(GetDlgItem(hWnd, 1202), 0, "(none)");
		HICON barrierico = LoadIcon(LoadLibrary(L"imageres.dll"), MAKEINTRESOURCE(1027));
		ImageList_AddIcon(hml, barrierico);
		ListView_SetImageList(GetDlgItem(hWnd, 1202), hml, LVSIL_SMALL);
		DestroyIcon(barrierico);

		for (const auto& entry : fs::recursive_directory_iterator(L"C:\\Windows\\Web\\Wallpaper"))
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
			}
		}

		if (isEn == 1)
		{
			ISlideshowSettings* st;
			if (g_osVersion.BuildNumber() >= 18362)
			{
				ITheme1903* th1903 = (ITheme1903*)th;
				th1903->get_SlideshowSettings(&st);
			}
			else if (g_osVersion.BuildNumber() >= 17763)
			{
				ITheme1809* th1809 = (ITheme1809*)th;
				th1809->get_SlideshowSettings(&st);
			}
			else
			{
				ITheme10* th10 = (ITheme10*)th;
				th10->get_SlideshowSettings(&st);
			}
			IWallpaperCollection* wlp;
			st->GetAllMatchingWallpapers(&wlp);
			int count = wlp->GetCount();
			for (int i = 0; i < count; i++)
			{
				LPWSTR path = { 0 };
				wlp->GetWallpaperAt(i, &path);
				LPCSTR lpcstrPath = ConvertStr(path);
				wallpapers.insert(lpcstrPath);
			}
		}
		int k = 1;
		for (auto path : wallpapers)
		{
			AddItem(GetDlgItem(hWnd, 1202), k, path);
			k++;
		}
		const wchar_t* items[] = { L"Centre", L"Tile", L"Stretch",  L"Fit",  L"Fill",  L"Span" };
		for (int i = 0; i < _countof(items); i++)
		{
			SendMessage(GetDlgItem(hWnd, 1205), CB_ADDSTRING, 0, (LPARAM)items[i]);
		}
		SendMessage(GetDlgItem(hWnd, 1205), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
		if (lstrlenW(ws) == 0)
		{
			currentIndex = 0;
			noWall = TRUE;
			EnableWindow(GetDlgItem(hWnd, 1205), false);
			ListView_SetItemState(GetDlgItem(hWnd, 1202), 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, 1202), 0, FALSE);
		}
		else
		{
			DESKTOP_WALLPAPER_POSITION pos;

			if (g_osVersion.BuildNumber() >= 18362)
			{
				ITheme1903* th1903 = (ITheme1903*)th;
				th1903->get_BackgroundPosition(&pos);
			}
			else if (g_osVersion.BuildNumber() >= 17763)
			{
				ITheme1809* th1809 = (ITheme1809*)th;
				th1809->get_BackgroundPosition(&pos);
			}
			else
			{
				ITheme10* th10 = (ITheme10*)th;
				th10->get_BackgroundPosition(&pos);
			}

			SendMessage(GetDlgItem(hWnd, 1205), CB_SETCURSEL, (WPARAM)pos, (LPARAM)0);
			lastpos = pos;

			LVFINDINFO findInfo = { 0 };
			findInfo.flags = LVFI_STRING;
			findInfo.psz = PathFindFileName(DecodeTranscodedImage().c_str());
			int inde = ListView_FindItem(GetDlgItem(hWnd, 1202), -1, &findInfo);
			if (inde == -1)
			{
				if (fs::exists(fs::path(DecodeTranscodedImage().c_str())))
				{
					inde = AddItem(GetDlgItem(hWnd, 1202), k, ConvertStr(DecodeTranscodedImage().c_str()));
				}
			}
			currentIndex = inde;

			ListView_SetItemState(GetDlgItem(hWnd, 1202), inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, 1202), inde, FALSE);
		}

		th->Release();
		//DeleteObject(bmp);
	}
	else if (uMsg == WM_COMMAND)
	{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (LOWORD(wParam) == 1205)
			{
				int index = (int)SendMessage(GetDlgItem(hWnd, 1205), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				if (index != lastpos)
					PropSheet_Changed(GetParent(hWnd), hWnd);

				WCHAR buffer[256];
				RECT rect;
				GetClientRect(GetDlgItem(hWnd, 1200), &rect);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				LVITEM item = { 0 };
				item.iItem = currentIndex;
				item.iSubItem = 0;
				item.pszText = buffer;
				item.cchTextMax = 256;
				item.mask = LVIF_TEXT | LVIF_PARAM;
				ListView_GetItem(GetDlgItem(hWnd, 1202), &item);
				wallpath = (LPWSTR)item.lParam;

				HBITMAP bmp = WallpaperAsBmp(width, height, wallpath, hWnd);
				SendMessage(GetDlgItem(hWnd, 1200), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);
				DeleteObject(bmp);
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
						int inde = AddItem(GetDlgItem(hWnd, 1202), ListView_GetItemCount(GetDlgItem(hWnd, 1202)), ConvertStr(pszFilePath));

						ListView_SetItemState(GetDlgItem(hWnd, 1202), inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						ListView_EnsureVisible(GetDlgItem(hWnd, 1202), inde, FALSE);
						CoTaskMemFree(pszFilePath);
					}
				}
				pfd->Release();
			}
			else if (LOWORD(wParam) == 1207)
			{
				COLORREF clr;
				pDesktopWallpaper->GetBackgroundColor(&clr);
				CHOOSECOLOR cc;
				COLORREF acrCustClr[16];
				ZeroMemory(&cc, sizeof(cc));
				cc.lStructSize = sizeof(cc);
				cc.hwndOwner = hWnd;
				cc.lpCustColors = acrCustClr;
				cc.rgbResult = clr;
				cc.Flags = CC_RGBINIT | CC_FULLOPEN;
				if (ChooseColor(&cc) == TRUE)
				{
					newColor = cc.rgbResult;
					if (noWall)
					{
						RECT rect;
						GetClientRect(GetDlgItem(hWnd, 1200), &rect);
						width = rect.right - rect.left;
						height = rect.bottom - rect.top;
						HBITMAP bmp = WallpaperAsBmp(width, height, wallpath, hWnd);
						SendMessage(GetDlgItem(hWnd, 1200), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);
						DeleteObject(bmp);
					}

					PropSheet_Changed(GetParent(hWnd), hWnd);
				}

			}
		}
	}
	else if (uMsg == WM_NOTIFY)
	{
		LPNMHDR pnmhdr = (LPNMHDR)lParam;
		if (pnmhdr->idFrom == 1202 && pnmhdr->code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uNewState & LVIS_SELECTED)
			{
				WCHAR buffer[256];
				RECT rect;
				GetClientRect(GetDlgItem(hWnd, 1200), &rect);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				currentIndex = pnmv->iItem;

				LVITEM item = { 0 };
				item.iItem = pnmv->iItem;
				item.iSubItem = 0;
				item.pszText = buffer;
				item.cchTextMax = 256;
				item.mask = LVIF_TEXT | LVIF_PARAM;
				ListView_GetItem(GetDlgItem(hWnd, 1202), &item);
				wallpath = (LPWSTR)item.lParam;

				if (StrCmpW(wallpath, L"(none)") == 0)
				{
					noWall = TRUE;
					wallpath = nullptr;
					EnableWindow(GetDlgItem(hWnd, 1205), false);
				}
				else
				{
					noWall = FALSE;
					EnableWindow(GetDlgItem(hWnd, 1205), true);
				}

				HBITMAP bmp = WallpaperAsBmp(width, height, wallpath, hWnd);
				SendMessage(GetDlgItem(hWnd, 1200), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);

				if (firstSelect)
				{
					wallpath = nullptr;
					firstSelect = FALSE;
				}

				PropSheet_Changed(GetParent(hWnd), hWnd);
				DeleteObject(bmp);
			}
		}
		if (pnmhdr->code == PSN_APPLY)
		{
			int index = (int)SendMessage(GetDlgItem(hWnd, 1205), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

			if (index != lastpos)
			{
				pDesktopWallpaper->SetPosition((DESKTOP_WALLPAPER_POSITION)index);
				lastpos = index;
			}
			if (newColor)
			{
				pDesktopWallpaper->SetBackgroundColor(newColor);
				newColor = NULL;
			}
			if (wallpath)
			{
				pDesktopWallpaper->Enable(true);
				pDesktopWallpaper->SetWallpaper(NULL, wallpath);
				wallpath = nullptr;
			}
			if (noWall)
			{
				pDesktopWallpaper->Enable(false);
			}

			PropSheet_UnChanged(GetParent(hWnd), hWnd);
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}
		if (pnmhdr->code == PSN_SETACTIVE)
		{

		}
	}
	return FALSE;
}

