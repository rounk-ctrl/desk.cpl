#include "pch.h"
#include "AppearanceDlgBox.h"
#include "helper.h"
#include "cscheme.h"

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

	for (int i = 1; i < 10; ++i) // 28
	{
		WCHAR szBuffer[40];
		if (LoadString(g_hThemeUI, 1400 + i, szBuffer, ARRAYSIZE(szBuffer)))
		{
			int index = ComboBox_AddString(hElementCombobox, szBuffer);
			ComboBox_SetItemData(hElementCombobox, index, &info[i-1]);
			
			if (i == 1)
			{
				ComboBox_SetCurSel(hElementCombobox, index);
				_UpdateControls(&info[i-1]);
				_UpdateBitmaps(&info[i-1]);
			}
		}
	}
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

void CAppearanceDlgBox::OnClose()
{
	EndDialog(0);
}
