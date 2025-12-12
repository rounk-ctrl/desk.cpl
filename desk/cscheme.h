#pragma once
#include "pch.h"
#include "desk.h"


DWORD GetNcSysColor(int nIndex);
HBRUSH GetNcSysColorBrush(int nIndex);
BOOL NcDrawFrameControl(HDC hdc, RECT* lprc, UINT uType, int type);
