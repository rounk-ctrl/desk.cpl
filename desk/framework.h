#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <cpl.h>
#include <prsht.h>
#include <objbase.h>
#include <winternl.h>
#include <gdiplus.h>
#include <Uxtheme.h>
#include <shobjidl_core.h>
#include <Shlwapi.h>
#include <commdlg.h>
#include <vssym32.h>
#include <windowsx.h>
#include <shellapi.h>

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlcrack.h>
#include <atlframe.h>

#pragma comment(lib, "Comctl32")
#pragma comment(lib, "Comdlg32")
#pragma comment(lib, "Msimg32")
#pragma comment(lib, "Gdiplus")
#pragma comment(lib, "Uxtheme")
#pragma comment(lib, "Shlwapi")
