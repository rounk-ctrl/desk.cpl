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

void ScaleLogFont(LOGFONT& lf, int dpi)
{
	lf.lfHeight = MulDiv(lf.lfHeight, dpi, 96);
}
