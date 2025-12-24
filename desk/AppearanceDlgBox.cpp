#include "pch.h"
#include "AppearanceDlgBox.h"
#include "helper.h"
#include "cscheme.h"
#include "fms.h"

using namespace Microsoft::WRL::Details;

BOOL CAppearanceDlgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hElementCombobox = GetDlgItem(1126);
	hSizeUpdown = GetDlgItem(1133);
	hColor1 = GetDlgItem(1135);
	hColor2 = GetDlgItem(1141);
	hFontCmb = GetDlgItem(1129);
	hFontSize = GetDlgItem(1130);
	hFontColor = GetDlgItem(1136);
	hBold = GetDlgItem(1131);
	hItalic = GetDlgItem(1132);
	hPreview = GetDlgItem(1470);
	size = GetClientSIZE(hPreview);

	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_APPEARANCE, nullptr, GetDpiForWindow(m_hWnd));

	Microsoft::WRL::ComPtr<IWindowConfig> pConfig;
	pWndPreview.As(&pConfig);
	pConfig->SetClassicPrev(TRUE);

	HBITMAP ebmp;
	pWndPreview->GetPreviewImage(&ebmp);
	HBITMAP hOld = Static_SetBitmap(hPreview, ebmp);
	if (hOld) DeleteBitmap(hOld);

	int in = 0;
	for (int i = 1; i < 28; ++i) // 28
	{
		WCHAR szBuffer[40];
		if (LoadString(g_hThemeUI, 1400 + i, szBuffer, ARRAYSIZE(szBuffer)))
		{
			if (i == 24) continue;

			int index = ComboBox_AddString(hElementCombobox, szBuffer);
			ComboBox_SetItemData(hElementCombobox, index, &info[in]);
			
			if (i == 1)
			{
				ComboBox_SetCurSel(hElementCombobox, index);
				_UpdateControls(&info[in]);
				_UpdateBitmaps(&info[in]);
				_UpdateSizeItem(&info[in]);
			}
			in++;
		}
	}

	wchar_t** ppFontList;
	int cFontList;
	GetFilteredFontFamilies(&cFontList, &ppFontList);
	for (UINT i = 0; i < cFontList; ++i)
	{
		ComboBox_AddString(hFontCmb, ppFontList[i]);
	}

	// create a dummy scheme
	CreateBlankScheme();
	return TRUE;
}

LRESULT CAppearanceDlgBox::OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	EndDialog(0);
	return 0;
}

LRESULT CAppearanceDlgBox::OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	EndDialog(1);
	return 0;
}

BOOL CAppearanceDlgBox::OnComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int index = ComboBox_GetCurSel(hElementCombobox);
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, index);

	_UpdateControls(tinfo);
	_UpdateBitmaps(tinfo);
	_UpdateSizeItem(tinfo);
	_UpdateFont(tinfo);

	return 0;
}

BOOL CAppearanceDlgBox::OnColorPick(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int index = ComboBox_GetCurSel(hElementCombobox);
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, index);

	COLORREF clr{};
	if (id == 1135) clr = NcGetSysColor(tinfo->color1Target);
	if (id == 1136) clr = NcGetSysColor(tinfo->fontColorTarget);
	if (id == 1141) clr = NcGetSysColor(tinfo->color2Target);

	CHOOSECOLOR cc = { 0 };
	if (ColorPicker(clr, hWnd, &cc) == TRUE)
	{
		_UpdateColorButton(hWnd, true, cc.rgbResult);
		
		WORD target;
		if (id == 1135) target = tinfo->color1Target;
		if (id == 1136) target = tinfo->fontColorTarget;
		if (id == 1141) target = tinfo->color2Target;
		selectedTheme->selectedScheme->rgb[target] = cc.rgbResult;
		if (id == 1135 && tinfo->color1Target == COLOR_DESKTOP) selectedTheme->newColor = cc.rgbResult;

		_UpdatePreview(TRUE);
	}
	return 0;
}

BOOL CAppearanceDlgBox::OnSpinnerChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	int index = ComboBox_GetCurSel(hElementCombobox);
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, index);

	if (tinfo && tinfo->activeButton & ACTIVE_SIZEITEM)
	{
		int value = GetDlgItemInt(id);
		NcUpdateSystemMetrics(tinfo->sizeTarget, value);

		if (tinfo->sizeTarget == SM_CYVSCROLL) NcUpdateSystemMetrics(SM_CXVSCROLL, value);
		if (tinfo->sizeTarget == SM_CYSIZE) NcUpdateSystemMetrics(SM_CXSIZE, value);

		_UpdatePreview(FALSE);
	}
	return 0;
}

