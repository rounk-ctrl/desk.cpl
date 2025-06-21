#pragma once
#include "pch.h"
#include "wndprvw.h"

class CScrSaverDlgProc
    : public WTL::CPropertyPageImpl<CScrSaverDlgProc>
{
public:
    enum { IDD = IDD_SCRSVRDLG };

private:
    BEGIN_MSG_MAP(CScrSaverDlgProc)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(1300, CBN_SELCHANGE, OnScreenSaverComboboxChange)
        COMMAND_HANDLER(1303, BN_CLICKED, OnScreenSaverSettings)
        COMMAND_HANDLER(1304, BN_CLICKED, OnScreenSaverPreview)
        COMMAND_HANDLER(1314, BN_CLICKED, OnPowerBtn)
        COMMAND_HANDLER(1320, BN_CLICKED, OnSecureCheck)
        NOTIFY_CODE_HANDLER(UDN_DELTAPOS, OnTimeChange)
        MESSAGE_HANDLER(WM_ACTIVATE, OnActivate)
        CHAIN_MSG_MAP(WTL::CPropertyPageImpl<CScrSaverDlgProc>)
    END_MSG_MAP()

    BOOL OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL OnScreenSaverComboboxChange(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnScreenSaverSettings(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnScreenSaverPreview(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnPowerBtn(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnSecureCheck(UINT code, UINT id, HWND hWnd, BOOL& bHandled);
    BOOL OnTimeChange(WPARAM wParam, LPNMHDR nmhdr, BOOL& bHandled);
    BOOL OnApply();
    BOOL OnSetActive();
    BOOL OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


    /// custom methods
    HBITMAP MonitorAsBmp(int width, int height, WORD id, COLORREF maskColor);
    VOID AddScreenSavers(HWND comboBox);
    VOID ScreenPreview(HWND preview);
    VOID ScreenSettings(HWND preview);


    /// custom variables
    HWND hScrPreview;
    HWND hEnergy;
    HWND hScrCombo;
    HWND hBtnSettings;
    HWND hBtnPreview;
    HWND hWndStatic;
    HWND updown;
    HWND secureCheck;
    SIZE scrSize;
    SIZE energySize;
    LPCWSTR selectedScrSaver{};
    PROCESS_INFORMATION pi2;
    Microsoft::WRL::ComPtr<IWindowPreview> pWndPreview;
};