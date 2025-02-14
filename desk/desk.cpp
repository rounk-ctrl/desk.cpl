#include "desk.h"
#include "pch.h"
#include "theme.h"
#include "uxtheme.h"
#include "helper.h"
#include "version.h"

namespace fs = std::filesystem;

HINSTANCE g_hinst;
IThemeManager2* pThemeManager = NULL;
IDesktopWallpaper* pDesktopWallpaper = NULL;
ULONG_PTR gdiplusToken;
LPWSTR wallpath{};
int lastpos{};

const IID IID_IThemeManager2 = { 0xc1e8c83e, 0x845d, 0x4d95, {0x81, 0xdb, 0xe2, 0x83, 0xfd, 0xff, 0xc0, 0x00} };

HBITMAP WallpaperAsBmp(int width, int height, WCHAR* path)
{
	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path, FALSE);
	if (bitmap)
	{
		Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
		if (!resized)
		{
			delete bitmap;
			return NULL;
		}

		Gdiplus::Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(IDB_BITMAP1));

		Gdiplus::Color transparentColor(255, 255, 0, 255);

		Gdiplus::ImageAttributes imgAttr;
		imgAttr.SetColorKey(transparentColor, transparentColor, Gdiplus::ColorAdjustTypeBitmap);


		Gdiplus::Graphics graphics(resized);
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		Gdiplus::Rect rect(0, 10, monitor->GetWidth(), monitor->GetHeight());
		graphics.DrawImage(monitor, rect,0,0, width, height, Gdiplus::UnitPixel, &imgAttr);
		graphics.DrawImage(bitmap, 15, 25, width-37, height-68);

		// create hbitmap
		HBITMAP hBitmap = NULL;
		resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);
		
		delete bitmap;
		delete resized;
		return hBitmap;
	}
	return NULL;
}

