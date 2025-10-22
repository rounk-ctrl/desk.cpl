﻿#include "pch.h"
#include "wndprvw.h"
#include "helper.h"
#include "uxtheme.h"

#ifndef _DEBUG
#undef RETURN_IF_FAILED
#undef RETURN_IF_NULL_ALLOC
#undef LOG_IF_FAILED

#define RETURN_IF_FAILED
#define RETURN_IF_NULL_ALLOC
#define LOG_IF_FAILED
#endif

using namespace Gdiplus;

CWindowPreview::CWindowPreview(SIZE const& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType, LPVOID hTheme, int dpi)
{
	_pwndInfo = pwndInfo;
	_wndInfoCount = wndInfoCount;
	_sizePreview = sizePreview;
	_pageType = pageType;
	_hTheme = hTheme;
	_fIsThemed = !IsClassicThemeEnabled();
	_dpiWindow = dpi;

	// always initialize variables
	_marFrame = {};
	_marMonitor = {};
	_szMenuBar = { 0, GetSystemMetrics(SM_CYMENU) };
	_bmpBin = nullptr;
	_bmpSolidColor = nullptr;
	_bmpWallpaper = nullptr;
	_bmpMonitor = nullptr;
	_bmpWindows = (Bitmap**)malloc(_wndInfoCount * sizeof(Bitmap*));
	for (int i = 0; i < _wndInfoCount; ++i)
	{
		_bmpWindows[i] = nullptr;
	}

}

CWindowPreview::~CWindowPreview()
{
	if (_hTheme)
	{
		_CleanupUxThemeFile(&_hTheme);
	}
}

HRESULT CWindowPreview::GetPreviewImage(HBITMAP* pbOut)
{
	HRESULT hr = S_OK;

	if (_pageType == PT_BACKGROUND || _pageType == PT_SCRSAVER)
	{
		hr = _DrawMonitor();
		RETURN_IF_FAILED(hr);
	}

	if (_pageType != PT_SCRSAVER)
	{
		hr = _RenderSolidColor();
		RETURN_IF_FAILED(hr);
	}

	if (_pageType == PT_THEMES || _pageType == PT_BACKGROUND)
	{
		hr = _RenderWallpaper();
		LOG_IF_FAILED(hr);
		// non fatal error
	}

	if (_pageType == PT_THEMES)
	{
		hr = _RenderBin();
		RETURN_IF_FAILED(hr);
	}

	if (_wndInfoCount > 0)
	{
		for (int i = 0; i < _wndInfoCount; ++i)
		{
			// adjust size for dpi
			int width = RECTWIDTH(_pwndInfo[i].wndPos);
			int height = RECTHEIGHT(_pwndInfo[i].wndPos);

			_pwndInfo[i].wndPos.left = MulDiv(_pwndInfo[i].wndPos.left, _dpiWindow, 96);
			_pwndInfo[i].wndPos.top = MulDiv(_pwndInfo[i].wndPos.top, _dpiWindow, 96);

			_pwndInfo[i].wndPos.right = _pwndInfo[i].wndPos.left + MulDiv(width, _dpiWindow, 96);
			_pwndInfo[i].wndPos.bottom = _pwndInfo[i].wndPos.top + MulDiv(height, _dpiWindow, 96);

			// fix window sizes which are based on preview size
			if (_pwndInfo[i].wndPos.right < 0)
			{
				_pwndInfo[i].wndPos.right = _sizePreview.cx + _pwndInfo[i].wndPos.right + _pwndInfo[i].wndPos.left;
			}

			if (_pwndInfo[i].wndPos.left < 0)
			{
				_pwndInfo[i].wndPos.left = (_sizePreview.cx / 2) + _pwndInfo[i].wndPos.left;
				_pwndInfo[i].wndPos.right = (_sizePreview.cx / 2) + _pwndInfo[i].wndPos.right;
			}

			hr = _RenderWindow(_pwndInfo[i], i);
			RETURN_IF_FAILED(hr);
		}
	}

	hr = _ComposePreview(pbOut);
	return hr;
}

