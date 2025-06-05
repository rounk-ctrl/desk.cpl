#include "pch.h"
#include "wndprvw.h"
#include "helper.h"

using namespace Gdiplus;

#define SPLIT_COLORREF(clr) GetRValue(clr), GetGValue(clr), GetBValue(clr)

CWindowPreview::CWindowPreview(SIZE& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType)
{
	_pwndInfo = pwndInfo;
	_wndInfoCount = wndInfoCount;
	_sizePreview = sizePreview;
	_pageType = pageType;
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

	_RenderSolidColor(&graphics);

	if (_pageType != PT_APPEARANCE)
	{
		_RenderWallpaper(&graphics);
	}

	if (_pageType == PT_THEMES)
	{
		_RenderBin(&graphics);
	}
	
	if (_wndInfoCount > 0)
	{
		for (int i = 0; i < _wndInfoCount; ++i)
		{
			_RenderWindow(_pwndInfo[i], &graphics);
		}
	}

	// create hbitmap
	RETURN_IF_NULL_ALLOC(*pbOut);
	return gdiBmp->GetHBITMAP(Color(0, 0, 0), pbOut) == Ok ? S_OK : E_FAIL;
}

HRESULT CWindowPreview::_RenderWindow(MYWINDOWINFO wndInfo, Graphics* pGraphics)
{
	return E_NOTIMPL;
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
	SHSTOCKICONINFO sii = { sizeof(sii) };
	SHGetStockIconInfo(SIID_RECYCLERFULL, SHGSI_ICON | SHGSI_SHELLICONSIZE, &sii);

	BOOL bRet = FALSE;

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
