#include "pch.h"
#include "AppearanceDlgBox.h"

BOOL CAppearanceDlgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hThemesCombobox = GetDlgItem(1400);

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

	for (ULONG j = 0; j < mapSize; j++)
	{
		ComboBox_AddString(hThemesCombobox, schemeMap[j].name);
	}
	return TRUE;
}

void CAppearanceDlgBox::OnClose()
{
	EndDialog(0);
}

VOID CAppearanceDlgBox::FillSchemeDataMap(LPCWSTR theme, int index)
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
