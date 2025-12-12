#include "pch.h"
#include "AppearanceDlgBox.h"
#include "AppearancePage.h"
#include "EffectsDlg.h"
#include "desk.h"
#include "helper.h"
#include "uxtheme.h"

using namespace Gdiplus;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;

#define HAS_NORMAL 0x1
#define HAS_LARGE 0x2
#define HAS_EXTRA_LARGE 0x4

BOOL CAppearanceDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_fFirstInit = TRUE;
	hPreviewWnd = GetDlgItem(1110);
	hThemesCombobox = GetDlgItem(1111);
	hColorCombobox = GetDlgItem(1114);
	hSizeCombobox = GetDlgItem(1116);
	size = GetClientSIZE(hPreviewWnd);

	if (!currentITheme)
	{
		LPWSTR path = nullptr;
		int cur;
		pThemeManager->GetCurrentTheme(&cur);
		pThemeManager->GetTheme(cur, &currentITheme);

		if (!IsClassicThemeEnabled())
		{
			ITheme* themeClass = new ITheme(currentITheme);
			LPWSTR path = nullptr;
			themeClass->get_VisualStyle(&path);

			StringCpy(selectedTheme->szMsstylePath, path);
		}
		else
		{
			StringCpy(selectedTheme->szMsstylePath, (LPWSTR)L"(classic)");
		}
	}

	WCHAR msstyledir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Resources\\Themes", msstyledir, MAX_PATH);
	LPCWSTR extensions[] = {L".msstyles"};
	EnumDir(msstyledir, extensions, ARRAYSIZE(extensions), msstyle, TRUE);

	// add classic style
	int index = ComboBox_AddString(hThemesCombobox, L"Windows Classic style");
	ComboBox_SetItemData(hThemesCombobox, index, L"(classic)");
	msstyle.push_back((LPWSTR)L"(classic)");

	HKEY key;
	RegOpenKeyEx(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes", 0, KEY_READ, &key);
	if (!key) return FALSE;

	RegQueryInfoKey(key, 0, 0, 0, 0, 0, 0, &mapSize, 0, 0, 0, 0);
	schemeMap = (SCHEMEDATA*)malloc(mapSize * sizeof(SCHEMEDATA));

	LSTATUS staus = ERROR_SUCCESS;
	for (DWORD i = 0; i <= mapSize; ++i)
	{
		if (staus != ERROR_SUCCESS) break;

		WCHAR value[256];
		DWORD dwType;
		DWORD dwSize = ARRAYSIZE(value);
		staus = RegEnumValue(key, i, value, &dwSize, 0, &dwType, NULL, NULL);
		if (dwType == REG_BINARY)
		{
			FillSchemeDataMap(value, i);
		}
	}
	RegCloseKey(key);

	for (LPCWSTR style : msstyle)
	{
		HMODULE hStyle = LoadLibraryEx(style, NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
		if (hStyle)
		{
			WCHAR name[MAX_PATH];
			LoadString(hStyle, 101, name, MAX_PATH);

			index = 0;
			if (lstrlenW(name) != 0)
			{
				index = ComboBox_AddString(hThemesCombobox, name);
			}
			else
			{
				index = ComboBox_AddString(hThemesCombobox, PathFindFileName(style));
			}
			ComboBox_SetItemData(hThemesCombobox, index, style);
			FreeLibrary(hStyle);
		}
	}

	int selindex = 0;
	if (IsClassicThemeEnabled())
	{
		selindex = ComboBox_FindString(hThemesCombobox, 0, L"Windows Classic style");
	}
	else
	{
		ITheme* themeClass = new ITheme(currentITheme);
		LPWSTR style = NULL;
		themeClass->get_VisualStyle(&style);
		for (int i = 0; i < ComboBox_GetCount(hThemesCombobox); ++i)
		{
			LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
			if (StrCmpI(data, style) == 0)
			{
				selindex = i;
				break;
			}
		}
	}
	ComboBox_SetCurSel(hThemesCombobox, selindex);

	_UpdateColorBox(msstyle[selindex]);
	_UpdateFontBox(msstyle[selindex]);

	HBITMAP ebmp;
	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_APPEARANCE, nullptr, GetDpiForWindow(m_hWnd));
	pWndPreview->GetPreviewImage(&ebmp);
	HBITMAP hPrev = Static_SetBitmap(hPreviewWnd, ebmp);
	if (hPrev) DeleteBitmap(hPrev);

	_fFirstInit = FALSE;

	return 0;
}

