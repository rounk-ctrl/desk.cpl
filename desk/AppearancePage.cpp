#include "pch.h"
#include "desk.h"
#include "AppearancePage.h"
#include <wil/registry.h>
namespace fs = std::filesystem;

#define READ_AT(TYPE, BIN, OFFSET) (*reinterpret_cast<TYPE*>((BIN) + (OFFSET)))
#define READ_STRING(TYPE, BIN, OFFSET) (reinterpret_cast<TYPE*>((BIN)+ (OFFSET)))

// 29 colors
#define MAX_COLORS (COLOR_GRADIENTINACTIVECAPTION + 1)

typedef struct tagNONCLIENTMETRICSW_2k
{
	UINT    cbSize;
	int     iBorderWidth;
	int     iScrollWidth;
	int     iScrollHeight;
	int     iCaptionWidth;
	int     iCaptionHeight;
	LOGFONTW lfCaptionFont;
	int     iSmCaptionWidth;
	int     iSmCaptionHeight;
	LOGFONTW lfSmCaptionFont;
	int     iMenuWidth;
	int     iMenuHeight;
	LOGFONTW lfMenuFont;
	LOGFONTW lfStatusFont;
	LOGFONTW lfMessageFont;
}  NONCLIENTMETRICSW_2k;
typedef struct {
	DWORD version;
	NONCLIENTMETRICSW_2k ncm;
	LOGFONT lfIconTitle;
	COLORREF rgb[MAX_COLORS];
	WCHAR name[40];
} SCHEMEDATA;

#ifdef _DEBUG

VOID DumpLogFont(LOGFONT font)
{
	printf("lfHeight: %d\n", font.lfHeight);
	printf("lfWidth: %d\n", font.lfWidth);
	printf("lfEscapement: %d\n", font.lfEscapement);
	printf("lfOrientation: %d\n", font.lfOrientation);
	printf("lfWeight: %d\n", font.lfWeight);
	printf("lfItalic: %d\n", font.lfItalic);
	printf("lfUnderline: %d\n", font.lfUnderline);
	printf("lfStrikeOut: %d\n", font.lfStrikeOut);
	printf("lfCharSet: %d\n", font.lfCharSet);
	printf("lfOutPrecision: %d\n", font.lfOutPrecision);
	printf("lfClipPrecision: %d\n", font.lfClipPrecision);
	printf("lfQuality: %d\n", font.lfQuality);
	printf("lfPitchAndFamily: %d\n", font.lfPitchAndFamily);
	wprintf(L"lfFaceName: %s\n",font.lfFaceName); // size: 64
}

