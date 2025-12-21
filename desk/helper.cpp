#include "pch.h"
#include "helper.h"
#include "desk.h"
#include "uxtheme.h"
#include <sddl.h>
#include <AclAPI.h>

#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)

VOID _TerminateProcess(PROCESS_INFORMATION& hp)
{
	if (hp.hProcess != nullptr)
	{
		TerminateProcess(hp.hProcess, 0);
		CloseHandle(hp.hThread);
		CloseHandle(hp.hProcess);
		hp.hProcess = nullptr;
		hp.hThread = nullptr;
	}
}

COLORREF GetDeskopColor()
{
	COLORREF clr;
	// colorref never has any alpha value, so a bogus value
	if (selectedTheme->newColor != 0xB0000000)
	{
		clr = selectedTheme->newColor;
	}
	else if (selectedTheme->useDesktopColor)
	{
		pDesktopWallpaper->GetBackgroundColor(&clr);
	}
	else
	{
		ITheme* themeClass = new ITheme(currentITheme);
		themeClass->GetBackgroundColor(&clr);
	}
	return clr;
}


void EnumDir(LPCWSTR directory, LPCWSTR* extensions, int cExtensions, std::vector<LPWSTR>& vec, BOOL fEnumChildDirs)
{
	WCHAR path[MAX_PATH];
	StringCchPrintf(path, ARRAYSIZE(path), L"%s\\*", directory);

	WIN32_FIND_DATAW data = { 0 };
	// FindExInfoBasic is faster?? according to msdn
	HANDLE hFind = FindFirstFileEx(path, FindExInfoBasic, &data, FindExSearchNameMatch, NULL, 0);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do
	{
		if (lstrcmp(data.cFileName, L"."))
		{
			if (lstrcmp(data.cFileName, L".."))
			{
				WCHAR fullPath[MAX_PATH];
				StringCchPrintf(fullPath, ARRAYSIZE(fullPath), L"%s\\%s", directory, data.cFileName);

				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fEnumChildDirs)
				{
					EnumDir(fullPath, extensions, cExtensions, vec, fEnumChildDirs);
				}
				else
				{
					for (int i = 0; i < cExtensions; ++i)
					{
						if (lstrcmp(PathFindExtension(data.cFileName), extensions[i]) == 0)
						{
							vec.push_back(_wcsdup(fullPath));
						}
					}
				}
			}
		}
	} while (FindNextFileW(hFind, &data));
	FindClose(hFind);
}

void FreeString(LPWSTR& str)
{
	delete[] str;
	str = nullptr;
}

void StringCpy(LPWSTR& dest, LPWSTR src)
{
	size_t len = wcslen(src) + 1;
	dest = new wchar_t[len];
	wcscpy_s(dest, len, src);

}

void FreeBitmap(Gdiplus::Bitmap** bmp)
{
	if (*bmp)
	{
		delete *bmp;
		*bmp = nullptr;
	}
}

HRESULT DrawBitmapIfNotNull(Gdiplus::Bitmap* bmp, Gdiplus::Graphics* graph, Gdiplus::Rect rect)
{
	if (bmp != nullptr)
	{
		return graph->DrawImage(bmp, rect) == Gdiplus::Ok ? S_OK : E_FAIL;
	}
	return E_FAIL;
}

HTHEME OpenNcThemeData(LPVOID file, LPCWSTR pszClassList)
{
	return file ? OpenThemeDataFromFile(file, NULL, pszClassList, 0) : OpenThemeData(NULL, pszClassList);
}

NTSTATUS _OpenThemeSection(ACCESS_MASK mask, HANDLE* hSection)
{
	DWORD sessionId;
	ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);

	WCHAR szNtSection[MAX_PATH];
	StringCchPrintf(szNtSection, ARRAYSIZE(szNtSection), L"\\Sessions\\%lu\\Windows\\ThemeSection", sessionId);

	UNICODE_STRING szNtString;
	RtlInitUnicodeString(&szNtString, szNtSection);

	OBJECT_ATTRIBUTES objAttributes;
	InitializeObjectAttributes(&objAttributes, &szNtString, OBJ_CASE_INSENSITIVE, NULL, NULL);

	return NtOpenSection(hSection, mask, &objAttributes);
}

// ugh
BOOL IsClassicThemeEnabled()
{
	if (GetThemeAppProperties() == 0)return TRUE;
	HANDLE hSection;
	_OpenThemeSection(READ_CONTROL, &hSection);

	DWORD neededSize = 0;
	GetKernelObjectSecurity(hSection, DACL_SECURITY_INFORMATION, nullptr, 0, &neededSize);
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, neededSize);
	GetKernelObjectSecurity(hSection, DACL_SECURITY_INFORMATION, pSD, neededSize, &neededSize);

	LPWSTR sddlString = nullptr;
	ConvertSecurityDescriptorToStringSecurityDescriptor(pSD, SDDL_REVISION, DACL_SECURITY_INFORMATION, &sddlString, nullptr);

	BOOL bRet = FALSE;
	if (StrStrI(sddlString, SDDL_CREATE_CHILD) == NULL)
	{
		bRet = TRUE;
	}

	LocalFree(pSD);
	LocalFree(sddlString);
	CloseHandle(hSection);
	return bRet;
}

// https://stackoverflow.com/questions/656542/trim-a-string-in-c
char* ltrim(char* s)
{
	while (isspace(*s)) s++;
	return s;
}

char* rtrim(char* s)
{
	char* back = s + strlen(s);
	while (isspace(*--back));
	*(back + 1) = '\0';
	return s;
}

