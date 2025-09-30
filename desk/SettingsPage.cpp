#include "pch.h"
#include "SettingsPage.h"
#include "helper.h"
#include "desk.h"
#include "ThemeChngDlg.h"

BOOL CSettingsDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_cmbMonitors = GetDlgItem(1800);
	_mulMonPreview = GetDlgItem(1801);
	_textDisplay = GetDlgItem(1811);
	_chkPrimary = GetDlgItem(1806);
	_chkExtend = GetDlgItem(1805);
	_textCurrentRes = GetDlgItem(1814);
	_trackResolution = GetDlgItem(1808);
	_cmbColors = GetDlgItem(1807);
	_clrPreview = GetDlgItem(1813);
	
	_GetDisplayMonitors();
	_SelectCurrentMonitor();
	_GetAllModes();
	_BuildColorList();
	return 0;
}

LRESULT CSettingsDlgProc::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HWND wnd = (HWND)lParam;
	if (wnd == _trackResolution)
	{
		int interactionType = LOWORD(wParam);
		int pos = 0;
		if (interactionType == TB_THUMBPOSITION || interactionType == TB_THUMBTRACK)
		{
			pos = HIWORD(wParam);
		}
		else
		{
			pos = (int)SendMessage(wnd, TBM_GETPOS, 0, 0);
		}

		_SetTrackbarModes(pos);
		SetModified(TRUE);
	}
	return 0;
}

BOOL CSettingsDlgProc::OnApply()
{
	int pos = (int)SendMessage(_trackResolution, TBM_GETPOS, 0, 0);
	RESINFO info = _arrResInfo[pos];

	printf("%d x %d\n", info.width, info.height);

	DEVMODE dm;
	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth = info.width;
	dm.dmPelsHeight = info.height;
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

	if (info.width != _currentResInfo.width && info.height != _currentResInfo.height)
	{
		if (ChangeDisplaySettings(&dm, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
		{
			ChangeDisplaySettings(&dm, 0);

			CThemeChngDlg dlg;
			if (dlg.DoModal() == 1)
			{
				dm.dmPelsWidth = _currentResInfo.width;
				dm.dmPelsHeight = _currentResInfo.height;
				ChangeDisplaySettings(&dm, 0);
				_SelectCurrentResolution();
			}
			else
			{
				_currentResInfo.height = info.height;
				_currentResInfo.width = info.width;
			}
		}
	}
	return 0;
}

void CSettingsDlgProc::_GetDisplayMonitors()
{
	int count = 1;

	DISPLAY_DEVICE dev;
	dev.cb = sizeof(DISPLAY_DEVICE);
	for (int i = 0; EnumDisplayDevices(NULL, i, &dev, NULL); ++i)
	{
		if (dev.StateFlags & DISPLAY_DEVICE_ACTIVE)
		{
			bool found = false;
			char monitorName[14] = { 0 };

			DISPLAY_DEVICE ddMon;
			ddMon.cb = sizeof(ddMon);
			for (int j = 0; EnumDisplayDevices(dev.DeviceName, j, &ddMon, EDD_GET_DEVICE_INTERFACE_NAME); ++j)
			{
				HDEVINFO hDevInfo = NULL;
				hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_MONITOR, NULL, NULL, DIGCF_DEVICEINTERFACE);

				SP_DEVICE_INTERFACE_DATA ifData = { sizeof(ifData) };
				SetupDiOpenDeviceInterface(hDevInfo, ddMon.DeviceID, 0, &ifData);

				SP_DEVINFO_DATA devInfo = { sizeof(devInfo) };
				SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifData, NULL, 0, NULL, &devInfo);

				HKEY key = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
				if (key != INVALID_HANDLE_VALUE)
				{
					BYTE* value;
					DWORD dwSize = 0;
					HRESULT hr = RegGetValue(key, NULL, L"EDID", RRF_RT_REG_BINARY, NULL, NULL, &dwSize);

					value = (BYTE*)malloc(dwSize);
					hr = RegGetValue(key, NULL, L"EDID", RRF_RT_REG_BINARY, NULL, value, &dwSize);

					for (DWORD i = 0; i < dwSize - 5; i++)
					{
						// https://en.wikipedia.org/wiki/Extended_Display_Identification_Data#Monitor_Descriptors

						if (value[i] == 0x00 && value[i + 1] == 0x00 &&
							value[i + 2] == 0x00 && value[i + 3] == 0xFC &&
							value[i + 4] == 0x00)
						{
							found = true;
							memcpy(monitorName, &value[i + 5], 13);
							monitorName[13] = '\0';

							trim(monitorName);
							free(value);
							break;
						}
					}
				}
				WCHAR* wideName = CA2W(monitorName);
				WCHAR name[256] = {};
				StringCchPrintf(name, ARRAYSIZE(name), L"%d. %s on %s", count, found ? wideName : ddMon.DeviceString, dev.DeviceString);

				int index = ComboBox_AddString(_cmbMonitors, name);
				ComboBox_SetItemData(_cmbMonitors, index, StrDup(dev.DeviceName));	// used to find current monitor later 
				count++;

				// cleanup
				SetupDiDeleteDeviceInterfaceData(hDevInfo, &ifData);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				RegCloseKey(key);
			}
		}
	}
}

