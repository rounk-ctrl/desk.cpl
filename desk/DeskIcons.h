#pragma once
#include "pch.h"


class CDesktopIconsDlg :
	public CPropertyPageImpl<CDesktopIconsDlg>
{
public:
	enum { IDD = 29952 };

private:
	BEGIN_MSG_MAP(CDesktopIconsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	LRESULT OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	void OnClose();

	HWND hComputer;
	HWND hUser;
	HWND hNetwork;
	HWND hRecycler;
	HWND hCpanel;
};

