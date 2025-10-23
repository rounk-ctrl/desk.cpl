#include "pch.h"
#include "AppearanceDlgBox.h"
#include "helper.h"

using namespace Microsoft::WRL::Details;

BOOL CAppearanceDlgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hThemesCombobox = GetDlgItem(1400);
	hPreview = GetDlgItem(1470);
	size = GetClientSIZE(hPreview);

	HBITMAP ebmp;
	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_APPEARANCE, nullptr, GetDpiForWindow(m_hWnd));

	Microsoft::WRL::ComPtr<IWindowConfig> pConfig;
	pWndPreview.As(&pConfig);
	pConfig->SetClassicPrev(TRUE);

	pWndPreview->GetPreviewImage(&ebmp);

	Static_SetBitmap(hPreview, ebmp);
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
