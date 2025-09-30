#include "pch.h"
#include "ThemeChngDlg.h"

BOOL CThemeChngDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_wndQuestion = GetDlgItem(1830);
	_wndTimerText = GetDlgItem(1831);
	_timerCount = 15;

	HICON hIcon = LoadIcon(NULL, IDI_QUESTION);
	::SendMessage(_wndQuestion, STM_SETICON, (WPARAM)hIcon, 0);
	DestroyIcon(hIcon);

	WCHAR text[32];
	StringCchPrintf(text, ARRAYSIZE(text), L"Reverting in %d seconds", _timerCount);
	::SetWindowText(_wndTimerText, text);

	SetTimer(67, 1000);
	return 0;
}

BOOL CThemeChngDlg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 67)
	{
		if (_timerCount > 0)
		{
			WCHAR text[32];
			StringCchPrintf(text, ARRAYSIZE(text), L"Reverting in %d seconds", _timerCount--);
			::SetWindowText(_wndTimerText, text);
		}
		else
		{
			KillTimer(67);
			EndDialog(1);
		}

	}
	return 0;
}

LRESULT CThemeChngDlg::OnYes(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	KillTimer(67);
	EndDialog(0);
	return 0;
}

LRESULT CThemeChngDlg::OnNo(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	KillTimer(67);
	EndDialog(1);
	return 0;
}

void CThemeChngDlg::OnClose()
{
	KillTimer(67);
	EndDialog(1);
}