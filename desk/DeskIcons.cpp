#include "pch.h"
#include "DeskIcons.h"

DEFINE_GUID(CLSID_UserProfile,
	0x59031a47, 0x3f72, 0x44a7, 0x89, 0xc5, 0x55, 0x95, 0xfe, 0x6b, 0x30, 0xee);

BOOL CDesktopIconsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hComputer = GetDlgItem(30078);
	hUser = GetDlgItem(30077);
	hNetwork = GetDlgItem(30080);
	hRecycler = GetDlgItem(30082);
	hCpanel = GetDlgItem(30083);

	HKEY key;
	RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel", 0, KEY_READ, &key);
	if (!key) return FALSE;

	DWORD size;
	RegQueryInfoKey(key, 0, 0, 0, 0, 0, 0, &size, 0, 0, 0, 0);

	LSTATUS staus = ERROR_SUCCESS;
	for (DWORD i = 0; i <= size; ++i)
	{
		if (staus != ERROR_SUCCESS) break;

		WCHAR value[256];
		DWORD dwType;
		DWORD dwSize = ARRAYSIZE(value);
		
		DWORD dwVal;
		DWORD dwSizeVal = sizeof(DWORD);
		staus = RegEnumValue(key, i, value, &dwSize, 0, &dwType, (LPBYTE)&dwVal, &dwSizeVal);
		if (dwType == REG_DWORD)
		{
			CLSID clsid;
			HRESULT hr = CLSIDFromString(value, &clsid);

			if (SUCCEEDED(hr))
			{
				if (IsEqualGUID(clsid, CLSID_MyComputer)) Button_SetCheck(hComputer, !dwVal);
				if (IsEqualGUID(clsid, CLSID_RecycleBin)) Button_SetCheck(hRecycler, !dwVal);
				if (IsEqualGUID(clsid, CLSID_ControlPanel)) Button_SetCheck(hCpanel, !dwVal);
				if (IsEqualGUID(clsid, CLSID_UserProfile)) Button_SetCheck(hUser, !dwVal);
				if (IsEqualGUID(clsid, CLSID_NetworkExplorerFolder)) Button_SetCheck(hNetwork, !dwVal);
			}
		}
	}
	RegCloseKey(key);

	return 0;
}

LRESULT CDesktopIconsDlg::OnOK(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	return LRESULT();
}

LRESULT CDesktopIconsDlg::OnCancel(UINT uNotifyCode, int nID, HWND hWnd, BOOL& bHandled)
{
	return LRESULT();
}

void CDesktopIconsDlg::OnClose()
{
}
