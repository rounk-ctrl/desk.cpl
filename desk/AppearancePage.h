#pragma once
#include "pch.h"
#include "wndprvw.h"

#define READ_AT(TYPE, BIN, OFFSET) (*reinterpret_cast<TYPE*>((BIN) + (OFFSET)))
#define READ_STRING(TYPE, BIN, OFFSET) (reinterpret_cast<TYPE*>((BIN)+ (OFFSET)))

class CAppearanceDlgProc
	: public WTL::CPropertyPageImpl<CAppearanceDlgProc>
{
public:
	enum {IDD = IDD_APPEARANCEDLG};

private:
	BEGIN_MSG_MAP(CAppearanceDlgProc)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(1111, CBN_SELCHANGE, OnComboboxChange)
		COMMAND_HANDLER(1114, CBN_SELCHANGE, OnClrComboboxChange)
		COMMAND_HANDLER(1116, CBN_SELCHANGE, OnFontComboboxChange)
		COMMAND_HANDLER(1117, BN_CLICKED, OnAdvanced)
		COMMAND_HANDLER(1118, BN_CLICKED, OnEffects)
		CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CAppearanceDlgProc>)
	END_MSG_MAP()

	BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnEffects(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnClrComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnFontComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
	BOOL OnSetActive();
	BOOL OnApply();

	void FillSchemeDataMap(LPCWSTR theme, int index);
	void _UpdateColorBox(LPWSTR data);
	void _UpdateFontBox(LPWSTR data);
	void _FixColorBox();


	HWND hThemesCombobox;
	HWND hColorCombobox;
	HWND hSizeCombobox;
	HWND hPreviewWnd;
	SIZE size;
	BOOL _fFirstInit;
	std::vector<LPWSTR> msstyle;
	Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;
	SCHEMEDATA* schemeMap = NULL;
	ULONG mapSize;

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
