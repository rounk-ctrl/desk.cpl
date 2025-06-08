#pragma once
#include "pch.h"
#include "desk.h"

enum PAGETYPE
{
	PT_THEMES,
	PT_BACKGROUND,
	PT_APPEARANCE,
};

enum WINDOWTYPE
{
	WT_ACTIVE,
	WT_INACTIVE,
	WT_MESSAGEBOX,
};

struct MYWINDOWINFO
{
	WINDOWTYPE wndType;
	RECT wndPos;
};


MIDL_INTERFACE("5E7FDCC0-A398-49C0-9652-2B441A04CFCD")
IWindowPreview : IUnknown
{
	STDMETHOD(GetPreviewImage)(HBITMAP* pbOut) PURE;
};

class CWindowPreview final: 
	public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
		, IWindowPreview
	>
{
public:
	CWindowPreview(SIZE const& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType, LPVOID hTheme);
	~CWindowPreview();

	//HRESULT RuntimeClassInitialize(SIZE& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType);

	//~ Begin IWindowPreview interface
	STDMETHODIMP GetPreviewImage(HBITMAP* pbOut);
	//~ End IWindowPreview interface

private:
	HRESULT _RenderWindow(MYWINDOWINFO wndInfo, Gdiplus::Graphics* pGraphics);
	HRESULT _RenderWallpaper(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderBin(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderSolidColor(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderCaption(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderCaptionButtons(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderCaptionText(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderScrollbar(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderFrame(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderContent(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);

	MYWINDOWINFO* _pwndInfo;
	int _wndInfoCount;
	SIZE _sizePreview;
	MARGINS _marFrame;
	PAGETYPE _pageType;
	void* _hTheme;
};
