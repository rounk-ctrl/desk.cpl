#include "desk.h"
#include "pch.h"
#include "helper.h"
#include "ThemesPage.h"
#include "theme.h"

namespace fs = std::filesystem;

HINSTANCE g_hinst;
IThemeManager2* pThemeManager = NULL;
IDesktopWallpaper* pDesktopWallpaper = NULL;
ULONG_PTR gdiplusToken;
LPWSTR wallpath{};
int lastpos{};
COLORREF newColor{};
BOOL noWall{};
BOOL firstSelect = TRUE;

const IID IID_IThemeManager2 = { 0xc1e8c83e, 0x845d, 0x4d95, {0x81, 0xdb, 0xe2, 0x83, 0xfd, 0xff, 0xc0, 0x00} };

HBITMAP WallpaperAsBmp(int width, int height, WCHAR* path)
{
	Gdiplus::Bitmap* resized = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	if (!resized)
	{
		return NULL;
	}

	Gdiplus::Bitmap* monitor = Gdiplus::Bitmap::FromResource(g_hinst, MAKEINTRESOURCEW(IDB_BITMAP1));

	Gdiplus::Color transparentColor(255, 255, 0, 255);

	Gdiplus::ImageAttributes imgAttr;
	imgAttr.SetColorKey(transparentColor, transparentColor, Gdiplus::ColorAdjustTypeBitmap);


	Gdiplus::Graphics graphics(resized);
	graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
	Gdiplus::Rect rect(0, 10, monitor->GetWidth(), monitor->GetHeight());
	graphics.DrawImage(monitor, rect, 0, 0, width, height, Gdiplus::UnitPixel, &imgAttr);

	Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path, FALSE);
	if (bitmap)
	{	
		graphics.DrawImage(bitmap, 15, 25, width-37, height-68);
	}
	
	if (!path)
	{
		COLORREF colorref;
		if (newColor)
			colorref = newColor;
		else
			colorref = GetSysColor(COLOR_BACKGROUND);

		Gdiplus::Color clr(255, GetRValue(colorref), GetGValue(colorref), GetBValue(colorref));
		Gdiplus::SolidBrush* br = new Gdiplus::SolidBrush(clr);	
		graphics.FillRectangle(br, 15, 25, width - 37, height - 68);
	}

	// create hbitmap
	HBITMAP hBitmap = NULL;
	resized->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hBitmap);

	delete bitmap;
	delete resized;
	return hBitmap;
}

std::set<LPCSTR, NaturalComparator> wallpapers;
const COMDLG_FILTERSPEC file_types[] = {
	{L"All Picture Files (*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png)", L"*.bmp;*.gif;*.jpg;*.jpeg;*.dib;*.png"},
};

