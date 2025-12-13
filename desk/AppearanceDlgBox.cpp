#include "pch.h"
#include "AppearanceDlgBox.h"
#include "helper.h"

using namespace Microsoft::WRL::Details;

BOOL CAppearanceDlgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hThemesCombobox = GetDlgItem(1126);
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

	for (int i = 1401; i < 1424; ++i)
	{
		if (i == 1412 || i == 1414 || i == 1416 || i == 1418 || i == 1419) continue;
		WCHAR szBuffer[40];
		LoadString(g_hinst, i, szBuffer, ARRAYSIZE(szBuffer));

		int index = ComboBox_AddString(hThemesCombobox, szBuffer);
		ComboBox_SetItemData(hThemesCombobox, index, i);
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

void CAppearanceDlgBox::OnClose()
{
	EndDialog(0);
}
