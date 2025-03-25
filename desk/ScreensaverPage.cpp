#include "ScreensaverPage.h"
#include "desk.h"
#include <wil/registry.h>
#include "helper.h"
namespace fs = std::filesystem;
LRESULT CALLBACK StaticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CScrSaverDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hScrPreview = GetDlgItem(1302);
	hEnergy = GetDlgItem(1305);
	hScrCombo = GetDlgItem(1300);
	hBtnSettings = GetDlgItem(1303);
	hBtnPreview = GetDlgItem(1304);
	updown = GetDlgItem(1307);
	secureCheck = GetDlgItem(1320);

	scrSize = GetClientSIZE(hScrPreview);
	energySize = GetClientSIZE(hEnergy);

	HBITMAP bmp = MonitorAsBmp(GETSIZE(scrSize), IDB_BITMAP1, RGB(255, 0, 255));
	Static_SetBitmap(hScrPreview, bmp);
	DeleteObject(bmp);

	bmp = MonitorAsBmp(GETSIZE(energySize), IDB_BITMAP2, RGB(255, 204, 204));
	Static_SetBitmap(hEnergy, bmp);
	DeleteObject(bmp);

	ComboBox_AddString(hScrCombo, L"(none)");
	AddScreenSavers(hScrCombo);

	auto result = wil::reg::try_get_value_string(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"SCRNSAVE.EXE");
	if (result.has_value())
	{
		selectedScrSaver = _wcsdup(result.value().c_str());

		HMODULE hScr = LoadLibrary(selectedScrSaver);
		if (hScr)
		{
			WCHAR name[MAX_PATH];
			LoadString(hScr, 1, name, MAX_PATH);
			ComboBox_SetCurSel(hScrCombo, ComboBox_FindString(hScrCombo, 0, name));
			FreeLibrary(hScr);
		}
	}
	else
	{
		selectedScrSaver = nullptr;
		ComboBox_SetCurSel(hScrCombo, 0);
		::EnableWindow(hBtnPreview, FALSE);
		::EnableWindow(hBtnSettings, FALSE);
	}
	ScreenPreview(hScrPreview);
	int timeout;
	SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &timeout, 0);
	SendMessage(updown, UDM_SETPOS32, 0, timeout / 60);

	BOOL isSecure;
	SystemParametersInfo(SPI_GETSCREENSAVESECURE, 0, &isSecure, 0);
	Button_SetCheck(secureCheck, isSecure);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnScreenSaverComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_TerminateProcess(pi);
	int index = ComboBox_GetCurSel(hScrCombo);
	int leng = ComboBox_GetLBTextLen(hScrCombo, index);
	WCHAR* name = (WCHAR*)malloc((leng + 1) * sizeof(WCHAR));
	ComboBox_GetLBText(hScrCombo, index, name);

	if (index == 0)
	{
		selectedScrSaver = nullptr;
		::EnableWindow(hBtnPreview, FALSE);
		::EnableWindow(hBtnSettings, FALSE);
	}
	else
	{
		selectedScrSaver = scrSaverMap[name].c_str();
		::EnableWindow(hBtnPreview, TRUE);
		::EnableWindow(hBtnSettings, TRUE);
	}

	ScreenPreview(hScrPreview);
	free(name);
	PropSheet_Changed(::GetParent(m_hWnd), hWnd);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnScreenSaverSettings(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_TerminateProcess(pi);
	::DestroyWindow(hWndStatic);
	hWndStatic = nullptr;
	HBITMAP bmp = MonitorAsBmp(GETSIZE(scrSize), IDB_BITMAP1, RGB(255, 0, 255));
	Static_SetBitmap(hScrPreview, bmp);
	DeleteObject(bmp);

	ScreenSettings(hScrPreview);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnScreenSaverPreview(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_TerminateProcess(pi);
	::DestroyWindow(hWndStatic);
	hWndStatic = nullptr;
	HBITMAP bmp = MonitorAsBmp(GETSIZE(scrSize), IDB_BITMAP1, RGB(255, 0, 255));
	Static_SetBitmap(hScrPreview, bmp);
	DeleteObject(bmp);

	ShellExecute(0, L"open", selectedScrSaver, NULL, NULL, SW_SHOW);
	return TRUE;
}

// to fix
BOOL CScrSaverDlgProc::OnPowerBtn(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	WCHAR xpcpl[MAX_PATH];
	ExpandEnvironmentStrings(L"shell32.dll,Control_RunDLL %windir%\\System32\\xppowerc.cpl", xpcpl, MAX_PATH);
	if (PathFileExists(xpcpl) == FALSE)
	{
		ExpandEnvironmentStrings(L"shell32.dll,Control_RunDLL %windir%\\System32\\powercfg.cpl", xpcpl, MAX_PATH);
	}
	ShellExecute(0, L"open", L"Rundll32.exe", xpcpl, 0, SW_SHOW);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnSecureCheck(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	PropSheet_Changed(::GetParent(m_hWnd), hWnd);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnTimeChange(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled)
{
	PropSheet_Changed(::GetParent(m_hWnd), updown);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnApply()
{
	int index = ComboBox_GetCurSel(hScrCombo);
	if (index == 0)
	{
		RegDeleteKeyValue(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"SCRNSAVE.EXE");
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}
	else
	{
		wil::reg::set_value_string(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"SCRNSAVE.EXE", selectedScrSaver);
		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETSCREENSAVESECURE, Button_GetCheck(secureCheck), NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		int timeout = SendMessage(updown, UDM_GETPOS32, 0, 0);
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, timeout * 60, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}

	PropSheet_UnChanged(::GetParent(m_hWnd), m_hWnd);
	SetWindowLongPtr(DWLP_MSGRESULT, PSNRET_NOERROR);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnSetActive()
{
	ScreenPreview(hScrPreview);
	return TRUE;
}

BOOL CScrSaverDlgProc::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (LOWORD(wParam) == WA_ACTIVE)
	{
		// show with new settings
		ScreenPreview(hScrPreview);

		CloseHandle(pi2.hThread);
		CloseHandle(pi2.hProcess);
		ZeroMemory(&pi2, sizeof(pi2));
	}
	return TRUE;
}


HBITMAP CScrSaverDlgProc::MonitorAsBmp(int width, int height, WORD id, COLORREF maskColor)
{
	Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (!resized)
	{
		return NULL;
	}

	Gdiplus::Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(id));

	int monitorwidth = GetSystemMetrics(SM_CXSCREEN);
	int monitorheight = GetSystemMetrics(SM_CYSCREEN);

	// pink
	Gdiplus::Color transparentColor(255, GetRValue(maskColor), GetGValue(maskColor), GetBValue(maskColor));

	Gdiplus::ImageAttributes imgAttr;
	imgAttr.SetColorKey(transparentColor, transparentColor, Gdiplus::ColorAdjustTypeBitmap);

	Gdiplus::Graphics graphics(resized);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	Gdiplus::Rect rect(0, 10, monitor->GetWidth(), monitor->GetHeight());
	// draw monitor
	graphics.DrawImage(monitor, rect, 0, 0, width, height, Gdiplus::UnitPixel, &imgAttr);

	if (id == IDB_BITMAP1)
	{
		int cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);

		Gdiplus::Rect prevrect(15, 25, width - 37, height - 68);

		HDC hScreenDC = ::GetDC(NULL);
		HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
		auto g_hbDesktop = CreateCompatibleBitmap(hScreenDC, cx, cy);
		SelectObject(hMemoryDC, g_hbDesktop);
		BitBlt(hMemoryDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);
		Gdiplus::Bitmap* bm = new Gdiplus::Bitmap(g_hbDesktop, NULL);
		graphics.DrawImage(bm, prevrect);

		delete bm;
		DeleteObject(g_hbDesktop);
	}

	// create hbitmap
	HBITMAP hBitmap = NULL;
	resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

	delete resized;
	delete monitor;
	return hBitmap;
}

VOID CScrSaverDlgProc::AddScreenSavers(HWND comboBox)
{
	WCHAR systemdir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\system32", systemdir, MAX_PATH);
	for (const auto& entry : fs::directory_iterator(systemdir))
	{
		if (entry.path().extension() == L".scr")
		{
			LPWSTR path = _wcsdup(entry.path().c_str());
			HMODULE hScr = LoadLibrary(path);
			WCHAR name[MAX_PATH];
			LoadString(hScr, 1, name, MAX_PATH);
			ComboBox_AddString(comboBox, name);
			scrSaverMap.insert({ std::wstring(name), entry.path() });
			FreeLibrary(hScr);
			free(path);
		}
	}
}

VOID CScrSaverDlgProc::ScreenPreview(HWND preview)
{
	_TerminateProcess(pi);
	if (!hWndStatic)
	{
		hWndStatic = CreateWindow(WC_STATIC, 0,
			WS_CHILD | WS_VISIBLE | SS_BLACKRECT, 15, 25, scrSize.cx - 37, scrSize.cy - 68,
			preview, NULL, g_hinst, NULL);
		::SetWindowLongPtr(hWndStatic, GWLP_WNDPROC, (LONG_PTR)StaticProc);
	}

	if (selectedScrSaver)
	{
		ZeroMemory(&pi, sizeof(pi));

		TCHAR cmdLine[256];
		wsprintf(cmdLine, L"%s /p %u", selectedScrSaver, (UINT_PTR)hWndStatic);

		STARTUPINFO si = { sizeof(si) };
		CreateProcess(0, cmdLine, 0, 0, FALSE, 0, 0, 0, &si, &pi);
	}
	else
	{
		if (hWndStatic)
		{
			::DestroyWindow(hWndStatic);
			hWndStatic = nullptr;
			HBITMAP bmp = MonitorAsBmp(GETSIZE(scrSize), IDB_BITMAP1, RGB(255, 0, 255));
			Static_SetBitmap(hScrPreview, bmp);
			DeleteObject(bmp);
		}
	}
}

VOID CScrSaverDlgProc::ScreenSettings(HWND preview)
{
	if (selectedScrSaver)
	{
		ZeroMemory(&pi2, sizeof(pi2));

		TCHAR cmdLine[256];
		wsprintf(cmdLine, L"%s /c /p %u", selectedScrSaver, (UINT_PTR)::GetParent(preview));

		STARTUPINFO si = { sizeof(si) };
		CreateProcess(0, cmdLine, 0, 0, FALSE, 0, 0, 0, &si, &pi2);

	}
}


LRESULT CALLBACK StaticProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_PAINT)
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &ps.rcPaint, hBrush);
		DeleteObject(hBrush);
		EndPaint(hWnd, &ps);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
