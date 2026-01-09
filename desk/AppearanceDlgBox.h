#pragma once
#include "pch.h"
#include "wndprvw.h"

enum ACTIVEBUTTONS
{
	ACTIVE_NONE = 1 << 0,
	ACTIVE_SIZEITEM = 1 << 1,
	ACTIVE_COLOR1 = 1 << 2,
	ACTIVE_COLOR2 = 1 << 3,
	ACTIVE_FONT = 1 << 4,
	ACTIVE_FONTCOLOR = 1 << 5,

	ACTIVE_ALL = ACTIVE_SIZEITEM | ACTIVE_COLOR1 | ACTIVE_COLOR2 | ACTIVE_FONT | ACTIVE_FONTCOLOR,
};

DEFINE_ENUM_FLAG_OPERATORS(ACTIVEBUTTONS);

typedef struct tagSCHEMEINFO
{
	ACTIVEBUTTONS activeButton;
	int muiIndex;
	WORD color1Target;
	WORD color2Target;
	WORD fontColorTarget;

	WORD sizeTarget;
	WORD fontTarget;
	// todo: 
} SCHEMEINFO;

class CAppearanceDlgBox :
	public CDialogImpl<CAppearanceDlgBox>
{
public:
	enum {IDD = IDD_APPEARANCEBOX };

private:
	BEGIN_MSG_MAP(CAppearanceDlgBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(1126, CBN_SELCHANGE, OnComboboxChange)
		COMMAND_HANDLER(1129, CBN_SELCHANGE, OnFontChange)
		COMMAND_HANDLER(1130, CBN_SELCHANGE, OnFontSizeChange)
		COMMAND_HANDLER(1130, CBN_EDITCHANGE, OnFontSizeEditChange)
		COMMAND_HANDLER(1135, BN_CLICKED, OnColorPick)
		COMMAND_HANDLER(1136, BN_CLICKED, OnColorPick)
		COMMAND_HANDLER(1141, BN_CLICKED, OnColorPick)
		COMMAND_HANDLER(1128, EN_CHANGE, OnSpinnerChange)
		NOTIFY_CODE_HANDLER(UDN_DELTAPOS, OnSpinnerDelta)
		COMMAND_HANDLER(1131, BN_CLICKED, OnStyle)
		COMMAND_HANDLER(1132, BN_CLICKED, OnStyle)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnClose();
	LRESULT OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	LRESULT OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	BOOL OnComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnColorPick(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnSpinnerChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnSpinnerDelta(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled);
	BOOL OnFontChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnFontSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnFontSizeEditChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnStyle(UINT code, UINT id, HWND hWnd, BOOL& bHandled);

	void _UpdateControls(SCHEMEINFO* info);
	void _UpdateBitmaps(SCHEMEINFO* info);
	void _UpdateColorButton(HWND hButton, bool isActive, COLORREF color);
	void _UpdateSizeItem(SCHEMEINFO* info);
	void _UpdateFont(SCHEMEINFO* info);

	// helpers
	void _UpdatePreview(BOOL fClr);
	LOGFONT* _GetLogFontPtr(SCHEMEINFO* info);

	HWND hElementCombobox;
	HWND hSizeUpdown;
	HWND hColor1;
	HWND hColor2;
	HWND hFontCmb;
	HWND hFontSize;
	HWND hFontColor;
	HWND hBold;
	HWND hItalic;
	HWND hPreview;
	SIZE size;
	BOOL fIsBold;
	BOOL fIsItalic;
	Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;

	// -ve sizes are fixed with preview size in wndprvw
	MYWINDOWINFO wnd[3] =
	{
		{
			WT_INACTIVE,
			{10, 10, -37, 10 + 134}
		},
		{
			WT_ACTIVE,
			{20, 35, -30, 35 + 134}
		},
		{
			WT_MESSAGEBOX,
			{-75, 64, 75, 64 + 98}
		}
	};

	SCHEMEINFO info[28] =
	{
		{	// desktop (1)
			.activeButton = ACTIVE_COLOR1,
			.color1Target = COLOR_DESKTOP,
		},
		{	//inactive titlebar (2)
			.activeButton = ACTIVE_ALL,
			.color1Target = COLOR_INACTIVECAPTION,
			.color2Target = COLOR_GRADIENTINACTIVECAPTION,
			.fontColorTarget = COLOR_INACTIVECAPTIONTEXT,
			.sizeTarget = SM_CYSIZE,
			.fontTarget = 0,
		},
		{	// inactive window border (3)
			.activeButton = ACTIVE_SIZEITEM | ACTIVE_COLOR1,
			.color1Target = COLOR_INACTIVEBORDER,
			.sizeTarget = SM_CYBORDER,
		},
		{	// active titlebar (4)
			.activeButton = ACTIVE_ALL,
			.color1Target = COLOR_ACTIVECAPTION,
			.color2Target = COLOR_GRADIENTACTIVECAPTION,
			.fontColorTarget = COLOR_CAPTIONTEXT,
			.sizeTarget = SM_CYSIZE,
			.fontTarget = 0,
		},
		{	// active window border (5)
			.activeButton = ACTIVE_SIZEITEM | ACTIVE_COLOR1,
			.color1Target = COLOR_ACTIVEBORDER,
			.sizeTarget = SM_CYBORDER,
		},
		{	// menu (6)
			.activeButton = ACTIVE_ALL & ~ACTIVE_COLOR2,
			.color1Target = COLOR_MENU,
			.fontColorTarget = COLOR_MENUTEXT,
			.sizeTarget = SM_CYMENUSIZE,
			.fontTarget = 2,
		},
		{	// selected items (7)
			.activeButton = ACTIVE_ALL & ~ACTIVE_COLOR2,
			.color1Target = COLOR_HIGHLIGHT,
			.fontColorTarget = COLOR_HIGHLIGHTTEXT,
			.sizeTarget = SM_CYMENUSIZE,
			.fontTarget = 2,
		},
		{	// window (8)
			.activeButton = ACTIVE_COLOR1 | ACTIVE_FONTCOLOR,
			.color1Target = COLOR_WINDOW,
			.fontColorTarget = COLOR_WINDOWTEXT,
		},
		{	// scrollbar (9)
			.activeButton = ACTIVE_SIZEITEM | ACTIVE_COLOR1,
			.color1Target = COLOR_SCROLLBAR,		// MOD
			.sizeTarget = SM_CYVSCROLL,				// change both
		},
		{	// 3d object (10)
			.activeButton = ACTIVE_COLOR1 | ACTIVE_FONTCOLOR,
			.color1Target = COLOR_BTNFACE,
			.fontColorTarget = COLOR_BTNTEXT,
		},
		{	// palette title (11)
			.activeButton = ACTIVE_SIZEITEM | ACTIVE_FONT,
			.sizeTarget = SM_CYSMSIZE,				// change both
			.fontTarget = 1,
		},
		{ }, // REMOVED (12)
		{	// caption buttons (13)
			.activeButton = ACTIVE_SIZEITEM,
			.sizeTarget = SM_CYSIZE,				// change both
		},
		{}, // REMOVED (14)
		{	// message box (15)
			.activeButton = ACTIVE_FONT | ACTIVE_FONTCOLOR,
			.fontColorTarget = COLOR_WINDOWTEXT,
			.fontTarget = 4,
		},
		{}, // REMOVED (16)
		{	// application background (17)
			.activeButton = ACTIVE_COLOR1,
			.color1Target = COLOR_APPWORKSPACE,
		},
		{}, // REMOVED (18)
		{},	// REMOVED (19)
		{	// horizontal icon (20)
			.activeButton = ACTIVE_SIZEITEM,		// TODO: fix
			.sizeTarget = SM_CXICONSPACING,			
		},
		{	// vertical icon (21)
			.activeButton = ACTIVE_SIZEITEM,		// TODO: fix
			.sizeTarget = SM_CYICONSPACING,
		},
		{	// tooltip (22)
			.activeButton = ACTIVE_COLOR1 | ACTIVE_FONT | ACTIVE_FONTCOLOR,
			.color1Target = COLOR_INFOBK,
			.fontColorTarget = COLOR_INFOTEXT,
			.fontTarget = 3,
		},
		{	// icon (23)
			.activeButton = ACTIVE_SIZEITEM | ACTIVE_FONT,
			.sizeTarget = SM_CXICON,
			.fontTarget = 5,
		}, 
		{}, // skip 24- small icon
		{	// disabled (25)
			.activeButton = ACTIVE_FONTCOLOR,
			.fontColorTarget = COLOR_GRAYTEXT,
		},
		{	// hyperlink (26)
			.activeButton = ACTIVE_FONTCOLOR,
			.fontColorTarget = COLOR_HOTLIGHT,
		},
		{	// padded border (27)
			.activeButton = ACTIVE_SIZEITEM,
			.sizeTarget = SM_CXPADDEDBORDER,
		}
	};
};