BOOL CAppearanceDlgProc::OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CAppearanceDlgBox dlg;
	dlg.DoModal();
	return 0;
}

BOOL CAppearanceDlgProc::OnEffects(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CEffectsDlg dlg;
	dlg.DoModal();
	return 0;
}

BOOL CAppearanceDlgProc::OnComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int i = ComboBox_GetCurSel(hThemesCombobox);
	LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);

	_UpdateColorBox(data);
	_UpdateFontBox(data);

	FreeString(selectedTheme->szMsstylePath);
	StringCpy(selectedTheme->szMsstylePath, data);
	selectedTheme->fMsstyleChanged = true;

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath), &ebmp, UPDATE_WINDOW);
	HBITMAP hPrev = Static_SetBitmap(hPreviewWnd, ebmp);
	if (hPrev) DeleteObject(hPrev);

	SetModified(TRUE);
	return 0;
}

BOOL CAppearanceDlgProc::OnClrComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int i = ComboBox_GetCurSel(hThemesCombobox);
	LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
	_UpdateFontBox(data);

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath), &ebmp, UPDATE_WINDOW);
	HBITMAP hPrev = Static_SetBitmap(hPreviewWnd, ebmp);
	if (hPrev) DeleteObject(hPrev);

	return 0;
}

BOOL CAppearanceDlgProc::OnSetActive()
{
	_TerminateProcess(pi);

	UINT flags = UPDATE_NONE;
	if (selectedTheme->newColor)
	{
		flags |= UPDATE_SOLIDCLR;
	}
	if (selectedTheme->fMsstyleChanged)
	{
		flags |= UPDATE_WINDOW;
		for (int i = 0; i < ComboBox_GetCount(hThemesCombobox); ++i)
		{
			LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
			if (StrCmpI(data, selectedTheme->szMsstylePath) == 0)
			{
				ComboBox_SetCurSel(hThemesCombobox, i);
				break;
			}
		}
		selectedTheme->fMsstyleChanged = false;
	}
	if (flags != UPDATE_NONE)
	{
		HBITMAP ebmp;
		pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath), &ebmp, flags);
		HBITMAP hPrev = Static_SetBitmap(hPreviewWnd, ebmp);
		if (hPrev) DeleteObject(hPrev);
	}

	return 0;
}

BOOL CAppearanceDlgProc::OnApply()
{
	int i = ComboBox_GetCurSel(hThemesCombobox);
	LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
	if (StrCmpI(data, L"(classic)") == 0)
	{
		// todo: 
	}
	else
	{
		SetSystemVisualStyle(selectedTheme->szMsstylePath, nullptr, nullptr, AT_NONE);
	}
	selectedTheme->fThemePgMsstyleUpdate = true;
	return 0;
}

void CAppearanceDlgProc::_UpdateColorBox(LPWSTR data)
{
	ComboBox_ResetContent(hColorCombobox);
	if (lstrcmp(data, L"(classic)") == 0)
	{
		for (ULONG j = 0; j < mapSize; j++)
		{
			int index = ComboBox_AddString(hColorCombobox, schemeMap[j].name);
			schemeMap[j].schemeMapIndex = j;

			ComboBox_SetItemData(hColorCombobox, index, &schemeMap[j]);
		}

		_FixColorBox();
	}
	else
	{
		HRESULT hr = S_OK;
		_THEMENAMEINFO name;
		for (int i = 0; SUCCEEDED(hr); i++)
		{
			hr = EnumThemeColors(data, NULL, i, &name);
			if (SUCCEEDED(hr))
			{
				ComboBox_AddString(hColorCombobox, name.szDisplayName);
			}
		}

	}

	// todo: detect classic scheme
	ComboBox_SetCurSel(hColorCombobox, 0);
	selectedTheme->selectedScheme = (SCHEMEDATA*)ComboBox_GetItemData(hColorCombobox, 0);

}

