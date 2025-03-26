#pragma once
#include "pch.h"

class CAppearanceDlgProc
	: public WTL::CPropertyPageImpl<CAppearanceDlgProc>
{
public:
	enum {IDD = IDD_APPEARANCEDLG};

private:
	BEGIN_MSG_MAP(CAppearanceDlgProc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CAppearanceDlgProc>)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	HWND hThemesCombobox;
	HWND hColorCombobox;
	HWND hSizeCombobox;
	std::vector<LPWSTR> msstyle;
};
