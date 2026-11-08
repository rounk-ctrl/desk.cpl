#include "pch.h"
#include "uxtheme.h"
#include "version.h"

GetThemeDefaults_t GetThemeDefaults;
LoaderLoadTheme_t LoaderLoadTheme;
OpenThemeDataFromFile_t OpenThemeDataFromFile;
EnumThemes_t EnumThemes;
EnumThemeColors_t EnumThemeColors;
EnumThemeSize_t EnumThemeSize;
ClearTheme_t ClearTheme;
DrawTextWithGlow_t DrawTextWithGlow;
SetSystemVisualStyle_t SetSystemVisualStyle;

void InitUxtheme()
{
	HMODULE hUxtheme = GetModuleHandle(L"uxtheme.dll");
	if (hUxtheme)
	{
		GetThemeDefaults = (GetThemeDefaults_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(7));
		EnumThemes = (EnumThemes_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(8));
		EnumThemeColors = (EnumThemeColors_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(9));
		EnumThemeSize = (EnumThemeSize_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(10));
		OpenThemeDataFromFile = (OpenThemeDataFromFile_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(16));
		ClearTheme = (ClearTheme_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(84));
		LoaderLoadTheme = (LoaderLoadTheme_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(92));
		//DrawTextWithGlow = (DrawTextWithGlow_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(126));

		SetSystemVisualStyle = (SetSystemVisualStyle_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(65));
	}
}


void CloseThemeFile(HANDLE theme)
{
	if (!theme) return;
	auto file = static_cast<UXTHEMEFILE*>(theme);
	if (file->_pbSharableData) UnmapViewOfFile(file->_pbSharableData);
	if (file->_pbNonSharableData) UnmapViewOfFile(file->_pbNonSharableData);
	if (ClearTheme && file->_hSharableSection && file->_hNonSharableSection)
		ClearTheme(file->_hSharableSection, file->_hNonSharableSection, FALSE);
	if (file->_hSharableSection) CloseHandle(file->_hSharableSection);
	if (file->_hNonSharableSection) CloseHandle(file->_hNonSharableSection);
	free(file);
}

HANDLE LoadThemeFromFilePath(PCWSTR szThemeFileName, PCWSTR colorName, PCWSTR sizeName)
{
	if (!szThemeFileName || !PathFileExists(szThemeFileName)
		|| !LoaderLoadTheme || !OpenThemeDataFromFile) return nullptr;

	WCHAR defColor[MAX_PATH + 1] = {};
	WCHAR defSize[MAX_PATH + 1] = {};
	if (!colorName || !*colorName || !sizeName || !*sizeName)
	{
		if (!GetThemeDefaults || FAILED(GetThemeDefaults(szThemeFileName,
			defColor, ARRAYSIZE(defColor), defSize, ARRAYSIZE(defSize)))) return nullptr;
		if (!colorName || !*colorName) colorName = defColor;
		if (!sizeName || !*sizeName) sizeName = defSize;
	}
	if (!*colorName || !*sizeName) return nullptr;

	auto file = static_cast<UXTHEMEFILE*>(calloc(1, sizeof(UXTHEMEFILE)));
	if (!file) return nullptr;
	memcpy(file->_szHead, "thmfile", sizeof(file->_szHead));
	memcpy(file->_szTail, "end", sizeof(file->_szTail));
	file->_Debug_Generation = file->_Debug_ChangeID = DWORD(-1);

	HRESULT hr;
	if (g_osVersion.BuildNumber() < 20000)
	{
		hr = LoaderLoadTheme(NULL, NULL, szThemeFileName, colorName, sizeName,
			&file->_hSharableSection, NULL, 0, &file->_hNonSharableSection, NULL, 0,
			NULL, NULL, 0, 0, FALSE);
	}
	else
	{
		hr = ((LoaderLoadTheme_t_win11)LoaderLoadTheme)(NULL, NULL, szThemeFileName, colorName, sizeName,
			&file->_hSharableSection, NULL, 0, &file->_hNonSharableSection, NULL, 0,
			NULL, NULL, 0, 0);
	}
	if (FAILED(hr) || !file->_hSharableSection || !file->_hNonSharableSection)
	{
		CloseThemeFile(file);
		return nullptr;
	}

	file->_pbSharableData = MapViewOfFile(file->_hSharableSection, FILE_MAP_READ, 0, 0, 0);
	file->_pbNonSharableData = MapViewOfFile(file->_hNonSharableSection, FILE_MAP_READ, 0, 0, 0);
	if (!file->_pbSharableData || !file->_pbNonSharableData)
	{
		CloseThemeFile(file);
		return nullptr;
	}
	return file;
}