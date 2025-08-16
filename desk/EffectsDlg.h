#pragma once
#include "pch.h"

enum EF_UPDATEFLAGS
{
	UPDATE_ANIM = 1 << 0,
	UPDATE_FONT = 1 << 1,
	UPDATE_ICONS = 1 << 2,
	UPDATE_DROPSHADOW = 1 << 3,
	UPDATE_WINDOWDRAG = 1 << 4,
	UPDATE_ALT = 1 << 5,
};
DEFINE_ENUM_FLAG_OPERATORS(EF_UPDATEFLAGS);

class CEffectsDlg :
    public CDialogImpl<CEffectsDlg>
{
public:
    enum { IDD = IDD_EFFECTSDLG };

private:
	BEGIN_MSG_MAP(CAppearanceDlgBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(1175, BN_CLICKED, OnAnimChk)
		COMMAND_HANDLER(1177, BN_CLICKED, OnFontChk)
		COMMAND_HANDLER(1182, CBN_SELCHANGE, OnAnimCmbChange)
		COMMAND_HANDLER(1184, CBN_SELCHANGE, OnFontCmbChange)
		COMMAND_HANDLER(1185, BN_CLICKED, OnShadowChk)
		COMMAND_HANDLER(1179, BN_CLICKED, OnWindowChk)
		COMMAND_HANDLER(1181, BN_CLICKED, OnAltChk)

		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnAnimChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnAnimCmbChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnFontChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnFontCmbChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnShadowChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnWindowChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnAltChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);


	LRESULT OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	LRESULT OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	void OnClose();

	HWND _chkAnim;
	HWND _cmbAnim;
	HWND _chkFont;
	HWND _cmbFont;
	HWND _chkShadow;
	HWND _chkDragWnd;
	HWND _chkAltIndicator;

	BOOL _fSmoothingEnabled;
	UINT _iSmoothingType;
	BOOL _fDropShadows;
	BOOL _fDragWindow;
	BOOL _fAltIndicator;
	UINT _iAnimEnabled;			// 0- disabled, 1- both enabled, 2- either enabled
	BOOL _fAnimType;			// 0- fade, 1-scroll

	EF_UPDATEFLAGS flags;
};

