#pragma once
#include "pch.h"
#include "wndprvw.h"

class CAppearanceDlgProc
	: public WTL::CPropertyPageImpl<CAppearanceDlgProc>
{
public:
	enum {IDD = IDD_APPEARANCEDLG};

private:
	BEGIN_MSG_MAP(CAppearanceDlgProc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(1111, CBN_SELCHANGE, OnComboboxChange)
		COMMAND_HANDLER(1117, BN_CLICKED, OnAdvanced)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CAppearanceDlgProc>)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnSetActive();


	HWND hThemesCombobox;
	HWND hColorCombobox;
	HWND hSizeCombobox;
	HWND hPreviewWnd;
	SIZE size;
	std::vector<LPWSTR> msstyle;
	Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;
};
