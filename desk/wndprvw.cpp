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
	
	// hell nah never not initializing a variable
	_marFrame = {};
	_marMonitor = {};
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
	Bitmap* gdiBmp = new Bitmap(GETSIZE(_sizePreview), PixelFormat32bppARGB);
	RETURN_IF_NULL_ALLOC(gdiBmp);

	Graphics graphics(gdiBmp);
	RETURN_IF_NULL_ALLOC(&graphics);
	graphics.SetInterpolationMode(InterpolationModeInvalid);

	HRESULT hr = S_OK;

	if (_pageType == PT_BACKGROUND || _pageType == PT_SCRSAVER)
	{
		hr = _DrawMonitor(&graphics);
		RETURN_IF_FAILED(hr);
	}

	if (_pageType == PT_SCRSAVER)
	{
		hr = _DesktopScreenShooter(&graphics);
		//return hr;
	}

	if (_pageType != PT_SCRSAVER)
	{
		hr = _RenderSolidColor(&graphics);
		RETURN_IF_FAILED(hr);
	}

	if (_pageType == PT_THEMES || _pageType == PT_BACKGROUND)
	{
		hr = _RenderWallpaper(&graphics);
		LOG_IF_FAILED(hr);
		// non fatal error
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
	delete gdiBmp;
	return hr;
}

HRESULT CWindowPreview::GetUpdatedPreviewImage(MYWINDOWINFO* pwndInfo, LPVOID hTheme, HBITMAP* pbOut)
{
	if (_hTheme)
	{
		_CleanupUxThemeFile(&_hTheme);
	}
	_pwndInfo = pwndInfo;
	_hTheme = hTheme;
	return GetPreviewImage(pbOut);
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

	int cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	Rect rect(0, 0, GETSIZE(_sizePreview));
	if (_marMonitor.cxLeftWidth > 0)
	{
		rect.X += _marMonitor.cxLeftWidth;
		rect.Y += _marMonitor.cyTopHeight;
		rect.Width -= _marMonitor.cxRightWidth;
		rect.Height -= _marMonitor.cyBottomHeight;
	}

	HDC hScreenDC = ::GetDC(NULL);
	HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
	auto g_hbDesktop = CreateCompatibleBitmap(hScreenDC, cx, cy);
	SelectObject(hMemoryDC, g_hbDesktop);
	BitBlt(hMemoryDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);

	Bitmap* bm = new Bitmap(g_hbDesktop, NULL);
	hr = pGraphics->DrawImage(bm, rect) == Ok ? S_OK : E_FAIL;

	delete bm;
	DeleteObject(g_hbDesktop);
	return hr;
}

HRESULT CWindowPreview::_DrawMonitor(Gdiplus::Graphics* pGraphics)
{
	HRESULT hr = S_OK;
	Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(IDB_BITMAP1));

	// pink
	Color transparentColor(255, 255, 0, 255);
	ImageAttributes imgAttr;
	imgAttr.SetColorKey(transparentColor, transparentColor, ColorAdjustTypeBitmap);

	// draw monitor
	Rect rect(0, 10, monitor->GetWidth(), monitor->GetHeight());
	pGraphics->DrawImage(monitor, rect, 0, 0, _sizePreview.cx, _sizePreview.cy, UnitPixel, &imgAttr);

	// set monitor margins
	_marMonitor.cxLeftWidth = 15;
	_marMonitor.cyTopHeight = 25;
	_marMonitor.cxRightWidth = 37;
	_marMonitor.cyBottomHeight = 68;

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

	hr = _RenderScrollbar(pGraphics, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	CloseThemeData(hTheme);
	return hr;
}