BOOL CAppearanceDlgBox::OnFontChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, ComboBox_GetCurSel(hElementCombobox));
	LOGFONT* lf = _GetLogFontPtr(tinfo);

	int index = ComboBox_GetCurSel(hFontCmb);
	int len = ComboBox_GetLBTextLen(hFontCmb, index) + 1;
	wchar_t* szDest = (wchar_t*)malloc(len * sizeof(wchar_t));

	ComboBox_GetLBText(hFontCmb, index, szDest);
	StringCchCopy(lf->lfFaceName, ARRAYSIZE(lf->lfFaceName), szDest);

	_UpdatePreview(FALSE);
	return 0;
}

BOOL CAppearanceDlgBox::OnFontSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, ComboBox_GetCurSel(hElementCombobox));
	LOGFONT* lf = _GetLogFontPtr(tinfo);

	int fontSize = 6 + ComboBox_GetCurSel(hFontSize);
	int height = -MulDiv(fontSize, 96, 72);
	int delta = -height + lf->lfHeight;

	if (tinfo->fontTarget == 0)
	{
		selectedTheme->selectedScheme->ncm.iCaptionHeight += delta;
	}
	else if (tinfo->fontTarget == 2)
	{
		selectedTheme->selectedScheme->ncm.iMenuHeight += delta;
	}

	lf->lfHeight = height;
	_UpdateSizeItem(tinfo);
	
	_UpdatePreview(FALSE);
	return 0;
}

BOOL CAppearanceDlgBox::OnFontSizeEditChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, ComboBox_GetCurSel(hElementCombobox));
	LOGFONT* lf = _GetLogFontPtr(tinfo);

	int len = ::ComboBox_GetTextLength(hFontSize) + 1;
	wchar_t* szDest = (wchar_t*)malloc(len * sizeof(wchar_t));
	::ComboBox_GetText(hFontSize, szDest, len);

	int fontSize = _wtoi(szDest);
	if (fontSize > 0)
	{
		int height = -MulDiv(fontSize, 96, 72);
		int delta = -height + lf->lfHeight;

		if (tinfo->fontTarget == 0)
		{
			selectedTheme->selectedScheme->ncm.iCaptionHeight += delta;
			lf->lfHeight = height;
		}
		else if (tinfo->fontTarget == 2)
		{
			selectedTheme->selectedScheme->ncm.iMenuHeight += delta;
			lf->lfHeight = height;
		}

		_UpdateSizeItem(tinfo);
	}

	_UpdatePreview(FALSE);
	return 0;
}

BOOL CAppearanceDlgBox::OnStyle(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	SCHEMEINFO* tinfo = (SCHEMEINFO*)ComboBox_GetItemData(hElementCombobox, ComboBox_GetCurSel(hElementCombobox));
	LOGFONT* lf = _GetLogFontPtr(tinfo);

	if (id == 1131)
	{
		lf->lfWeight = fIsBold ? FW_NORMAL : FW_BOLD;
		fIsBold = !fIsBold;
		Button_SetCheck(hBold, fIsBold);
	}
	if (id == 1132)
	{
		lf->lfItalic = !fIsItalic;
		fIsItalic = !fIsItalic;
		Button_SetCheck(hItalic, fIsItalic);
	}
	_UpdatePreview(FALSE);
	return 0;
}

void CAppearanceDlgBox::_UpdateControls(SCHEMEINFO* info)
{
	::EnableWindow(hSizeUpdown, info->activeButton & ACTIVE_SIZEITEM);
	::EnableWindow((HWND)SendMessage(hSizeUpdown, UDM_GETBUDDY, 0, 0), info->activeButton & ACTIVE_SIZEITEM);
	::EnableWindow(hColor1, info->activeButton & ACTIVE_COLOR1);
	::EnableWindow(hColor2, info->activeButton & ACTIVE_COLOR2);
	::EnableWindow(hFontCmb, info->activeButton & ACTIVE_FONT);
	::EnableWindow(hFontSize, info->activeButton & ACTIVE_FONT);
	::EnableWindow(hFontColor, info->activeButton & ACTIVE_FONTCOLOR);
	::EnableWindow(hBold, info->activeButton & ACTIVE_FONT);
	::EnableWindow(hItalic, info->activeButton & ACTIVE_FONT);
}

