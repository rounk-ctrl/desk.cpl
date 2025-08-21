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

enum COLORMODES
{
	E4_BIT = 1 << 0,
	E8_BIT = 1 << 1,
	E16_BIT = 1 << 2,
	E24_BIT = 1 << 3,
	E32_BIT = 1 << 4,
};
DEFINE_ENUM_FLAG_OPERATORS(COLORMODES);

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
	void _BuildColorList();

	HWND _cmbMonitors;
	HWND _textDisplay;
	HWND _chkPrimary;
	HWND _chkExtend;
	HWND _mulMonPreview;
	HWND _textCurrentRes;
	HWND _trackResolution;
	HWND _cmbColors;
	std::vector<RESINFO> _arrResInfo;
	std::vector<DWORD> _arrSupportedBpp;
	CURRENT_RESINFO _currentResInfo;
};