HRESULT CWindowPreview::_RenderWallpaper(Graphics* pGraphics)
{
	HRESULT hr = S_OK;

	// todo: adjust wallpaper based on fit type 
	Rect rect(0, 0, GETSIZE(_sizePreview));
	if (_marMonitor.cxLeftWidth > 0)
	{
		rect.X += _marMonitor.cxLeftWidth;
		rect.Y += _marMonitor.cyTopHeight;
		rect.Width -= _marMonitor.cxRightWidth;
		rect.Height -= _marMonitor.cyBottomHeight;
	}
	// _AdjustRectForPreview(&rect);

	Bitmap* bitmap = Bitmap::FromFile(selectedTheme->wallpaperPath, FALSE);
	RETURN_IF_NULL_ALLOC(bitmap);

	hr = pGraphics->DrawImage(bitmap, rect) == Ok ? S_OK : E_FAIL;
	delete bitmap;
	return hr;
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
	COLORREF clr{};
	if (selectedTheme->newColor)
	{
		clr = selectedTheme->newColor;
	}
	else if (selectedTheme->useDesktopColor)
	{
		pDesktopWallpaper->GetBackgroundColor(&clr);
	}
	else
	{
		ITheme* themeClass = new ITheme(currentITheme);
		themeClass->GetBackgroundColor(&clr);
	}
	SolidBrush backgroundBrush(Color(SPLIT_COLORREF(clr)));

	Rect rect(0, 0, GETSIZE(_sizePreview));
	if (_marMonitor.cxLeftWidth > 0)
	{
		rect.X += _marMonitor.cxLeftWidth;
		rect.Y += _marMonitor.cyTopHeight;
		rect.Width -= _marMonitor.cxRightWidth;
		rect.Height -= _marMonitor.cyBottomHeight;
	}

	return Ok == pGraphics->FillRectangle(&backgroundBrush, rect) ? S_OK : E_FAIL;
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

	hr = _RenderCaptionButtons(hdc, hTheme, wndInfo);
	RETURN_IF_FAILED(hr);

	pGraphics->ReleaseHDC(hdc);
	return hr;
}

HRESULT CWindowPreview::_RenderCaptionButtons(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;

	SIZE size = { 0 };
	GetThemePartSize(hTheme, hdc, WP_CLOSEBUTTON, CBS_NORMAL, NULL, TS_TRUE, &size);

	// uxtheme.dll _GetNcBtnMetrics 
	int cxEdge = GetSystemMetrics(SM_CXEDGE);
	int cyEdge = GetSystemMetrics(SM_CYEDGE);

	int cyBtn = GetThemeSysSize(hTheme, SM_CYSIZE);
	int cxBtn = MulDiv(cyBtn, size.cx, size.cy);

	// remove padding
	cyBtn -= (cyEdge * 2);
	cxBtn -= (cyEdge * 2);

	RECT crc = wndInfo.wndPos;
	crc.right -= _marFrame.cxRightWidth + cxEdge;
	crc.left = crc.right - cxBtn;
	crc.top += _marFrame.cyTopHeight - GetSystemMetrics(SM_CYFRAME) - cyBtn;
	crc.bottom = crc.top + cyBtn;
	DrawThemeBackground(hTheme, hdc, WP_CLOSEBUTTON, CBS_NORMAL, &crc, NULL);

	
	if (wndInfo.wndType != WT_MESSAGEBOX)
	{
		// max button
		int width = cxBtn + cxEdge;
		crc.left -= width;
		crc.right -= width;
		DrawThemeBackground(hTheme, hdc, WP_MAXBUTTON, MAXBS_NORMAL, &crc, NULL);

		// min button
		crc.left -= width;
		crc.right -= width;
		DrawThemeBackground(hTheme, hdc, WP_MINBUTTON, MINBS_NORMAL, &crc, NULL);
	}
	
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
	LOGFONT font{};
	GetThemeSysFont(hTheme, TMT_CAPTIONFONT, &font);

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

	rc.top += _marFrame.cyTopHeight + _marFrame.cyBottomHeight - GetSystemMetrics(SM_CYFRAME) - RECTHEIGHT(rcheight);
	rc.bottom = rc.top + RECTHEIGHT(rcheight);

	COLORREF clr = GetThemeSysColor(hTheme, COLOR_CAPTIONTEXT);

	DTTOPTS dt = { sizeof(dt) };
	dt.dwFlags = DTT_TEXTCOLOR;
	dt.crText = clr;
	hr = DrawThemeTextEx(hTheme, hdc, WP_CAPTION, frameState, text, -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &rc, &dt);
	RETURN_IF_FAILED(hr);

	SelectObject(hdc, hOldFont);
	DeleteObject(fon);
	return hr;
}

