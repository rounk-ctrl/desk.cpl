#include "pch.h"
#include "wndprvw.h"
#include "helper.h"
#include "uxtheme.h"

using namespace Gdiplus;

#define SPLIT_COLORREF(clr) GetRValue(clr), GetGValue(clr), GetBValue(clr)

CWindowPreview::CWindowPreview(SIZE const& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType, LPVOID hTheme)
{
	_pwndInfo = pwndInfo;
	_wndInfoCount = wndInfoCount;
	_sizePreview = sizePreview;
	_pageType = pageType;
	_hTheme = hTheme;
}

CWindowPreview::~CWindowPreview()
{
}

HRESULT CWindowPreview::GetPreviewImage(HBITMAP* pbOut)
{
	Bitmap* gdiBmp = new Bitmap(GETSIZE(_sizePreview), PixelFormat32bppARGB);
	RETURN_IF_NULL_ALLOC(gdiBmp);

	Graphics graphics(gdiBmp);
	RETURN_IF_NULL_ALLOC(&graphics);
	graphics.SetInterpolationMode(InterpolationModeInvalid);

	HRESULT hr = _RenderSolidColor(&graphics);
	RETURN_IF_FAILED(hr);

	if (_pageType != PT_APPEARANCE)
	{
		hr = _RenderWallpaper(&graphics);
		RETURN_IF_FAILED(hr);
	}

	if (_pageType == PT_THEMES)
	{
		hr = _RenderBin(&graphics);
		RETURN_IF_FAILED(hr);
	}
	
	if (_wndInfoCount > 0)
	{
		for (int i = 0; i < _wndInfoCount; ++i)
		{
			hr = _RenderWindow(_pwndInfo[i], &graphics);
			RETURN_IF_FAILED(hr);
		}
	}

	// create hbitmap
	hr = gdiBmp->GetHBITMAP(Color(0, 0, 0), pbOut) == Ok ? S_OK : E_FAIL;
	return hr;
}

