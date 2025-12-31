#include "pch.h"
#include "EffectsDlg.h"


BOOL CEffectsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_chkAnim = GetDlgItem(1175);
	_cmbAnim = GetDlgItem(1182);
	_chkFont = GetDlgItem(1177);
	_cmbFont = GetDlgItem(1184);
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

	SystemParametersInfo(SPI_GETFONTSMOOTHINGTYPE, 0, &_iSmoothingType, 0);
	ComboBox_SetCurSel(_cmbFont, _iSmoothingType - 1);

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
	_fAnimType = index;
	ComboBox_SetCurSel(_cmbAnim, index);

	SystemParametersInfo(SPI_GETDROPSHADOW, 0, &_fDropShadows, 0);
	Button_SetCheck(_chkShadow, _fDropShadows);

	SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, 0, &_fDragWindow, 0);
	Button_SetCheck(_chkDragWnd, _fDragWindow);

	SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &_fAltIndicator, 0);
	Button_SetCheck(_chkAltIndicator, !_fAltIndicator);

	return TRUE;
}

BOOL CEffectsDlg::OnAnimChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	BOOL bChecked = Button_GetCheck(hWnd);
	::ComboBox_Enable(_cmbAnim, !bChecked);
	Button_SetCheck(hWnd, !bChecked);
	_iAnimEnabled = !bChecked;

	flags |= UPDATE_ANIM;
	return 0;
}

BOOL CEffectsDlg::OnAnimCmbChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	// 0- fade, 1- scroll
	int index = ComboBox_GetCurSel(hWnd);
	_fAnimType = index;

	flags |= UPDATE_ANIM;
	return 0;
}

BOOL CEffectsDlg::OnFontChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	BOOL bChecked = Button_GetCheck(hWnd);
	::ComboBox_Enable(_cmbFont, bChecked);
	_fSmoothingEnabled = bChecked;

	flags |= UPDATE_FONT;
	return 0;
}

BOOL CEffectsDlg::OnFontCmbChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_iSmoothingType = ComboBox_GetCurSel(hWnd) + 1;

	flags |= UPDATE_FONT;
	return 0;
}

BOOL CEffectsDlg::OnShadowChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_fDropShadows = Button_GetCheck(hWnd);

	flags |= UPDATE_DROPSHADOW;
	return 0;
}

BOOL CEffectsDlg::OnWindowChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_fDragWindow = Button_GetCheck(hWnd);

	flags |= UPDATE_WINDOWDRAG;
	return 0;
}

BOOL CEffectsDlg::OnAltChk(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_fAltIndicator = !Button_GetCheck(hWnd);

	flags |= UPDATE_ALT;
	return 0;
}

LRESULT CEffectsDlg::OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{	
	if (flags & UPDATE_ANIM)
	{
		// set both menu and tooltip, like xp
		SystemParametersInfo(SPI_SETMENUANIMATION, 0, (PVOID)(_iAnimEnabled > 0), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETTOOLTIPANIMATION, 0, (PVOID)(_iAnimEnabled > 0), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

		SystemParametersInfo(SPI_SETMENUFADE, 0, (PVOID)!_fAnimType, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETTOOLTIPFADE, 0, (PVOID)!_fAnimType, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}

	if (flags & UPDATE_FONT)
	{
		SystemParametersInfo(SPI_SETFONTSMOOTHING, _fSmoothingEnabled, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETFONTSMOOTHINGTYPE, 0, (PVOID)_iSmoothingType, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}

	if (flags & UPDATE_DROPSHADOW)
	{
		SystemParametersInfo(SPI_SETDROPSHADOW, 0, (PVOID)_fDropShadows, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}

	if (flags & UPDATE_WINDOWDRAG)
	{
		SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, _fDragWindow, 0, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}

	if (flags & UPDATE_ALT)
	{
		SystemParametersInfo(SPI_SETKEYBOARDCUES, 0, (PVOID)_fAltIndicator, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}
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
