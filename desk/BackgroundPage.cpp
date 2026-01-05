/* ---------------------------------------------------------
* Background page
*
* Responsible for the "Desktop" tab
*
------------------------------------------------------------*/

#include "pch.h"
#include "BackgroundPage.h"
#include "desk.h"
#include "helper.h"
#include "wndprvw.h"
#include "DeskIcons.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;

std::vector<LPWSTR> wallpapers;
const COMDLG_FILTERSPEC file_types[] = {
	{L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)",
	L"*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png"},
};


BOOL CBackgroundDlgProc::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	firstInit = TRUE;
	hListView = GetDlgItem(1202);
	hBackPreview = GetDlgItem(1200);
	hPosCombobox = GetDlgItem(1205);
	backPreviewSize = GetClientSIZE(hBackPreview);

	if (!currentITheme)
	{
		int cur;
		pThemeManager->GetCurrentTheme(&cur);
		pThemeManager->GetTheme(cur, &currentITheme);
	}

	RECT rect;
	::GetClientRect(hListView, &rect);
	AddColumn(hListView, rect.right - rect.left - 30);

	WCHAR szText[20];
	LoadString(g_hThemeUI, 2022, szText, ARRAYSIZE(szText));

	AddItem(hListView, 0, szText);
	HICON barrierico = LoadIcon(LoadLibraryEx(L"imageres.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE), MAKEINTRESOURCE(1027));
	ImageList_AddIcon(hml, barrierico);
	ListView_SetImageList(hListView, hml, LVSIL_SMALL);
	DestroyIcon(barrierico);

	WCHAR wallpaperdir[MAX_PATH];
	WCHAR windowswaldir[MAX_PATH];
	ExpandEnvironmentStrings(L"%windir%\\Web\\Wallpaper", wallpaperdir, MAX_PATH);
	ExpandEnvironmentStrings(L"%windir%", windowswaldir, MAX_PATH);

	LPCWSTR extensions[] = { L".jpg",  L".png", L".bmp", L".jpeg", L".dib", L".gif"};

	// this basically combines wallpaperdir and windowswaldir
	EnumDir(wallpaperdir, extensions, ARRAYSIZE(extensions), wallpapers, TRUE);
	EnumDir(windowswaldir, extensions, ARRAYSIZE(extensions), wallpapers, FALSE);

	// this basically alphabetically sorts the wallpaper file names, for example if you have
	// a few files in windir and a few in web it will put windir files at the end and the web files on top
	// this basically fixes that issue and makes it like how it was in windows xp
	std::sort(wallpapers.begin(), wallpapers.end(), [](LPWSTR a, LPWSTR b) {
		LPCWSTR fileA = PathFindFileNameW(a);
		LPCWSTR fileB = PathFindFileNameW(b);
		int cmpResult = StrCmpLogicalW(fileA, fileB);
		return cmpResult < 0;
	});
	
	// start with k=1, k=0 is (none)
	int k = 1;
	for (auto path : wallpapers)
	{
		AddItem(hListView, k++, path);
	}

	HINSTANCE hThemeCpl = LoadLibraryEx(L"themecpl.dll", 0, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32);

	// combobox with positions of wallpaper
	for (int i = 0; i < 6; i++)
	{
		// cool ms
		if (i == 3) i = 4;
		else if (i == 4) i = 3;

		WCHAR string[10];
		LoadString(hThemeCpl, 500 + (8 - i), string, 10);
		ComboBox_AddString(hPosCombobox, string);
		
		if (i == 4) i = 3;
		else if (i == 3) i = 4;
	}
	FreeLibrary(hThemeCpl);

	DESKTOP_WALLPAPER_POSITION pos;
	pDesktopWallpaper->GetPosition(&pos);
	ComboBox_SetCurSel(hPosCombobox, pos);

	if (selectedTheme->newColor == NULL)
	{
		WCHAR ws[MAX_PATH] = { 0 };
		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);

		selectedTheme->wallpaperType = WT_PICTURE;
		selectedTheme->wallpaperPath = ws;

		selectedTheme->useDesktopColor = true;
		selectedTheme->newColor = 0xB0000000;
	}

	AddMissingWallpapers(currentITheme);
	SelectCurrentWallpaper(currentITheme);

	_UpdateButtonBmp();

	firstInit = FALSE;
	return 0;
}

BOOL CBackgroundDlgProc::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	pWndPreview = nullptr;
	return 0;
}

BOOL CBackgroundDlgProc::OnBgSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	selectedTheme->posChanged = ComboBox_GetCurSel(hPosCombobox);

	HBITMAP bmp;
	pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &bmp, UPDATE_WALLPAPER);
	SetBitmap(hBackPreview, bmp);

	SetModified(TRUE);
	return 0;
}

