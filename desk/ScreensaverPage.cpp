#include "ScreensaverPage.h"
#include "desk.h"
#include <wil/registry.h>
namespace fs = std::filesystem;

HWND hScrPreview;
HWND hEnergy;
HWND hScrCombo;
int scrWidth{};
int scrHeight{};
int energyWidth{};
int energyHeight{};
LPWSTR selectedScrSaver{};
typedef LRESULT(CALLBACK* ScreenSaverProc_t)(HWND, UINT, WPARAM, LPARAM);

HBITMAP MonitorAsBmp(int width, int height, WORD id, COLORREF maskColor)
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
	
	// create hbitmap
	HBITMAP hBitmap = NULL;
	resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

	delete resized;
	delete monitor;
	return hBitmap;
}

VOID AddScreenSavers(HWND comboBox)
{
	WCHAR systemdir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\system32", systemdir, MAX_PATH);
	for (const auto& entry : fs::directory_iterator(systemdir))
	{
		if (entry.path().extension() == L".scr")
		{
			HMODULE hScr = LoadLibrary(_wcsdup(entry.path().c_str()));
			WCHAR name[MAX_PATH];
			LoadString(hScr, 1, name, MAX_PATH);
			ComboBox_AddString(comboBox, name);
			FreeLibrary(hScr);
		}
	}
}

HWND ScreenPreview(HWND preview)
{
	HWND hWnd = CreateWindow(WC_STATIC, 0,
		WS_CHILD | WS_VISIBLE, 15, 25, scrWidth-37, scrHeight-68,
		preview, NULL, g_hinst, NULL);

	if (selectedScrSaver)
	{
		TCHAR cmdLine[256];
		wsprintf(cmdLine, L"%s /p %u", selectedScrSaver, (UINT_PTR)hWnd);

		STARTUPINFO si = { sizeof(si) };
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
		PROCESS_INFORMATION pi = { 0 };
		CreateProcess(0, cmdLine, 0, 0, FALSE, 0, 0, 0, &si, &pi);
	}
	return hWnd;
}

LRESULT CALLBACK ScrSaverDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_INITDIALOG)
	{
		hScrPreview = GetDlgItem(hWnd, 1302);
		hEnergy = GetDlgItem(hWnd, 1305);
		hScrCombo = GetDlgItem(hWnd, 1300);
		RECT rect;
		GetClientRect(hScrPreview, &rect);
		scrWidth = rect.right - rect.left;
		scrHeight = rect.bottom - rect.top;
		GetClientRect(hEnergy, &rect);
		energyWidth= rect.right - rect.left;
		energyHeight = rect.bottom - rect.top;

		HBITMAP bmp = MonitorAsBmp(scrWidth, scrHeight, IDB_BITMAP1, RGB(255, 0, 255));
		Static_SetBitmap(hScrPreview, bmp);
		DeleteObject(bmp);

		bmp = MonitorAsBmp(energyWidth, energyHeight, IDB_BITMAP2,RGB(255,204,204));
		Static_SetBitmap(hEnergy, bmp);
		DeleteObject(bmp);
		
		ComboBox_AddString(hScrCombo, L"(none)");
		AddScreenSavers(hScrCombo);

		auto result = wil::reg::try_get_value_string(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"SCRNSAVE.EXE");
		if (result.has_value())
		{
			selectedScrSaver = _wcsdup(result.value().c_str());

			HMODULE hScr = LoadLibrary(selectedScrSaver);
			WCHAR name[MAX_PATH];
			LoadString(hScr, 1, name, MAX_PATH);
			ComboBox_SetCurSel(hScrCombo, ComboBox_FindString(hScrCombo, 0, name));
			FreeLibrary(hScr);
		}
		else
		{
			selectedScrSaver = nullptr;
			ComboBox_SetCurSel(hScrCombo, 0);
		}
		ScreenPreview(hScrPreview);
	}
	return FALSE;
}