HRESULT CWindowPreview::GetUpdatedPreviewImage(MYWINDOWINFO* pwndInfo, LPVOID hTheme, HBITMAP* pbOut, UINT flags)
{
	if (_hTheme) _CleanupUxThemeFile(&_hTheme);
	_hTheme = hTheme;
	_pwndInfo = pwndInfo;
	if (hTheme == nullptr) _fIsThemed = 0;
	else
	{
		if (lstrcmp(selectedTheme->szMsstylePath, L"(classic)") == 0) _fIsThemed = 0;
		else _fIsThemed = 1;
	}

	if (flags & UPDATE_SOLIDCLR) _RenderSolidColor();
	if (flags & UPDATE_WALLPAPER) _RenderWallpaper();
	if (flags & UPDATE_BIN) _RenderBin();
	if (flags & UPDATE_WINDOW)
	{
		for (int i = 0; i < _wndInfoCount; ++i)
		{
			_RenderWindow(_pwndInfo[i], i);
		}
	}

	return _ComposePreview(pbOut);
}

HRESULT CWindowPreview::GetMonitorMargins(MARGINS* pOut)
{
	*pOut = _marMonitor;
	return S_OK;
}

HRESULT CWindowPreview::_ComposePreview(HBITMAP* pbOut)
{
	Bitmap* gdiBmp = new Bitmap(GETSIZE(_sizePreview), PixelFormat32bppARGB);
	RETURN_IF_NULL_ALLOC(gdiBmp);

	Graphics graphics(gdiBmp);
	RETURN_IF_NULL_ALLOC(&graphics);
	graphics.SetInterpolationMode(InterpolationModeInvalid);

	HRESULT hr = S_OK;

	Rect rect(0, 0, GETSIZE(_sizePreview));
	hr = DrawBitmapIfNotNull(_bmpMonitor, &graphics, rect);

	if (_marMonitor.cxLeftWidth > 0)
	{
		rect.X += _marMonitor.cxLeftWidth;
		rect.Y += _marMonitor.cyTopHeight;
		rect.Width = 152;
		rect.Height = 112;
	}
	hr = DrawBitmapIfNotNull(_bmpSolidColor, &graphics, rect);
	hr = DrawBitmapIfNotNull(_bmpWallpaper, &graphics, rect);

	if (_pageType == PT_SCRSAVER)
	{
		hr = _DesktopScreenShooter(&graphics);
	}

	int xOff = MulDiv(48, _dpiWindow, 96);
	int yOff = MulDiv(40, _dpiWindow, 96);

	rect = { _sizePreview.cx - xOff, _sizePreview.cy - yOff, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON)};
	hr = DrawBitmapIfNotNull(_bmpBin, &graphics, rect);

	for (int i = 0; i < _wndInfoCount; ++i)
	{
		rect = { _pwndInfo[i].wndPos.left, _pwndInfo[i].wndPos.top, RECTWIDTH(_pwndInfo[i].wndPos), RECTHEIGHT(_pwndInfo[i].wndPos) };

		if (_pageType == PT_APPEARANCE && !_fIsThemed)
		{
			rect.Y += 5;
			if (_pwndInfo[i].wndType == WT_INACTIVE)
			{
				rect.Height += 10;
			}
			if (_pwndInfo[i].wndType == WT_ACTIVE)
			{
				rect.X -= 6;
				rect.Y -= 2;
				rect.Width += 12;
				rect.Height -= 5;
			}
			if (_pwndInfo[i].wndType == WT_MESSAGEBOX)
			{
				rect.X = 22;
				rect.Y += 10 + 35;
				rect.Width += 60;
				rect.Height -= 30;
			}
		}
		hr = DrawBitmapIfNotNull(_bmpWindows[i], &graphics, rect);
	}

	// create hbitmap
	hr = gdiBmp->GetHBITMAP(Color(0, 0, 0), pbOut) == Ok ? S_OK : E_FAIL;
	delete gdiBmp;
	return hr;
}

HRESULT CWindowPreview::_CleanupUxThemeFile(void** hFile)
{
	UXTHEMEFILE* ltf = (UXTHEMEFILE*)*hFile;

	// unmaps the sections
	if (ltf->_pbSharableData) UnmapViewOfFile(ltf->_pbSharableData);
	if (ltf->_pbNonSharableData) UnmapViewOfFile(ltf->_pbNonSharableData);

	// called in CUxThemeFile::CloseFile
	ClearTheme(ltf->_hSharableSection, ltf->_hNonSharableSection, FALSE);
	free(ltf);
	*hFile = NULL;

	return S_OK;
}