BOOL CBackgroundDlgProc::OnBrowse(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	std::wstring path;

	IFileDialog* pfd;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		// get options
		DWORD dwFlags;
		hr = pfd->GetOptions(&dwFlags);

		// set the file types
		hr = pfd->SetFileTypes(ARRAYSIZE(file_types), file_types);

		// the first element from the array
		hr = pfd->SetFileTypeIndex(1);

		pfd->SetTitle(L"Browse");

		// Show the dialog
		hr = pfd->Show(hWnd);

		if (SUCCEEDED(hr))
		{
			IShellItem* psiResult;
			hr = pfd->GetResult(&psiResult);
			if (SUCCEEDED(hr)) {
				PWSTR pszFilePath = NULL;
				hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr)) {
					path = pszFilePath;
					CoTaskMemFree(pszFilePath);
				}
			}
			pfd->Release();
		}
	}
	else
	{
		wchar_t szFile[MAX_PATH];
		OPENFILENAME ofn = { sizeof(ofn) };
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)\0*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE)
		{
			path = ofn.lpstrFile;
		}
	}
	if (!path.empty())
	{
		int inde = AddItem(hListView, ListView_GetItemCount(hListView), path.c_str());
		ListView_SetItemState(hListView, inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(hListView, inde, FALSE);
	}
	return 0;
}

BOOL CBackgroundDlgProc::OnColorPick(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	CHOOSECOLOR cc = {0};
	if (ColorPicker(GetDeskopColor(), hWnd, &cc) == TRUE)
	{
		selectedTheme->newColor = cc.rgbResult;

		_UpdateButtonBmp();

		HBITMAP hBmp;
		pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &hBmp, UPDATE_SOLIDCLR);
		SetBitmap(hBackPreview, hBmp);

		SetModified(TRUE);
	}
	return 0;
}

BOOL CBackgroundDlgProc::OnDeskCustomize(UINT code, UINT id, HWND hWnd, BOOL& bHandled)
{
	WTL::CPropertySheet sheet(L"Display Properties");
	sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;

	// use the dialog template from shell32
	HINSTANCE hShell32 = LoadLibraryEx(L"shell32.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hShell32)
	{
		HINSTANCE hOldRes = _AtlBaseModule.GetResourceInstance();
		_AtlBaseModule.SetResourceInstance(hShell32);

		CDesktopIconsDlg dlg;
		sheet.AddPage(dlg);
		_AtlBaseModule.SetResourceInstance(hOldRes);

		FreeLibrary(hShell32);
	}
	sheet.DoModal();

	return 0;
}

BOOL CBackgroundDlgProc::OnWallpaperSelection(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)nmhdr;
	if (pnmv->uNewState & LVIS_SELECTED)
	{
		selectedIndex = pnmv->iItem;
		LPWSTR path = GetWallpaperPath(hListView, pnmv->iItem);

		if (selectedIndex == 0)
		{
			::EnableWindow(hPosCombobox, false);

			selectedTheme->wallpaperPath = L"";
			selectedTheme->wallpaperType = WT_NOWALL;

		}
		else
		{
			::EnableWindow(hPosCombobox, true);

			// set new path
			selectedTheme->wallpaperPath = path;
			selectedTheme->wallpaperType = WT_PICTURE;
		}


		selectedTheme->customWallpaperSelection = !selectionPicker;
		
		HBITMAP bmp;
		if (!pWndPreview)
		{
			pWndPreview = Make<CWindowPreview>(backPreviewSize, nullptr, 0, PAGETYPE::PT_BACKGROUND, nullptr, GetDpiForWindow(m_hWnd));
			pWndPreview->GetPreviewImage(&bmp);
		}
		else
		{
			pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &bmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
		}
		SetBitmap(hBackPreview, bmp);

		SetModified(TRUE);
	}
	return 0;
}

BOOL CBackgroundDlgProc::OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	printf("WM_SETTINGCHANGED:\n");
	AddMissingWallpapers(currentITheme);
	SelectCurrentWallpaper(currentITheme);
	return 0;
}

BOOL CBackgroundDlgProc::OnApply()
{
	if (selectedTheme->posChanged != -1)
	{
		int index = ComboBox_GetCurSel(hPosCombobox);
		pDesktopWallpaper->SetPosition((DESKTOP_WALLPAPER_POSITION)index);
		selectedTheme->posChanged = -1;
	}
	if (selectedTheme->newColor != 0xB0000000)
	{
		pDesktopWallpaper->SetBackgroundColor(selectedTheme->newColor);
		selectedTheme->newColor = 0xB0000000;
	}
	if (selectedTheme->customWallpaperSelection)
	{
		pDesktopWallpaper->Enable(selectedTheme->wallpaperType == WT_PICTURE);
		if (selectedTheme->wallpaperType == WT_PICTURE)
		{
			pDesktopWallpaper->SetWallpaper(NULL, selectedTheme->wallpaperPath.c_str());
		}
		selectedTheme->customWallpaperSelection = false;
	}

	selectedTheme->updateWallThemesPg = true;
	SetModified(FALSE);

	HBITMAP bmp;
	selectedTheme->useDesktopColor = true;
	pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &bmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
	SetBitmap(hBackPreview, bmp);
	_UpdateButtonBmp();

	return 0;
}

