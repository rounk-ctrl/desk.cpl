#pragma once
#include "pch.h"

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

class CAppearanceDlgBox :
	public CDialogImpl<CAppearanceDlgBox>
{
public:
	enum {IDD = IDD_APPEARANCEBOX };

private:
	BEGIN_MSG_MAP(CAppearanceDlgBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		//COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		//COMMAND_ID_HANDLER(IDOK, OnOK)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnClose();

	VOID FillSchemeDataMap(LPCWSTR theme, int index);


	HWND hThemesCombobox;
	SCHEMEDATA* schemeMap = NULL;
	ULONG mapSize;

};