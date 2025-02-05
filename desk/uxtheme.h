#pragma once
#include "framework.h"

typedef HRESULT(*GetThemeDefaults_t)(
	LPCWSTR pszThemeFileName,
	LPWSTR pszColorName,
	DWORD dwColorNameLen,
	LPWSTR pszSizeName,
	DWORD dwSizeNameLen
	);
GetThemeDefaults_t GetThemeDefaults;

typedef HRESULT(*LoaderLoadTheme_t)(
	HANDLE hThemeFile,
	HINSTANCE hInstance,
	LPCWSTR pszThemeFileName,
	LPCWSTR pszColorParam,
	LPCWSTR pszSizeParam,
	OUT HANDLE* hSharableSection,
	LPWSTR pszSharableSectionName,
	int cchSharableSectionName,
	OUT HANDLE* hNonsharableSection,
	LPWSTR pszNonsharableSectionName,
	int cchNonsharableSectionName,
	PVOID pfnCustomLoadHandler,
	OUT HANDLE* hReuseSection,
	int a,
	int b,
	BOOL fEmulateGlobal
	);
LoaderLoadTheme_t LoaderLoadTheme;

typedef HRESULT(*LoaderLoadTheme_t_win11)(
	HANDLE hThemeFile,
	HINSTANCE hInstance,
	LPCWSTR pszThemeFileName,
	LPCWSTR pszColorParam,
	LPCWSTR pszSizeParam,
	OUT HANDLE* hSharableSection,
	LPWSTR pszSharableSectionName,
	int cchSharableSectionName,
	OUT HANDLE* hNonsharableSection,
	LPWSTR pszNonsharableSectionName,
	int cchNonsharableSectionName,
	PVOID pfnCustomLoadHandler,
	OUT HANDLE* hReuseSection,
	int a,
	int b
	);

typedef HTHEME(*OpenThemeDataFromFile_t)(
    HANDLE hThemeFile,
    HWND hWnd,
    LPCWSTR pszClassList,
    DWORD dwFlags
    // DWORD unknown,
    // bool a
    );
OpenThemeDataFromFile_t OpenThemeDataFromFile;

typedef struct _UXTHEMEFILE
{
    char header[7]; // must be "thmfile"
    LPVOID sharableSectionView;
    HANDLE hSharableSection;
    LPVOID nsSectionView;
    HANDLE hNsSection;
    char end[3]; // must be "end"
} UXTHEMEFILE, * LPUXTHEMEFILE;

void InitUxtheme()
{
    HMODULE hUxtheme = GetModuleHandle(L"uxtheme.dll");
    if (hUxtheme)
    {
        GetThemeDefaults = (GetThemeDefaults_t)GetProcAddress(hUxtheme, (LPCSTR)7);
        LoaderLoadTheme = (LoaderLoadTheme_t)GetProcAddress(hUxtheme, (LPCSTR)92);
        OpenThemeDataFromFile = (OpenThemeDataFromFile_t)GetProcAddress(hUxtheme, (LPCSTR)16);
        FreeLibrary(hUxtheme);
    }
}

HANDLE LoadThemeFromFilePath(PCWSTR szThemeFileName)
{
    HRESULT hr = S_OK;

    OSVERSIONINFOW verInfo = { 0 };

    WCHAR defColor[MAX_PATH];
    WCHAR defSize[MAX_PATH];

    hr = GetThemeDefaults(
        szThemeFileName,
        defColor,
        ARRAYSIZE(defColor),
        defSize,
        ARRAYSIZE(defSize)
    );

    HANDLE hSharableSection;
    HANDLE hNonsharableSection;

    if (verInfo.dwBuildNumber < 20000)
    {
        hr = LoaderLoadTheme(
            NULL,
            NULL,
            szThemeFileName,
            defColor,
            defSize,
            &hSharableSection,
            NULL,
            0,
            &hNonsharableSection,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            FALSE
        );
    }
    else
    {
        hr = ((LoaderLoadTheme_t_win11)LoaderLoadTheme)(
            NULL,
            NULL,
            szThemeFileName,
            defColor,
            defSize,
            &hSharableSection,
            NULL,
            0,
            &hNonsharableSection,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL
            );
    }

    HANDLE g_hLocalTheme = malloc(sizeof(UXTHEMEFILE));
    if (g_hLocalTheme)
    {
        UXTHEMEFILE* ltf = (UXTHEMEFILE*)g_hLocalTheme;
        lstrcpyA(ltf->header, "thmfile");
        lstrcpyA(ltf->header, "end");
        ltf->sharableSectionView = MapViewOfFile(hSharableSection, 4, 0, 0, 0);
        ltf->hSharableSection = hSharableSection;
        ltf->nsSectionView = MapViewOfFile(hNonsharableSection, 4, 0, 0, 0);
        ltf->hNsSection = hNonsharableSection;
    }
    else
    {
        hr = E_FAIL;
    }
    return g_hLocalTheme;
}