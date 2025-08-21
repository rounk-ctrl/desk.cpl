#pragma once
#include "pch.h"

typedef struct
{
	DWORD width;
	DWORD height;
} RESINFO;

typedef struct
{
	DWORD width;
	DWORD height;
	DWORD freq;
	DWORD bpp;
} CURRENT_RESINFO;

class CSettingsDlgProc
	: public WTL::CPropertyPageImpl<CSettingsDlgProc>
{
public:
	enum { IDD = IDD_SETTINGSDLG };

private:
	BEGIN_MSG_MAP(CSettingsDlgProc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CSettingsDlgProc>)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void _GetDisplayMonitors();
	void _SelectCurrentMonitor();
	void _GetAllModes();
	void _SetTrackbarModes(int modenum);

	HWND _cmbMonitors;
	HWND _textDisplay;
	HWND _chkPrimary;
	HWND _chkExtend;
	HWND _mulMonPreview;
	HWND _textCurrentRes;
	HWND _trackResolution;
	std::vector<RESINFO> _arrResInfo;
	CURRENT_RESINFO _currentResInfo;
};