HRESULT CWindowPreview::_DesktopScreenShooter(Graphics* pGraphics)
{
	HRESULT hr = S_OK;

	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);

	Rect rect(0, 0, GETSIZE(_sizePreview));
	if (_marMonitor.cxLeftWidth > 0)
	{
		rect.X += _marMonitor.cxLeftWidth;
		rect.Y += _marMonitor.cyTopHeight;
		rect.Width = 152;
		rect.Height = 112;
	}

	HDC hScreenDC = ::GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	HBITMAP _hbDesktop = CreateCompatibleBitmap(hScreenDC, cx, cy);
	HBITMAP oldBmp = (HBITMAP)SelectObject(hMemoryDC, _hbDesktop);
	BitBlt(hMemoryDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);

	Bitmap* bm = new Bitmap(_hbDesktop, NULL);
	hr = pGraphics->DrawImage(bm, rect) == Ok ? S_OK : E_FAIL;

	SelectObject(hMemoryDC, oldBmp);
	DeleteBitmap(_hbDesktop);
	DeleteBitmap(oldBmp);
	ReleaseDC(0, hScreenDC);
	DeleteDC(hMemoryDC);
	delete bm;
	return hr;
}

HRESULT CWindowPreview::_DrawMonitor()
{
	HRESULT hr = S_OK;
	Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(IDB_BITMAP1));

	// pink
	Color transparentColor(255, 255, 0, 255);
	ImageAttributes imgAttr;
	imgAttr.SetColorKey(transparentColor, transparentColor, ColorAdjustTypeBitmap);

	// cache it to improve performance
	FreeBitmap(&_bmpMonitor);
	_bmpMonitor = new Bitmap(_sizePreview.cx, _sizePreview.cy);
	Graphics graphics(_bmpMonitor);

	// draw monitor
	int xOff = (_sizePreview.cx / 2) - (monitor->GetWidth() / 2);
	int yOff = (_sizePreview.cy / 2) - (monitor->GetHeight() / 2);
	Rect rect(xOff, yOff,
		monitor->GetWidth(), monitor->GetHeight());
	graphics.DrawImage(monitor, rect, 0, 0, monitor->GetWidth(), monitor->GetHeight(), UnitPixel, &imgAttr);

	// set monitor margins
	_marMonitor = {xOff + 16, 0, yOff + 17, 0};

	delete monitor;
	return hr;
}

