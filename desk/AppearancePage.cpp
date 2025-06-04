#include "pch.h"
#include "AppearanceDlgBox.h"
#include "AppearancePage.h"
#include "desk.h"
#include "helper.h"
#include "uxtheme.h"
namespace fs = std::filesystem;
using namespace Gdiplus;

BOOL CAppearanceDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hPreviewWnd = GetDlgItem(1110);
	hThemesCombobox = GetDlgItem(1111);
	hColorCombobox = GetDlgItem(1114);
	hSizeCombobox = GetDlgItem(1116);
	size = GetClientSIZE(hPreviewWnd);

	WCHAR msstyledir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Resources\\Themes", msstyledir, MAX_PATH);
	for (const auto& entry : fs::recursive_directory_iterator(msstyledir, fs::directory_options::skip_permission_denied))
	{
		if (entry.is_regular_file() && (entry.path().extension() == L".msstyles"))
		{
			LPWSTR lpwstrPath = _wcsdup(entry.path().c_str());
			msstyle.push_back(lpwstrPath);
		}
	}

	for (LPCWSTR style : msstyle)
	{
		HMODULE hStyle = LoadLibrary(style);
		if (hStyle)
		{
			WCHAR name[MAX_PATH];
			LoadString(hStyle, 101, name, MAX_PATH);
			if (lstrlenW(name) != 0)
			{
				int index = ComboBox_AddString(hThemesCombobox, name);
				ComboBox_SetItemData(hThemesCombobox, index, style);
			}
			else
			{
				ComboBox_AddString(hThemesCombobox, PathFindFileName(style));
			}
			FreeLibrary(hStyle);
		}
	}

	int selindex = 0;

	ITheme* themeClass = new ITheme(currentITheme);
	LPWSTR style;
	themeClass->get_VisualStyle(&style);
	for (int i = 0; i < ComboBox_GetCount(hThemesCombobox); ++i)
	{
		LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
		if (StrCmpI(data, style) == 0)
		{
			selindex = i;
			ComboBox_SetCurSel(hThemesCombobox, i);
			break;
		}
	}

	_THEMENAMEINFO name;
	HRESULT hr = S_OK;
	for (int i = 0; SUCCEEDED(hr); i++)
	{
		hr = EnumThemeColors(msstyle[selindex], NULL, i, &name);
		if (SUCCEEDED(hr))
		{
			if (lstrlenW(name.szDisplayName) == 0)
			{
				ComboBox_AddString(hColorCombobox, name.szName);
			}
			else
			{
				ComboBox_AddString(hColorCombobox, name.szDisplayName);
			}
		}
	}

	hr = S_OK;
	for (int i = 0; SUCCEEDED(hr); i++)
	{
		hr = EnumThemeSize(msstyle[selindex], NULL, i, &name);
		if (SUCCEEDED(hr))
		{
			if (lstrlenW(name.szDisplayName) == 0)
			{
				ComboBox_AddString(hSizeCombobox, name.szName);
			}
			else
			{
				ComboBox_AddString(hSizeCombobox, name.szDisplayName);
			}
		}
	}

	
	ComboBox_SetCurSel(hColorCombobox, 0);
	ComboBox_SetCurSel(hSizeCombobox, 0);

	HBITMAP bmp = WindowPreviewBmp(GETSIZE(size));
	Static_SetBitmap(hPreviewWnd, bmp);

	return 0;
}

BOOL CAppearanceDlgProc::OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CAppearanceDlgBox dlg;
	dlg.DoModal();
	return 0;
}

HBITMAP CAppearanceDlgProc::WindowPreviewBmp(int newwidth, int newheight)
{
	Bitmap* bmp = new Bitmap(newwidth, newheight, PixelFormat32bppARGB);
	if (!bmp) return NULL;

	Graphics graphics(bmp);
	graphics.SetInterpolationMode(InterpolationModeInvalid);

	COLORREF clr = selectedTheme->newColor;
	SolidBrush backgroundBrush(Color(GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	graphics.FillRectangle(&backgroundBrush, Rect(0, 0, newwidth, newheight));

	HBITMAP hBitmap = NULL;
	bmp->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);
	


	delete bmp;
	return hBitmap;
}
