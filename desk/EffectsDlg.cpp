#include "pch.h"
#include "EffectsDlg.h"


BOOL CEffectsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_chkFont = GetDlgItem(1177);
	_cmbFont = GetDlgItem(1184);
	_chkAnim = GetDlgItem(1175);
	_cmbAnim = GetDlgItem(1182);
	_chkShadow = GetDlgItem(1185);
	_chkDragWnd = GetDlgItem(1179);
	_chkAltIndicator = GetDlgItem(1181);

	SystemParametersInfo(SPI_GETFONTSMOOTHING, NULL, &_fSmoothingEnabled, 0);
	Button_SetCheck(_chkFont, _fSmoothingEnabled);
	::ComboBox_Enable(_cmbFont, _fSmoothingEnabled);

	// all font smoothing types
	LPCWSTR items[] = { L"Standard", L"ClearType", };
	for (int i = 0; i < _countof(items); i++)
	{
		ComboBox_AddString(_cmbFont, items[i]);
	}
	UINT iSmoothingType;
	SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &iSmoothingType, 0);
	ComboBox_SetCurSel(_cmbFont, iSmoothingType - 1);

	BOOL fMenuAnim, fToolTipAnim;
	SystemParametersInfo(SPI_GETMENUANIMATION, 0, &fMenuAnim, 0);
	SystemParametersInfo(SPI_GETTOOLTIPANIMATION, 0, &fToolTipAnim, 0);

	if (fMenuAnim && fToolTipAnim) _iAnimEnabled = 1;
	else if (fMenuAnim || fToolTipAnim) _iAnimEnabled = 2;
	else _iAnimEnabled = 0;

	Button_SetCheck(_chkAnim, _iAnimEnabled);
	::ComboBox_Enable(_cmbAnim, _iAnimEnabled > 0 ? 1 : 0);

	// all scroll types
	LPCWSTR itemsS[] = { L"Fade effect", L"Scroll effect", };
	for (int i = 0; i < _countof(itemsS); i++)
	{
		ComboBox_AddString(_cmbAnim, itemsS[i]);
	}

	BOOL fTooltipFade, fMenuFade;
	SystemParametersInfo(SPI_GETTOOLTIPFADE, 0, &fTooltipFade, 0);
	SystemParametersInfo(SPI_GETMENUFADE, 0, &fMenuFade, 0);

	int index = 0;

	// check both
	if (fToolTipAnim) index = fTooltipFade ? 0 : 1;
	if (fMenuAnim) index = fMenuFade ? 0 : 1;
	ComboBox_SetCurSel(_cmbAnim, index);

	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &_fDropShadows, 0);
	Button_SetCheck(_chkShadow, _fDropShadows);

	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &_fDragWindow, 0);
	Button_SetCheck(_chkDragWnd, _fDragWindow);

	SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &_fAltIndicator, 0);
	Button_SetCheck(_chkAltIndicator, !_fAltIndicator);

	return TRUE;
}

BOOL CEffectsDlg::OnFontChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	BOOL bChecked = Button_GetCheck(hWnd);
	::ComboBox_Enable(_cmbFont, bChecked);
	return 0;
}

BOOL CEffectsDlg::OnAnimChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	BOOL bChecked = Button_GetCheck(hWnd);
	::ComboBox_Enable(_cmbAnim, !bChecked);
	Button_SetCheck(_chkAnim, !bChecked);
	return 0;
}

LRESULT CEffectsDlg::OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{	
	EndDialog(0);
	return 0;
}

LRESULT CEffectsDlg::OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	EndDialog(1);
	return 0;
}

void CEffectsDlg::OnClose()
{
	EndDialog(0);
}
