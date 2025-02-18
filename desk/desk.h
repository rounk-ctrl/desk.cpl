#pragma once
#include "pch.h"
#include "theme.h"

extern HINSTANCE g_hinst;
extern IThemeManager2* pThemeManager;
extern IDesktopWallpaper* pDesktopWallpaper;
extern LPWSTR wallpath;
extern COLORREF newColor;
extern int lastpos;
extern BOOL noWall;
extern BOOL firstSelect;
extern IUnknown* currentITheme;
extern int currentIThemeIndex;
extern bool bglock; 
extern bool thlock;
