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
		DrawTextWithGlow = (DrawTextWithGlow_t)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(126));
		FreeLibrary(hUxtheme);
	}
}


HANDLE LoadThemeFromFilePath(PCWSTR szThemeFileName)
{
	WCHAR defColor[MAX_PATH];
	WCHAR defSize[MAX_PATH];

	HRESULT hr = GetThemeDefaults(szThemeFileName, defColor, ARRAYSIZE(defColor), defSize, ARRAYSIZE(defSize));

	HANDLE hSharableSection;
	HANDLE hNonsharableSection;
	if (g_osVersion.BuildNumber() < 20000)
	{
		hr = LoaderLoadTheme(NULL, NULL, szThemeFileName, defColor, defSize, &hSharableSection, NULL, 0, &hNonsharableSection, NULL, 0, NULL, NULL, NULL, NULL, FALSE);
	}
	else
	{
		hr = ((LoaderLoadTheme_t_win11)LoaderLoadTheme)(NULL, NULL, szThemeFileName, defColor, defSize, &hSharableSection, NULL, 0, &hNonsharableSection, NULL, 0, NULL, NULL, NULL, NULL);
	}

	HANDLE _hLocalTheme = malloc(sizeof(UXTHEMEFILE));
	if (_hLocalTheme)
	{
		UXTHEMEFILE* ltf = (UXTHEMEFILE*)_hLocalTheme;
		memcpy(ltf->_szHead, "thmfile", ARRAYSIZE(ltf->_szHead));
		memcpy(ltf->_szTail, "end", ARRAYSIZE(ltf->_szTail));

		ltf->_pbSharableData = MapViewOfFile(hSharableSection, FILE_MAP_READ, 0, 0, 0);
		ltf->_hSharableSection = hSharableSection;
		ltf->_pbNonSharableData = MapViewOfFile(hNonsharableSection, FILE_MAP_READ, 0, 0, 0);
		ltf->_hNonSharableSection = hNonsharableSection;
	}
	else
	{
		hr = E_FAIL;
	}
	return _hLocalTheme;
}