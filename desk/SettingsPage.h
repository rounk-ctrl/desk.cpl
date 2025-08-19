#pragma once
#include "pch.h"


class CSettingsDlgProc
	: public WTL::CPropertyPageImpl<CSettingsDlgProc>
{
public:
	enum { IDD = IDD_SETTINGSDLG };

private:
	BEGIN_MSG_MAP(CSettingsDlgProc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CSettingsDlgProc>)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	HWND _cmbMonitors;

};