HBITMAP ThemePreviewBmp(int newwidth, int newheight, WCHAR* wallpaperPath, HANDLE hFile)
{
	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(wallpaperPath, FALSE);
	if (bitmap)
	{
		Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(newwidth, newheight, PixelFormat32bppARGB);
		if (!resized)
		{
			delete bitmap;
			return NULL;
		}

		Gdiplus::Graphics graphics(resized);
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.DrawImage(bitmap, 0, 0, newwidth, newheight);

		HDC hdcgraphic = graphics.GetHDC();
		if (!fs::exists(fs::path(wallpaperPath)))
		{
			RECT rect = { 0,0, newwidth, newheight };
			FillRect(hdcgraphic, &rect, GetSysColorBrush(COLOR_BACKGROUND));
		}

		HMODULE imageres = LoadLibrary(L"imageres.dll");
		HICON ico = (HICON)LoadImage(imageres, MAKEINTRESOURCE(54), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

		// bin
		DrawIconEx(hdcgraphic, newwidth - 48, newheight - 40, ico, 32, 32, 0, NULL, DI_NORMAL);
		graphics.ReleaseHDC(hdcgraphic);

		// draw window
		int x = 30;
		int y = 30;
		int width = 260;
		int height = 100;
		hdcgraphic = graphics.GetHDC();
		HTHEME hTheme;
		if (hFile)
			hTheme = OpenThemeDataFromFile(hFile, NULL, L"Window", 0);
		else
			hTheme = OpenThemeData(NULL, L"Window");

		// caption
		RECT rect = { x, y, width, y + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER)};
		DrawThemeBackground(hTheme, hdcgraphic, WP_CAPTION, FS_ACTIVE, &rect, NULL);

		// bottom frame
		MARGINS mar;
		rect.top = y + GetSystemMetrics(SM_CYCAPTION) + height + GetSystemMetrics(SM_CYFRAME);
		rect.bottom = rect.top + GetSystemMetrics(SM_CYFRAME)+ GetSystemMetrics(SM_CXPADDEDBORDER);
		DrawThemeBackground(hTheme, hdcgraphic, WP_FRAMEBOTTOM, 1, &rect, NULL);

		// left frame
		rect.left = x;
		rect.top = y + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		rect.right = x + GetSystemMetrics(SM_CXPADDEDBORDER) + GetSystemMetrics(SM_CXFRAME);
		rect.bottom = y + GetSystemMetrics(SM_CYCAPTION) + height + GetSystemMetrics(SM_CYFRAME);
		DrawThemeBackground(hTheme, hdcgraphic, WP_FRAMELEFT, FS_ACTIVE, &rect, NULL);

		// right frame
		rect.left = width - GetSystemMetrics(SM_CXPADDEDBORDER) - GetSystemMetrics(SM_CXFRAME);
		rect.right = width;
		DrawThemeBackground(hTheme, hdcgraphic, WP_FRAMERIGHT, FS_ACTIVE, &rect, NULL);

		// window color
		rect.left = x + GetSystemMetrics(SM_CXPADDEDBORDER) + GetSystemMetrics(SM_CXFRAME);
		rect.right = width - GetSystemMetrics(SM_CXPADDEDBORDER) - GetSystemMetrics(SM_CXFRAME);
		rect.top += GetSystemMetrics(SM_CXPADDEDBORDER);
		RECT oldrect1 = rect;
		COLORREF bgColor;
		HRESULT hr = GetThemeColor(hTheme, 0, 0, TMT_FILLCOLOR, &bgColor);
		if (SUCCEEDED(hr)) 
			FillRect(hdcgraphic, &rect, CreateSolidBrush(bgColor));
		else
			FillRect(hdcgraphic, &rect, GetSysColorBrush(COLOR_WINDOW));

		//scrollba
		SIZE size = { 0 };
		HTHEME hTheme1;
		if (hFile)
			hTheme1 = OpenThemeDataFromFile(hFile,NULL, L"Scrollbar", 0);
		else
			hTheme1 = OpenThemeData(NULL, L"Scrollbar");

		GetThemePartSize(hTheme1, hdcgraphic, SBP_THUMBBTNVERT, SCRBS_NORMAL, NULL, TS_TRUE, &size);
		rect.left = width - GetSystemMetrics(SM_CXPADDEDBORDER) - GetSystemMetrics(SM_CXFRAME) - size.cx;
		rect.right = rect.left + size.cx;
		DrawThemeBackground(hTheme1, hdcgraphic, SBP_LOWERTRACKHORZ, SCRBS_NORMAL, &rect, 0);
		int oldbot = rect.bottom;
		GetThemePartSize(hTheme1, hdcgraphic, SBP_ARROWBTN, ABS_UPNORMAL, NULL, TS_TRUE, &size);
		rect.bottom = rect.top + size.cy;
		DrawThemeBackground(hTheme1, hdcgraphic, SBP_ARROWBTN, ABS_UPNORMAL, &rect, 0);
		rect.top += size.cy;
		rect.bottom = rect.top + size.cy;
		DrawThemeBackground(hTheme1, hdcgraphic, SBP_THUMBBTNVERT, SCRBS_NORMAL, &rect, 0);
		rect.top = oldbot - size.cy;
		rect.bottom = oldbot;
		DrawThemeBackground(hTheme1, hdcgraphic, SBP_ARROWBTN, ABS_DOWNNORMAL, &rect, 0);

		// caption button
		GetThemePartSize(hTheme, hdcgraphic, WP_CLOSEBUTTON, CBS_NORMAL, NULL, TS_TRUE, &size);
		// use contentmargin if exists
		MARGINS btnMar;
		GetThemeMargins(hTheme, hdcgraphic, WP_CLOSEBUTTON, 0, TMT_CONTENTMARGINS, NULL, &btnMar);
		if (btnMar.cxLeftWidth==0) GetThemeMargins(hTheme, hdcgraphic, WP_CLOSEBUTTON, 0, TMT_SIZINGMARGINS, NULL, &btnMar);

		size.cx = (static_cast<double>(size.cx) / (size.cy+btnMar.cxRightWidth)) * GetSystemMetrics(SM_CYCAPTION) -2;
		size.cy = GetSystemMetrics(SM_CYSIZE) - GetSystemMetrics(SM_CYEDGE) - GetSystemMetrics(SM_CYFRAME) + 2;
		rect.left = rect.right - size.cx - btnMar.cxLeftWidth;
		rect.top = y + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME)- 2 - size.cy +GetSystemMetrics(SM_CXPADDEDBORDER);
		rect.bottom = rect.top + size.cy;
		rect.right -= btnMar.cxLeftWidth;
		DrawThemeBackground(hTheme, hdcgraphic, WP_CLOSEBUTTON, CBS_NORMAL, &rect, NULL);
		rect.left -= size.cx + btnMar.cxLeftWidth;
		rect.right -= size.cx + btnMar.cxLeftWidth;
		DrawThemeBackground(hTheme, hdcgraphic, WP_MAXBUTTON, MAXBS_NORMAL, &rect, NULL);
		rect.left -= size.cx + btnMar.cxLeftWidth;
		rect.right -= size.cx + btnMar.cxLeftWidth;
		DrawThemeBackground(hTheme, hdcgraphic, WP_MINBUTTON, MINBS_NORMAL, &rect, NULL);

		// title
		GetThemeMargins(hTheme, hdcgraphic, WP_CAPTION, 0, TMT_CAPTIONMARGINS, NULL, &mar);
		COLORREF clr;
		hr =GetThemeColor(hTheme, WP_CAPTION, 0, TMT_TEXTCOLOR, &clr);
		NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		HFONT fon = CreateFontIndirect(&ncm.lfCaptionFont);
		SelectObject(hdcgraphic, fon);
		SetBkMode(hdcgraphic, TRANSPARENT);
		RECT rlc = { x + GetSystemMetrics(SM_CXPADDEDBORDER) + GetSystemMetrics(SM_CXFRAME) + mar.cxLeftWidth, 
			rect.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) +GetSystemMetrics(SM_CXPADDEDBORDER) , 500.0f, GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER) };
		DrawThemeText(hTheme, hdcgraphic,0,0, L"Active Window", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, 0, &rlc);

		HFONT font = CreateFontIndirect(&ncm.lfMessageFont);
		SelectObject(hdcgraphic, font);
		oldrect1.bottom = oldrect1.top + 10;
		DrawThemeText(hTheme, hdcgraphic,0,0, L"Window Text", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, 0, &oldrect1);

		graphics.ReleaseHDC(hdcgraphic);

		// create hbitmap
		HBITMAP hBitmap = NULL;
		resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

		// cleanup
		delete bitmap;
		delete resized;
		DestroyIcon(ico);
		DeleteObject(fon);
		DeleteObject(font);
		CloseThemeData(hTheme);
		CloseThemeData(hTheme1);
		FreeLibrary(imageres);

		// bless
		if (hFile)
		{
			UXTHEMEFILE* ltf = (UXTHEMEFILE*)hFile;
			if (ltf->sharableSectionView) UnmapViewOfFile(ltf->sharableSectionView);
			if (ltf->nsSectionView) UnmapViewOfFile(ltf->nsSectionView);
			if (ltf->hSharableSection) CloseHandle(ltf->hSharableSection);
			if (ltf->hNsSection) CloseHandle(ltf->hNsSection);
			free(ltf);
			//CloseHandle(hFile);
		}
		return hBitmap;
	}
	return NULL;
}

