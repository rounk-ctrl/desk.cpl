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
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_HANDLER(1101, CBN_SELCHANGE, OnThemeComboboxChange)
        CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CThemeDlgProc>)
    END_MSG_MAP()

    BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnThemeComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnApply();
    BOOL OnSetActive();

    // custom methods
    void UpdateThemeInfo(LPWSTR ws, int currThem);

    /// variables
    HWND hCombobox;
    HWND hPreview;
    SIZE size;
    Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;

    MYWINDOWINFO wnd[1] =
    {
        {
            WT_ACTIVE,
            {33, 33, 250, 33 + 124}
        }
    };

};