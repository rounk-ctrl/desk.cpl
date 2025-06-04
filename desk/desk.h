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

// convenient struct with various information used by various pages
struct THEMEINFO {
	WALLPAPER_TYPE wallpaperType;
	LPWSTR wallpaperPath;
	COLORREF newColor;
	bool customWallpaperSelection = false;
	int posChanged = -1;
	bool useDesktopColor = false;
	bool updateWallThemesPg = false;
};

// global HINSTANCE for the current app
extern HINSTANCE g_hinst;

// ThemeManager2 interface
extern IThemeManager2* pThemeManager;

// DesktopWallpaper (8+) interface
extern IDesktopWallpaper* pDesktopWallpaper;

// current theme's ITheme interface, defined as a IUnknown for wrapper
// USAGE: ITheme* = new ITheme(currentITheme);
extern IUnknown* currentITheme;

// various theme information used across multiple pages
// defined as a struct for convenience
extern THEMEINFO* selectedTheme;

// bool to keep track if the selection in wallpaper listview is programmatically done or not
extern BOOL selectionPicker;

// process information for the loaded screen saver preview
// defined here so it can be killed by other pages
extern PROCESS_INFORMATION pi;
