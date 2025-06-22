#include "pch.h"
#include "ScreensaverPage.h"
#include "desk.h"
#include "helper.h"

using namespace Microsoft::WRL::Details;
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


	pWndPreview = Make<CWindowPreview>(scrSize, nullptr, 0, PAGETYPE::PT_SCRSAVER, nullptr);
	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	Static_SetBitmap(hScrPreview, bmp);
	DeleteObject(bmp);

	bmp = MonitorAsBmp(GETSIZE(energySize), IDB_BITMAP2, RGB(255, 204, 204));
	Static_SetBitmap(hEnergy, bmp);
	DeleteObject(bmp);

	ComboBox_AddString(hScrCombo, L"(none)");
	AddScreenSavers(hScrCombo);

	WCHAR path[MAX_PATH];
	DWORD size = sizeof(path);
	LRESULT status = RegGetValue(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"SCRNSAVE.EXE", RRF_RT_REG_SZ, NULL, &path, &size);
	if (status == ERROR_SUCCESS)
	{
		selectedScrSaver = _wcsdup(path);

		HMODULE hScr = LoadLibraryEx(selectedScrSaver, NULL, LOAD_LIBRARY_AS_DATAFILE);
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
	return 0;
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
		selectedScrSaver = (LPCWSTR)ComboBox_GetItemData(hScrCombo, index);
		::EnableWindow(hBtnPreview, TRUE);
		::EnableWindow(hBtnSettings, TRUE);
	}

	ScreenPreview(hScrPreview);
	free(name);

	SetModified(TRUE);
	return 0;
}

BOOL CScrSaverDlgProc::OnScreenSaverSettings(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_TerminateProcess(pi);
	::DestroyWindow(hWndStatic);
	hWndStatic = nullptr;

	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	Static_SetBitmap(hScrPreview, bmp);
	DeleteObject(bmp);

	ScreenSettings(hScrPreview);
	return 0;
}

BOOL CScrSaverDlgProc::OnScreenSaverPreview(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	_TerminateProcess(pi);
	::DestroyWindow(hWndStatic);
	hWndStatic = nullptr;

	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	Static_SetBitmap(hScrPreview, bmp);
	DeleteObject(bmp);

	ShellExecute(0, L"open", selectedScrSaver, NULL, NULL, SW_SHOW);
	return 0;
}

BOOL CScrSaverDlgProc::OnPowerBtn(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	WCHAR xpcpl[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\System32\\xppowerc.cpl", xpcpl, MAX_PATH);
	if (PathFileExists(xpcpl) == FALSE)
	{
		ExpandEnvironmentStrings(L"%windir%\\System32\\powercfg.cpl", xpcpl, MAX_PATH);
	}
	WCHAR execCmd[MAX_PATH] = L"shell32.dll,Control_RunDLL ";
	StringCchCat(execCmd, ARRAYSIZE(execCmd), xpcpl);

	ShellExecute(0, L"open", L"Rundll32.exe", execCmd, 0, SW_SHOW);
	return 0;
}

BOOL CScrSaverDlgProc::OnSecureCheck(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	SetModified(TRUE);
	return 0;
}

BOOL CScrSaverDlgProc::OnTimeChange(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled)
{
	SetModified(TRUE);
	return 0;
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
		HKEY key;
		RegOpenKey(HKEY_CURRENT_USER, L"Control Panel\\Desktop", &key);
		if (key)
		{
			RegSetValueEx(key, L"SCRNSAVE.EXE", 0, REG_SZ, (const BYTE*)selectedScrSaver, ((DWORD)wcslen(selectedScrSaver) + 1) * sizeof(wchar_t));
			RegCloseKey(key);
		}

		SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		SystemParametersInfo(SPI_SETSCREENSAVESECURE, Button_GetCheck(secureCheck), NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
		int timeout = (int)SendMessage(updown, UDM_GETPOS32, 0, 0);
		SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, timeout * 60, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
	}

	SetModified(FALSE);
	return 0;
}

BOOL CScrSaverDlgProc::OnSetActive()
{
	ScreenPreview(hScrPreview);
	return 0;
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
	return 0;
}


HBITMAP CScrSaverDlgProc::MonitorAsBmp(int width, int height, WORD id, COLORREF maskColor)
{
	Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (!resized)
	{
		return NULL;
	}

	Gdiplus::Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(id));

	// pink
	Gdiplus::Color transparentColor(255, GetRValue(maskColor), GetGValue(maskColor), GetBValue(maskColor));

	Gdiplus::ImageAttributes imgAttr;
	imgAttr.SetColorKey(transparentColor, transparentColor, Gdiplus::ColorAdjustTypeBitmap);

	Gdiplus::Graphics graphics(resized);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	Gdiplus::Rect rect(0, 10, monitor->GetWidth(), monitor->GetHeight());

	// draw bmp
	graphics.DrawImage(monitor, rect, 0, 0, width, height, Gdiplus::UnitPixel, &imgAttr);

	// create hbitmap
	HBITMAP hBitmap = NULL;
	resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

	delete resized;
	delete monitor;
	return hBitmap;
}

VOID CScrSaverDlgProc::AddScreenSavers(HWND comboBox)
{
	std::vector<LPWSTR> scrsavers;

	WCHAR systemdir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\system32", systemdir, MAX_PATH);
	LPCWSTR extensions[] = { L".scr" };
	EnumDir(systemdir, extensions, ARRAYSIZE(extensions), scrsavers, FALSE);

	for (LPWSTR path : scrsavers)
	{
		HMODULE hScr = LoadLibraryEx(path, NULL, LOAD_LIBRARY_AS_DATAFILE);
		WCHAR name[MAX_PATH];
		if (!hScr) continue;

		LoadString(hScr, 1, name, MAX_PATH);
		FreeLibrary(hScr);
		int index = ComboBox_AddString(comboBox, name);
		ComboBox_SetItemData(comboBox, index, path);
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
			HBITMAP bmp;
			pWndPreview->GetPreviewImage(&bmp);
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