char* trim(char* s)
{
	return rtrim(ltrim(s));
}


wchar_t* strCut(wchar_t* s, const wchar_t* pattern)
{
	if (wchar_t* p = wcsstr(s, pattern))
	{
		wchar_t* q = p + lstrlen(pattern);
		while (*p++ = *q++);
	}
	return s;
}


void ScaleNonClientMetrics(NONCLIENTMETRICS& ncm, int dpi)
{
	ncm.iScrollHeight = MulDiv(ncm.iScrollHeight, dpi, 96);
	ncm.iScrollWidth = MulDiv(ncm.iScrollWidth, dpi, 96);
	ncm.iCaptionHeight = MulDiv(ncm.iCaptionHeight, dpi, 96);
	ncm.iCaptionWidth = MulDiv(ncm.iCaptionWidth, dpi, 96);

	ScaleLogFont(ncm.lfCaptionFont, dpi);
	ncm.iSmCaptionHeight = MulDiv(ncm.iSmCaptionHeight, dpi, 96);
	ncm.iSmCaptionWidth = MulDiv(ncm.iSmCaptionWidth, dpi, 96);

	ScaleLogFont(ncm.lfSmCaptionFont, dpi);
	ncm.iMenuHeight = MulDiv(ncm.iMenuHeight, dpi, 96);
	ncm.iMenuWidth = MulDiv(ncm.iMenuWidth, dpi, 96);

	ScaleLogFont(ncm.lfMenuFont, dpi);
	ScaleLogFont(ncm.lfStatusFont, dpi);
	ScaleLogFont(ncm.lfMessageFont, dpi);
}

void ScaleNonClientMetrics(NONCLIENTMETRICSW_2k& ncm, int dpi)
{
	ncm.iScrollHeight = MulDiv(ncm.iScrollHeight, dpi, 96);
	ncm.iScrollWidth = MulDiv(ncm.iScrollWidth, dpi, 96);
	ncm.iCaptionHeight = MulDiv(ncm.iCaptionHeight, dpi, 96);
	ncm.iCaptionWidth = MulDiv(ncm.iCaptionWidth, dpi, 96);

	ncm.iSmCaptionHeight = MulDiv(ncm.iSmCaptionHeight, dpi, 96);
	ncm.iSmCaptionWidth = MulDiv(ncm.iSmCaptionWidth, dpi, 96);

	ncm.iMenuHeight = MulDiv(ncm.iMenuHeight, dpi, 96);
	ncm.iMenuWidth = MulDiv(ncm.iMenuWidth, dpi, 96);

}

void ScaleLogFont(LOGFONT& lf, int dpi)
{
	lf.lfHeight = MulDiv(lf.lfHeight, dpi, 96);
}


HRESULT GetSolidBtnBmp(COLORREF clr, int dpi, SIZE size, HBITMAP* pbOut)
{
	int i = MulDiv(10, dpi, 96);
	Gdiplus::Bitmap bmp(size.cx - i, size.cy - i);
	Gdiplus::Graphics g(&bmp);
	Gdiplus::SolidBrush brush(Gdiplus::Color(SPLIT_COLORREF(clr)));
	g.FillRectangle(&brush, 0, 0, bmp.GetWidth(), bmp.GetHeight());

	return bmp.GetHBITMAP(Gdiplus::Color(0, 0, 0), pbOut) == Gdiplus::Ok ? S_OK : E_FAIL;
}

BOOL ColorPicker(COLORREF clr, HWND hWnd, CHOOSECOLOR* clrOut)
{
	static COLORREF acrCustClr[16];

	CHOOSECOLOR cc = { 0 };
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hWnd;
	cc.lpCustColors = acrCustClr;
	cc.rgbResult = clr;
	cc.Flags = CC_RGBINIT | CC_FULLOPEN;

	BOOL out = ChooseColor(&cc);
	*clrOut = cc;
	return out;
}

// callee must free allocated SCHEMEDATA when done
void CreateBlankScheme()
{
	if (!selectedTheme->selectedScheme)
	{
		COLORREF rgb[MAX_COLORS];
		for (int i = 0; i < MAX_COLORS; ++i)
		{
			rgb[i] = GetSysColor(i);
		}

		NONCLIENTMETRICSW ncm = { sizeof(ncm) };
		SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, NULL, 96);

		NONCLIENTMETRICSW_2k ncm2k;
		memcpy(&ncm2k, &ncm, sizeof(ncm2k));

		LOGFONT lfIcon;
		SystemParametersInfoForDpi(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, NULL, 96);

		SCHEMEDATA* currentData = (SCHEMEDATA*)malloc(sizeof(SCHEMEDATA));
		if (currentData)
		{
			currentData->lfIconTitle = lfIcon;
			currentData->ncm = ncm2k;
			memcpy(currentData->rgb, rgb, sizeof(rgb));
			currentData->variant = 0x8;			// indicates custom theme
			currentData->dpiScaled = false;

			selectedTheme->selectedScheme = currentData;
		}
	}
	else
	{
		SCHEMEDATA* currentData = (SCHEMEDATA*)malloc(sizeof(SCHEMEDATA));
		*currentData = *selectedTheme->selectedScheme;
		currentData->variant = 0x8;		// indicates custom theme

		if (selectedTheme->selectedScheme->variant == 0x8)
		{
			free(selectedTheme->selectedScheme);
			selectedTheme->selectedScheme = NULL;
		}

		selectedTheme->selectedScheme = currentData;
	}
}