void CAppearanceDlgBox::_UpdateBitmaps(SCHEMEINFO* info)
{
	_UpdateColorButton(hColor1, info->activeButton & ACTIVE_COLOR1, NcGetSysColor(info->color1Target));
	_UpdateColorButton(hColor2, info->activeButton & ACTIVE_COLOR2, NcGetSysColor(info->color2Target));
	_UpdateColorButton(hFontColor, info->activeButton & ACTIVE_FONTCOLOR, NcGetSysColor(info->fontColorTarget));
}

void CAppearanceDlgBox::_UpdateColorButton(HWND hButton, bool isActive, COLORREF color)
{
	HBITMAP bmp = NULL;
	if (isActive)
	{
		GetSolidBtnBmp(color, GetDpiForWindow(m_hWnd), GetClientSIZE(hButton), &bmp);
	}

	HBITMAP hOld = Button_SetBitmap(hButton, bmp);
	if (hOld) DeleteBitmap(hOld);
}

void CAppearanceDlgBox::_UpdateSizeItem(SCHEMEINFO* info)
{
	if (info->activeButton & ACTIVE_SIZEITEM)
	{
		WCHAR szText[5];
		StringCchPrintf(szText, ARRAYSIZE(szText), L"%d", NcGetSystemMetrics(info->sizeTarget));
		::SetWindowText((HWND)SendMessage(hSizeUpdown, UDM_GETBUDDY, 0, 0), szText);
	}
	else
	{
		::SetWindowText((HWND)SendMessage(hSizeUpdown, UDM_GETBUDDY, 0, 0), L"");
	}
}

void CAppearanceDlgBox::_UpdateFont(SCHEMEINFO* info)
{
	if (info->activeButton & ACTIVE_FONT)
	{
		LOGFONT* lf = _GetLogFontPtr(info);

		int index = ComboBox_FindString(hFontCmb, -1, lf->lfFaceName);
		ComboBox_SetCurSel(hFontCmb, index);

		int fontSize = -MulDiv(lf->lfHeight, 72, 96);
		for (int i = 6; i < 6 * 4; ++i)
		{
			wchar_t buffer[3];
			wsprintf(buffer, L"%d", i);
			ComboBox_AddString(hFontSize, buffer);
		}

		wchar_t buffer[3];
		wsprintf(buffer, L"%d", fontSize);
		ComboBox_SetCurSel(hFontSize, ComboBox_FindString(hFontSize, -1, buffer));

		fIsBold = lf->lfWeight == FW_BOLD;
		fIsItalic = lf->lfItalic;
		Button_SetCheck(hBold, fIsBold);
		Button_SetCheck(hItalic, fIsItalic);
	}
	else
	{
		::ComboBox_SetText(hFontCmb, L"");
		::ComboBox_SetText(hFontSize, L"");
		Button_SetCheck(hBold, 0);
		Button_SetCheck(hItalic, 0);
	}
}

void CAppearanceDlgBox::_UpdatePreview(BOOL fClr)
{
	UPDATEFLAGS flag = UPDATE_WINDOW;
	if (fClr) flag |= UPDATE_SOLIDCLR;

	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, nullptr, &ebmp, UPDATE_WINDOW);
	HBITMAP hPrev = Static_SetBitmap(hPreview, ebmp);
	DeleteObject(hPrev);
	DeleteObject(ebmp);
}

LOGFONT* CAppearanceDlgBox::_GetLogFontPtr(SCHEMEINFO* info)
{
	LOGFONT* lf = NULL;
	switch (info->fontTarget)
	{
		case 0: lf = &selectedTheme->selectedScheme->ncm.lfCaptionFont; break;
		case 1: lf = &selectedTheme->selectedScheme->ncm.lfSmCaptionFont; break;
		case 2: lf = &selectedTheme->selectedScheme->ncm.lfMenuFont; break;
		case 3: lf = &selectedTheme->selectedScheme->ncm.lfStatusFont; break;
		case 4: lf = &selectedTheme->selectedScheme->ncm.lfMessageFont; break;
		case 5: lf = &selectedTheme->selectedScheme->lfIconTitle; break;
		default: break;
	}
	return lf;
}

void CAppearanceDlgBox::OnClose()
{
	HBITMAP hOld = Static_SetBitmap(hPreview, NULL);
	if (hOld) DeleteBitmap(hOld);

	EndDialog(1);
}
