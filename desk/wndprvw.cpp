/* ---------------------------------------------------------
* Window Preview
*
* Responsible for rendering the preview
*
------------------------------------------------------------*/
#include "pch.h"
#include "wndprvw.h"
#include "helper.h"
#include "uxtheme.h"
#include "cscheme.h"

#ifndef _DEBUG
#undef RETURN_IF_FAILED
#undef RETURN_IF_NULL_ALLOC
#undef LOG_IF_FAILED

#define RETURN_IF_FAILED
#define RETURN_IF_NULL_ALLOC
#define LOG_IF_FAILED
#endif

#define ADJUSTDPI(x) MulDiv(x, _dpiWindow, 96)
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
	_iCurrentWnd = 0;
	_bmpBin = nullptr;
	_bmpSolidColor = nullptr;
	_bmpWallpaper = nullptr;
	_bmpMonitor = nullptr;
	_bmpWindows = (Bitmap**)malloc(_wndInfoCount * sizeof(Bitmap*));
	for (int i = 0; i < _wndInfoCount; ++i)
	{
		_bmpWindows[i] = nullptr;
	}

	cs_dpi = dpi;
	_rcBounds = (RECT*)calloc(10, sizeof(RECT));

	// keep it loaded
	_bmpMonitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(IDB_BITMAP1));
}

CWindowPreview::~CWindowPreview()
{
	if (_bmpMonitor) delete _bmpMonitor;
	if (_bmpSolidColor) delete _bmpSolidColor;
	if (_bmpWallpaper) delete _bmpWallpaper;
	if (_bmpBin) delete _bmpBin;

	if (_bmpWindows)
	{
		for (int i = 0; i < _wndInfoCount; ++i)
		{
			if (_bmpWindows[i]) delete _bmpWindows[i];
		}
		free(_bmpWindows);
	}

	if (_hTheme)
	{
		_CleanupUxThemeFile(&_hTheme);
	}

	CloseThemeData(_hWndTheme);
	CloseThemeData(_hScrlTheme);
}

HRESULT CWindowPreview::GetPreviewImage(HBITMAP* pbOut)
{
	HRESULT hr = S_OK;
	_hWndTheme = OpenNcThemeData(_hTheme, L"Window");
	_hScrlTheme = OpenNcThemeData(_hTheme, L"Scrollbar");
	_CalculateRectsOfElements();

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
		if (selectedTheme->szMsstylePath.compare(L"(classic)") == 0) _fIsThemed = 0;
		else _fIsThemed = 1;
	}

	CloseThemeData(_hWndTheme);
	CloseThemeData(_hScrlTheme);
	_hWndTheme = OpenNcThemeData(_hTheme, L"Window");
	_hScrlTheme = OpenNcThemeData(_hTheme, L"Scrollbar");
	_CalculateRectsOfElements();


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

HRESULT CWindowPreview::GetMonitorOffset(SIZE* pOut)
{
	*pOut = { _rcMonitorInside.X, _rcMonitorInside.Y };
	return S_OK;
}

HRESULT CWindowPreview::SetClassicPrev(BOOL fEnable)
{
	_fIsThemed = !fEnable;
	return S_OK;
}

HRESULT CWindowPreview::GetBoundingRect(int elmId, RECT* pRect)
{
	return E_NOTIMPL;
}

HRESULT CWindowPreview::_ComposePreview(HBITMAP* pbOut)
{
	if (pbOut && *pbOut)
	{
		DeleteObject(*pbOut);
		*pbOut = nullptr;
	}

	Bitmap* gdiBmp = new Bitmap(GETSIZE(_sizePreview), PixelFormat32bppARGB);
	RETURN_IF_NULL_ALLOC(gdiBmp);

	HRESULT hr = S_OK;

	Graphics graphics(gdiBmp);
	RETURN_IF_NULL_ALLOC(&graphics);
	graphics.SetInterpolationMode(InterpolationModeInvalid);

	// draw monitor bitmap if it exists
	{
		Color transparentColor(255, 255, 0, 255);
		ImageAttributes imgAttr;
		imgAttr.SetColorKey(transparentColor, transparentColor, ColorAdjustTypeBitmap);
		graphics.DrawImage(_bmpMonitor, _rcMonitor,
			0, 0, _bmpMonitor->GetWidth(), _bmpMonitor->GetHeight(),
			UnitPixel, &imgAttr);
	}

	Gdiplus::Rect rc = _rcPreview;
	if (_pageType == PT_BACKGROUND || _pageType == PT_SCRSAVER) rc = _rcMonitorInside;

	// draw solid color behind
	hr = DrawBitmapIfNotNull(_bmpSolidColor, &graphics, rc);

	// now draw wallpaper on top of it
	graphics.SetClip(rc);
	_AdjustAndDrawWallpaper(&graphics, rc);
	graphics.ResetClip();

	// draw the current screenshot
	if (_pageType == PT_SCRSAVER) hr = _DesktopScreenShooter(&graphics);

	// draw bin
	hr = DrawBitmapIfNotNull(_bmpBin, &graphics, _rcBin);

	Rect rect(0, 0, GETSIZE(_sizePreview));
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

	free(*hFile);
	*hFile = NULL;

	return S_OK;
}

