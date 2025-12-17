#pragma once
#include "pch.h"
#include "wndprvw.h"

class CAppearanceDlgBox :
	public CDialogImpl<CAppearanceDlgBox>
{
public:
	enum {IDD = IDD_APPEARANCEBOX };

private:
	BEGIN_MSG_MAP(CAppearanceDlgBox)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnClose();
	LRESULT OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);
	LRESULT OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled);

	void _SetComboboxData();


	HWND hElementCombobox;
	HWND hPreview;
	SIZE size;
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
};