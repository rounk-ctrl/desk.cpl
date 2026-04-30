#pragma once
#include "pch.h"
#include "desk.h"

enum PAGETYPE
{
	PT_THEMES,
	PT_BACKGROUND,
	PT_SCRSAVER,
	PT_APPEARANCE,
};

enum WINDOWTYPE
{
	WT_ACTIVE = 0,
	WT_INACTIVE,
	WT_MESSAGEBOX = 7,
};

enum UPDATEFLAGS
{
	UPDATE_NONE = 0,
	UPDATE_SOLIDCLR = 1,
	UPDATE_WALLPAPER = 2,
	UPDATE_BIN = 4,
	UPDATE_WINDOW = 8,
	UPDATE_ALL = UPDATE_SOLIDCLR | UPDATE_WALLPAPER | UPDATE_BIN | UPDATE_WINDOW
};
DEFINE_ENUM_FLAG_OPERATORS(UPDATEFLAGS);

struct MYWINDOWINFO
{
	WINDOWTYPE wndType;
	RECT wndPos;
};


MIDL_INTERFACE("5E7FDCC0-A398-49C0-9652-2B441A04CFCD")
IWindowPreview : IUnknown
{
	STDMETHOD(GetPreviewImage)(HBITMAP* pbOut) = 0;
	STDMETHOD(GetUpdatedPreviewImage)(MYWINDOWINFO* pwndInfo, LPVOID hTheme, HBITMAP* pbOut, UINT flags) = 0;
};

MIDL_INTERFACE("E8EABE39-532B-442B-BE2C-E3FBEAD7AECE")
IWindowConfig : IUnknown
{
	STDMETHOD(GetMonitorOffset)(SIZE* pOut) = 0;
	STDMETHOD(SetClassicPrev)(BOOL fEnable) = 0;
};

MIDL_INTERFACE("77BE699F-116A-4354-9B31-1DE029996302")
IWindowMetrics : IUnknown
{
	STDMETHOD(GetBoundingRect)(int elmId, RECT* pRect) = 0;
};


class CWindowPreview final: 
	public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
		, IWindowPreview
		, IWindowConfig
	>
{
public:
	CWindowPreview(SIZE const& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType, LPVOID hTheme, int dpi);
	~CWindowPreview() override;

	//~ Begin IWindowPreview interface
	STDMETHODIMP GetPreviewImage(HBITMAP* pbOut);
	STDMETHODIMP GetUpdatedPreviewImage(MYWINDOWINFO* pwndInfo, LPVOID hTheme, HBITMAP* pbOut, UINT flags);
	//~ End IWindowPreview interface

	//~ Begin IWindowConfig interface
	STDMETHODIMP GetMonitorOffset(SIZE* pOut);
	STDMETHODIMP SetClassicPrev(BOOL fEnable);
	//~ End IWindowConfig interface

	//~ Begin IWindowMetrics interface
	STDMETHODIMP GetBoundingRect(int elmId, RECT* pRect);
	//~ End IWindowMetrics interface

private:
	HRESULT _CleanupUxThemeFile(void** hFile);
	HRESULT _DesktopScreenShooter(Gdiplus::Graphics* pGraphics);
	HRESULT _RenderWindow(MYWINDOWINFO wndInfo, int index);
	HRESULT _RenderWallpaper();
	HRESULT _AdjustAndDrawWallpaper(Gdiplus::Graphics* pGraphics, Gdiplus::Rect rc);

	HRESULT _CalculateRectsOfElements();
	HRESULT _CalculateFrameMargins();
	HRESULT _CalculateWindowRects();

	// render elements
	HRESULT _RenderBin();
	HRESULT _RenderSolidColor();
	HRESULT _RenderCaption(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderCaptionButtons(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderCaptionText(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderScrollbar(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderFrame(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderContent(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderMenuBar(Gdiplus::Graphics* pGraphics, MYWINDOWINFO wndInfo);
	HRESULT _RenderMenuItem(HDC hdc, RECT* rc, int type);
	HRESULT _ComposePreview(HBITMAP* pbOut);

	// variables
	MYWINDOWINFO* _pwndInfo;
	int _wndInfoCount;
	int  _iCurrentWnd;
	SIZE _sizePreview;
	MARGINS _marFrame;
	PAGETYPE _pageType;
	void* _hTheme;
	HTHEME _hWndTheme;
	BOOL _fIsThemed;
	int _dpiWindow;

	Gdiplus::Rect _rcPreview;
	Gdiplus::Rect _rcBin;
	Gdiplus::Rect _rcMonitor;
	Gdiplus::Rect _rcMonitorInside;

	// array for each element of a window
	RECT* _rcBounds;
	RECT _rcMargin;

	// layer bitmaps, compose all for the final preview
	Gdiplus::Bitmap* _bmpSolidColor;
	Gdiplus::Bitmap* _bmpWallpaper;
	Gdiplus::Bitmap* _bmpBin;
	Gdiplus::Bitmap* _bmpMonitor;
	Gdiplus::Bitmap** _bmpWindows;
};