HRESULT CWindowPreview::_RenderWindow(MYWINDOWINFO wndInfo, Graphics* pGraphics)
{
	HRESULT hr = S_OK;
	HTHEME hTheme = _hTheme ? OpenThemeDataFromFile(_hTheme, NULL, L"Window", 0) : OpenThemeData(NULL, L"Window");
	
	hr = _RenderFrame(pGraphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderCaption(pGraphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	hr = _RenderContent(pGraphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	return hr;
}

HRESULT CWindowPreview::_RenderWallpaper(Graphics* pGraphics)
{
	// todo: adjust wallpaper based on fit type 
	Rect rect(0, 0, GETSIZE(_sizePreview));
	// _AdjustRectForPreview(&rect);

	Bitmap* bitmap = Bitmap::FromFile(selectedTheme->wallpaperPath, FALSE);
	RETURN_IF_NULL_ALLOC(bitmap);
	
	return pGraphics->DrawImage(bitmap, rect) == Ok ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderBin(Graphics* pGraphics)
{
	BOOL bRet = FALSE;
	SHSTOCKICONINFO sii = { sizeof(sii) };
	SHGetStockIconInfo(SIID_RECYCLERFULL, SHGSI_ICON | SHGSI_SHELLICONSIZE, &sii);

	HDC hdcgraphic = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdcgraphic);
	if (sii.hIcon)
	{
		// fix this
		bRet = DrawIconEx(hdcgraphic, _sizePreview.cx - 48, _sizePreview.cy - 40, sii.hIcon, 32, 32, 0, NULL, DI_NORMAL);
	}
	pGraphics->ReleaseHDC(hdcgraphic);
	return bRet ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderSolidColor(Graphics* pGraphics)
{
	// todo: move selectedTheme to this class
	COLORREF clr = selectedTheme->newColor ? selectedTheme->newColor : GetSysColor(COLOR_BACKGROUND);
	SolidBrush backgroundBrush(Color(SPLIT_COLORREF(clr)));

	return Ok == pGraphics->FillRectangle(&backgroundBrush, Rect(0, 0, GETSIZE(_sizePreview))) ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderCaption(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	FRAMESTATES frameState = wndInfo.wndType == WT_INACTIVE ? FS_INACTIVE : FS_ACTIVE;

	// caption frame
	RECT crc = wndInfo.wndPos;
	crc.bottom = crc.top + _marFrame.cyTopHeight;
	hr = DrawThemeBackground(hTheme, hdc, WP_CAPTION, frameState, &crc, NULL);
	RETURN_IF_FAILED(hr);

	hr = _RenderCaptionText(hdc, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderCaptionText(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	CAPTIONSTATES frameState = wndInfo.wndType == WT_INACTIVE ? CS_INACTIVE : CS_ACTIVE;

	MARGINS mar;
	hr = GetThemeMargins(hTheme, hdc, WP_CAPTION, frameState, TMT_CAPTIONMARGINS, NULL, &mar);
	RETURN_IF_FAILED(hr);

	// set proper font
	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

	HFONT fon = CreateFontIndirect(&ncm.lfCaptionFont);
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

	RECT rc;
	hr = GetThemeTextExtent(hTheme, hdc, WP_CAPTION, frameState, text, -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &wndInfo.wndPos, &rc);
	RETURN_IF_FAILED(hr);

	rc.left += _marFrame.cxLeftWidth + mar.cxLeftWidth;
	rc.right += _marFrame.cxLeftWidth + mar.cxLeftWidth;
	rc.top -= _marFrame.cyBottomHeight - 1;
	rc.bottom -= _marFrame.cyBottomHeight - 1;

	// get height
	RECT rcheight = { 0,0,0,0 };
	DrawText(hdc, text, -1, &rcheight, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_CALCRECT);

	rc.top += _marFrame.cyTopHeight - _marFrame.cyBottomHeight -4;
	rc.bottom = rc.top + RECTHEIGHT(rcheight);

	DTTOPTS dt = { sizeof(dt) };
	dt.dwFlags = DTT_TEXTCOLOR | DTT_COLORPROP;
	dt.crText = RGB(255, 255, 255);
	dt.iColorPropId = TMT_TEXTCOLOR;
	hr = DrawThemeTextEx(hTheme, hdc, WP_CAPTION, frameState, text, -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &rc, &dt);
	RETURN_IF_FAILED(hr);

	SelectObject(hdc, hOldFont);
	DeleteObject(fon);
	return hr;
}

HRESULT CWindowPreview::_RenderScrollbar(Gdiplus::Graphics* pGraphics)
{
	return E_NOTIMPL;
}

HRESULT CWindowPreview::_RenderFrame(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	// calculate frame margins
	// probably make another function
	_marFrame.cxLeftWidth = GetSystemMetrics(SM_CXPADDEDBORDER) + GetSystemMetrics(SM_CXFRAME);
	_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
	_marFrame.cyTopHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
	_marFrame.cyBottomHeight = GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);

	FRAMESTATES frameState = wndInfo.wndType == WT_INACTIVE ? FS_INACTIVE : FS_ACTIVE;

	// todo: split this ??
	RECT crc = wndInfo.wndPos;
	crc.top = crc.bottom + _marFrame.cyTopHeight;
	crc.bottom = crc.top + _marFrame.cyBottomHeight;
	hr = DrawThemeBackground(hTheme, hdc, WP_FRAMEBOTTOM, frameState, &crc, NULL);
	RETURN_IF_FAILED(hr);

	crc = wndInfo.wndPos;
	crc.top += _marFrame.cyTopHeight;
	crc.right = crc.left + _marFrame.cxLeftWidth;
	crc.bottom += _marFrame.cyTopHeight;
	hr = DrawThemeBackground(hTheme, hdc, WP_FRAMELEFT, frameState, &crc, NULL);
	RETURN_IF_FAILED(hr);

	crc.left = wndInfo.wndPos.right - _marFrame.cxRightWidth;
	crc.right = wndInfo.wndPos.right;
	hr = DrawThemeBackground(hTheme, hdc, WP_FRAMERIGHT, frameState, &crc, NULL);
	RETURN_IF_FAILED(hr);

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderContent(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);
	BOOL fIsMessageBox = wndInfo.wndType == WT_MESSAGEBOX;

	COLORREF clr = GetSysColor(fIsMessageBox ? COLOR_3DFACE : COLOR_WINDOW);
	if (!fIsMessageBox) GetThemeColor(hTheme, 0, 0, TMT_FILLCOLOR, &clr);

	RECT crc = wndInfo.wndPos;
	crc.left += _marFrame.cxLeftWidth;
	crc.top += _marFrame.cyTopHeight;
	crc.bottom += _marFrame.cyTopHeight;
	crc.right -= _marFrame.cxRightWidth;

	// idk how it works but u cant use gdi+ to draw a rect, and then use gdi to draw the text on it
	FillRect(hdc, &crc, CreateSolidBrush(clr));

	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

	if (wndInfo.wndType == WT_ACTIVE)
	{
		HFONT fon = CreateFontIndirect(&ncm.lfMessageFont);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		RECT crc = wndInfo.wndPos;
		crc.top += _marFrame.cyTopHeight;
		crc.bottom = crc.top + 10;
		crc.left += _marFrame.cxLeftWidth;
		DrawThemeText(hTheme, hdc, 0, 0, L"Window Text", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, 0, &crc);

		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	if (wndInfo.wndType == WT_MESSAGEBOX)
	{
		HFONT fon = CreateFontIndirect(&ncm.lfMessageFont);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		RECT crc = wndInfo.wndPos;
		crc.top += _marFrame.cyTopHeight;
		crc.left += _marFrame.cxLeftWidth;
		crc.right -= _marFrame.cxRightWidth;

		int horPos = RECTWIDTH(crc) / 2;
		int verPos = RECTHEIGHT(crc) / 2;

		crc.top += verPos + 2;
		crc.bottom += verPos - 7;
		crc.left += 38;
		crc.right -= 38;

		// load button theme
		HTHEME hThemeBtn = _hTheme ? OpenThemeDataFromFile(_hTheme, NULL, L"Button", 0) : OpenThemeData(NULL, L"Button");
		DrawThemeBackground(hThemeBtn, hdc, BP_PUSHBUTTON, PBS_DEFAULTED, &crc, NULL);
		DrawThemeText(hTheme, hdc, 0, 0, L"OK", -1, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_VCENTER, 0, &crc);

		CloseThemeData(hThemeBtn);
		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	pGraphics->ReleaseHDC(hdc);
	return hr;
}
