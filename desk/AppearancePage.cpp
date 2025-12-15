#include "pch.h"
#include "AppearanceDlgBox.h"
#include "AppearancePage.h"
#include "EffectsDlg.h"
#include "desk.h"
#include "helper.h"
#include "uxtheme.h"
#include "cscheme.h"

using namespace Gdiplus;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;

void DumpNonClientMetrics(NONCLIENTMETRICSW ncm);
void ScaleLogFont(LOGFONT& lf, int dpi);
void ScaleNonClientMetrics(NONCLIENTMETRICS& ncm, int dpi);

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
	LPCWSTR extensions[] = { L".msstyles" };
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

	/*
	NONCLIENTMETRICS ncm = {};
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	DumpNonClientMetrics(ncm);
	*/

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
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath), &ebmp, UPDATE_SOLIDCLR | UPDATE_WINDOW);
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
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath), &ebmp, UPDATE_SOLIDCLR | UPDATE_WINDOW);
	HBITMAP hPrev = Static_SetBitmap(hPreviewWnd, ebmp);
	if (hPrev) DeleteObject(hPrev);

	SetModified(TRUE);
	return 0;
}

BOOL CAppearanceDlgProc::OnSetActive()
{
	_TerminateProcess(pi);

	UINT flags = UPDATE_NONE;
	if (selectedTheme->newColor != 0xB0000000)
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
		int elements[29] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28 };
		SetSysColors(29, elements, selectedTheme->selectedScheme->rgb);
		
		NONCLIENTMETRICS ncm;
		memcpy(&ncm, &selectedTheme->selectedScheme->ncm, sizeof(NONCLIENTMETRICSW_2k));
		ncm.cbSize = sizeof(ncm);
		ncm.iPaddedBorderWidth = 0;

		LOGFONT lfIcon = selectedTheme->selectedScheme->lfIconTitle;

		// bruhhh
		ScaleNonClientMetrics(ncm, GetDpiForWindow(m_hWnd));
		ScaleLogFont(lfIcon, GetDpiForWindow(m_hWnd));
		SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT), (PVOID)&lfIcon, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), (PVOID)&ncm, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
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
	int index = -1;
	if (lstrcmp(data, L"(classic)") == 0)
	{
		for (ULONG i = 0; i < mapSize; ++i)
		{
			BOOL isSame = 1;
			//wprintf(L"scheme %d----%s\n", i, schemeMap[i].name);
			for (ULONG j = 0; j < MAX_COLORS; j++)
			{
				if (!isSame) break;
				if (GetRValue(GetSysColor(j)) != GetRValue(schemeMap[i].rgb[j])
					|| GetGValue(GetSysColor(j)) != GetGValue(schemeMap[i].rgb[j])
					|| GetBValue(GetSysColor(j)) != GetBValue(schemeMap[i].rgb[j]))
				{
					isSame = 0;
				}
			}
			WCHAR* szSearch = schemeMap[i].name;

			if (wcsstr(szSearch, L"(large)"))
			{
				if (index != -1) isSame = 0;
				else szSearch = strCut(szSearch, L" (large)");
			}
			if (wcsstr(szSearch, L"(extra large)"))
			{
				if (index != -1) isSame = 0;
				else szSearch = strCut(szSearch, L" (extra large)");
			}

			if (isSame)
			{
				index = ComboBox_FindString(hColorCombobox, -1, szSearch);
				//printf("\n\n\nSAME index: %d\n\n\n", index);
			}
		}
		if (index == -1)
		{
			int ij = ComboBox_InsertString(hColorCombobox, 0, L"(current)");
			ComboBox_SetItemData(hColorCombobox, ij, NULL);
			index += 1;
		}
		ComboBox_SetCurSel(hColorCombobox, index);
		selectedTheme->selectedScheme = (SCHEMEDATA*)ComboBox_GetItemData(hColorCombobox, index);
	}
	else index = 0;
	ComboBox_SetCurSel(hColorCombobox, index);
}

