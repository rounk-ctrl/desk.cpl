#include "pch.h"
#include "AppearanceDlgBox.h"
#include "AppearancePage.h"
#include "desk.h"
#include "helper.h"
#include "uxtheme.h"

using namespace Gdiplus;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;

BOOL CAppearanceDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hPreviewWnd = GetDlgItem(1110);
	hThemesCombobox = GetDlgItem(1111);
	hColorCombobox = GetDlgItem(1114);
	hSizeCombobox = GetDlgItem(1116);
	size = GetClientSIZE(hPreviewWnd);

	WCHAR msstyledir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Resources\\Themes", msstyledir, MAX_PATH);
	LPCWSTR extensions[] = {L".msstyles"};
	EnumDir(msstyledir, extensions, ARRAYSIZE(extensions), msstyle, TRUE);

	for (LPCWSTR style : msstyle)
	{
		HMODULE hStyle = LoadLibraryEx(style, NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hStyle)
		{
			WCHAR name[MAX_PATH];
			LoadString(hStyle, 101, name, MAX_PATH);

			int index = 0;
			if (lstrlenW(name) != 0)
			{
				index = ComboBox_AddString(hThemesCombobox, name);
			}
			else
			{
				index = ComboBox_AddString(hThemesCombobox, PathFindFileName(style));
			}
			ComboBox_SetItemData(hThemesCombobox, index, style);
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

	MYWINDOWINFO wnd[3] =
	{
		{
			WT_INACTIVE,
			{10, 10, 10 + 320, 10 + 104}
		},
		{
			WT_ACTIVE,
			{25, 35, 25 + 320, 35 + 104}
		},
		{
			WT_MESSAGEBOX,
			{(size.cx / 2) - 75,60,(size.cx / 2) + 75,60 + 70}
		}
	};
	pWndPreview = Make<CWindowPreview>(size, wnd, (int)ARRAYSIZE(wnd), PAGETYPE::PT_APPEARANCE, nullptr);
	HBITMAP bmp;
	pWndPreview->GetPreviewImage(&bmp);
	Static_SetBitmap(hPreviewWnd,bmp);
	DeleteObject(bmp);

	return 0;
}

BOOL CAppearanceDlgProc::OnAdvanced(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CAppearanceDlgBox dlg;
	dlg.DoModal();
	return 0;
}

BOOL CAppearanceDlgProc::OnComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	return 0;
}

BOOL CAppearanceDlgProc::OnSetActive()
{
	ITheme* themeClass = new ITheme(currentITheme);
	LPWSTR style;
	themeClass->get_VisualStyle(&style);
	for (int i = 0; i < ComboBox_GetCount(hThemesCombobox); ++i)
	{
		LPWSTR data = (LPWSTR)ComboBox_GetItemData(hThemesCombobox, i);
		if (StrCmpI(data, style) == 0)
		{
			ComboBox_SetCurSel(hThemesCombobox, i);
			break;
		}
	}

	// my bad
	MYWINDOWINFO wnd[3] =
	{
		{
			WT_INACTIVE,
			{10, 10, 10 + 320, 10 + 104}
		},
		{
			WT_ACTIVE,
			{25, 35, 25 + 320, 35 + 104}
		},
		{
			WT_MESSAGEBOX,
			{(size.cx / 2) - 75,60,(size.cx / 2) + 75,60 + 70}
		}
	};
	HBITMAP ebmp;
	pWndPreview->GetUpdatedPreviewImage(wnd, LoadThemeFromFilePath(style), &ebmp);
	Static_SetBitmap(hPreviewWnd, ebmp);
	DeleteBitmap(ebmp);

	_TerminateProcess(pi);
	return 0;
}