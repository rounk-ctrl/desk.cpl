#pragma once
#include "framework.h"
#include "strnatcmp.h"

struct NaturalComparator {
	bool operator()(const LPCSTR& a, const LPCSTR& b) const {
		return strnatcasecmp(a, b) < 0;
	}
};

LPCSTR ConvertStr(LPCWSTR wideStr) {
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
	char* narrowStr = new char[size_needed];
	WideCharToMultiByte(CP_ACP, 0, wideStr, -1, narrowStr, size_needed, NULL, NULL);
	return narrowStr;
}

LPWSTR ConvertStr2(LPCSTR narrowStr) {
	int size_needed = MultiByteToWideChar(CP_ACP, 0, narrowStr, -1, NULL, 0);

	LPWSTR wideStr = new WCHAR[size_needed];
	MultiByteToWideChar(CP_ACP, 0, narrowStr, -1, wideStr, size_needed);
	return wideStr;
}

std::wstring DecodeTranscodedImage()
{
	HKEY hKey;
	const wchar_t* subKey = L"Control Panel\\Desktop";
	const wchar_t* valueName = L"TranscodedImageCache";

	RegOpenKeyExW(HKEY_CURRENT_USER, subKey, 0, KEY_READ, &hKey);
	std::vector<BYTE> data(1024);
	DWORD dataSize = static_cast<DWORD>(data.size());
	RegQueryValueExW(hKey, valueName, nullptr, nullptr, data.data(), &dataSize);
	RegCloseKey(hKey);

	std::wstring wallpaperPath(reinterpret_cast<wchar_t*>(data.data() + 24));
	return wallpaperPath;
}

int AddItem(HWND hListView, int rowIndex, LPCSTR text)
{
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iItem = rowIndex;
	lvItem.iSubItem = 0;
	lvItem.pszText = (LPWSTR)PathFindFileName(ConvertStr2(text));
	lvItem.lParam = (LPARAM)ConvertStr2(text);

	return ListView_InsertItem(hListView, &lvItem);
}