void CAppearanceDlgProc::_UpdateFontBox(LPWSTR data)
{
	ComboBox_ResetContent(hSizeCombobox);
	if (lstrcmp(data, L"(classic)") == 0)
	{
		int index = ComboBox_GetCurSel(hColorCombobox);
		SCHEMEDATA* data = (SCHEMEDATA*)ComboBox_GetItemData(hColorCombobox, index);
		selectedTheme->selectedScheme = data;
		selectedTheme->newColor = NcGetSysColor(COLOR_BACKGROUND);

		if (data)
		{
			if (data->variant & HAS_NORMAL) ComboBox_AddString(hSizeCombobox, L"Normal");
			if (data->variant & HAS_EXTRA_LARGE) ComboBox_AddString(hSizeCombobox, L"Extra Large");
			if (data->variant & HAS_LARGE) ComboBox_AddString(hSizeCombobox, L"Large");
		}
		else
		{
			ComboBox_AddString(hSizeCombobox, L"Normal");
		}
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

void ScaleNonClientMetrics(NONCLIENTMETRICS& ncm, int dpi)
{
	ncm.iScrollHeight = MulDiv(ncm.iScrollHeight, dpi, 96);
	ncm.iScrollWidth = MulDiv(ncm.iScrollWidth, dpi, 96);
	ncm.iCaptionHeight = MulDiv(ncm.iCaptionHeight, dpi, 96);
	ncm.iCaptionWidth = MulDiv(ncm.iCaptionWidth, dpi, 96);

	ScaleLogFont(ncm.lfCaptionFont, dpi);
	ncm.iSmCaptionHeight = MulDiv(ncm.iSmCaptionHeight, dpi, 96);
	ncm.iSmCaptionWidth = MulDiv(ncm.iSmCaptionWidth, dpi, 96);

	ScaleLogFont(ncm.lfSmCaptionFont, dpi);
	ncm.iMenuHeight = MulDiv(ncm.iMenuHeight, dpi, 96);
	ncm.iMenuWidth = MulDiv(ncm.iMenuWidth, dpi, 96);

	ScaleLogFont(ncm.lfMenuFont, dpi);
	ScaleLogFont(ncm.lfStatusFont, dpi);
	ScaleLogFont(ncm.lfMessageFont, dpi);
}

void ScaleLogFont(LOGFONT& lf, int dpi)
{
	lf.lfHeight = MulDiv(lf.lfHeight, dpi, 96);
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


void DumpNonClientMetrics(NONCLIENTMETRICSW ncm)
{
	printf("cbSize: %d\n", ncm.cbSize);
	printf("iBorderWidth: %d\n", ncm.iBorderWidth);
	printf("iScrollWidth: %d\n", ncm.iScrollWidth);
	printf("iScrollHeight: %d\n", ncm.iScrollHeight);
	printf("iCaptionWidth: %d\n", ncm.iCaptionWidth);
	printf("iCaptionHeight: %d\n", ncm.iCaptionHeight);
	printf("\nlfCaptionFont: \n");
	DumpLogFont(ncm.lfCaptionFont);
	printf("\niSmCaptionWidth: %d\n", ncm.iSmCaptionWidth);
	printf("iSmCaptionHeight: %d\n", ncm.iSmCaptionHeight);
	printf("\nlfSmCaptionFont: \n");
	DumpLogFont(ncm.lfSmCaptionFont);
	printf("\niMenuWidth: %d\n", ncm.iMenuWidth);
	printf("iMenuHeight: %d\n", ncm.iMenuHeight);
	printf("\nlfMenuFont: \n");
	DumpLogFont(ncm.lfMenuFont);
	printf("\nlfStatusFont: \n");
	DumpLogFont(ncm.lfStatusFont);
	printf("\nlfMessageFont: \n");
	DumpLogFont(ncm.lfMessageFont);

	// vista
	//printf("\niPaddedBorderWidth: % d\n", ncm.iPaddedBorderWidth);
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
