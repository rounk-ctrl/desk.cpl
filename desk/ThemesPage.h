#pragma once
#include "pch.h"
#include "wndprvw.h"

class CThemeDlgProc
    : public WTL::CPropertyPageImpl<CThemeDlgProc>
{
public:
    enum { IDD = IDD_THEMEDLG };

private:
    BEGIN_MSG_MAP(CThemeDlgProc)
        CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CThemeDlgProc>)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(1101, CBN_SELCHANGE, OnThemeComboboxChange)
    END_MSG_MAP()

    BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnThemeComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnApply();
    BOOL OnSetActive();

    // custom methods
    HBITMAP ThemePreviewBmp(int newwidth, int newheight, WCHAR* wallpaperPath, HANDLE hFile, COLORREF clrBg);
    void UpdateThemeInfo(LPWSTR ws, int currThem);

    /// variables
    HWND hCombobox;
    HWND hPreview;
    SIZE size;
    Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;
};