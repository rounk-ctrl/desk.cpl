#pragma once
#include "pch.h"

class CVersion
{
private:
	RTL_OSVERSIONINFOEXW m_osvi;
	void _FillVersionInfo();

public:
	CVersion();
	ULONG BuildNumber();
	ULONG MajorVersion();
	ULONG MinorVersion();
};

extern CVersion g_osVersion;