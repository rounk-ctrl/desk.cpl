#include "AppearancePage.h"
#include <wil\registry.h>

#define READ_AT(TYPE, BIN, OFFSET) (*reinterpret_cast<TYPE*>((BIN).data() + (OFFSET)))
#define READ_STRING(TYPE, BIN, OFFSET) (reinterpret_cast<TYPE*>((BIN).data() + (OFFSET)))

// 29 colors
#define MAX_COLORS (COLOR_GRADIENTINACTIVECAPTION + 1)

typedef struct {
	DWORD version;
	NONCLIENTMETRICS ncm;
	LOGFONT lfIconTitle;
	COLORREF rgb[MAX_COLORS];
} SCHEMEDATA;

VOID DumpLogFont(std::vector<BYTE> value, int offset)
{
	printf("lfHeight: %d\n", READ_AT(LONG, value, offset));
	printf("lfWidth: %d\n", READ_AT(LONG, value, offset + 4));
	printf("lfEscapement: %d\n", READ_AT(LONG, value, offset + 8));
	printf("lfOrientation: %d\n", READ_AT(LONG, value, offset + 12));
	printf("lfWeight: %d\n", READ_AT(LONG, value, offset + 16));
	printf("lfItalic: %d\n", READ_AT(BYTE, value, offset + 20));
	printf("lfUnderline: %d\n", READ_AT(BYTE, value, offset + 21));
	printf("lfStrikeOut: %d\n", READ_AT(BYTE, value, offset + 22));
	printf("lfCharSet: %d\n", READ_AT(BYTE, value, offset + 23));
	printf("lfOutPrecision: %d\n", READ_AT(BYTE, value, offset + 24));
	printf("lfClipPrecision: %d\n", READ_AT(BYTE, value, offset + 25));
	printf("lfQuality: %d\n", READ_AT(BYTE, value, offset + 26));
	printf("lfPitchAndFamily: %d\n", READ_AT(BYTE, value, offset + 27));
	wprintf(L"lfFaceName: %s\n", READ_STRING(WCHAR, value, offset + 28)); // size: 64
}

VOID DumpData(LPCWSTR theme)
{
	auto value = wil::reg::get_value_binary(HKEY_CURRENT_USER, L"Control Panel\\Appearance\\Schemes", theme, RRF_RT_REG_BINARY);
	if (!value.empty())
	{
		wprintf(L"\nDumping theme info on %s:\n", theme);
		printf("Version: %u\n", READ_AT(DWORD, value, 0));

		printf("\nNONCLIENTMETRICS:\n");
		printf("cbSize: %u\n", READ_AT(UINT, value, 4));
		printf("iBorderWidth: %u\n", READ_AT(INT, value, 8));
		printf("iScrollWidth: %u\n", READ_AT(INT, value, 12));
		printf("iScrollHeight: %u\n", READ_AT(INT, value, 16));
		printf("iCaptionWidth: %u\n", READ_AT(INT, value, 20));
		printf("iCaptionHeight: %u\n", READ_AT(INT, value, 24));

		printf("\nLOGFONT: lfCaptionFont:\n");
		DumpLogFont(value, 28);

		printf("\n");
		printf("iSmCaptionWidth: %u\n", READ_AT(INT, value, 120));
		printf("iSmCaptionHeight: %u\n", READ_AT(INT, value, 124));

		printf("\nLOGFONT: lfSmCaptionFont:\n");
		DumpLogFont(value, 128);

		printf("\n");
		printf("iMenuWidth: %u\n", READ_AT(INT, value, 220));
		printf("iMenuHeight: %u\n", READ_AT(INT, value, 224));

		printf("\nLOGFONT: lfMenuFont:\n");
		DumpLogFont(value, 228);

		printf("\nLOGFONT: lfStatusFont:\n");
		DumpLogFont(value, 320);

		printf("\nLOGFONT: lfMessageFont:\n");
		DumpLogFont(value, 412);

		// doesnt have padded border ??
		printf("\nLOGFONT: lfIconTitle:\n");
		DumpLogFont(value, 504);

		SCHEMEDATA data;

		//596
		int start = 596;
		for (int i = 0; i < MAX_COLORS; i++)
		{
			data.rgb[i] = READ_AT(COLORREF, value, start + (4 * i));
		}

		COLORREF bgColor = data.rgb[COLOR_BACKGROUND];
		wprintf(L"\nCOLOR_BACKGROUND %d,%d,%d\n", GetRValue(bgColor), GetGValue(bgColor), GetBValue(bgColor));

	}
}

LRESULT CALLBACK AppearanceDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		DumpData(L"@themeui.dll,-850");
	}
	return FALSE;
}
