#pragma once
#include "pch.h"
#include "wndprvw.h"

class CBackgroundDlgProc
    : public CPropertyPageImpl<CBackgroundDlgProc>
{
public:
    enum {IDD = IDD_BACKGROUNDDLG};

private:
    BEGIN_MSG_MAP(CBackgroundDlgProc)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(1205, CBN_SELCHANGE, OnBgSizeChange)
        COMMAND_HANDLER(1203, BN_CLICKED, OnBrowse)
        COMMAND_HANDLER(1207, BN_CLICKED, OnColorPick)
        NOTIFY_HANDLER(1202, LVN_ITEMCHANGED, OnWallpaperSelection)
        MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
        CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CBackgroundDlgProc>)
    END_MSG_MAP()


    BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnBgSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnBrowse(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnColorPick(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnWallpaperSelection(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled);
    BOOL OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnApply();
    BOOL OnSetActive();

    /// custom methods
    int AddItem(HWND hListView, int rowIndex, LPCWSTR text);
    int AddColumn(HWND hListView, int width);
    LPWSTR GetWallpaperPath(HWND hListView, int iIndex);
    BOOL ColorPicker(HWND hWnd, CHOOSECOLOR* clrOut);
    void AddMissingWallpapers(IUnknown* th);
    void SelectCurrentWallpaper(IUnknown* th);

    /// custom variables
    HIMAGELIST hml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
    BOOL firstInit;
    int selectedIndex;
    HWND hListView;
    HWND hBackPreview;
    HWND hPosCombobox;
    SIZE backPreviewSize;
    Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;
};