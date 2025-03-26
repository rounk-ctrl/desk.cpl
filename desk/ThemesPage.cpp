#include "pch.h"
#include "ThemesPage.h"
#include "desk.h"
#include "uxtheme.h"
#include "helper.h"
#include "theme.h"
namespace fs = std::filesystem;

BOOL CThemeDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// global variables
	hCombobox = GetDlgItem(1101);
	hPreview = GetDlgItem(1103);
	size = GetClientSIZE(hPreview);

	// initialize theme manager
	InitUxtheme();

	HBITMAP bmp;
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

	// set the preview bitmap to the static control
	SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
	bmp = ThemePreviewBmp(GETSIZE(size), ws, NULL, GetSysColor(COLOR_BACKGROUND));
	Static_SetBitmap(hPreview, bmp);
	DeleteObject(bmp);

	// update THEMEINFO
	UpdateThemeInfo(ws, currThem);
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

	int hc;
	themeClass->GetHighContrast(&hc);

	COLORREF clr;
	themeClass->GetBackgroundColor(&clr);
	printf("%d, %d, %d", GetRValue(clr), GetGValue(clr), GetBValue(clr));

	// set the preview bitmap to the static control
	HBITMAP ebmp = ThemePreviewBmp(GETSIZE(size), ws, LoadThemeFromFilePath(path), clr);
	Static_SetBitmap(hPreview, ebmp);

	// update THEMEINFO
	UpdateThemeInfo(ws, index);

	SetModified(TRUE);
	DeleteObject(ebmp);
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
	pThemeManager->SetCurrentTheme(m_hWnd, index, !!TRUE, apply_flags, 0);

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
		HBITMAP ebmp = ThemePreviewBmp(GETSIZE(size), selectedTheme->wallpaperPath, LoadThemeFromFilePath(path), clr);
		Static_SetBitmap(hPreview, ebmp);

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
	// update THEMEINFO
	if (lstrlenW(ws) == 0 || PathFileExists(ws) == FALSE)
	{
		// no wallpaper applied
		selectedTheme->wallpaperType = WT_NOWALL;
		if (selectedTheme->wallpaperPath != nullptr)
		{
			delete[] selectedTheme->wallpaperPath;
		}
		selectedTheme->wallpaperPath = nullptr;
	}
	else
	{
		selectedTheme->wallpaperType = WT_PICTURE;
		size_t len = wcslen(ws) + 1;
		selectedTheme->wallpaperPath = new wchar_t[len];
		wcscpy_s(selectedTheme->wallpaperPath, len, ws);
	}
	// common properties
	selectedTheme->newColor = NULL;
	selectedTheme->customWallpaperSelection = false;
	selectedTheme->posChanged = -1;
	selectedTheme->useDesktopColor = false;
}

HBITMAP CThemeDlgProc::ThemePreviewBmp(int newwidth, int newheight, WCHAR* wallpaperPath, HANDLE hFile, COLORREF clrBg)
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
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeInvalid);

		HDC hdcgraphic = graphics.GetHDC();
		RECT clrrect = { 0,0, newwidth, newheight };
		if (selectedTheme->newColor)
			FillRect(hdcgraphic, &clrrect, CreateSolidBrush(selectedTheme->newColor));
		else
			FillRect(hdcgraphic, &clrrect, CreateSolidBrush(clrBg));
		graphics.ReleaseHDC(hdcgraphic);

		if (wallpaperPath && PathFileExists(wallpaperPath))
		{
			Gdiplus::Rect prevrect(0, 0, newwidth, newheight);
			DESKTOP_WALLPAPER_POSITION pos;
			if (selectedTheme->posChanged != -1)
			{
				pos = (DESKTOP_WALLPAPER_POSITION)selectedTheme->posChanged;
			}
			else
			{
				pDesktopWallpaper->GetPosition(&pos);
			}

			if (pos == DWPOS_CENTER)
			{
				graphics.SetClip(prevrect);

				int monitorwidth = GetSystemMetrics(SM_CXSCREEN);
				int monitorheight = GetSystemMetrics(SM_CYSCREEN);

				double sX = static_cast<double>(bitmap->GetWidth()) / monitorwidth;
				double sY = static_cast<double>(bitmap->GetHeight()) / monitorheight;

				int newprewidth = (int)sX * prevrect.Width;
				int newpreheight = (int)sY * prevrect.Height;
				prevrect.Width = newprewidth;
				prevrect.Height = newpreheight;

				int marX = ((newwidth)-newprewidth) / 2;
				int marY = ((newheight)-newpreheight) / 2;
				prevrect.X += marX;
				prevrect.Y += marY;
				graphics.DrawImage(bitmap, prevrect);
			}
			else if (pos == DWPOS_TILE)
			{
				graphics.SetClip(prevrect);

				int monitorwidth = GetSystemMetrics(SM_CXSCREEN);
				int monitorheight = GetSystemMetrics(SM_CYSCREEN);

				double sX = static_cast<double>(bitmap->GetWidth()) / monitorwidth;
				double sY = static_cast<double>(bitmap->GetHeight()) / monitorheight;

				int newprewidth = (int)sX * prevrect.Width;
				int newpreheight = (int)sY * prevrect.Height;
				prevrect.Width = newprewidth;
				prevrect.Height = newpreheight;

				int sideImages = (newwidth / newprewidth) + 1;
				int topImages = (newheight / newpreheight) + 1;

				for (int i = 0; i < topImages; i++)
				{
					for (int j = 0; j < sideImages; j++)
					{
						graphics.DrawImage(bitmap, prevrect);
						prevrect.X += prevrect.Width;
					}
					prevrect.X = 0;
					prevrect.Y += prevrect.Height;
				}
			}
			else
			{
				graphics.DrawImage(bitmap, prevrect);
			}
		}
		SHSTOCKICONINFO sii = { sizeof(sii) };
		SHGetStockIconInfo(SIID_RECYCLERFULL, SHGSI_ICON | SHGSI_SHELLICONSIZE, &sii);

		// bin
		hdcgraphic = graphics.GetHDC();
		if (sii.hIcon)
		{
			DrawIconEx(hdcgraphic, newwidth - 48, newheight - 40, sii.hIcon, 32, 32, 0, NULL, DI_NORMAL);
		}
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

		size.cx = (static_cast<double>(size.cx) / (size.cy + btnMar.cxRightWidth)) * GetSystemMetrics(SM_CYCAPTION);
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
		DeleteObject(fon);
		DeleteObject(font);
		CloseThemeData(hTheme);
		CloseThemeData(hTheme1);

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
