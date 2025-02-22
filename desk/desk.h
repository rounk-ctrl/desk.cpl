#pragma once
#include "pch.h"
#include "theme.h"

// why dont winapi have this bruh
// set a static to display a bitmap
#define Static_SetBitmap(hwndCtl, hBmp)  \
	((HBITMAP)(UINT_PTR)SNDMSG((hwndCtl), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)(HBITMAP)(hBmp)))

enum WALLPAPER_TYPE {
	WT_NOWALL = 1,
	WT_PICTURE,
	WT_SLIDESHOW
};

struct THEMEINFO {
	WALLPAPER_TYPE wallpaperType;
	LPWSTR wallpaperPath;
	COLORREF newColor;
	bool customWallpaperSelection = false;
	bool posChanged = false;
};

extern HINSTANCE g_hinst;
extern IThemeManager2* pThemeManager;
extern IDesktopWallpaper* pDesktopWallpaper;

//todo: remove
extern IUnknown* currentITheme;

extern THEMEINFO* selectedTheme;
extern BOOL selectionPicker;