void CSettingsDlgProc::_SelectCurrentMonitor()
{
	POINT pt;
	GetCursorPos(&pt);
	HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	
	MONITORINFOEX mi = { sizeof(mi) };
	GetMonitorInfo(hMon, &mi);

	int count = ComboBox_GetCount(_cmbMonitors);
	if (count == 1)
	{
		::ShowWindow(_cmbMonitors, SW_HIDE);
		::ShowWindow(_chkExtend, SW_HIDE);
		::ShowWindow(_chkPrimary, SW_HIDE);
		::ShowWindow(_mulMonPreview, SW_HIDE);

		ComboBox_SetCurSel(_cmbMonitors, 0);
		int leng = ComboBox_GetLBTextLen(_cmbMonitors, 0);
		WCHAR* name = (WCHAR*)malloc((leng + 1) * sizeof(WCHAR));
		ComboBox_GetLBText(_cmbMonitors, 0, name);

		WCHAR finalName[64];
		StringCchPrintf(finalName, ARRAYSIZE(finalName), L"%s", name + 3);
		::SetWindowText(_textDisplay, finalName);
	}
	else
	{
		::ShowWindow(_textDisplay, SW_HIDE);

		for (int i = 0; i < count; ++i)
		{
			LPCWSTR data = (LPCWSTR)ComboBox_GetItemData(_cmbMonitors, i);
			if (StrCmpI(data, mi.szDevice) == 0)
			{
				ComboBox_SetCurSel(_cmbMonitors, i);
			}
		}
	}
}

bool Compare(const RESINFO& a, const RESINFO& b)
{
	if (a.width != b.width)
	{
		return a.width < b.width;
	}
	return a.height < b.height;
}

void CSettingsDlgProc::_GetAllModes()
{
	_arrResInfo.clear();

	int index = ComboBox_GetCurSel(_cmbMonitors);
	LPCWSTR data = (LPCWSTR)ComboBox_GetItemData(_cmbMonitors, index);

	DEVMODE devMode = {};
	devMode.dmSize = sizeof(devMode);

	// do current
	EnumDisplaySettings(data, ENUM_CURRENT_SETTINGS, &devMode);
	_currentResInfo.width = devMode.dmPelsWidth;
	_currentResInfo.height = devMode.dmPelsHeight;
	_currentResInfo.bpp = devMode.dmBitsPerPel;
	_currentResInfo.freq = devMode.dmDisplayFrequency;

	// do all modes of current display
	for (DWORD i = 0; EnumDisplaySettings(data, i, &devMode); ++i)
	{
		BOOL exists = FALSE;
		for (int j = 0; j < _arrResInfo.size(); j++)
		{
			if (_arrResInfo[j].width == devMode.dmPelsWidth &&
				_arrResInfo[j].height == devMode.dmPelsHeight)
			{
				exists = TRUE;
				break;
			}
		}

		if (!exists)
		{
			_arrResInfo.push_back({ devMode.dmPelsWidth, devMode.dmPelsHeight });
		}

		if (std::find(_arrSupportedBpp.begin(), _arrSupportedBpp.end(), devMode.dmBitsPerPel) == _arrSupportedBpp.end())
		{
			_arrSupportedBpp.push_back(devMode.dmBitsPerPel);
		}
	}

	std::sort(_arrResInfo.begin(), _arrResInfo.end(), Compare);
	SendMessage(_trackResolution, TBM_SETRANGE, TRUE, MAKELPARAM(0, _arrResInfo.size() - 1));

	_SelectCurrentResolution();
}

