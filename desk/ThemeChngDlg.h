#pragma once
#include "pch.h"

class CThemeChngDlg :
	public CDialogImpl<CThemeChngDlg>
{
public:
	enum { IDD = IDD_THEMECHNGDLG };

private:
	BEGIN_MSG_MAP(CThemeChngDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(6, OnYes)
		COMMAND_ID_HANDLER(7, OnNo)
		MSG_WM_CLOSE(OnClose)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnYes(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	LRESULT OnNo(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	void OnClose();

	HWND _wndTimerText;
	HWND _wndQuestion;
	int _timerCount;
};