HRESULT CWindowPreview::_RenderWindow(MYWINDOWINFO wndInfo, int index)
{
	HRESULT hr = S_OK;
	HTHEME hTheme = OpenNcThemeData(_hTheme, L"Window");

	// my bad
	// separate this
	if (_pageType == PT_APPEARANCE && !_fIsThemed)
	{
		if (wndInfo.wndType == WT_INACTIVE)
		{
			wndInfo.wndPos.bottom += 10;
		}
		if (wndInfo.wndType == WT_ACTIVE)
		{
			wndInfo.wndPos.right += 12;
			wndInfo.wndPos.bottom -= 5;
		}
		if (wndInfo.wndType == WT_MESSAGEBOX)
		{
			wndInfo.wndPos.bottom -= 30;
			wndInfo.wndPos.right += 60;
		}
	}

	// remove top and left values
	// we add them back while composing
	wndInfo.wndPos.bottom -= wndInfo.wndPos.top;
	wndInfo.wndPos.right -= wndInfo.wndPos.left;
	wndInfo.wndPos.left = 0;
	wndInfo.wndPos.top = 0;

	// cache it to improve performance
	FreeBitmap(&_bmpWindows[index]);
	_bmpWindows[index] = new Bitmap(RECTWIDTH(wndInfo.wndPos), RECTHEIGHT(wndInfo.wndPos));
	Graphics* graphics = Gdiplus::Graphics::FromImage(_bmpWindows[index]);

	hr = _RenderFrame(graphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderCaption(graphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderContent(graphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderScrollbar(graphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	CloseThemeData(hTheme);
	return hr;
}

HRESULT CWindowPreview::_RenderWallpaper()
{
	HRESULT hr = S_OK;

	// cache it to improve performance
	FreeBitmap(&_bmpWallpaper);
	_bmpWallpaper = Bitmap::FromFile(selectedTheme->wallpaperPath, FALSE);
	return hr;
}

HRESULT CWindowPreview::_RenderBin()
{
	BOOL bRet = FALSE;
	SHSTOCKICONINFO sii = { sizeof(sii) };
	SHGetStockIconInfo(SIID_RECYCLERFULL, SHGSI_ICON | SHGSI_SHELLICONSIZE, &sii);

	int size = GetSystemMetrics(SM_CXICON);

	// cache it to improve performance
	FreeBitmap(&_bmpBin);
	_bmpBin = new Bitmap(size, size);
	Graphics graphics(_bmpBin);

	// todo: try DIB
	HDC hdcgraphic = graphics.GetHDC();
	HDC memdc = CreateCompatibleDC(hdcgraphic);


	HBITMAP hbitmap = CreateDiscardableBitmap(hdcgraphic, size, size);
	HBITMAP oldBmp = (HBITMAP)SelectObject(memdc, hbitmap);
	if (sii.hIcon)
	{
		bRet = DrawIconEx(memdc, 0, 0, sii.hIcon, size, size, 0, NULL, DI_NORMAL);
		bRet = AlphaBlend(hdcgraphic, 0, 0, size, size,
			memdc, 0, 0, size, size, BLENDFUNCTION(AC_SRC_OVER, 0, 255, AC_SRC_ALPHA));
	}
	SelectObject(memdc, oldBmp);
	DeleteBitmap(oldBmp);
	DeleteBitmap(hbitmap);
	DeleteDC(memdc);

	graphics.ReleaseHDC(hdcgraphic);
	return bRet ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderSolidColor()
{
	// todo: move selectedTheme to this class
	COLORREF clr = GetDeskopColor();
	SolidBrush backgroundBrush(Color(SPLIT_COLORREF(clr)));

	Rect rect(0, 0, GETSIZE(_sizePreview));

	// cache it to improve performance
	FreeBitmap(&_bmpSolidColor);
	_bmpSolidColor = new Bitmap(rect.Width, rect.Height);
	Graphics graphics(_bmpSolidColor);

	return Ok == graphics.FillRectangle(&backgroundBrush, rect) ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderCaption(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	FRAMESTATES frameState = wndInfo.wndType == WT_INACTIVE ? FS_INACTIVE : FS_ACTIVE;

	// caption frame
	RECT crc = wndInfo.wndPos;
	if (!_fIsThemed) crc.top += GetSystemMetrics(SM_CYFRAME);
	crc.bottom = crc.top + _marFrame.cyTopHeight;

	if (_fIsThemed)
	{
		hr = DrawThemeBackground(hTheme, hdc, WP_CAPTION, frameState, &crc, NULL);
	}
	else
	{
		// do we have gradients
		BOOL fGradients = FALSE;
		SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &fGradients, 0);

		COLORREF clrCaption = GetSysColor(wndInfo.wndType == WT_INACTIVE ? COLOR_INACTIVECAPTION : COLOR_ACTIVECAPTION);
		if (fGradients)
		{
			COLORREF clrGradient = GetSysColor(wndInfo.wndType == WT_INACTIVE ? COLOR_GRADIENTINACTIVECAPTION : COLOR_GRADIENTACTIVECAPTION);

			TRIVERTEX tex[2]{};
			tex[0].x = crc.left + _marFrame.cxLeftWidth;
			tex[0].y = crc.top;
			tex[0].Red = GetRValue(clrCaption) << 8;
			tex[0].Green = GetGValue(clrCaption) << 8;
			tex[0].Blue = GetBValue(clrCaption) << 8;

			tex[1].x = crc.right - _marFrame.cxRightWidth;
			tex[1].y = crc.bottom;
			tex[1].Red = GetRValue(clrGradient) << 8;
			tex[1].Green = GetGValue(clrGradient) << 8;
			tex[1].Blue = GetBValue(clrGradient) << 8;

			GRADIENT_RECT rect = { 0,1 };
			hr = GradientFill(hdc, tex, ARRAYSIZE(tex), &rect, 1, GRADIENT_FILL_RECT_H) == TRUE ? S_OK : E_FAIL;

			// 1px border below caption
			HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);

			POINT pt[2]{};
			pt[0] = { crc.left + _marFrame.cxLeftWidth, crc.top + _marFrame.cyTopHeight };
			pt[1] = { crc.right - _marFrame.cxRightWidth, crc.top + _marFrame.cyTopHeight };
			Polyline(hdc, pt, ARRAYSIZE(pt));

			SelectObject(hdc, oldPen);
			DeletePen(pen);
		}
	}
	RETURN_IF_FAILED(hr);

	hr = _RenderCaptionText(hdc, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderCaptionButtons(hdc, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderCaptionButtons(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	CLOSEBUTTONSTATES btnState = wndInfo.wndType == WT_INACTIVE ? (CLOSEBUTTONSTATES)5 : CBS_NORMAL;

	SIZE size = { 0 };
	GetThemePartSize(hTheme, hdc, WP_CLOSEBUTTON, CBS_NORMAL, NULL, TS_TRUE, &size);

	// uxtheme.dll _GetNcBtnMetrics 
	int cxEdge = GetSystemMetrics(SM_CXEDGE);
	int cyEdge = GetSystemMetrics(SM_CYEDGE);

	int cyBtn = _fIsThemed ? GetThemeSysSize(hTheme, SM_CYSIZE) : GetSystemMetrics(SM_CYSIZE);
	int cxBtn = _fIsThemed ? MulDiv(cyBtn, size.cx, size.cy) : GetSystemMetrics(SM_CYSIZE);

	// remove padding
	cyBtn -= (cyEdge * 2);
	cxBtn -= _fIsThemed ? (cyEdge * 2) : cyEdge;

	RECT crc = wndInfo.wndPos;
	crc.right -= _marFrame.cxRightWidth + cxEdge;
	crc.left = crc.right - cxBtn;
	crc.top += _fIsThemed ? _marFrame.cyTopHeight - GetSystemMetrics(SM_CYFRAME) - cyBtn
		: ((_marFrame.cyTopHeight - cyBtn) / 2) + GetSystemMetrics(SM_CYFRAME);
	crc.bottom = crc.top + cyBtn;

	_fIsThemed ? DrawThemeBackground(hTheme, hdc, WP_CLOSEBUTTON, btnState, &crc, NULL)
		: DrawFrameControl(hdc, &crc, DFC_CAPTION, DFCS_CAPTIONCLOSE) == TRUE ? S_OK : E_FAIL;

	if (wndInfo.wndType != WT_MESSAGEBOX)
	{
		// max button
		int width = cxBtn + cxEdge;
		crc.left -= width;
		crc.right -= width;
		_fIsThemed ? DrawThemeBackground(hTheme, hdc, WP_MAXBUTTON, btnState, &crc, NULL)
			: DrawFrameControl(hdc, &crc, DFC_CAPTION, DFCS_CAPTIONMAX) == TRUE ? S_OK : E_FAIL;

		// min button
		if (!_fIsThemed)
		{
			width = cxBtn;
		}
		crc.left -= width;
		crc.right -= width;
		_fIsThemed ? DrawThemeBackground(hTheme, hdc, WP_MINBUTTON, btnState, &crc, NULL)
			: DrawFrameControl(hdc, &crc, DFC_CAPTION, DFCS_CAPTIONMIN) == TRUE ? S_OK : E_FAIL;
	}

	return hr;
}

HRESULT CWindowPreview::_RenderCaptionText(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	CAPTIONSTATES frameState = wndInfo.wndType == WT_INACTIVE ? CS_INACTIVE : CS_ACTIVE;

	MARGINS mar = { 0 };
	if (_fIsThemed)
	{
		hr = GetThemeMargins(hTheme, hdc, WP_CAPTION, frameState, TMT_CAPTIONMARGINS, NULL, &mar);
		RETURN_IF_FAILED(hr);
	}

	// set proper font
	LOGFONT font{};
	if (_fIsThemed)
	{
		GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &font);
	}
	else
	{
		NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		font = ncm.lfCaptionFont;
	}

	HFONT fon = CreateFontIndirect(&font);
	HFONT hOldFont = (HFONT)SelectObject(hdc, fon);
	SetBkMode(hdc, TRANSPARENT);

	LPCWSTR text = L"";
	switch (wndInfo.wndType)
	{
	case WT_ACTIVE:
		text = L"Active Window";
		break;
	case WT_INACTIVE:
		text = L"Inactive Window";
		break;
	case WT_MESSAGEBOX:
		text = L"Message Box";
		break;
	}

	RECT rc = wndInfo.wndPos;
	if (_fIsThemed)
	{
		hr = GetThemeTextExtent(hTheme, hdc, WP_CAPTION, frameState, text, -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &wndInfo.wndPos, &rc);
		RETURN_IF_FAILED(hr);
	}

	rc.left += _marFrame.cxLeftWidth + mar.cxLeftWidth;
	rc.right += _marFrame.cxLeftWidth + mar.cxLeftWidth;
	rc.top -= _marFrame.cyBottomHeight;
	rc.bottom -= _marFrame.cyBottomHeight;

	// get height
	RECT rcheight = { 0,0,0,0 };
	DrawText(hdc, text, -1, &rcheight, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT);

	if (_fIsThemed)
	{
		rc.top += _marFrame.cyTopHeight + _marFrame.cyBottomHeight - GetSystemMetrics(SM_CYFRAME) - RECTHEIGHT(rcheight);
	}
	else
	{
		rc.top += ((_marFrame.cyTopHeight - RECTHEIGHT(rcheight)) / 2) + GetSystemMetrics(SM_CXFRAME);
		rc.left += GetSystemMetrics(SM_CXBORDER);
	}
	rc.bottom = rc.top + RECTHEIGHT(rcheight);

	bool fIsInactive = wndInfo.wndType == WT_INACTIVE;
	COLORREF clr = _fIsThemed ? GetThemeSysColor(hTheme, fIsInactive ? COLOR_INACTIVECAPTIONTEXT : COLOR_CAPTIONTEXT)
		: GetSysColor(fIsInactive ? COLOR_INACTIVECAPTIONTEXT : COLOR_CAPTIONTEXT);

	if (_fIsThemed)
	{
		DTTOPTS dt = { sizeof(dt) };
		dt.dwFlags = DTT_TEXTCOLOR;
		dt.crText = clr;
		hr = DrawThemeTextEx(hTheme, hdc, WP_CAPTION, frameState, text, -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &rc, &dt);
	}
	else
	{
		SetTextColor(hdc, clr);
		hr = DrawText(hdc, text, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER);
	}
	RETURN_IF_FAILED(hr);

	SelectObject(hdc, hOldFont);
	DeleteObject(fon);
	return hr;
}

HRESULT CWindowPreview::_RenderScrollbar(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	if (wndInfo.wndType != WT_ACTIVE) return hr;

	HTHEME hThemeScrl = OpenNcThemeData(_hTheme, L"Scrollbar");
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	RECT crc = wndInfo.wndPos;
	crc.top += _marFrame.cyTopHeight;

	SIZE size = { 0 };
	GetThemePartSize(hThemeScrl, hdc, SBP_ARROWBTN, ABS_UPNORMAL, NULL, TS_TRUE, &size);

	int width = _fIsThemed ? max(GetThemeSysSize(hTheme, SM_CXVSCROLL), size.cx) : GetSystemMetrics(SM_CXVSCROLL);
	int height = _fIsThemed ? max(GetThemeSysSize(hTheme, SM_CXVSCROLL), size.cy) : GetSystemMetrics(SM_CXVSCROLL);

	crc.left = crc.right - _marFrame.cxRightWidth - width;
	crc.right = crc.left + width;
	crc.bottom -= _marFrame.cyBottomHeight + 2;
	if (!_fIsThemed)
	{
		if (wndInfo.wndType == WT_ACTIVE) crc.top += _szMenuBar.cy;
	}

	if (_fIsThemed)
	{
		DrawThemeBackground(hThemeScrl, hdc, SBP_LOWERTRACKVERT, SCRBS_NORMAL, &crc, 0);
	}
	else
	{
		FillRect(hdc, &crc, GetSysColorBrush(COLOR_SCROLLBAR));
	}

	crc.bottom = crc.top + height;
	_fIsThemed ? DrawThemeBackground(hThemeScrl, hdc, SBP_ARROWBTN, ABS_UPNORMAL, &crc, 0)
		: DrawFrameControl(hdc, &crc, DFC_SCROLL, DFCS_SCROLLUP) == TRUE ? S_OK : E_FAIL;

	crc.top += height;
	crc.bottom = crc.top + height;
	if (_fIsThemed) DrawThemeBackground(hThemeScrl, hdc, SBP_THUMBBTNVERT, SCRBS_NORMAL, &crc, 0);

	crc.top = wndInfo.wndPos.bottom - height;
	crc.bottom = wndInfo.wndPos.bottom;
	crc.bottom -= _marFrame.cyBottomHeight + 2;
	crc.top -= _marFrame.cyBottomHeight + 2;

	_fIsThemed ? DrawThemeBackground(hThemeScrl, hdc, SBP_ARROWBTN, ABS_DOWNNORMAL, &crc, 0)
		: DrawFrameControl(hdc, &crc, DFC_SCROLL, DFCS_SCROLLDOWN) == TRUE ? S_OK : E_FAIL;

	CloseThemeData(hThemeScrl);
	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderFrame(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	// calculate frame margins
	// probably make another function
	if (_fIsThemed)
	{
		int cxPaddedBorder = GetThemeSysSize(hTheme, SM_CXPADDEDBORDER);
		int cyCaptionHeight = GetThemeSysSize(hTheme, SM_CYSIZE) + cxPaddedBorder + 2; // i think
		_marFrame.cxLeftWidth = cxPaddedBorder + GetSystemMetrics(SM_CXFRAME);
		_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
		_marFrame.cyTopHeight = cyCaptionHeight + GetSystemMetrics(SM_CYFRAME);
		_marFrame.cyBottomHeight = GetSystemMetrics(SM_CYFRAME) + cxPaddedBorder - 2;
	}
	else
	{
		// todo: account for padded borders
		_marFrame.cxLeftWidth = GetSystemMetrics(SM_CXFRAME);
		_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
		_marFrame.cyTopHeight = GetSystemMetrics(SM_CYCAPTION) - 1; // why is it +1
		_marFrame.cyBottomHeight = 0;
	}

	FRAMESTATES frameState = wndInfo.wndType == WT_INACTIVE ? FS_INACTIVE : FS_ACTIVE;

	// todo: split this ??
	RECT crc = wndInfo.wndPos;
	if (!_fIsThemed)
	{
		bool fIsMessageBox = wndInfo.wndType == WT_MESSAGEBOX;

		if (fIsMessageBox)
		{
			int offset = GetSystemMetrics(SM_CXBORDER);
			InflateRect(&crc, -offset, -offset);
		}
		DrawEdge(hdc, &crc, EDGE_RAISED, BF_RECT);

		if (!fIsMessageBox)
		{
			int count = GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXEDGE) - GetSystemMetrics(SM_CXBORDER);
			InflateRect(&crc, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CXEDGE));
			for (int i = 0; i < count; i++)
			{
				bool fInactiveWnd = wndInfo.wndType == WT_INACTIVE;
				// framerect draws a 1px border
				// so do (frame - edge - border) times
				FrameRect(hdc, &crc, GetSysColorBrush(fInactiveWnd ? COLOR_INACTIVEBORDER : COLOR_ACTIVEBORDER));
				InflateRect(&crc, -1, -1);
			}
		}
		else
		{
			int offset = GetSystemMetrics(SM_CXEDGE);
			InflateRect(&crc, -offset, -offset);
		}
		FrameRect(hdc, &crc, GetSysColorBrush(COLOR_3DFACE));
		InflateRect(&crc, -1, -1);
	}

	crc = wndInfo.wndPos;
	crc.top = crc.bottom - (_marFrame.cyBottomHeight + 2);
	if (_fIsThemed)
	{
		hr = DrawThemeBackground(hTheme, hdc, WP_FRAMEBOTTOM, frameState, &crc, NULL);
		RETURN_IF_FAILED(hr);
	}

	crc = wndInfo.wndPos;
	crc.top += _marFrame.cyTopHeight;
	crc.right = crc.left + _marFrame.cxLeftWidth;
	crc.bottom -= _marFrame.cyBottomHeight + 2;
	if (_fIsThemed)
	{
		hr = DrawThemeBackground(hTheme, hdc, WP_FRAMELEFT, frameState, &crc, NULL);
		RETURN_IF_FAILED(hr);
	}

	crc.left = wndInfo.wndPos.right - _marFrame.cxRightWidth;
	crc.right = wndInfo.wndPos.right;
	if (_fIsThemed)
	{
		hr = DrawThemeBackground(hTheme, hdc, WP_FRAMERIGHT, frameState, &crc, NULL);
		RETURN_IF_FAILED(hr);
	}

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderContent(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);
	BOOL fIsMessageBox = wndInfo.wndType == WT_MESSAGEBOX;

	COLORREF clr = _fIsThemed ? GetThemeSysColor(hTheme, fIsMessageBox ? COLOR_3DFACE : COLOR_WINDOW)
		: GetSysColor(fIsMessageBox ? COLOR_3DFACE : COLOR_WINDOW);

	RECT crc = wndInfo.wndPos;
	crc.left += _marFrame.cxLeftWidth;
	crc.top += _marFrame.cyTopHeight;
	if (!_fIsThemed && wndInfo.wndType == WT_MESSAGEBOX) crc.top += GetSystemMetrics(SM_CYFRAME);
	if (!_fIsThemed && wndInfo.wndType == WT_ACTIVE) crc.top += _szMenuBar.cy + GetSystemMetrics(SM_CYFRAME);

	crc.bottom -= _marFrame.cyBottomHeight + 2;
	int count = GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXEDGE) - GetSystemMetrics(SM_CXBORDER);
	if (!_fIsThemed) crc.bottom -= GetSystemMetrics(SM_CYBORDER) + count;
	crc.right -= _marFrame.cxRightWidth;

	if (!_fIsThemed && !fIsMessageBox)
	{
		// QUIRK: same behaviour as old desk.cpl
		if (wndInfo.wndType == WT_ACTIVE) DrawEdge(hdc, &crc, EDGE_SUNKEN, BF_RECT);

		if (wndInfo.wndType == WT_ACTIVE)
		{
			// 1px border above edge
			HPEN pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);

			POINT pt[2]{};
			pt[0] = { crc.left, crc.top - 1 };
			pt[1] = { crc.right, crc.top - 1 };
			Polyline(hdc, pt, ARRAYSIZE(pt));

			SelectObject(hdc, oldPen);
			DeletePen(pen);
		}

		// offset the rect by edge to actually show the inner border
		InflateRect(&crc, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));

		// update margins
		_marFrame.cxLeftWidth += GetSystemMetrics(SM_CXEDGE);
		_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
		_marFrame.cyTopHeight += GetSystemMetrics(SM_CYEDGE) + GetSystemMetrics(SM_CYFRAME);
		_marFrame.cyBottomHeight += GetSystemMetrics(SM_CYFRAME);
	}

	// idk how it works but u cant use gdi+ to draw a rect, and then use gdi to draw the text on it
	// QUIRK: same behaviour as old desk.cpl
	if (_fIsThemed || (wndInfo.wndType != WT_INACTIVE && !_fIsThemed)) FillRect(hdc, &crc, CreateSolidBrush(clr));

	LOGFONT font{};
	if (_fIsThemed)
	{
		GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &font);
	}
	else
	{
		NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		font = ncm.lfMessageFont;
	}

	if (wndInfo.wndType == WT_ACTIVE)
	{
		if (!_fIsThemed) font.lfWeight = FW_BOLD;
		HFONT fon = CreateFontIndirect(&font);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		RECT crc = wndInfo.wndPos;
		crc.top += _marFrame.cyTopHeight;
		if (!_fIsThemed) crc.top += _szMenuBar.cy;


		// get height
		RECT rcheight = { 0,0,0,0 };
		DrawText(hdc, L"Window Text", -1, &rcheight, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT);

		crc.bottom = crc.top + RECTHEIGHT(rcheight);
		crc.left += _marFrame.cxLeftWidth;

		COLORREF clr = _fIsThemed ? GetThemeSysColor(hTheme, COLOR_WINDOWTEXT) : GetSysColor(COLOR_WINDOWTEXT);

		if (_fIsThemed)
		{

			DTTOPTS dt = { sizeof(dt) };
			dt.dwFlags = DTT_TEXTCOLOR;
			dt.crText = clr;
			hr = DrawThemeTextEx(hTheme, hdc, 0, 0, L"Window Text", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &crc, &dt);
		}
		else
		{
			SetTextColor(hdc, clr);
			crc.left += GetSystemMetrics(SM_CXBORDER);

			hr = DrawText(hdc, L"Window Text", -1, &crc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER);
		}
		RETURN_IF_FAILED(hr);

		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	if (wndInfo.wndType == WT_MESSAGEBOX)
	{
		HFONT fon = CreateFontIndirect(&font);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		COLORREF clr = _fIsThemed ? GetThemeSysColor(hTheme, COLOR_BTNTEXT) : GetSysColor(COLOR_BTNTEXT);
		SetTextColor(hdc, clr);

		// load button theme
		if (_fIsThemed)
		{
			InflateRect(&crc, MulDiv(-30, _dpiWindow, 96), MulDiv(-16, _dpiWindow, 96));

			HTHEME hThemeBtn = OpenNcThemeData(_hTheme, L"Button");
			DrawThemeBackground(hThemeBtn, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, &crc, NULL);
			CloseThemeData(hThemeBtn);
		}
		else
		{
			crc.left += GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXBORDER);
			crc.right -= GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXBORDER);
			DrawText(hdc, L"Message Text", -1, &crc, DT_LEFT | DT_TOP | DT_SINGLELINE);

			crc.top += 17;
			crc.bottom -= 2;
			crc.left += 62;
			crc.right -= 62;
			DrawFrameControl(hdc, &crc, DFC_BUTTON, DFCS_BUTTONPUSH);
		}
		DrawText(hdc, L"OK", -1, &crc, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_VCENTER);

		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	pGraphics->ReleaseHDC(hdc);
	return hr;
}
