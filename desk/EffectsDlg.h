#pragma once
#include "pch.h"

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
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnFontChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnAnimChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
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
	BOOL _fDropShadows;
	BOOL _fDragWindow;
	BOOL _fAltIndicator;
	UINT _iAnimEnabled;
};

