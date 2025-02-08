#include "version.h"

CVersion g_osVersion;

typedef NTSTATUS(NTAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOEXW);

void CVersion::_FillVersionInfo()
{
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	RtlGetVersion_t RtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion");
	if (RtlGetVersion)
	{
		RtlGetVersion(&m_osvi);
	}
}

CVersion::CVersion()
{
	ZeroMemory(&m_osvi, sizeof(RTL_OSVERSIONINFOEXW));
	m_osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
}

ULONG CVersion::BuildNumber()
{
	if (!m_osvi.dwBuildNumber)
		_FillVersionInfo();
	return m_osvi.dwBuildNumber;
}

ULONG CVersion::MajorVersion()
{
	if (!m_osvi.dwMajorVersion)
		_FillVersionInfo();
	return m_osvi.dwMajorVersion;
}

ULONG CVersion::MinorVersion()
{
	if (!m_osvi.dwMinorVersion)
		_FillVersionInfo();
	return m_osvi.dwMinorVersion;
}