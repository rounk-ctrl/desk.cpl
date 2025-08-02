#include "pch.h"
#include "AppearanceDlgBox.h"

BOOL CAppearanceDlgBox::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hThemesCombobox = GetDlgItem(1400);

	return TRUE;
}

void CAppearanceDlgBox::OnClose()
{
	EndDialog(0);
}
