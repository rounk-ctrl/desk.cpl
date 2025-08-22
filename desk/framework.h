#pragma once

#include <strsafe.h>
#include <vector>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <winternl.h>
#include <cpl.h>
#include <objbase.h>
#include <gdiplus.h>
#include <Uxtheme.h>
#include <shobjidl_core.h>
#include <Shlwapi.h>
#include <commdlg.h>
#include <vssym32.h>
#include <windowsx.h>
#include <shellapi.h>
#include <wrl.h>


#ifdef _DEBUG
#include <wil\result.h>
#endif

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlcrack.h>
#include <atlframe.h>


#include <SetupAPI.h>
#include <initguid.h>
#include <Ntddvdeo.h>

#pragma comment(lib, "Comctl32")
#pragma comment(lib, "Comdlg32")
#pragma comment(lib, "Gdiplus")
#pragma comment(lib, "Uxtheme")
#pragma comment(lib, "Shlwapi")
#pragma comment(lib, "setupapi.lib")
