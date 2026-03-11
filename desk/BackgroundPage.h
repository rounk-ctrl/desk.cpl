#pragma once
#include "pch.h"
#include "wndprvw.h"

#define WM_SLIDESHOW_BEGIN  (WM_USER + 1)  // deselect all
#define WM_ADD_SLIDESHOW_ITEMS (WM_USER + 2)  // select wallpaper

class CBackgroundDlgProc
    : public CPropertyPageImpl<CBackgroundDlgProc>
{
public:
    enum {IDD = IDD_BACKGROUNDDLG};

private:
    BEGIN_MSG_MAP(CBackgroundDlgProc)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        COMMAND_HANDLER(1205, CBN_SELCHANGE, OnBgSizeChange)
        COMMAND_HANDLER(1203, BN_CLICKED, OnBrowse)
        COMMAND_HANDLER(1207, BN_CLICKED, OnColorPick)
        COMMAND_HANDLER(1208, BN_CLICKED, OnDeskCustomize)
        NOTIFY_HANDLER(1202, LVN_ITEMCHANGED, OnWallpaperSelection)
        MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)

        MESSAGE_HANDLER(WM_SLIDESHOW_BEGIN, OnSlideshowBegin)
        MESSAGE_HANDLER(WM_ADD_SLIDESHOW_ITEMS, OnAddSlideshowItems)
        CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CBackgroundDlgProc>)
    END_MSG_MAP()


    BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnBgSizeChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnBrowse(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnColorPick(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnDeskCustomize(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnWallpaperSelection(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled);
    BOOL OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnSlideshowBegin(UINT, WPARAM, LPARAM, BOOL&);
    BOOL OnAddSlideshowItems(UINT, WPARAM, LPARAM, BOOL&);
    BOOL OnApply();
    BOOL OnSetActive();

    /// custom methods
    int AddItem(HWND hListView, int rowIndex, LPCWSTR text);
    int AddColumn(HWND hListView, int width);
    LPWSTR GetWallpaperPath(HWND hListView, int iIndex);
    void AddMissingWallpapers(IUnknown* th);
    void SelectCurrentWallpaper();
    void _UpdateButtonBmp();
    void _UpdatePreview(UINT uFlags);

    /// custom variables
    HIMAGELIST hml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
    HWND hListView;
    HWND hBackPreview;
    HWND hPosCombobox;
    SIZE backPreviewSize;
    int selCount;
    BOOL fWallpaperApply;
    std::vector<LPWSTR> slideshowWallpapers;
    Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;
};