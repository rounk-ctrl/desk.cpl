#pragma once
#include "framework.h"

typedef struct _UXTHEMEFILE
{
	char _szHead[8]; // must be "thmfile"
	LPVOID _pbSharableData;
	HANDLE _hSharableSection;
	LPVOID _pbNonSharableData;
	HANDLE _hNonSharableSection;
	char _szTail[4]; // must be "end"
} UXTHEMEFILE, * LPUXTHEMEFILE;

struct _THEMENAMEINFO
{
	WCHAR szName[MAX_PATH + 1];
	WCHAR szDisplayName[MAX_PATH + 1];
	WCHAR szTooltip[MAX_PATH + 1];
};

typedef HRESULT(WINAPI *GetThemeDefaults_t)(
	_In_ LPCWSTR pszThemeFileName,
	_Out_ LPWSTR pszDefaultColor,
	_In_ UINT cchMaxColorChars,
	_Out_ LPWSTR pszDefaultSize,
	_In_ UINT cchMaxSizeChars);

typedef HRESULT(WINAPI *LoaderLoadTheme_t)(
	_In_opt_ HANDLE hFile,
	_In_opt_ HINSTANCE hInst,
	_In_ LPCWSTR pszThemeName,
	_In_ LPCWSTR pszColorParam,
	_In_ LPCWSTR pszSizeParam,
	_Out_ HANDLE* phSharableSection,
	_Out_opt_ LPWSTR pszSharableSectionName,
	_In_opt_ int cchSharableSectionName,
	_Out_ HANDLE* phNonSharableSection,
	_Out_opt_ LPWSTR pszNonSharableSectionName,
	_In_opt_ int cchNonsharableSectionName,
	_In_opt_ PVOID pfnCustomLoadHandler,
	_Out_opt_ HANDLE* phReuseSection,
	_In_opt_ int iCurrentScreenPpi,
	_In_opt_ int wCurrentLangID,
	_In_opt_ BOOL fGlobalTheme);

typedef HRESULT(WINAPI *LoaderLoadTheme_t_win11)(
	_In_opt_ HANDLE hFile,
	_In_opt_ HINSTANCE hInst,
	_In_ LPCWSTR pszThemeName,
	_In_ LPCWSTR pszColorParam,
	_In_ LPCWSTR pszSizeParam,
	_Out_ HANDLE* phSharableSection,
	_Out_opt_ LPWSTR pszSharableSectionName,
	_In_opt_ int cchSharableSectionName,
	_Out_ HANDLE* phNonSharableSection,
	_Out_opt_ LPWSTR pszNonSharableSectionName,
	_In_opt_ int cchNonsharableSectionName,
	_In_opt_ PVOID pfnCustomLoadHandler,
	_Out_opt_ HANDLE* phReuseSection,
	_In_opt_ int iCurrentScreenPpi,
	_In_opt_ int wCurrentLangID
	// _In_opt_ BOOL fGlobalTheme <- removed in 11
	);

typedef HTHEME(WINAPI *OpenThemeDataFromFile_t)(
	_In_ HANDLE hLoadedThemeFile,
	_In_opt_ HWND hwnd,
	_In_ LPCWSTR pszClassList,
	_In_ BOOL fClient);


typedef HRESULT(WINAPI* EnumThemeColors_t)(
	_In_ LPWSTR pszThemeFileName, 
	_In_opt_ LPWSTR pszColorScheme,
	_In_ DWORD dwColorNum,
	_Out_ _THEMENAMEINFO* ptn);

typedef HRESULT(WINAPI* EnumThemeSize_t)(
	_In_ LPWSTR pszThemeFileName, 
	_In_opt_ LPWSTR pszSizeName, 
	_In_ DWORD dwSizeIndex,
	_Out_ _THEMENAMEINFO* ptn);

// expose the variables
extern GetThemeDefaults_t GetThemeDefaults;
extern LoaderLoadTheme_t LoaderLoadTheme;
extern OpenThemeDataFromFile_t OpenThemeDataFromFile;
extern EnumThemeColors_t EnumThemeColors;
extern EnumThemeSize_t EnumThemeSize;

void InitUxtheme();
HANDLE LoadThemeFromFilePath(PCWSTR szThemeFileName);