LRESULT CALLBACK ThemeDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int width{};
	int height{};
	if (uMsg == WM_INITDIALOG)
	{
		//std::vector<LPWSTR> themes;
		int count{};
		pThemeManager->GetThemeCount(&count);
		for (auto i = 0; i < count; ++i)
		{
			// same across all w10
			IUnknown* the;
			pThemeManager->GetTheme(i, &the);
			ITheme10* them = (ITheme10*)the;
			LPWSTR str = nullptr;
			them->get_DisplayName(&str);
			SendMessage(GetDlgItem(hWnd, 1101), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)str);
			them->Release();
		}
		int currThem{};
		pThemeManager->GetCurrentTheme(&currThem);
		SendMessage(GetDlgItem(hWnd, 1101), CB_SETCURSEL, (WPARAM)currThem, (LPARAM)0);

		RECT rect;
		GetClientRect(GetDlgItem(hWnd, 1103), &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		HBITMAP bmp;
		
		WCHAR ws[MAX_PATH] = {0};
		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
		bmp = ThemePreviewBmp(width, height, ws, NULL);
		SendMessage(GetDlgItem(hWnd, 1103), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);
		DeleteObject(bmp);
	}
	else if (uMsg == WM_COMMAND)
	{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (LOWORD(wParam) == 1101)
			{
				HWND hcombo = (HWND)lParam;
				int currThem{};
				pThemeManager->GetCurrentTheme(&currThem);
				int index = (int)SendMessage(hcombo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

				if (index != currThem)
					PropSheet_Changed(GetParent(hWnd), hWnd);
				else
					PropSheet_UnChanged(GetParent(hWnd), hWnd);

				IUnknown* th;
				pThemeManager->GetTheme(index, &th);
				WCHAR* ws;
				
				if (g_osVersion.BuildNumber() >= 18362)
				{
					ITheme1903* th1903 = (ITheme1903*)th;
					th1903->get_Background(&ws);
				}	
				else if (g_osVersion.BuildNumber() >= 17763)
				{
					ITheme1809* th1809 = (ITheme1809*)th;
					th1809->get_Background(&ws);
				}
				else
				{
					ITheme10* th10 = (ITheme10*)th;
					th10->get_Background(&ws);
				}

				RECT rect;
				GetClientRect(GetDlgItem(hWnd, 1103), &rect);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				LPWSTR path = nullptr;
				if (g_osVersion.BuildNumber() >= 17763)
				{
					// this func is same for 1809 and 1903
					ITheme1809* th1809 = (ITheme1809*)th;
					th1809->get_VisualStyle(&path);
				}
				else
				{
					ITheme10* th10 = (ITheme10*)th;
					th10->get_VisualStyle(&path);
				}
				HBITMAP ebmp = ThemePreviewBmp(width, height, ws, LoadThemeFromFilePath(path));
				SendMessage(GetDlgItem(hWnd, 1103), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)ebmp);
				th->Release();
				SysFreeString(ws);
				DeleteObject(ebmp);
			}
		}
	}
	else if (uMsg == WM_NOTIFY)
	{
		NMHDR* pnmh = (NMHDR*)lParam;
		if (pnmh->code == PSN_APPLY)
		{
			ULONG apply_flags = 0;
			int index = (int)SendMessage(GetDlgItem(hWnd, 1101), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			if (wallpath)
				apply_flags |= THEMETOOL_APPLY_FLAG_IGNORE_BACKGROUND;
			pThemeManager->SetCurrentTheme(hWnd, index, !!TRUE, apply_flags, 0);

			PropSheet_UnChanged(GetParent(hWnd), hWnd);
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}
	}
	return FALSE;
}

