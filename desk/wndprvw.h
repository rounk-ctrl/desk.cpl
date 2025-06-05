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

class CWindowPreview : 
	public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
		, IWindowPreview
	>
{
public:
	CWindowPreview(SIZE& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType);
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
	HRESULT _RenderCaption(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderScrollbar(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderFrame(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderContent(Gdiplus::Graphics* pGraphics);

	MYWINDOWINFO* _pwndInfo;
	int _wndInfoCount;
	SIZE _sizePreview;
	MARGINS _marFrame;
	PAGETYPE _pageType;
};