LRESULT CALLBACK BackgroundDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int width{};
	int height{};
	if (uMsg == WM_INITDIALOG)
	{
		ListView_DeleteAllItems(GetDlgItem(hWnd, 1202));
		WCHAR ws[MAX_PATH] = { 0 };
		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
		RECT rect;
		GetClientRect(GetDlgItem(hWnd, 1200), &rect);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;

		IUnknown* th;
		int currThem{};
		pThemeManager->GetCurrentTheme(&currThem);
		pThemeManager->GetTheme(currThem, &th);

		// 1- enabled
		// 0- disabled
		int isEn = 0;
		if (g_osVersion.BuildNumber() >= 18362)
		{
			ITheme1903* th1903 = (ITheme1903*)th;
			th1903->IsSlideshowEnabled(&isEn);
		}
		else if (g_osVersion.BuildNumber() >= 17763)
		{
			ITheme1809* th1809 = (ITheme1809*)th;
			th1809->IsSlideshowEnabled(&isEn);
		}
		else
		{
			ITheme10* th10 = (ITheme10*)th;
			th10->IsSlideshowEnabled(&isEn);
		}


		GetClientRect(GetDlgItem(hWnd, 1202), &rect);
		width = rect.right - rect.left-30;
		LVCOLUMN lvc = { 0 };
		lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvc.cx = width;
		lvc.pszText = (LPWSTR)L"";
		lvc.iSubItem = 0;
		SendMessage(GetDlgItem(hWnd, 1202), LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);


		AddItem(GetDlgItem(hWnd, 1202), 0, "(none)");

		for (const auto& entry : fs::recursive_directory_iterator(L"C:\\Windows\\Web\\Wallpaper"))
		{
			if (entry.is_regular_file() && (entry.path().extension() == L".jpg" || entry.path().extension() == L".png"))
			{
				LPWSTR lpwstrPath = _wcsdup(entry.path().c_str());
				LPCSTR path = ConvertStr(lpwstrPath);
				wallpapers.insert(path);
			}
		}

		if (isEn == 1)
		{
			ISlideshowSettings* st;
			if (g_osVersion.BuildNumber() >= 18362)
			{
				ITheme1903* th1903 = (ITheme1903*)th;
				th1903->get_SlideshowSettings(&st);
			}
			else if (g_osVersion.BuildNumber() >= 17763)
			{
				ITheme1809* th1809 = (ITheme1809*)th;
				th1809->get_SlideshowSettings(&st);
			}
			else
			{
				ITheme10* th10 = (ITheme10*)th;
				th10->get_SlideshowSettings(&st);
			}
			IWallpaperCollection* wlp;
			st->GetAllMatchingWallpapers(&wlp);
			int count = wlp->GetCount();
			for (int i = 0; i < count; i++)
			{
				LPWSTR path = { 0 };
				wlp->GetWallpaperAt(i, &path);
				LPCSTR lpcstrPath = ConvertStr(path);
				wallpapers.insert(lpcstrPath);
			}
		}
		int k = 1;
		for (auto path : wallpapers)
		{
			AddItem(GetDlgItem(hWnd, 1202), k, path);
			k++;
		}
		const wchar_t* items[] = { L"Centre", L"Tile", L"Stretch",  L"Fit",  L"Fill",  L"Span" };
		for (int i = 0; i < _countof(items); i++)
		{
			SendMessage(GetDlgItem(hWnd, 1205), CB_ADDSTRING, 0, (LPARAM)items[i]);
		}
		SendMessage(GetDlgItem(hWnd, 1205), CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		SystemParametersInfo(SPI_GETDESKWALLPAPER, MAX_PATH, ws, 0);
		if (lstrlenW(ws) == 0)
		{
			noWall = TRUE;
			EnableWindow(GetDlgItem(hWnd, 1205), false);
			ListView_SetItemState(GetDlgItem(hWnd, 1202), 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, 1202), 0, FALSE);
		}
		else
		{
			DESKTOP_WALLPAPER_POSITION pos;

			if (g_osVersion.BuildNumber() >= 18362)
			{
				ITheme1903* th1903 = (ITheme1903*)th;
				th1903->get_BackgroundPosition(&pos);
			}
			else if (g_osVersion.BuildNumber() >= 17763)
			{
				ITheme1809* th1809 = (ITheme1809*)th;
				th1809->get_BackgroundPosition(&pos);
			}
			else
			{
				ITheme10* th10 = (ITheme10*)th;
				th10->get_BackgroundPosition(&pos);
			}

			SendMessage(GetDlgItem(hWnd, 1205), CB_SETCURSEL, (WPARAM)pos, (LPARAM)0);
			lastpos = pos;

			LVFINDINFO findInfo = { 0 };
			findInfo.flags = LVFI_STRING;
			findInfo.psz = PathFindFileName(DecodeTranscodedImage().c_str());
			int inde = ListView_FindItem(GetDlgItem(hWnd, 1202), -1, &findInfo);
			if (inde == -1)
			{
				if (fs::exists(fs::path(DecodeTranscodedImage().c_str())))
				{
					inde = AddItem(GetDlgItem(hWnd, 1202), k, ConvertStr(DecodeTranscodedImage().c_str()));
				}
			}

			ListView_SetItemState(GetDlgItem(hWnd, 1202), inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			ListView_EnsureVisible(GetDlgItem(hWnd, 1202), inde, FALSE);
		}

		th->Release();
		//DeleteObject(bmp);
	}
	else if (uMsg == WM_COMMAND)
	{
		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			if (LOWORD(wParam) == 1205)
			{
				int index = (int)SendMessage(GetDlgItem(hWnd, 1205), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				if (index != lastpos)
					PropSheet_Changed(GetParent(hWnd), hWnd);

			}
		}
		else if (HIWORD(wParam) == BN_CLICKED)
		{
			if (LOWORD(wParam) == 1203)
			{
				CComPtr<IFileDialog> pfd;
				HRESULT hr = pfd.CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER);

				if (FAILED(hr))
					return E_FAIL;

				// get options
				DWORD dwFlags;
				hr = pfd->GetOptions(&dwFlags);
				if (FAILED(hr))
					return E_FAIL;

				// set the file types
				hr = pfd->SetFileTypes(ARRAYSIZE(file_types), file_types);
				if (FAILED(hr))
					return E_FAIL;

				// the first element from the array
				hr = pfd->SetFileTypeIndex(1);
				if (FAILED(hr))
					return E_FAIL;

				pfd->SetTitle(L"Browse");
				if (FAILED(hr))
					return E_FAIL;

				// Show the dialog
				hr = pfd->Show(hWnd);
				if (FAILED(hr))
					return E_FAIL;

				CComPtr<IShellItem> psiResult;
				hr = pfd->GetResult(&psiResult);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath = NULL;
					hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr)) {
						int inde = AddItem(GetDlgItem(hWnd, 1202), ListView_GetItemCount(GetDlgItem(hWnd, 1202)), ConvertStr(pszFilePath));

						ListView_SetItemState(GetDlgItem(hWnd, 1202), inde, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						ListView_EnsureVisible(GetDlgItem(hWnd, 1202), inde, FALSE);
						CoTaskMemFree(pszFilePath);
					}
				}
				pfd.Release();
			}
			else if (LOWORD(wParam) == 1207)
			{
				COLORREF clr;
				pDesktopWallpaper->GetBackgroundColor(&clr);
				CHOOSECOLOR cc;
				COLORREF acrCustClr[16];
				ZeroMemory(&cc, sizeof(cc));
				cc.lStructSize = sizeof(cc);
				cc.hwndOwner = hWnd;
				cc.lpCustColors = acrCustClr;
				cc.rgbResult = clr;
				cc.Flags = CC_RGBINIT | CC_FULLOPEN;
				if (ChooseColor(&cc) == TRUE)
				{
					newColor = cc.rgbResult;
					if (noWall)
					{
						RECT rect;
						GetClientRect(GetDlgItem(hWnd, 1200), &rect);
						width = rect.right - rect.left;
						height = rect.bottom - rect.top;
						HBITMAP bmp = WallpaperAsBmp(width, height, wallpath);
						SendMessage(GetDlgItem(hWnd, 1200), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);
						DeleteObject(bmp);
					}

					PropSheet_Changed(GetParent(hWnd), hWnd);
				}

			}
		}
	}
	else if (uMsg == WM_NOTIFY) 
	{
		LPNMHDR pnmhdr = (LPNMHDR)lParam;
		if (pnmhdr->idFrom == 1202 && pnmhdr->code == LVN_ITEMCHANGED)
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
			if (pnmv->uNewState & LVIS_SELECTED) 
			{ 
				WCHAR buffer[256];
				RECT rect;
				GetClientRect(GetDlgItem(hWnd, 1200), &rect);
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				LVITEM item = { 0 };
				item.iItem = pnmv->iItem;
				item.iSubItem = 0;
				item.pszText = buffer;
				item.cchTextMax = 256;
				item.mask = LVIF_TEXT | LVIF_PARAM;
				ListView_GetItem(GetDlgItem(hWnd, 1202), &item);
				wallpath = (LPWSTR)item.lParam;

				if (StrCmpW(wallpath, L"(none)") == 0)
				{
					noWall = TRUE;
					wallpath = nullptr;
					EnableWindow(GetDlgItem(hWnd, 1205), false);
				}
				else
				{
					EnableWindow(GetDlgItem(hWnd, 1205), true);
				}

				HBITMAP bmp = WallpaperAsBmp(width, height, wallpath);
				SendMessage(GetDlgItem(hWnd, 1200), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)bmp);

				if (firstSelect)
				{
					wallpath = nullptr;
					firstSelect = FALSE;
				}

				PropSheet_Changed(GetParent(hWnd), hWnd);
				DeleteObject(bmp);
			}
		}
		if (pnmhdr->code == PSN_APPLY)
		{
			int index = (int)SendMessage(GetDlgItem(hWnd, 1205), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

			if (index != lastpos)
			{
				pDesktopWallpaper->SetPosition((DESKTOP_WALLPAPER_POSITION)index);
				lastpos = index;
			}
			if (newColor)
			{
				pDesktopWallpaper->SetBackgroundColor(newColor);
				newColor = NULL;
			}
			if (wallpath)
			{
				pDesktopWallpaper->Enable(true);
				pDesktopWallpaper->SetWallpaper(NULL, wallpath);
				wallpath = nullptr;
			}
			if (noWall)
			{
				pDesktopWallpaper->Enable(false);
				noWall = FALSE;
			}

			PropSheet_UnChanged(GetParent(hWnd), hWnd);
			SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_NOERROR);
			return TRUE;
		}
		if (pnmhdr->code == PSN_SETACTIVE)
		{

		}
	}
	return FALSE;
}


