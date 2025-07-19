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

enum _THEMECALLBACK
{
	TCB_THEMENAME,
	TCB_COLORSCHEME,
	TCB_SIZENAME,
	TCB_SUBSTTABLE,
	TCB_CDFILENAME,
	TCB_CDFILECOMBO,
	TCB_FILENAME,
	TCB_DOCPROPERTY,
	TCB_NEEDSUBST,
	TCB_MINCOLORDEPTH,
	TCB_FONT,
	TCB_MIRRORIMAGE,
	TCB_LOCALIZABLE_RECT,
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

// useless, loads signed themes
typedef BOOL(CALLBACK* EnumThemesFunc)(
	_THEMECALLBACK tcb, 
	LPCWSTR pszFileName, 
	LPCWSTR pszDisplayName, 
	LPCWSTR pszToolTip, 
	int, 
	LPARAM lParam);

typedef HRESULT(WINAPI* EnumThemes_t)(
	_In_ LPCWSTR pszThemeRoot,
	_In_ EnumThemesFunc lpEnumFunc,
	_In_opt_ LPARAM lParam);

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

typedef HRESULT(WINAPI* ClearTheme_t)(
	_In_ void* hSharableSection, 
	_In_ void* hNonSharableSection, 
	_In_ BOOL fForce);

typedef HRESULT(WINAPI* DrawTextWithGlow_t)(
	HDC hdcMem,
	LPCWSTR pszText,
	int cch,
	RECT* prc,
	DWORD dwFlags,
	COLORREF crText,
	COLORREF crGlow,
	int nGlowRadius,
	unsigned int nGlowIntensity,
	int fPreMultiply,
	int(__fastcall* pfnDrawTextCallback)(HDC, wchar_t*, int, RECT*, unsigned int, __int64),
	LPARAM lParam);

enum ApplyThemeFlags
{
	AT_NONE = 0,
	AT_SET_METRICS = 0x1,
	AT_VALIDATE = 0x2,
	AT_FORCE_GLOBAL = 0x4, 
	AT_RESET_COLORIZATION = 0x8,  // calls DwmpResetColorizationParameter
	AT_NO_REGISTRY_UPDATE = 0x10,  
	AT_SET_METRICS_NOASYNC = 0x20, 
	AT_FORCE_COLORIZATION = 0x40,  // use with 0x1 for DwmpResetColorizationParameter
};

// AT_VALIDATE must be NOT set, or it fails with E_INVALIDARG
// pszColorParam and pszSizeParam are optional since if they are null, it will get the default stuff
// if pszVisualStyleFile isnt null, it appends 8 to dwFlags
// if pszVisualStyleFile is null, it calls CThemeServices::ApplyTheme with only dwFlags
typedef HRESULT(WINAPI* SetSystemVisualStyle_t)(
	_In_ LPCWSTR pszVisualStyleFile,
	_In_opt_ LPCWSTR pszColorParam,
	_In_opt_ LPCWSTR pszSizeParam,
	_In_ ApplyThemeFlags dwFlags);

// SetSystemVisualStyle uxtheme 65
// GetThemeClass uxtheme 74
// ClearTheme uxtheme 84
// DrawTextWithGlow uxtheme 126

// expose the variables
extern GetThemeDefaults_t GetThemeDefaults;
extern LoaderLoadTheme_t LoaderLoadTheme;
extern OpenThemeDataFromFile_t OpenThemeDataFromFile;
extern EnumThemeColors_t EnumThemeColors;
extern EnumThemeSize_t EnumThemeSize;
extern ClearTheme_t ClearTheme;
extern EnumThemes_t EnumThemes;
extern DrawTextWithGlow_t DrawTextWithGlow;
extern SetSystemVisualStyle_t SetSystemVisualStyle;

void InitUxtheme();
HANDLE LoadThemeFromFilePath(PCWSTR szThemeFileName);