void CAppearanceDlgProc::_UpdateFontBox(LPWSTR data)
{
	ComboBox_ResetContent(hSizeCombobox);
	if (lstrcmp(data, L"(classic)") == 0)
	{
		int index = ComboBox_GetCurSel(hColorCombobox);
		SCHEMEDATA* data = (SCHEMEDATA*)ComboBox_GetItemData(hColorCombobox, index);
		selectedTheme->selectedScheme = data;
		if (data->variant & HAS_NORMAL) ComboBox_AddString(hSizeCombobox, L"Normal");
		if (data->variant & HAS_EXTRA_LARGE) ComboBox_AddString(hSizeCombobox, L"Extra Large");
		if (data->variant & HAS_LARGE) ComboBox_AddString(hSizeCombobox, L"Large");
	}
	else
	{
		HRESULT hr = S_OK;
		_THEMENAMEINFO name;

		for (int i = 0; SUCCEEDED(hr); i++)
		{
			hr = EnumThemeSize(data, NULL, i, &name);
			if (SUCCEEDED(hr))
			{
				ComboBox_AddString(hSizeCombobox, name.szDisplayName);
			}
		}
	}
	ComboBox_SetCurSel(hSizeCombobox, 0);
}

void CAppearanceDlgProc::_FixColorBox()
{
	// dont know any better way
	// note: they are in alphabetical order in the combobox
	for (int i = 0; ; ++i)
	{
		if (i >= ComboBox_GetCount(hColorCombobox)) break;

		int size = ComboBox_GetLBTextLen(hColorCombobox, i);
		WCHAR* value = new WCHAR[size + 1];
		ComboBox_GetLBText(hColorCombobox, i, value);
		
		SCHEMEDATA* data = (SCHEMEDATA*)ComboBox_GetItemData(hColorCombobox, i);

		// check variants
		DWORD flags = HAS_NORMAL;
		data->variant = flags;
		LPCWSTR variants[] = { L" (large)", L" (extra large)" };
		for (int j = 0; j < ARRAYSIZE(variants); ++j)
		{
			size_t totalSize = wcslen(value) + wcslen(variants[j]) + 1;
			LPWSTR dest = new WCHAR[totalSize];
			dest[0] = L'\0';

			StringCchCat(dest, totalSize, value);
			StringCchCat(dest, totalSize, variants[j]);

			int index = ComboBox_FindString(hColorCombobox, i, dest);
			if (index != CB_ERR)
			{
				ComboBox_DeleteString(hColorCombobox, index);
				if (j == 0) flags = flags | HAS_LARGE;
				if (j == 1) flags = flags | HAS_EXTRA_LARGE;

				// set info
				data->variant = flags;
			}
			delete[] dest;
		}

		// case where normal variant doesnt exist (Pumpkin (large))
		flags = 0;
		for (int j = 0; j < ARRAYSIZE(variants); ++j)
		{
			if (StrStrI(value, variants[j]) != NULL)
			{
				if (j == 0) flags = flags | HAS_LARGE;
				if (j == 1) flags = flags | HAS_EXTRA_LARGE;

				StrTrim(value, variants[j]);

				SCHEMEDATA* data = (SCHEMEDATA*)ComboBox_GetItemData(hColorCombobox, i);
				data->variant = flags;

				ComboBox_DeleteString(hColorCombobox, i);
				int index = ComboBox_AddString(hColorCombobox, value);
				ComboBox_SetItemData(hColorCombobox, index, data);
			}
		}

		delete[] value;
	}
}


VOID CAppearanceDlgProc::FillSchemeDataMap(LPCWSTR theme, int index)
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

	free(value);
}


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
	wprintf(L"lfFaceName: %s\n", font.lfFaceName); // size: 64
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
