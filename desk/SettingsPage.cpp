#include "pch.h"
#include "SettingsPage.h"
#include "helper.h"

BOOL CSettingsDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_cmbMonitors = GetDlgItem(1800);
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
					DWORD dwSize;
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
							break;
						}
					}
				}
				WCHAR* wideName = CA2W(monitorName);
				WCHAR name[256] = {};
				StringCchPrintf(name, ARRAYSIZE(name), L"%d. %s on %s", count, found ? wideName : ddMon.DeviceString, dev.DeviceString);

				ComboBox_AddString(_cmbMonitors, name);
				count++;

				// cleanup
				SetupDiDeleteDeviceInterfaceData(hDevInfo, &ifData);
				SetupDiDestroyDeviceInfoList(hDevInfo);
				RegCloseKey(key);
			}
		}
	}

	return 0;
}