HRESULT CWindowPreview::_DesktopScreenShooter(Graphics* pGraphics)
{
	HRESULT hr = S_OK;

	int cx = NcGetSystemMetrics(SM_CXSCREEN);
	int cy = NcGetSystemMetrics(SM_CYSCREEN);

	Bitmap* bm = new Bitmap(cx, cy);
	Graphics g(bm);
	HDC hdc = g.GetHDC();

	HDC hScreenDC = ::GetDC(NULL);
	BitBlt(hdc, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);
	g.ReleaseHDC(hdc);

	hr = pGraphics->DrawImage(bm, _rcMonitorInside) == Ok ? S_OK : E_FAIL;

	delete bm;
	ReleaseDC(NULL, hScreenDC);
	return hr;
}

HRESULT CWindowPreview::_RenderWindow(MYWINDOWINFO wndInfo, int index)
{
	HRESULT hr = S_OK;
	_iCurrentWnd = index;

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

	_rcMargin = wndInfo.wndPos;
	_CalculateWindowRects();

	FreeBitmap(&_bmpWindows[index]);
	_bmpWindows[index] = new Bitmap(RECTWIDTH(wndInfo.wndPos), RECTHEIGHT(wndInfo.wndPos));
	Graphics* graphics = Gdiplus::Graphics::FromImage(_bmpWindows[index]);

	hr = _RenderFrame(graphics, _hWndTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderCaption(graphics, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderContent(graphics, _hWndTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderScrollbar(graphics, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderMenuBar(graphics, wndInfo);
	RETURN_IF_FAILED(hr);

	delete graphics;
	return hr;
}

HRESULT CWindowPreview::_RenderWallpaper()
{
	HRESULT hr = S_OK;
	FreeBitmap(&_bmpWallpaper);
	_bmpWallpaper = Bitmap::FromFile(selectedTheme->wallpaperPath.c_str(), FALSE);
	return hr;
}

HRESULT CWindowPreview::_AdjustAndDrawWallpaper(Gdiplus::Graphics* pGraphics, Gdiplus::Rect rc)
{
	if (!_bmpWallpaper) return E_FAIL;

	int monitorwidth = GetSystemMetrics(SM_CXSCREEN);
	int monitorheight = GetSystemMetrics(SM_CYSCREEN);

	DESKTOP_WALLPAPER_POSITION pos = (DESKTOP_WALLPAPER_POSITION)selectedTheme->posChanged;
	if (selectedTheme->posChanged == -1)
	{
		pDesktopWallpaper->GetPosition(&pos);
	}

	float scaleX = (float)rc.Width / monitorwidth;
	float scaleY = (float)rc.Height / monitorheight;

	float scaledW = (float)_bmpWallpaper->GetWidth() * scaleX;
	float scaledH = (float)_bmpWallpaper->GetHeight() * scaleY;


	if (pos == DWPOS_CENTER)
	{
		rc.X += (int)((rc.Width - scaledW) / 2);
		rc.Y += (int)((rc.Height - scaledH) / 2);

		rc.Width = (int)scaledW;
		rc.Height = (int)scaledH;
	}
	else if (pos == DWPOS_FIT)
	{
		float ratio = scaledW / scaledH;
		float newW = ratio * rc.Height;
		if (newW > rc.Width)
		{
			ratio = scaledH / scaledW;
			float newH = ratio * rc.Width;

			rc.Y += (int)((rc.Height - newH) / 2);
			rc.Height = (int)newH;
		}
		else
		{
			rc.X += (int)((rc.Width - newW) / 2);
			rc.Width = (int)newW;
		}
	}
	else if (pos == DWPOS_FILL)
	{
		float ratio = scaledH / scaledW;
		float newH = ratio * rc.Width;
		if (newH < rc.Height)
		{
			ratio = scaledW / scaledH;
			float newW = ratio * rc.Height;

			rc.X += (int)((rc.Width - newW) / 2);
			rc.Width = (int)newW;
		}
		else
		{
			rc.Y += (int)((rc.Height - newH) / 2);
			rc.Height = (int)newH;
		}
	}
	else if (pos == DWPOS_TILE)
	{
		int sideImages = (int)((rc.Width / scaledW) + 1);
		int topImages = (int)((rc.Height / scaledH) + 1);

		rc.Width = (int)scaledW;
		rc.Height = (int)scaledH;

		for (int i = 0; i < topImages; i++)
		{
			for (int j = 0; j < sideImages; j++)
			{
				DrawBitmapIfNotNull(_bmpWallpaper, pGraphics, rc);
				rc.X += rc.Width;
			}
			rc.X = _rcMonitorInside.X;
			rc.Y += rc.Height;
		}
		return S_OK;
	}

	DrawBitmapIfNotNull(_bmpWallpaper, pGraphics, rc);
	return S_OK;
}

HRESULT CWindowPreview::_CalculateRectsOfElements()
{
	// bin
	_rcBin = {
		_sizePreview.cx - ADJUSTDPI(48),
		_sizePreview.cy - ADJUSTDPI(40),
		NcGetSystemMetrics(SM_CXICON),
		NcGetSystemMetrics(SM_CYICON)
	};

	// preview
	_rcPreview = {
		0,
		0,
		_sizePreview.cx,
		_sizePreview.cy
	};

	// monitor
	int xOff = (_sizePreview.cx / 2) - (_bmpMonitor->GetWidth() / 2);
	int yOff = (_sizePreview.cy / 2) - (_bmpMonitor->GetHeight() / 2);
	_rcMonitor = {
		xOff,
		yOff,
		(int)_bmpMonitor->GetWidth(),
		(int)_bmpMonitor->GetHeight()
	};

	_rcMonitorInside = {
		_rcMonitor.X + 16,
		_rcMonitor.Y + 17,
		152,
		112
	};

	return S_OK;
}

HRESULT CWindowPreview::_CalculateFrameMargins()
{
	// calculate frame margins
	if (_fIsThemed)
	{
		int cxPaddedBorder = GetThemeSysSize(_hWndTheme, SM_CXPADDEDBORDER);
		int cyCaptionHeight = GetThemeSysSize(_hWndTheme, SM_CYSIZE) + cxPaddedBorder + 2; // i think
		_marFrame.cxLeftWidth = cxPaddedBorder + NcGetSystemMetrics(SM_CXFRAME);
		_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
		_marFrame.cyTopHeight = cyCaptionHeight + NcGetSystemMetrics(SM_CYFRAME);
		_marFrame.cyBottomHeight = NcGetSystemMetrics(SM_CYFRAME) + cxPaddedBorder - 2;
	}
	else
	{
		// todo: account for padded borders
		_marFrame.cxLeftWidth = NcGetSystemMetrics(SM_CXEDGE) + NcGetSystemMetrics(SM_CXBORDER) + 1;
		_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
		_marFrame.cyTopHeight = NcGetSystemMetrics(SM_CYSIZE) - 1;
		_marFrame.cyBottomHeight = 0;
	}

	return S_OK;
}

HRESULT CWindowPreview::_CalculateWindowRects()
{
	_CalculateFrameMargins();

	// caption
	if (_fIsThemed)
	{
		_rcBounds[0] = {
			.left = 0,
			.top = 0,
			.right = RECTWIDTH(_rcMargin),
			.bottom = _marFrame.cyTopHeight + 1,
		};
	}
	else
	{
		int edge = NcGetSystemMetrics(SM_CYEDGE) + NcGetSystemMetrics(SM_CYBORDER) + 1;
		_rcBounds[0] = {
			.left = _marFrame.cxLeftWidth,
			.top = edge,
			.right = _rcMargin.right - _marFrame.cxLeftWidth,
			.bottom = edge + _marFrame.cyTopHeight + 1,
		};

		if (_pwndInfo[_iCurrentWnd].wndType == WT_MESSAGEBOX)
			_rcBounds[0].right += NcGetSystemMetrics(SM_CXBORDER);
	}

	// caption text
	if (_fIsThemed)
	{
		MARGINS mar = { 0 };
		GetThemeMargins(_hWndTheme, NULL, WP_CAPTION, CS_ACTIVE, TMT_CAPTIONMARGINS, NULL, &mar);

		_rcBounds[1] = {
			.left = _rcBounds[0].left + _marFrame.cxLeftWidth + mar.cxLeftWidth,
			.top = _rcBounds[0].bottom - (GetThemeSysSize(_hWndTheme, SM_CYSIZE) / 2) - 2, // remove RECTHEIGHT 
			.right = _rcBounds[0].right,
			.bottom = 0
		};
	}
	else
	{
		_rcBounds[1] = {
			.left = _rcBounds[0].left + NcGetSystemMetrics(SM_CXFRAME) - NcGetSystemMetrics(SM_CXEDGE),
			.top = _rcBounds[0].top + (NcGetSystemMetrics(SM_CYSIZE) / 2), // remove RECTHEIGHT
			.right = _rcBounds[0].right,
			.bottom = 0
		};
	}
	_rcBounds[1].bottom = _rcBounds[1].top; // add RECTHEIGHT

	// caption button
	// uxtheme.dll _GetNcBtnMetrics
	int cxEdge = NcGetSystemMetrics(SM_CXEDGE);
	int cyEdge = NcGetSystemMetrics(SM_CYEDGE);

	SIZE size = { 0 };
	GetThemePartSize(_hWndTheme, NULL, WP_CLOSEBUTTON, CBS_NORMAL, NULL, TS_TRUE, &size);

	int cyBtn = _fIsThemed ? GetThemeSysSize(_hWndTheme, SM_CYSIZE) : NcGetSystemMetrics(SM_CYSIZE);
	int cxBtn = _fIsThemed ? MulDiv(cyBtn, size.cx, size.cy) : NcGetSystemMetrics(SM_CYSIZE);

	// remove padding
	cyBtn -= (cyEdge * 2);
	cxBtn -= _fIsThemed ? (cyEdge * 2) : cyEdge;
	if (_fIsThemed)
	{
		// close btn
		_rcBounds[2] = {
			.left = _rcBounds[0].right - _marFrame.cxRightWidth - cxEdge - cxBtn,
			.top = _rcBounds[0].top + _marFrame.cyTopHeight - NcGetSystemMetrics(SM_CYFRAME) - cyBtn,
			.right = _rcBounds[0].right - _marFrame.cxRightWidth - cxEdge,
			.bottom = cyBtn
		};
	}
	else
	{
		// close btn
		_rcBounds[2] = {
			.left = _rcBounds[0].right - cxEdge - cxBtn,
			.top = _rcBounds[0].top + ((NcGetSystemMetrics(SM_CYSIZE) - cyBtn) / 2),
			.right = _rcBounds[0].right - cxEdge,
			.bottom = cyBtn
		};
	}
	_rcBounds[2].bottom += _rcBounds[2].top;

	// max btn
	_rcBounds[3] = _rcBounds[2];
	OffsetRect(&_rcBounds[3], -(cxBtn + cxEdge), 0);

	// min btn
	_rcBounds[4] = _rcBounds[3];
	OffsetRect(&_rcBounds[4], -(_fIsThemed ? cxBtn + cxEdge : cxBtn), 0);

	// window content
	if (_fIsThemed)
	{
		_rcBounds[5] = {
			.left = _marFrame.cxLeftWidth,
			.top = _rcBounds[0].bottom - 1,
			.right = _rcBounds[0].right - _marFrame.cxRightWidth,
			.bottom = _rcMargin.bottom - _marFrame.cyBottomHeight - 2
		};
	}
	else
	{
		_rcBounds[5] = {
			.left = _marFrame.cxLeftWidth,
			.top = _rcBounds[0].bottom + 2, // border
			.right = _rcBounds[0].right,
			.bottom = _rcMargin.bottom - _marFrame.cyBottomHeight - 2
		};

		int count = NcGetSystemMetrics(SM_CXFRAME) - NcGetSystemMetrics(SM_CXEDGE) - NcGetSystemMetrics(SM_CXBORDER);
		_rcBounds[5].bottom -= NcGetSystemMetrics(SM_CYBORDER) + count;

		if (_pwndInfo[_iCurrentWnd].wndType == WT_MESSAGEBOX) _rcBounds[5].top -= 1;
		if (_pwndInfo[_iCurrentWnd].wndType == WT_ACTIVE)
		{
			_rcBounds[5].top += NcGetSystemMetrics(SM_CYMENUSIZE);
			_rcBounds[5].bottom -= NcGetSystemMetrics(SM_CYBORDER) - 1;
		}
	}

	// inner window
	_rcBounds[6] = _rcBounds[5];
	if (!_fIsThemed && _pwndInfo[_iCurrentWnd].wndType == WT_ACTIVE) InflateRect(&_rcBounds[6], -NcGetSystemMetrics(SM_CXEDGE), -NcGetSystemMetrics(SM_CYEDGE));

	// window button
	_rcBounds[7] = _rcBounds[6];
	if (_fIsThemed)
	{
		InflateRect(&_rcBounds[7], MulDiv(-30, _dpiWindow, 96), MulDiv(-16, _dpiWindow, 96));
	}
	else
	{
		int center = (RECTWIDTH(_rcBounds[7]) / 2) + (NcGetSystemMetrics(SM_CXBORDER) * 2);
		_rcBounds[7].bottom -= MulDiv(3, _dpiWindow, 96);
		_rcBounds[7].left = center - MulDiv(35, _dpiWindow, 96);
		_rcBounds[7].right = center + MulDiv(35, _dpiWindow, 96);
		_rcBounds[7].top = _rcBounds[7].bottom - MulDiv(24, _dpiWindow, 96);
	}
	
	// menu bar
	_rcBounds[8] = _rcBounds[0];
	_rcBounds[8].top += _marFrame.cyTopHeight + NcGetSystemMetrics(SM_CYEDGE);
	_rcBounds[8].bottom = _rcBounds[8].top + NcGetSystemMetrics(SM_CYMENUSIZE);

	// scroll bar
	_rcBounds[9] = _rcBounds[6];
	GetThemePartSize(_hScrlTheme, NULL, SBP_ARROWBTN, ABS_UPNORMAL, NULL, TS_TRUE, &_sizeScrollbar);
	int width = _fIsThemed ? max(GetThemeSysSize(_hWndTheme, SM_CXVSCROLL), _sizeScrollbar.cx) : NcGetSystemMetrics(SM_CXVSCROLL);
	_rcBounds[9].left = _rcBounds[9].right - width;

	return S_OK;
}

HRESULT CWindowPreview::_RenderBin()
{
	BOOL bRet = 1;
	SHSTOCKICONINFO sii = { sizeof(sii) };
	SHGetStockIconInfo(SIID_RECYCLERFULL, SHGSI_ICON | SHGSI_SHELLICONSIZE, &sii);

	int size = NcGetSystemMetrics(SM_CXICON);

	FreeBitmap(&_bmpBin);
	_bmpBin = new Bitmap(size, size);
	Graphics graphics(_bmpBin);
	graphics.Clear(Color(0, 0, 0, 0));

	if (sii.hIcon)
	{
		BITMAPINFO bi = {};
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = size;
		bi.bmiHeader.biHeight = -size;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;

		void* pBits = NULL;
		HBITMAP hDIB = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &pBits, NULL, 0);

		if (hDIB && pBits)
		{
			HDC hdcMem = CreateCompatibleDC(NULL);
			HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hDIB);

			DrawIconEx(hdcMem, 0, 0, sii.hIcon, size, size, 0, NULL, DI_NORMAL);
			Bitmap tempBmp(size, size, size * 4, PixelFormat32bppPARGB, (BYTE*)pBits);
			graphics.DrawImage(&tempBmp, 0, 0);

			// cleanup
			SelectObject(hdcMem, hOldBmp);
			DeleteDC(hdcMem);
			DeleteObject(hDIB);
		}
		DestroyIcon(sii.hIcon);
	}

	return bRet ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderSolidColor()
{
	COLORREF clr = GetDeskopColor();
	SolidBrush backgroundBrush(Color(SPLIT_COLORREF(clr)));

	FreeBitmap(&_bmpSolidColor);
	_bmpSolidColor = new Bitmap(_rcPreview.Width, _rcPreview.Height);
	Graphics graphics(_bmpSolidColor);

	return Ok == graphics.FillRectangle(&backgroundBrush, _rcPreview) ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderCaption(Graphics* pGraphics, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	FRAMESTATES frameState = wndInfo.wndType == WT_INACTIVE ? FS_INACTIVE : FS_ACTIVE;

	// caption frame
	if (_fIsThemed)
	{
		hr = DrawThemeBackground(_hWndTheme, hdc, WP_CAPTION, frameState, &_rcBounds[0], NULL);
	}
	else
	{
		// do we have gradients
		BOOL fGradients = FALSE;
		SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &fGradients, 0);

		if (fGradients)
		{
			COLORREF clrCaption = NcGetSysColor(wndInfo.wndType == WT_INACTIVE ? COLOR_INACTIVECAPTION : COLOR_ACTIVECAPTION);
			COLORREF clrGradient = NcGetSysColor(wndInfo.wndType == WT_INACTIVE ? COLOR_GRADIENTINACTIVECAPTION : COLOR_GRADIENTACTIVECAPTION);

			TRIVERTEX tex[2] = {
				{
					_rcBounds[0].left,
					_rcBounds[0].top,
					(COLOR16)(GetRValue(clrCaption) << 8),
					(COLOR16)(GetGValue(clrCaption) << 8),
					(COLOR16)(GetBValue(clrCaption) << 8),
				},
				{
					_rcBounds[0].right,
					_rcBounds[0].bottom,
					(COLOR16)(GetRValue(clrGradient) << 8),
					(COLOR16)(GetGValue(clrGradient) << 8),
					(COLOR16)(GetBValue(clrGradient) << 8),
				}
			};

			GRADIENT_RECT rect = { 0,1 };
			hr = GradientFill(hdc, tex, ARRAYSIZE(tex), &rect, 1, GRADIENT_FILL_RECT_H) == TRUE ? S_OK : E_FAIL;
		}
		else
		{
			HBRUSH br = NcGetSysColorBrush(wndInfo.wndType == WT_INACTIVE ? COLOR_INACTIVECAPTION : COLOR_ACTIVECAPTION);
			FillRect(hdc, &_rcBounds[0], br);
			if (selectedTheme->selectedScheme) DeleteBrush(br);
		}

		// 1px border below caption
		HPEN pen = CreatePen(PS_SOLID, 1, NcGetSysColor(COLOR_3DFACE));
		HPEN oldPen = (HPEN)SelectObject(hdc, pen);

		_marFrame.cyTopHeight += NcGetSystemMetrics(SM_CXBORDER);

		POINT pt[2] = {
			{ _rcBounds[0].left, _rcBounds[0].bottom },
			{ _rcBounds[0].right, _rcBounds[0].bottom }
		};
		Polyline(hdc, pt, ARRAYSIZE(pt));

		SelectObject(hdc, oldPen);
		DeletePen(pen);
	}
	RETURN_IF_FAILED(hr);

	hr = _RenderCaptionText(hdc, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderCaptionButtons(hdc, wndInfo);
	RETURN_IF_FAILED(hr);

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderCaptionButtons(HDC hdc, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	CLOSEBUTTONSTATES btnState = wndInfo.wndType == WT_INACTIVE ? (CLOSEBUTTONSTATES)5 : CBS_NORMAL;

	_fIsThemed ? DrawThemeBackground(_hWndTheme, hdc, WP_CLOSEBUTTON, btnState, &_rcBounds[2], NULL)
		: NcDrawFrameControl(hdc, &_rcBounds[2], DFC_CAPTION, 1) == TRUE ? S_OK : E_FAIL;

	if (wndInfo.wndType != WT_MESSAGEBOX)
	{
		// max button
		_fIsThemed ? DrawThemeBackground(_hWndTheme, hdc, WP_MAXBUTTON, btnState, &_rcBounds[3], NULL)
			: NcDrawFrameControl(hdc, &_rcBounds[3], DFC_CAPTION, 2) == TRUE ? S_OK : E_FAIL;

		// min button
		_fIsThemed ? DrawThemeBackground(_hWndTheme, hdc, WP_MINBUTTON, btnState, &_rcBounds[4], NULL)
			: NcDrawFrameControl(hdc, &_rcBounds[4], DFC_CAPTION, 3) == TRUE ? S_OK : E_FAIL;
	}

	return hr;
}

HRESULT CWindowPreview::_RenderCaptionText(HDC hdc, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;

	// set proper font
	LOGFONT font{};
	if (_fIsThemed)
	{
		GetThemeSysFont(_hWndTheme, TMT_CAPTIONFONT, &font);
	}
	else
	{
		if (selectedTheme->selectedScheme)
		{
			font = selectedTheme->selectedScheme->ncm.lfCaptionFont;
			ScaleLogFont(font, _dpiWindow);
		}
		else
		{
			NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
			font = ncm.lfCaptionFont;
		}
	}

	HFONT fon = CreateFontIndirect(&font);
	HFONT hOldFont = (HFONT)SelectObject(hdc, fon);
	SetBkMode(hdc, TRANSPARENT);

	int id = 1449 + (int)wndInfo.wndType;
	WCHAR szText[20];
	LoadString(g_hThemeUI, id, szText, ARRAYSIZE(szText));

	// get height
	RECT rcheight = { 0 };
	DrawText(hdc, szText, -1, &rcheight, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT);

	_rcBounds[1].top -= RECTHEIGHT(rcheight);
	_rcBounds[1].bottom += RECTHEIGHT(rcheight);

	bool fIsInactive = wndInfo.wndType == WT_INACTIVE;
	CAPTIONSTATES frameState = fIsInactive ? CS_INACTIVE : CS_ACTIVE;

	COLORREF clr = _fIsThemed ? GetThemeSysColor(_hWndTheme, fIsInactive ? COLOR_INACTIVECAPTIONTEXT : COLOR_CAPTIONTEXT)
		: NcGetSysColor(fIsInactive ? COLOR_INACTIVECAPTIONTEXT : COLOR_CAPTIONTEXT);

	if (_fIsThemed)
	{
		DTTOPTS dt = { sizeof(dt) };
		dt.dwFlags = DTT_TEXTCOLOR;
		dt.crText = clr;
		hr = DrawThemeTextEx(_hWndTheme, hdc, WP_CAPTION, frameState, szText, -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &_rcBounds[1], &dt);
	}
	else
	{
		SetTextColor(hdc, clr);
		hr = DrawText(hdc, szText, -1, &_rcBounds[1], DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER);
	}
	RETURN_IF_FAILED(hr);

	SelectObject(hdc, hOldFont);
	DeleteObject(fon);
	return hr;
}

HRESULT CWindowPreview::_RenderScrollbar(Graphics* pGraphics, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	if (wndInfo.wndType != WT_ACTIVE) return hr;

	HDC hdc = pGraphics->GetHDC();
	int height = _fIsThemed ? max(GetThemeSysSize(_hWndTheme, SM_CYVSCROLL), _sizeScrollbar.cy) : NcGetSystemMetrics(SM_CYVSCROLL);

	// scroll bar background
	RECT crc = _rcBounds[9];
	if (_fIsThemed)
	{
		DrawThemeBackground(_hScrlTheme, hdc, SBP_LOWERTRACKVERT, SCRBS_NORMAL, &crc, 0);
	}
	else
	{
		HBRUSH br = NcGetSysColorBrush(COLOR_SCROLLBAR);
		FillRect(hdc, &crc, br);
		if (selectedTheme->selectedScheme) DeleteBrush(br);
	}

	// up button
	crc.bottom = crc.top + height;
	_fIsThemed ? DrawThemeBackground(_hScrlTheme, hdc, SBP_ARROWBTN, ABS_UPNORMAL, &crc, 0)
		: NcDrawFrameControl(hdc, &crc, DFC_SCROLL, 1) == TRUE ? S_OK : E_FAIL;

	// scroll thumb 
	OffsetRect(&crc, 0, height);
	if (_fIsThemed) DrawThemeBackground(_hScrlTheme, hdc, SBP_THUMBBTNVERT, SCRBS_NORMAL, &crc, 0);

	// down button
	crc = _rcBounds[9];
	crc.top = crc.bottom - height;
	_fIsThemed ? DrawThemeBackground(_hScrlTheme, hdc, SBP_ARROWBTN, ABS_DOWNNORMAL, &crc, 0)
		: NcDrawFrameControl(hdc, &crc, DFC_SCROLL, 2) == TRUE ? S_OK : E_FAIL;

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderFrame(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	FRAMESTATES frameState = wndInfo.wndType == WT_INACTIVE ? FS_INACTIVE : FS_ACTIVE;

	// todo: split this ??
	RECT crc = wndInfo.wndPos;
	if (!_fIsThemed)
	{
		bool fIsMessageBox = wndInfo.wndType == WT_MESSAGEBOX;

		if (fIsMessageBox)
		{
			int offset = NcGetSystemMetrics(SM_CXBORDER);
			InflateRect(&crc, -offset, -offset);
			crc.bottom += offset - 1;
			crc.right += offset;
		}
		DrawEdge(hdc, &crc, EDGE_RAISED, BF_RECT);

		if (!fIsMessageBox)
		{
			int count = NcGetSystemMetrics(SM_CXBORDER);
			InflateRect(&crc, -NcGetSystemMetrics(SM_CXEDGE), -NcGetSystemMetrics(SM_CXEDGE));
			for (int i = 0; i < count; i++)
			{
				bool fInactiveWnd = wndInfo.wndType == WT_INACTIVE;
				// framerect draws a 1px border
				HBRUSH br = NcGetSysColorBrush(fInactiveWnd ? COLOR_INACTIVEBORDER : COLOR_ACTIVEBORDER);
				FrameRect(hdc, &crc, br);
				InflateRect(&crc, -1, -1);

				if (selectedTheme->selectedScheme) DeleteBrush(br);
			}
		}
		else
		{
			int offset = NcGetSystemMetrics(SM_CXEDGE);
			InflateRect(&crc, -offset, -offset);
		}
		HBRUSH br = NcGetSysColorBrush(COLOR_3DFACE);
		FrameRect(hdc, &crc, br);
		InflateRect(&crc, -1, -1);

		if (selectedTheme->selectedScheme) DeleteBrush(br);
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
		: NcGetSysColor(fIsMessageBox ? COLOR_3DFACE : COLOR_WINDOW);
	
	if (!_fIsThemed && !fIsMessageBox)
	{
		// QUIRK: same behaviour as old desk.cpl
		if (wndInfo.wndType == WT_ACTIVE)
		{
			DrawEdge(hdc, &_rcBounds[5], EDGE_SUNKEN, BF_RECT);

			// 1px border above edge
			HPEN pen = CreatePen(PS_SOLID, 1, NcGetSysColor(COLOR_3DFACE));
			HPEN oldPen = (HPEN)SelectObject(hdc, pen);

			POINT pt[2]{};
			pt[0] = { _rcBounds[5].left, _rcBounds[5].top - 1 };
			pt[1] = { _rcBounds[5].right, _rcBounds[5].top - 1 };
			Polyline(hdc, pt, ARRAYSIZE(pt));

			SelectObject(hdc, oldPen);
			DeletePen(pen);
		}
	}

	// idk how it works but u cant use gdi+ to draw a rect, and then use gdi to draw the text on it
	// QUIRK: same behaviour as old desk.cpl
	if (_fIsThemed || (wndInfo.wndType != WT_INACTIVE && !_fIsThemed))
	{
		HBRUSH hbr = CreateSolidBrush(clr);
		FillRect(hdc, &_rcBounds[6], hbr);
		DeleteObject(hbr);
	}

	LOGFONT font{};
	if (_fIsThemed)
	{
		GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &font);
	}
	else
	{
		if (selectedTheme->selectedScheme)
		{
			font = selectedTheme->selectedScheme->ncm.lfMessageFont;
			ScaleLogFont(font, _dpiWindow);
		}
		else
		{
			NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
			font = ncm.lfMessageFont;
		}
	}

	if (wndInfo.wndType == WT_ACTIVE)
	{
		if (!_fIsThemed) font.lfWeight = FW_BOLD;
		HFONT fon = CreateFontIndirect(&font);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		WCHAR szText[20];
		LoadString(g_hThemeUI, 1460, szText, ARRAYSIZE(szText));

		RECT crc = _rcBounds[6];

		COLORREF clr = _fIsThemed ? GetThemeSysColor(hTheme, COLOR_WINDOWTEXT) : NcGetSysColor(COLOR_WINDOWTEXT);
		if (_fIsThemed)
		{
			DTTOPTS dt = { sizeof(dt) };
			dt.dwFlags = DTT_TEXTCOLOR;
			dt.crText = clr;
			hr = DrawThemeTextEx(hTheme, hdc, 0, 0, szText, -1, DT_LEFT | DT_TOP | DT_SINGLELINE, &crc, &dt);
		}
		else
		{
			crc.left += 1;
			SetTextColor(hdc, clr);
			hr = DrawText(hdc, szText, -1, &crc, DT_LEFT | DT_TOP | DT_SINGLELINE);
		}
		RETURN_IF_FAILED(hr);

		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	if (wndInfo.wndType == WT_MESSAGEBOX)
	{
		HFONT fon = CreateFontIndirect(&font);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		// load button theme
		if (_fIsThemed)
		{
			HTHEME hThemeBtn = OpenNcThemeData(_hTheme, L"Button");
			DrawThemeBackground(hThemeBtn, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, &_rcBounds[7], NULL);
			CloseThemeData(hThemeBtn);
		}
		else
		{
			COLORREF clr = NcGetSysColor(COLOR_WINDOWTEXT);
			SetTextColor(hdc, clr);

			WCHAR szText[20];
			LoadString(g_hThemeUI, 1461, szText, ARRAYSIZE(szText));

			RECT crc = _rcBounds[6];
			crc.left += NcGetSystemMetrics(SM_CXEDGE) + 1;
			DrawText(hdc, szText, -1, &crc, DT_LEFT | DT_TOP | DT_SINGLELINE);

			NcDrawFrameControl(hdc, &_rcBounds[7], DFC_BUTTON, DFCS_BUTTONPUSH);
		}

		COLORREF clr = _fIsThemed ? GetThemeSysColor(hTheme, COLOR_BTNTEXT) : NcGetSysColor(COLOR_BTNTEXT);
		SetTextColor(hdc, clr);

		WCHAR szText[5];
		LoadString(g_hThemeUI, 1458, szText, ARRAYSIZE(szText));
		DrawText(hdc, szText, -1, &_rcBounds[7], DT_CENTER | DT_TOP | DT_SINGLELINE | DT_VCENTER);

		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderMenuBar(Gdiplus::Graphics* pGraphics, MYWINDOWINFO wndInfo)
{
	if (wndInfo.wndType != WT_ACTIVE || _fIsThemed) return S_OK;
	RECT crc = _rcBounds[8];
	
	HDC dc = pGraphics->GetHDC();
	HBRUSH br = NcGetSysColorBrush(COLOR_MENU);
	FillRect(dc, &crc, br);
	if (selectedTheme->selectedScheme) DeleteBrush(br);

	InflateRect(&crc, 0, -1);
	_RenderMenuItem(dc, &crc, 1);
	_RenderMenuItem(dc, &crc, 2);
	_RenderMenuItem(dc, &crc, 3);

	pGraphics->ReleaseHDC(dc);
	return S_OK;
}

HRESULT CWindowPreview::_RenderMenuItem(HDC hdc, RECT* rc, int type)
{
	int id = 1454 + (type - 1);

	WCHAR szText[20];
	LoadString(g_hThemeUI, id, szText, ARRAYSIZE(szText));

	LOGFONT lf;
	if (selectedTheme->selectedScheme)
	{
		lf = selectedTheme->selectedScheme->ncm.lfMenuFont;
		ScaleLogFont(lf, _dpiWindow);
	}
	else
	{
		NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
		lf = ncm.lfMenuFont;
	}

	HFONT fon = CreateFontIndirect(&lf);
	HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

	// get height
	RECT rcheight = { 0,0,0,0 };
	DrawText(hdc, szText, -1, &rcheight, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT);

	// 7 padding on either side of the text
	rc->right = rc->left + RECTWIDTH(rcheight) + (7 * 2);
	if (type == 3) DrawEdge(hdc, rc, BDR_SUNKENOUTER, BF_RECT);

	if (type == 2 || type == 3)
	{
		OffsetRect(rc, 1, 1);
	}
	COLORREF clr = type == 2 ? RGB(255, 255, 255) : NcGetSysColor(COLOR_MENUTEXT);
	SetTextColor(hdc, clr);
	DrawText(hdc, szText, -1, rc, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_VCENTER);
	if (type == 2)
	{
		OffsetRect(rc, -1, -1);

		// todo: fix green text in high contrast
		clr = NcGetSysColor(COLOR_GRAYTEXT);
		SetTextColor(hdc, clr);
		DrawText(hdc, szText, -1, rc, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_VCENTER);

	}

	SelectObject(hdc, hOldFont);
	DeleteObject(fon);

	// update the rectangle
	rc->left += RECTWIDTH(rcheight) + (7 * 2) + 1;
	return S_OK;
}