std::set<LPCSTR, NaturalComparator> wallpapers;

LRESULT CALLBACK BackgroundDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int width{};
	int height{};
	if (uMsg == WM_INITDIALOG)
	{
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
		width = rect.right - rect.left-30;
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvc.cx = width;
		lvc.pszText = (LPWSTR)L"";
		lvc.iSubItem = 0;
		SendMessage(GetDlgItem(hWnd, 1202), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);


		for (const auto& entry : fs::recursive_directory_iterator(L"C:\\Windows\\Web\\Wallpaper"))
		{
			if (entry.is_regular_file() && (entry.path().extension() == L".jpg" || entry.path().extension() == L".png"))
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
		int k = 0;
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

		ListView_SetItemState(GetDlgItem(hWnd, 1202), inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(GetDlgItem(hWnd, 1202), inde, FALSE);

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

				LVITEM item = { 0 };
				item.iItem = pnmv->iItem;
				item.iSubItem = 0;
				item.pszText = buffer;
				item.cchTextMax = 256;
				item.mask = LVIF_TEXT | LVIF_PARAM;
				ListView_GetItem(GetDlgItem(hWnd, 1202), &item);
				wallpath = (LPWSTR)item.lParam;

				HBITMAP bmp = WallpaperAsBmp(width, height, wallpath);
				SendMessage(GetDlgItem(hWnd, 1200), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);

				PropSheet_Changed(GetParent(hWnd), hWnd);
				DeleteObject(bmp);
			}
		}
		if (pnmhdr->code == PSN_APPLY)
		{
			int index = (int)SendMessage(GetDlgItem(hWnd, 1205), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

			pDesktopWallpaper->SetPosition((DESKTOP_WALLPAPER_POSITION)index);
			lastpos = index;

			if (wallpath)
			{
				pDesktopWallpaper->SetWallpaper(NULL, wallpath);
				wallpath = nullptr;
			}

			PropSheet_UnChanged(GetParent(hWnd), hWnd);
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}
	}
	return FALSE;
}


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

	InitUxtheme();

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