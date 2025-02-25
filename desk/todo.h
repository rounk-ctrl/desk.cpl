#pragma once
/*
#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>

CComModule _Module;
CAtlWinModule _AtlWinModule;

BOOL CALLBACK FindWorkerW(HWND hwnd, LPARAM lParam)
{
	HWND hwndShellDll = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
	if (hwndShellDll)
	{
		HWND* pHwndWorkerW = reinterpret_cast<HWND*>(lParam);
		*pHwndWorkerW = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:
	{
		AtlAxCreateControl(L"https://www.google.com", hwnd, NULL, NULL);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DWORD WINAPI BrowserThread(LPVOID lpParam)
{
	CoInitialize(NULL);

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = g_hinst;
	wc.lpszClassName = L"ActiveDesktop";
	RegisterClass(&wc);

	AtlAxWinInit();

	HWND hwndProgman = FindWindow(L"Progman", L"Program Manager");

	SendMessage(hwndProgman, 0x052C, 0xD, 0);
	SendMessage(hwndProgman, 0x052C, 0xD, 1);

	HWND hwndWorkerW = NULL;
	EnumWindows(FindWorkerW, reinterpret_cast<LPARAM>(&hwndWorkerW));

	HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, L"ActiveDesktop", L"ActiveDesktop",
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
		hwndWorkerW, NULL, g_hinst, NULL);

	RECT rect;
	GetClientRect(hwnd, &rect);

	SetWindowLong(hwnd, GWL_EXSTYLE,
		GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);

	SetWindowLong(hwndWorkerW, GWL_EXSTYLE,
		GetWindowLong(hwndWorkerW, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CoUninitialize();
	return 0;
}
*/