BOOL CBackgroundDlgProc::OnSetActive()
{
	selectionPicker = false;
	_UpdateButtonBmp();
	if (!selectedTheme->customWallpaperSelection && !firstInit)
	{
		AddMissingWallpapers(currentITheme);
		SelectCurrentWallpaper(currentITheme);

		if (selectedTheme->posChanged == -1)
		{
			// update wallpaper position 
			DESKTOP_WALLPAPER_POSITION pos;
			pDesktopWallpaper->GetPosition(&pos);
			ComboBox_SetCurSel(hPosCombobox, pos);
		}
	}
	// special case where preview wont update if (none) 
	// and background color changes due to theme change
	// just update each time u activate
	if (!firstInit && selectedIndex == 0)
	{
		HBITMAP bmp; 
		if (pWndPreview)
		{
			pWndPreview->GetUpdatedPreviewImage(nullptr, nullptr, &bmp, UPDATE_WALLPAPER | UPDATE_SOLIDCLR);
			SetBitmap(hBackPreview, bmp);
		}
	}
	_TerminateProcess(pi);
	return 0;
}


#pragma region ListView helpers
int CBackgroundDlgProc::AddItem(HWND hListView, int rowIndex, LPCWSTR text)
{
	if (text)
	{
		// gimmick
		SHFILEINFO sh{};
		SHGetFileInfo(text, FILE_ATTRIBUTE_NORMAL, &sh, sizeof(SHFILEINFO), SHGFI_ICON);
		ImageList_AddIcon(hml, sh.hIcon);
	}

	// why is winapi so ass
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvItem.iItem = rowIndex;
	lvItem.iSubItem = 0;
	lvItem.iImage = rowIndex;
	lvItem.pszText = (LPWSTR)PathFindFileName(text);
	lvItem.lParam = (LPARAM)StrDup(text);

	return ListView_InsertItem(hListView, &lvItem);
}

int CBackgroundDlgProc::AddColumn(HWND hListView, int width)
{
	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.cx = width;
	lvc.pszText = (LPWSTR)L"";
	lvc.iSubItem = 0;
	return ListView_InsertColumn(hListView, 0, &lvc);
}

LPWSTR CBackgroundDlgProc::GetWallpaperPath(HWND hListView, int iIndex)
{
	LVITEM item = { 0 };
	item.iItem = iIndex;
	item.iSubItem = 0;
	item.cchTextMax = 256;
	item.mask = LVIF_PARAM;
	ListView_GetItem(hListView, &item);
	return (LPWSTR)item.lParam;
}
#pragma endregion

void CBackgroundDlgProc::AddMissingWallpapers(IUnknown* th)
{
	std::vector<LPWSTR> missingWall;

	BOOL isEn = 0;
	auto themeClass = std::make_unique<CTheme>(th);
	themeClass->IsSlideshowEnabled(&isEn);
	if (isEn)
	{
		ComPtr<ISlideshowSettings> st = NULL;
		themeClass->get_SlideshowSettings(&st);

		ComPtr<IWallpaperCollection> wlp;
		st->GetAllMatchingWallpapers(&wlp);

		int count = wlp->GetCount();
		for (int i = 0; i < count; i++)
		{
			LPWSTR path = { 0 };
			wlp->GetWallpaperAt(i, &path);
			missingWall.push_back(path);
		}
	}
	for (auto path : missingWall)
	{
		LVFINDINFO findInfo = { 0 };
		findInfo.flags = LVFI_STRING;
		findInfo.psz = PathFindFileName(path);
		int inde = ListView_FindItem(hListView, -1, &findInfo);
		if (inde == -1)
		{
			AddItem(hListView, ListView_GetItemCount(hListView), path);
		}
	}

	LVFINDINFO findInfo = { 0 };
	findInfo.flags = LVFI_STRING;
	findInfo.psz = PathFindFileName(selectedTheme->wallpaperPath.c_str());
	int inde = ListView_FindItem(hListView, -1, &findInfo);
	if (inde == -1 && !selectedTheme->wallpaperPath.empty())
	{
		if (PathFileExists(selectedTheme->wallpaperPath.c_str()))
		{
			AddItem(hListView, ListView_GetItemCount(hListView), selectedTheme->wallpaperPath.c_str());
		}
	}
}

void CBackgroundDlgProc::SelectCurrentWallpaper(IUnknown* th)
{
	::EnableWindow(hPosCombobox, selectedTheme->wallpaperType != WT_NOWALL);
	int index = 0;

	if (selectedTheme->wallpaperType == WT_PICTURE)
	{
		LVFINDINFO findInfo = { 0 };
		findInfo.flags = LVFI_STRING;
		findInfo.psz = PathFindFileName(selectedTheme->wallpaperPath.c_str());
		index = ListView_FindItem(hListView, -1, &findInfo);
	}

	ListView_SetItemState(hListView, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	ListView_EnsureVisible(hListView, index, FALSE);
}

void CBackgroundDlgProc::_UpdateButtonBmp()
{
	HBITMAP hBmp;
	GetSolidBtnBmp(GetDeskopColor(), GetDpiForWindow(m_hWnd), GetClientSIZE(GetDlgItem(1207)), &hBmp);
	HBITMAP hOld = Button_SetBitmap(GetDlgItem(1207), hBmp);
	DeleteBitmap(hOld);
	DeleteBitmap(hBmp);
}