void CSettingsDlgProc::_SetTrackbarModes(int modenum)
{
	SendMessage(_trackResolution, TBM_SETPOS, TRUE, (LPARAM)modenum);

	WCHAR str[64];
	StringCchPrintf(str, ARRAYSIZE(str), L"%d X %d pixels", _arrResInfo[modenum].width, _arrResInfo[modenum].height);
	::SetWindowText(_textCurrentRes, str);
}

void CSettingsDlgProc::_BuildColorList()
{
	std::sort(_arrSupportedBpp.begin(), _arrSupportedBpp.end());

	COLORMODES modes{};
	for (int i = 0; i < _arrSupportedBpp.size(); ++i)
	{
		if (_arrSupportedBpp[i] == 4)
		{
			int index = ComboBox_AddString(_cmbColors, L"Lowest (4 bit)");
			ComboBox_SetItemData(_cmbColors, index, 4);
		}
		if (_arrSupportedBpp[i] == 8)
		{
			int index = ComboBox_AddString(_cmbColors, L"Low (8 bit)");
			ComboBox_SetItemData(_cmbColors, index, 8);
		}
		if (_arrSupportedBpp[i] == 16)
		{
			int index = ComboBox_AddString(_cmbColors, L"Medium (16 bit)");
			ComboBox_SetItemData(_cmbColors, index, 16);
		}
		if (_arrSupportedBpp[i] == 24)
		{
			int index = ComboBox_AddString(_cmbColors, L"High (24 bit)");
			ComboBox_SetItemData(_cmbColors, index, 24);
		}
		if (_arrSupportedBpp[i] == 32)
		{
			int index = ComboBox_AddString(_cmbColors, L"Highest (32 bit)");
			ComboBox_SetItemData(_cmbColors, index, 32);
		}
	}

	for (int i = 0; i < ComboBox_GetCount(_cmbColors); ++i)
	{
		int bpp = (int)ComboBox_GetItemData(_cmbColors, i);
		if (bpp == _currentResInfo.bpp)
		{
			ComboBox_SetCurSel(_cmbColors, i);
		}
	}
	_UpdateColorPreview();
}

void CSettingsDlgProc::_UpdateColorPreview()
{
	int index = ComboBox_GetCurSel(_cmbColors);
	int bpp = (int)ComboBox_GetItemData(_cmbColors, index);
	
	int img = 120;
	switch (bpp)
	{
	case 8:
		img += 1;
		break;
	case 16:
		img += 3;
		break;
	case 24:
		img += 4;
		break;
	case 32:
		img += 5;
		break;
	default:
		break;
	}
	printf("image index: %d\n", img);

	SIZE size = GetClientSIZE(_clrPreview);
	HBITMAP bmp = (HBITMAP)LoadImage(g_hinst, MAKEINTRESOURCE(img), IMAGE_BITMAP, size.cx, 0, LR_DEFAULTCOLOR);
	Static_SetBitmap(_clrPreview, bmp);
}

void CSettingsDlgProc::_SelectCurrentResolution()
{
	for (int i = 0; i < _arrResInfo.size(); ++i)
	{
		if (_arrResInfo[i].width == _currentResInfo.width
			&& _arrResInfo[i].height == _currentResInfo.height)
		{
			_SetTrackbarModes(i);
			break;
		}
	}
}
