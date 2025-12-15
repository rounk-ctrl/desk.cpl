#pragma once
#include "pch.h"
#include "desk.h"

extern int cs_dpi;

DWORD NcGetSysColor(int nIndex);
HBRUSH NcGetSysColorBrush(int nIndex);
BOOL NcDrawFrameControl(HDC hdc, RECT* lprc, UINT uType, int type);
int NcGetSystemMetrics(int nIndex);
