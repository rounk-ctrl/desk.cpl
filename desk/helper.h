#pragma once
#include "framework.h"
#include "strnatcmp.h"

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

#define GETSIZE(size) (size).cx, (size).cy
#define SPLIT_COLORREF(clr) GetRValue(clr), GetGValue(clr), GetBValue(clr)

static VOID _TerminateProcess(PROCESS_INFORMATION& hp)
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

inline SIZE GetClientSIZE(HWND _hwnd)
{
	RECT rect;
	GetClientRect(_hwnd, &rect);
	return { RECTWIDTH(rect), RECTHEIGHT(rect) };
}

static COLORREF GetDeskopColor()
{
	COLORREF clr;
	if (selectedTheme->newColor)
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


static void EnumDir(LPCWSTR directory, LPCWSTR* extensions, int cExtensions, std::vector<LPWSTR>& vec)
{
	WCHAR path[MAX_PATH];
	StringCchPrintf(path, ARRAYSIZE(path), L"%s\\*", directory);

	WIN32_FIND_DATAW data;
	HANDLE hFind = FindFirstFile(path, &data);
	if (hFind == INVALID_HANDLE_VALUE) return;

	do
	{
		if (lstrcmp(data.cFileName, L"."))
		{
			if (lstrcmp(data.cFileName, L".."))
			{
				WCHAR fullPath[MAX_PATH];
				StringCchPrintf(fullPath, ARRAYSIZE(fullPath), L"%s\\%s", directory, data.cFileName);

				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					EnumDir(fullPath, extensions, cExtensions, vec);
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
