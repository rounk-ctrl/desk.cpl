#pragma once
#include "framework.h"

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

#define GETSIZE(size) (size).cx, (size).cy
#define SPLIT_COLORREF(clr) GetRValue(clr), GetGValue(clr), GetBValue(clr)

#pragma comment(lib, "ntdll.lib")
// ignore warning, resolved by ntdll.lib
extern "C" NTSTATUS NTAPI NtOpenSection(
	OUT PHANDLE SectionHandle,
	IN ACCESS_MASK DesiredAccess,
	IN POBJECT_ATTRIBUTES ObjectAttributes
);


void _TerminateProcess(PROCESS_INFORMATION& hp);
COLORREF GetDeskopColor();
void EnumDir(LPCWSTR directory, LPCWSTR* extensions, int cExtensions, std::vector<LPWSTR>& vec, BOOL fEnumChildDirs);
void FreeString(LPWSTR& str);
void StringCpy(LPWSTR& dest, LPWSTR src);
void FreeBitmap(Gdiplus::Bitmap** bmp);
HRESULT DrawBitmapIfNotNull(Gdiplus::Bitmap* bmp, Gdiplus::Graphics* graph, Gdiplus::Rect rect);
HTHEME OpenNcThemeData(LPVOID file, LPCWSTR pszClassList);
BOOL IsClassicThemeEnabled();
char* trim(char* s);

inline SIZE GetClientSIZE(HWND _hwnd)
{
	RECT rect;
	GetClientRect(_hwnd, &rect);
	return { RECTWIDTH(rect), RECTHEIGHT(rect) };
}