VOID DumpData(LPCWSTR theme)
{
	BYTE* value;
	DWORD dwSize;
	HRESULT hr = RegGetValue(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes", theme, RRF_RT_REG_BINARY, NULL, NULL, &dwSize);

	value = (BYTE*)malloc(dwSize);
	hr = RegGetValue(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes", theme, RRF_RT_REG_BINARY, NULL, value, &dwSize);

	if (value)
	{
		SCHEMEDATA data;
		wprintf(L"\nDumping theme info on %s:\n", theme);

		data.version = READ_AT(DWORD, value, 0);
		printf("Version: %u\n", data.version);
		
		data.ncm = READ_AT(NONCLIENTMETRICSW_2k, value, 4);
		printf("\nNONCLIENTMETRICS:\n");
		printf("cbSize: %u\n", data.ncm.cbSize);
		printf("iBorderWidth: %u\n", data.ncm.iBorderWidth);
		printf("iScrollWidth: %u\n", data.ncm.iScrollWidth);
		printf("iScrollHeight: %u\n", data.ncm.iScrollHeight);
		printf("iCaptionWidth: %u\n", data.ncm.iCaptionWidth);
		printf("iCaptionHeight: %u\n", data.ncm.iCaptionHeight);

		printf("\nLOGFONT: lfCaptionFont:\n");
		DumpLogFont(data.ncm.lfCaptionFont);

		printf("\n");
		printf("iSmCaptionWidth: %u\n", data.ncm.iSmCaptionWidth);
		printf("iSmCaptionHeight: %u\n", data.ncm.iSmCaptionHeight);

		printf("\nLOGFONT: lfSmCaptionFont:\n");
		DumpLogFont(data.ncm.lfSmCaptionFont);

		printf("\n");
		printf("iMenuWidth: %u\n", data.ncm.iMenuWidth);
		printf("iMenuHeight: %u\n", data.ncm.iMenuHeight);

		printf("\nLOGFONT: lfMenuFont:\n");
		DumpLogFont(data.ncm.lfMenuFont);

		printf("\nLOGFONT: lfStatusFont:\n");
		DumpLogFont(data.ncm.lfStatusFont);

		printf("\nLOGFONT: lfMessageFont:\n");
		DumpLogFont(data.ncm.lfMessageFont);

		// doesnt have padded border ??
		data.lfIconTitle = READ_AT(LOGFONTW, value, 504);
		printf("\nLOGFONT: lfIconTitle:\n");
		DumpLogFont(data.lfIconTitle);
		
		//596
		int start = 596;
		for (int i = 0; i < MAX_COLORS; i++)
		{
			data.rgb[i] = READ_AT(COLORREF, value, start + (4 * i));
		}

		COLORREF bgColor = data.rgb[COLOR_BACKGROUND];
		wprintf(L"\nCOLOR_BACKGROUND %d,%d,%d\n", GetRValue(bgColor), GetGValue(bgColor), GetBValue(bgColor));

		free(value);
	}
	
}

#endif

/*
* classic appearance dialog code

	SCHEMEDATA* schemeMap = NULL;
	UINT mapSize;


VOID FillSchemeDataMap(LPCWSTR theme, int index)
{
	BYTE* value;
	DWORD dwSize;
	HRESULT hr = RegGetValue(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes", theme, RRF_RT_REG_BINARY, NULL, NULL, &dwSize);

	value = (BYTE*)malloc(dwSize);
	hr = RegGetValue(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes", theme, RRF_RT_REG_BINARY, NULL, value, &dwSize);

	SCHEMEDATA data = {};
	data.version = READ_AT(DWORD, value, 0);
	data.ncm = READ_AT(NONCLIENTMETRICSW_2k, value, 4);
	data.lfIconTitle = READ_AT(LOGFONTW, value, 504);
	int start = 596;
	for (int i = 0; i < MAX_COLORS; i++)
	{
		data.rgb[i] = READ_AT(COLORREF, value, start + (4 * i));
	}

	size_t len = wcslen(theme) + 1;
	wcscpy_s(data.name, len, theme);
	schemeMap[index] = data;
}

		auto key = wil::reg::open_unique_key(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes");
		mapSize = wil::reg::get_child_value_count(key.get());
		schemeMap = (SCHEMEDATA*)malloc(mapSize * sizeof(SCHEMEDATA));

		int i = 0;
		for (const auto& key_data : wil::make_range(wil::reg::value_iterator{ key.get() }, wil::reg::value_iterator{}))
		{
			if (key_data.type == REG_BINARY)
			{
				FillSchemeDataMap(key_data.name.c_str(), i);
				i++;
			}
		}

		for (UINT j = 0; j < mapSize; j++)
		{
			ComboBox_AddString(hThemesCombobox, schemeMap[j].name);
		}
*/


typedef struct tagTHEMENAMES
{
	WCHAR szName[MAX_PATH + 1];
	WCHAR szDisplayName[MAX_PATH + 1];
	WCHAR szTooltip[MAX_PATH + 1];
} THEMENAMES, * PTHEMENAMES;

typedef HRESULT(WINAPI* EnumThemeColors_t)(LPWSTR pszThemeFileName, LPWSTR pszSizeName, DWORD dwColorNum, PTHEMENAMES pszColorNames);
typedef HRESULT(WINAPI* EnumThemeSize_t)(LPWSTR pszThemeFileName, LPWSTR pszSizeName, DWORD dwColorNum, PTHEMENAMES pszColorNames);
EnumThemeColors_t EnumThemeColors;
EnumThemeSize_t EnumThemeSize;

BOOL CAppearanceDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hThemesCombobox = GetDlgItem(1111);
	hColorCombobox = GetDlgItem(1114);
	hSizeCombobox = GetDlgItem(1116);

	WCHAR msstyledir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Resources\\Themes", msstyledir, MAX_PATH);
	for (const auto& entry : fs::recursive_directory_iterator(msstyledir, fs::directory_options::skip_permission_denied))
	{
		if (entry.is_regular_file() && (entry.path().extension() == L".msstyles"))
		{
			LPWSTR lpwstrPath = _wcsdup(entry.path().c_str());
			msstyle.push_back(lpwstrPath);
		}
	}

	for (LPCWSTR style : msstyle)
	{
		HMODULE hStyle = LoadLibrary(style);
		if (hStyle)
		{
			WCHAR name[MAX_PATH];
			LoadString(hStyle, 101, name, MAX_PATH);
			if (lstrlenW(name) != 0)
				ComboBox_AddString(hThemesCombobox, name);
			else
				ComboBox_AddString(hThemesCombobox, PathFindFileName(style));
			FreeLibrary(hStyle);
		}
	}

	EnumThemeColors = (EnumThemeColors_t)GetProcAddress(LoadLibraryW(L"uxtheme.dll"), MAKEINTRESOURCEA(9));
	EnumThemeSize = (EnumThemeSize_t)GetProcAddress(LoadLibraryW(L"uxtheme.dll"), MAKEINTRESOURCEA(10));
	THEMENAMES name;
	HRESULT hr = S_OK;
	for (int i = 0; hr >= 0; i++)
	{
		hr = EnumThemeColors(msstyle[5], NULL, i, &name);
		if (SUCCEEDED(hr))
		{
			if (lstrlenW(name.szDisplayName) == 0)
				ComboBox_AddString(hColorCombobox, name.szName);
			else
				ComboBox_AddString(hColorCombobox, name.szDisplayName);
		}
	}
	hr = S_OK;
	for (int i = 0; hr >= 0; i++)
	{
		hr = EnumThemeSize(msstyle[5], NULL, i, &name);
		if (SUCCEEDED(hr))
		{
			if (lstrlenW(name.szDisplayName) == 0)
				ComboBox_AddString(hSizeCombobox, name.szName);
			else
				ComboBox_AddString(hSizeCombobox, name.szDisplayName);
		}
	}

	ITheme* themeClass = new ITheme(currentITheme);
	LPWSTR style;
	themeClass->get_VisualStyle(&style);

	HMODULE hStyle = LoadLibrary(style);
	if (hStyle)
	{
		WCHAR name[MAX_PATH];
		LoadString(hStyle, 101, name, MAX_PATH);
		FreeLibrary(hStyle);

		ComboBox_SetCurSel(hThemesCombobox, ComboBox_FindString(hThemesCombobox, 0, name));
	}
	
	ComboBox_SetCurSel(hColorCombobox, 0);
	ComboBox_SetCurSel(hSizeCombobox, 0);

	return 0;
}