LRESULT CALLBACK ScrSaverDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

LRESULT CALLBACK AppearanceDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

LRESULT CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}


void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
	switch (uMsg)
	{
		//called before the dialog is created, hwndPropSheet = NULL, lParam points to dialog resource
	case PSCB_PRECREATE:
	{
		LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;

		if (!(lpTemplate->style & WS_SYSMENU))
		{
			lpTemplate->style |= WS_SYSMENU;
		}
	}
	break;

	//called after the dialog is created
	case PSCB_INITIALIZED:
		break;

	}
}
void PropertySheetMoment()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HRESULT hr = CoCreateInstance(CLSID_ThemeManager2, NULL, CLSCTX_INPROC_SERVER, IID_IThemeManager2, (void**)&pThemeManager);
	pThemeManager->Init(ThemeInitNoFlags);

	hr = CoCreateInstance(CLSID_DesktopWallpaper, NULL, CLSCTX_ALL, IID_IDesktopWallpaper, (void**)&pDesktopWallpaper);

	PROPSHEETPAGE psp[5];
	PROPSHEETHEADER psh;

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE;
	psp[0].hInstance = g_hinst;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_THEMEDLG);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = ThemeDlgProc;
	psp[0].pszTitle = TEXT("Themes");
	psp[0].lParam = 0;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE;
	psp[1].hInstance = g_hinst;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_BACKGROUNDDLG);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = BackgroundDlgProc;
	psp[1].pszTitle = TEXT("Background");
	psp[1].lParam = 0;

	psp[2].dwSize = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags = PSP_USETITLE;
	psp[2].hInstance = g_hinst;
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_SCRSVRDLG);
	psp[2].pszIcon = NULL;
	psp[2].pfnDlgProc = ScrSaverDlgProc;
	psp[2].pszTitle = TEXT("Screen Saver");
	psp[2].lParam = 0;

	psp[3].dwSize = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags = PSP_USETITLE;
	psp[3].hInstance = g_hinst;
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_APPEARANCEDLG);
	psp[3].pszIcon = NULL;
	psp[3].pfnDlgProc = AppearanceDlgProc;
	psp[3].pszTitle = TEXT("Appearance");
	psp[3].lParam = 0;

	psp[4].dwSize = sizeof(PROPSHEETPAGE);
	psp[4].dwFlags = PSP_USETITLE;
	psp[4].hInstance = g_hinst;
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_SETTINGSDLG);
	psp[4].pszIcon = NULL;
	psp[4].pfnDlgProc = SettingsDlgProc;
	psp[4].pszTitle = TEXT("Settings");
	psp[4].lParam = 0;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
	psh.hwndParent = 0;
	psh.hInstance = g_hinst;
	psh.pszIcon = 0;
	psh.pszCaption = TEXT("Display Properties");
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = (PFNPROPSHEETCALLBACK)PropSheetCallback;

	PropertySheet(&psh);

	// cleanup
	Gdiplus::GdiplusShutdown(gdiplusToken);
	pThemeManager->Release();
}

extern "C" LONG APIENTRY CPlApplet(
	HWND hwndCPL,       // handle of Control Panel window
	UINT uMsg,          // message
	LONG_PTR lParam1,       // first message parameter
	LONG_PTR lParam2        // second message parameter
)
{
	LPCPLINFO lpCPlInfo;
	LONG retCode = 0;

	switch (uMsg)
	{
	case CPL_INIT:
		return TRUE;

	case CPL_GETCOUNT:
		return 1L;

	case CPL_INQUIRE:
		lpCPlInfo = (LPCPLINFO)lParam2;
		lpCPlInfo->idIcon = IDI_ICON1;
		lpCPlInfo->idName = IDS_THEMESCPL;
		lpCPlInfo->idInfo = IDS_THEMESDESC;
		lpCPlInfo->lData = 0L;
		break;
	case CPL_DBLCLK:
		PropertySheetMoment();
		break;
	}
	return retCode;
}