#include "pch.h"
#include "desk.h"
#include "AppearancePage.h"
#include <wil/registry.h>
#include "AppearanceDlgBox.h"
namespace fs = std::filesystem;

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
		hr = EnumThemeColors(msstyle[0], NULL, i, &name);
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
		hr = EnumThemeSize(msstyle[0], NULL, i, &name);
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

BOOL CAppearanceDlgProc::OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CAppearanceDlgBox dlg;
	dlg.DoModal();
	return 0;
}
