#include "ThemesPage.h"
#include "desk.h"
#include "uxtheme.h"
#include "theme.h"
namespace fs = std::filesystem;

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
		if (!wallpaperPath)
		{
			RECT rect = { 0,0, newwidth, newheight };
			if (newColor)
				FillRect(hdcgraphic, &rect, CreateSolidBrush(newColor));
			else
				FillRect(hdcgraphic, &rect, GetSysColorBrush(COLOR_BACKGROUND));

		}
		else if (!fs::exists(fs::path(wallpaperPath)))
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
		RECT rect = { x, y, width, y + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER) };
		DrawThemeBackground(hTheme, hdcgraphic, WP_CAPTION, FS_ACTIVE, &rect, NULL);

		// bottom frame
		MARGINS mar;
		rect.top = y + GetSystemMetrics(SM_CYCAPTION) + height + GetSystemMetrics(SM_CYFRAME);
		rect.bottom = rect.top + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
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
			hTheme1 = OpenThemeDataFromFile(hFile, NULL, L"Scrollbar", 0);
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
		if (btnMar.cxLeftWidth == 0) GetThemeMargins(hTheme, hdcgraphic, WP_CLOSEBUTTON, 0, TMT_SIZINGMARGINS, NULL, &btnMar);

		size.cx = (static_cast<double>(size.cx) / (size.cy + btnMar.cxRightWidth)) * GetSystemMetrics(SM_CYCAPTION) - 2;
		size.cy = GetSystemMetrics(SM_CYSIZE) - GetSystemMetrics(SM_CYEDGE) - GetSystemMetrics(SM_CYFRAME) + 2;
		rect.left = rect.right - size.cx - btnMar.cxLeftWidth;
		rect.top = y + GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) - 2 - size.cy + GetSystemMetrics(SM_CXPADDEDBORDER);
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
		hr = GetThemeColor(hTheme, WP_CAPTION, 0, TMT_TEXTCOLOR, &clr);
		NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		HFONT fon = CreateFontIndirect(&ncm.lfCaptionFont);
		SelectObject(hdcgraphic, fon);
		SetBkMode(hdcgraphic, TRANSPARENT);
		RECT rlc = { x + GetSystemMetrics(SM_CXPADDEDBORDER) + GetSystemMetrics(SM_CXFRAME) + mar.cxLeftWidth,
			rect.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER) , 500.0f, GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXPADDEDBORDER) };
		DrawThemeText(hTheme, hdcgraphic, 0, 0, L"Active Window", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, 0, &rlc);

		HFONT font = CreateFontIndirect(&ncm.lfMessageFont);
		SelectObject(hdcgraphic, font);
		oldrect1.bottom = oldrect1.top + 10;
		DrawThemeText(hTheme, hdcgraphic, 0, 0, L"Window Text", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, 0, &oldrect1);

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
		InitUxtheme();

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
		pThemeManager->GetTheme(currThem, &currentITheme);
		currentIThemeIndex = currThem;

		RECT rect;
		GetClientRect(GetDlgItem(hWnd, 1103), &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		HBITMAP bmp;

		WCHAR ws[MAX_PATH] = { 0 };
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

				currentITheme->Release();
				pThemeManager->GetTheme(index, &currentITheme);
				currentIThemeIndex = index;

				WCHAR* ws;

				if (g_osVersion.BuildNumber() >= 18362)
				{
					ITheme1903* th1903 = (ITheme1903*)currentITheme;
					th1903->get_Background(&ws);
				}
				else if (g_osVersion.BuildNumber() >= 17763)
				{
					ITheme1809* th1809 = (ITheme1809*)currentITheme;
					th1809->get_Background(&ws);
				}
				else
				{
					ITheme10* th10 = (ITheme10*)currentITheme;
					th10->get_Background(&ws);
				}
				if (!wallpath)
				{
					if (lstrlenW(ws) == 0)
						noWall = TRUE;
					else
						noWall = FALSE;
				}
				RECT rect;
				GetClientRect(GetDlgItem(hWnd, 1103), &rect);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				LPWSTR path = nullptr;
				if (g_osVersion.BuildNumber() >= 17763)
				{
					// this func is same for 1809 and 1903
					ITheme1809* th1809 = (ITheme1809*)currentITheme;
					th1809->get_VisualStyle(&path);
				}
				else
				{
					ITheme10* th10 = (ITheme10*)currentITheme;
					th10->get_VisualStyle(&path);
				}
				HBITMAP ebmp = ThemePreviewBmp(width, height, ws, LoadThemeFromFilePath(path));
				SendMessage(GetDlgItem(hWnd, 1103), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)ebmp);

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
			if (newColor)
				apply_flags |= THEMETOOL_APPLY_FLAG_IGNORE_COLOR;

			pThemeManager->SetCurrentTheme(hWnd, index, !!TRUE, apply_flags, 0);
			currentIThemeIndex = index;

			PropSheet_UnChanged(GetParent(hWnd), hWnd);
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}
		else if (pnmh->code == PSN_SETACTIVE)
		{
			bglock = TRUE;
			if (thlock)
			{
				thlock = FALSE;
				RECT rect;
				GetClientRect(GetDlgItem(hWnd, 1103), &rect);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				HBITMAP bmp;
				WCHAR ws[MAX_PATH] = { 0 };
				SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
				if (wallpath)
					bmp = ThemePreviewBmp(width, height, wallpath, NULL);
				else if (noWall)
					bmp = ThemePreviewBmp(width, height, NULL, NULL);
				else
					bmp = ThemePreviewBmp(width, height, ws, NULL);
				SendMessage(GetDlgItem(hWnd, 1103), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);
				DeleteObject(bmp);
			}
		}
	}
	return FALSE;
}
