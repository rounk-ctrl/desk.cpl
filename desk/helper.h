#pragma once
#include "framework.h"
#include "desk.h"

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
void FreeBitmap(Gdiplus::Bitmap** bmp);
HRESULT DrawBitmapIfNotNull(Gdiplus::Bitmap* bmp, Gdiplus::Graphics* graph, Gdiplus::Rect rect);
HTHEME OpenNcThemeData(LPVOID file, LPCWSTR pszClassList);
BOOL IsClassicThemeEnabled();
char* trim(char* s);
wchar_t* strCut(wchar_t* s, const wchar_t* pattern);
void ScaleLogFont(LOGFONT& lf, int dpi);
void ScaleNonClientMetrics(NONCLIENTMETRICS& ncm, int dpi);
void ScaleNonClientMetrics(NONCLIENTMETRICSW_2k& ncm, int dpi);
HRESULT GetSolidBtnBmp(COLORREF clr, int dpi, SIZE size, HBITMAP* pbOut);
BOOL ColorPicker(COLORREF clr, HWND hWnd, CHOOSECOLOR* clrOut);
void CreateBlankScheme();
void SetBitmap(HWND hWnd, HBITMAP hBmp);

inline SIZE GetClientSIZE(HWND _hwnd)
{
	RECT rect;
	GetClientRect(_hwnd, &rect);
	return { RECTWIDTH(rect), RECTHEIGHT(rect) };
}

