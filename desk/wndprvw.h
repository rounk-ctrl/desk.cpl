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
	WT_ACTIVE,
	WT_INACTIVE,
	WT_MESSAGEBOX,
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

class CWindowPreview final: 
	public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
		, IWindowPreview
	>
{
public:
	CWindowPreview(SIZE const& sizePreview, MYWINDOWINFO* pwndInfo, int wndInfoCount, PAGETYPE pageType, LPVOID hTheme);
	~CWindowPreview() override;

	//~ Begin IWindowPreview interface
	STDMETHODIMP GetPreviewImage(HBITMAP* pbOut);
	STDMETHODIMP GetUpdatedPreviewImage(MYWINDOWINFO* pwndInfo, LPVOID hTheme, HBITMAP* pbOut, UINT flags);
	//~ End IWindowPreview interface

private:
	HRESULT _CleanupUxThemeFile(void** hFile);
	HRESULT _DesktopScreenShooter(Gdiplus::Graphics* pGraphics);
	HRESULT _DrawMonitor();
	HRESULT _RenderWindow(MYWINDOWINFO wndInfo, int index);
	HRESULT _RenderWallpaper();
	HRESULT _RenderBin();
	HRESULT _RenderSolidColor();
	HRESULT _RenderCaption(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderCaptionButtons(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderCaptionText(HDC hdc, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderScrollbar(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderFrame(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _RenderContent(Gdiplus::Graphics* pGraphics, HTHEME hTheme, MYWINDOWINFO wndInfo);
	HRESULT _ComposePreview(HBITMAP* pbOut);

	// variables
	MYWINDOWINFO* _pwndInfo;
	int _wndInfoCount;
	SIZE _sizePreview;
	MARGINS _marFrame;
	MARGINS _marMonitor;
	PAGETYPE _pageType;
	void* _hTheme;
	BOOL _fIsThemed;
	SIZE _szMenuBar;

	// layer bitmaps, compose all for the final preview
	Gdiplus::Bitmap* _bmpSolidColor;
	Gdiplus::Bitmap* _bmpWallpaper;
	Gdiplus::Bitmap* _bmpBin;
	Gdiplus::Bitmap* _bmpMonitor;
	Gdiplus::Bitmap** _bmpWindows; // todo: maybe split this ??
};
