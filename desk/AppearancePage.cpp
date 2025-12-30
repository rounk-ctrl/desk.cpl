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

#define HAS_NORMAL 0x1
#define HAS_LARGE 0x2
#define HAS_EXTRA_LARGE 0x4
#define CUSTOM_SCHEME 0x8

BOOL CAppearanceDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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

		selectedTheme->szMsstylePath = L"(classic)";
		if (!IsClassicThemeEnabled())
		{
			auto themeClass = std::make_unique<ITheme>(currentITheme);

			LPWSTR path = nullptr;
			themeClass->get_VisualStyle(&path);

			selectedTheme->szMsstylePath = path;
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
		auto themeClass = std::make_unique<ITheme>(currentITheme);

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
	SetBitmap(hPreviewWnd, ebmp);

	return 0;
}

BOOL CAppearanceDlgProc::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	pWndPreview = nullptr;
	return 0;
}

BOOL CAppearanceDlgProc::OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CAppearanceDlgBox dlg;
	int ret = dlg.DoModal();

	if (ret == 1)
	{
		if (selectedTheme->selectedScheme)
		{
			if (selectedTheme->selectedScheme->variant == CUSTOM_SCHEME)
			{
				free(selectedTheme->selectedScheme);
				selectedTheme->selectedScheme = NULL;
			}
		}
	}
	else
	{
		SetModified(TRUE);
	}

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str()), &ebmp, UPDATE_SOLIDCLR | UPDATE_WINDOW);
	SetBitmap(hPreviewWnd, ebmp);
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

	selectedTheme->szMsstylePath = data;
	selectedTheme->fMsstyleChanged = true;

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str()), &ebmp, UPDATE_SOLIDCLR | UPDATE_WINDOW);
	SetBitmap(hPreviewWnd, ebmp);

	SetModified(TRUE);
	return 0;
}

BOOL CAppearanceDlgProc::OnClrComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int i = ComboBox_GetCurSel(hThemesCombobox);
	LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
	_UpdateFontBox(data);

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str()), &ebmp, UPDATE_SOLIDCLR | UPDATE_WINDOW);
	SetBitmap(hPreviewWnd, ebmp);

	SetModified(TRUE);
	return 0;
}

BOOL CAppearanceDlgProc::OnFontComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int i = ComboBox_GetCurSel(hSizeCombobox);
	SCHEMEDATA* data = (SCHEMEDATA*)ComboBox_GetItemData(hSizeCombobox, i);

	if (selectedTheme->selectedScheme)
	{
		if (selectedTheme->selectedScheme->variant == CUSTOM_SCHEME)
		{
			free(selectedTheme->selectedScheme);
			selectedTheme->selectedScheme = NULL;
		}
	}
	selectedTheme->selectedScheme = data;
	selectedTheme->newColor = NcGetSysColor(COLOR_BACKGROUND);

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str()), &ebmp, UPDATE_SOLIDCLR | UPDATE_WINDOW);
	SetBitmap(hPreviewWnd, ebmp);

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
			if (selectedTheme->szMsstylePath.compare(data) == 0)
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
		pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(selectedTheme->szMsstylePath.c_str()), &ebmp, flags);
		SetBitmap(hPreviewWnd, ebmp);
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
		if (!selectedTheme->selectedScheme->dpiScaled)
		{
			ScaleNonClientMetrics(ncm, GetDpiForWindow(m_hWnd));
		}
		else
		{
			// scale the fonts
			ScaleLogFont(ncm.lfCaptionFont, GetDpiForWindow(m_hWnd));
			ScaleLogFont(ncm.lfSmCaptionFont, GetDpiForWindow(m_hWnd));
			ScaleLogFont(ncm.lfMenuFont, GetDpiForWindow(m_hWnd));
			ScaleLogFont(ncm.lfStatusFont, GetDpiForWindow(m_hWnd));
			ScaleLogFont(ncm.lfMessageFont, GetDpiForWindow(m_hWnd));
		}
		ScaleLogFont(lfIcon, GetDpiForWindow(m_hWnd));
		SystemParametersInfo(SPI_SETICONTITLELOGFONT, sizeof(LOGFONT), (PVOID)&lfIcon, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), (PVOID)&ncm, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

		// delete temp scheme on combobox change
	}
	else
	{
		SetSystemVisualStyle(selectedTheme->szMsstylePath.c_str(), nullptr, nullptr, AT_NONE);
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

		if (selectedTheme->selectedScheme)
		{
			if (selectedTheme->selectedScheme->variant == CUSTOM_SCHEME)
			{
				free(selectedTheme->selectedScheme);
				selectedTheme->selectedScheme = NULL;
			}
		}
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
			if (data->variant & HAS_NORMAL)
			{
				int i = ComboBox_AddString(hSizeCombobox, L"Normal");
				ComboBox_SetItemData(hSizeCombobox, i, &schemeMap[data->schemeMapIndex]);
			}
			if (data->variant & HAS_EXTRA_LARGE)
			{
				int i = ComboBox_AddString(hSizeCombobox, L"Extra Large");
				ComboBox_SetItemData(hSizeCombobox, i, &schemeMap[data->schemeMapIndex + 1]);
			}
			if (data->variant & HAS_LARGE)
			{
				int i = ComboBox_AddString(hSizeCombobox, L"Large");
				ComboBox_SetItemData(hSizeCombobox, i, &schemeMap[data->schemeMapIndex + 2]);
			}
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

	if (wcsstr(theme, L"@"))
	{
		WCHAR buffer[256];
		HRESULT hr = SHLoadIndirectString(theme, buffer, ARRAYSIZE(buffer), NULL);
		if (SUCCEEDED(hr))
		{
			wcscpy_s(data.name, 40, buffer);
		}
		else
		{
			size_t len = wcslen(theme) + 1;
			wcscpy_s(data.name, len, theme);
		}
	}
	else
	{
		size_t len = wcslen(theme) + 1;
		wcscpy_s(data.name, len, theme);
	}
	schemeMap[index] = data;

	free(value);
}
