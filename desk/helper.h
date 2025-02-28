#pragma once
#include "framework.h"
#include "strnatcmp.h"
#include <wil/registry.h>

struct NaturalComparator {
	bool operator()(const LPCSTR& a, const LPCSTR& b) const {
		return strnatcasecmp(a, b) < 0;
	}
};

static LPCSTR ConvertStr(LPCWSTR wideStr) {
	int size_needed = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
	char* narrowStr = new char[size_needed];
	WideCharToMultiByte(CP_ACP, 0, wideStr, -1, narrowStr, size_needed, NULL, NULL);
	return narrowStr;
}

static LPWSTR ConvertStr2(LPCSTR narrowStr) {
	int size_needed = MultiByteToWideChar(CP_ACP, 0, narrowStr, -1, NULL, 0);

	LPWSTR wideStr = new WCHAR[size_needed];
	MultiByteToWideChar(CP_ACP, 0, narrowStr, -1, wideStr, size_needed);
	return wideStr;
}

static std::wstring DecodeTranscodedImage()
{
	auto data = wil::reg::try_get_value_binary(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"TranscodedImageCache", RRF_RT_REG_BINARY);

	if (data.has_value())
	{
		std::wstring wallpaperPath(reinterpret_cast<wchar_t*>(data.value().data() + 24));
		return wallpaperPath;
	}
	return NULL;
}

static VOID _TerminateProcess(PROCESS_INFORMATION& hp)
{
	if (hp.hProcess != nullptr)
	{
		TerminateProcess(hp.hProcess, 0);
		CloseHandle(hp.hThread);
		CloseHandle(hp.hProcess);
		hp.hProcess = nullptr;
		hp.hThread = nullptr;
	}
}