HRESULT CWindowPreview::_RenderScrollbar(Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo)
{
	HRESULT hr = S_OK;
	if (wndInfo.wndType != WT_ACTIVE) return hr;

	HTHEME hThemeScrl = _hTheme ? OpenThemeDataFromFile(_hTheme, NULL, L"Scrollbar", 0) : OpenThemeData(NULL, L"Scrollbar");
	HDC hdc = pGraphics->GetHDC();
	RETURN_IF_NULL_ALLOC(hdc);

	RECT crc = wndInfo.wndPos;
	crc.top += _marFrame.cyTopHeight;

	SIZE size = { 0 };
	GetThemePartSize(hThemeScrl, hdc, SBP_ARROWBTN, ABS_UPNORMAL, NULL, TS_TRUE, &size);

	int width = max(GetThemeSysSize(hTheme, SM_CXVSCROLL), size.cx);
	int height = max(GetThemeSysSize(hTheme, SM_CXVSCROLL), size.cy);
	crc.left = crc.right - _marFrame.cxRightWidth - width;
	crc.right = crc.left + width;
	crc.bottom += _marFrame.cyTopHeight;
	DrawThemeBackground(hThemeScrl, hdc, SBP_LOWERTRACKVERT, SCRBS_NORMAL, &crc, 0);

	crc.bottom = crc.top + height;
	DrawThemeBackground(hThemeScrl, hdc, SBP_ARROWBTN, ABS_UPNORMAL, &crc, 0);

	crc.top += height;
	crc.bottom = crc.top + height;
	DrawThemeBackground(hThemeScrl, hdc, SBP_THUMBBTNVERT, SCRBS_NORMAL, &crc, 0);

	crc.top = wndInfo.wndPos.bottom + _marFrame.cyTopHeight - height;
	crc.bottom = crc.top + height;
	DrawThemeBackground(hThemeScrl, hdc, SBP_ARROWBTN, ABS_DOWNNORMAL, &crc, 0);

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
	int cxPaddedBorder = GetThemeSysSize(hTheme, SM_CXPADDEDBORDER);
	int cyCaptionHeight = GetThemeSysSize(hTheme, SM_CYSIZE) + cxPaddedBorder +2; // i think
	_marFrame.cxLeftWidth = cxPaddedBorder + GetSystemMetrics(SM_CXFRAME);
	_marFrame.cxRightWidth = _marFrame.cxLeftWidth;
	_marFrame.cyTopHeight = cyCaptionHeight + GetSystemMetrics(SM_CYFRAME);
	_marFrame.cyBottomHeight = GetSystemMetrics(SM_CYFRAME) + cxPaddedBorder - 2;

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

	COLORREF clr =  GetThemeSysColor(hTheme, fIsMessageBox ? COLOR_3DFACE : COLOR_WINDOW);

	RECT crc = wndInfo.wndPos;
	crc.left += _marFrame.cxLeftWidth;
	crc.top += _marFrame.cyTopHeight;
	crc.bottom += _marFrame.cyTopHeight;
	crc.right -= _marFrame.cxRightWidth;

	// idk how it works but u cant use gdi+ to draw a rect, and then use gdi to draw the text on it
	FillRect(hdc, &crc, CreateSolidBrush(clr));

	LOGFONT font{};
	GetThemeSysFont(hTheme, TMT_MSGBOXFONT, &font);

	if (wndInfo.wndType == WT_ACTIVE)
	{
		HFONT fon = CreateFontIndirect(&font);
		HFONT hOldFont = (HFONT)SelectObject(hdc, fon);

		RECT crc = wndInfo.wndPos;
		crc.top += _marFrame.cyTopHeight;
		crc.bottom = crc.top + 10;
		crc.left += _marFrame.cxLeftWidth;

		COLORREF clr = GetThemeSysColor(hTheme, COLOR_WINDOWTEXT);

		DTTOPTS dt = { sizeof(dt) };
		dt.dwFlags = DTT_TEXTCOLOR;
		dt.crText = clr;
		DrawThemeTextEx(hTheme, hdc, 0, 0, L"Window Text", -1, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_VCENTER, &crc, &dt);

		SelectObject(hdc, hOldFont);
		DeleteObject(fon);
	}

	if (wndInfo.wndType == WT_MESSAGEBOX)
	{
		HFONT fon = CreateFontIndirect(&font);
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
