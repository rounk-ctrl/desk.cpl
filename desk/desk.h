#pragma once
#include "pch.h"
#include "theme.h"


enum WALLPAPER_TYPE {
	WT_NOWALL = 1,
	WT_PICTURE,
	WT_SLIDESHOW
};

struct THEMEINFO {
	WALLPAPER_TYPE wallpaperType;
	LPWSTR wallpaperPath;
	COLORREF newColor;
	int selectedThemeIndex;
	bool customWallpaperSelection = FALSE;
};

extern HINSTANCE g_hinst;
extern IThemeManager2* pThemeManager;
extern IDesktopWallpaper* pDesktopWallpaper;

//todo: remove
extern LPWSTR wallpath;
extern COLORREF newColor;
extern int lastpos;
extern BOOL noWall;
extern BOOL firstSelect;
extern IUnknown* currentITheme;
extern int currentIThemeIndex;

extern THEMEINFO selectedTheme;
