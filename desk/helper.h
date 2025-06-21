#pragma once
#include "framework.h"
#include "strnatcmp.h"

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

#define GETSIZE(size) (size).cx, (size).cy
#define SPLIT_COLORREF(clr) GetRValue(clr), GetGValue(clr), GetBValue(clr)

void _TerminateProcess(PROCESS_INFORMATION& hp);
COLORREF GetDeskopColor();
void EnumDir(LPCWSTR directory, LPCWSTR* extensions, int cExtensions, std::vector<LPWSTR>& vec, BOOL fEnumChildDirs);

inline SIZE GetClientSIZE(HWND _hwnd)
{
	RECT rect;
	GetClientRect(_hwnd, &rect);
	return { RECTWIDTH(rect), RECTHEIGHT(rect) };
}

