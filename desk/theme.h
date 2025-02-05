#pragma once
#include "pch.h"

static constexpr GUID CLSID_ThemeManager2 = { 0x9324da94, 0x50ec, 0x4a14, { 0xa7, 0x70, 0xe9, 0x0c, 0xa0, 0x3e, 0x7c, 0x8f } };

enum THEME_MANAGER_INITIALIZATION_FLAGS : unsigned
{
    ThemeInitNoFlags = 0,
    ThemeInitCurrentThemeOnly = 1 << 0,
    ThemeInitFlagUnk1 = 1 << 1,
    ThemeInitFlagUnk2 = 1 << 2,
};

enum tagTHEMECAT {};

struct IWallpaperCollection : IUnknown
{
    virtual UINT WINAPI GetCount() = 0;
    virtual HRESULT WINAPI GetWallpaperAt(unsigned long, LPWSTR*) = 0;
};

struct ISlideshowSettings : IUnknown
{
    virtual HRESULT WINAPI Apply(bool) = 0;
    virtual BOOL WINAPI IsEqual(ISlideshowSettings*) = 0;
    virtual INT64 WINAPI GetInterval() = 0;
    virtual INT64 WINAPI GetShuffle() = 0;
    virtual HRESULT WINAPI UselessFunc1() = 0;
    virtual unsigned __int16* WINAPI GetFeedUrl() = 0;
    virtual BOOL WINAPI IsRssFeed() = 0;
    virtual HRESULT WINAPI SetSourceDirectory(const unsigned __int16*) = 0;
    virtual HRESULT WINAPI GetAllMatchingWallpapers(IWallpaperCollection**) = 0;
};

// const CThemeFile::`vftable'{for `ITheme'}
struct ITheme : IUnknown
{
public:
    virtual HRESULT WINAPI get_DisplayName(LPWSTR*) = 0;
    virtual HRESULT WINAPI put_DisplayName(unsigned short*) = 0;
    virtual HRESULT WINAPI get_ScreenSaver(unsigned short**) = 0;
    virtual HRESULT WINAPI put_ScreenSaver(unsigned short*) = 0;
    virtual HRESULT WINAPI get_VisualStyle(LPWSTR*) = 0;
    virtual HRESULT WINAPI put_VisualStyle(unsigned short*) = 0;
    virtual HRESULT WINAPI get_VisualStyleColor(unsigned short**) = 0;
    virtual HRESULT WINAPI put_VisualStyleColor(unsigned short*) = 0;
    virtual HRESULT WINAPI get_VisualStyleSize(unsigned short**) = 0;
    virtual HRESULT WINAPI put_VisualStyleSize(unsigned short*) = 0;
    virtual HRESULT WINAPI get_VisualStyleVersion(int*) = 0;
    virtual HRESULT WINAPI put_VisualStyleVersion(int) = 0;
    virtual HRESULT WINAPI get_ColorizationColor(unsigned long*) = 0;
    virtual HRESULT WINAPI put_ColorizationColor(unsigned long) = 0;
    virtual HRESULT WINAPI get_ThemeId(struct _GUID*) = 0;
    virtual HRESULT WINAPI put_ThemeId(struct _GUID const&) = 0;
    virtual HRESULT WINAPI get_Background(WCHAR**) = 0;
    virtual HRESULT WINAPI put_Background(unsigned short*) = 0;
    virtual HRESULT WINAPI get_BackgroundPosition(enum  DESKTOP_WALLPAPER_POSITION*) = 0;
    virtual HRESULT WINAPI put_BackgroundPosition(enum  DESKTOP_WALLPAPER_POSITION) = 0;
    virtual HRESULT WINAPI get_BackgroundWriteTime(struct _FILETIME*) = 0;
    virtual HRESULT WINAPI put_BackgroundWriteTime(struct _FILETIME const*) = 0;
    virtual HRESULT WINAPI ClearBackgroundWriteTime(void) = 0;
    virtual HRESULT WINAPI get_SlideshowSettings(struct ISlideshowSettings**) = 0;
    virtual HRESULT WINAPI put_SlideshowSettings(struct ISlideshowSettings*) = 0;
    virtual HRESULT WINAPI get_SlideshowSourceDirectory(LPWSTR*) = 0;
    virtual HRESULT WINAPI put_SlideshowSourceDirectory(unsigned short*) = 0;
    virtual HRESULT WINAPI get_RSSFeed(unsigned short**) = 0;
    virtual HRESULT WINAPI IsSlideshowEnabled(int*) = 0;
    virtual HRESULT WINAPI GetSlideshowSettingsWithoutFiles(struct ISlideshowSettings**) = 0;
    virtual HRESULT WINAPI GetPath(short, unsigned short**) = 0;
    virtual HRESULT WINAPI SetPath(unsigned short*) = 0;
    virtual HRESULT WINAPI GetCursor(unsigned short*, unsigned short**) = 0;
    virtual HRESULT WINAPI SetCursor(unsigned short*, unsigned short*) = 0;
    virtual HRESULT WINAPI GetSoundSchemeName(unsigned short**) = 0;
    virtual HRESULT WINAPI SetSoundSchemeName(unsigned short*) = 0;
    virtual HRESULT WINAPI GetSound(unsigned short*, unsigned int, unsigned short**) = 0;
    virtual HRESULT WINAPI SetSound(unsigned short*, unsigned short*) = 0;
    virtual HRESULT WINAPI GetAllSoundEvents(unsigned short**) = 0;
    virtual HRESULT WINAPI GetDesktopIcon(unsigned short*, int, unsigned short**) = 0;
    virtual HRESULT WINAPI GetDefaultDesktopIcon(unsigned short*, unsigned short**) = 0;
    virtual HRESULT WINAPI SetDesktopIcon(unsigned short*, unsigned short*) = 0;
    virtual HRESULT WINAPI GetCategory(enum  tagTHEMECAT*) = 0;
    virtual HRESULT WINAPI GetLogonBackgroundFlag(int*) = 0;
    virtual HRESULT WINAPI SetLogonBackgroundFlag(void) = 0;
    virtual HRESULT WINAPI ClearLogonBackgroundFlag(void) = 0;
    virtual HRESULT WINAPI GetAutoColorization(int*) = 0;
    virtual HRESULT WINAPI SetAutoColorization(int) = 0;
    virtual HRESULT WINAPI GetMultimonBackgroundsEnabled(int*) = 0;
    virtual HRESULT WINAPI SetMultimonBackgroundsEnabled(int) = 0;
    virtual HRESULT WINAPI GetMultimonBackground(unsigned int, unsigned short**) = 0;
    virtual HRESULT WINAPI SetMultimonBackground(unsigned int, unsigned short*) = 0;
    virtual HRESULT WINAPI GetHighContrast(int*) = 0;
    virtual HRESULT WINAPI SetHighContrast(int) = 0;
    virtual HRESULT WINAPI GetThemeMagicValue(unsigned short**) = 0;
    virtual HRESULT WINAPI SetThemeMagicValue(unsigned short*) = 0;
    virtual HRESULT WINAPI GetThemeColor(unsigned short const*, unsigned short**) = 0;
    virtual HRESULT WINAPI GetThemeImage(int, struct HBITMAP__**) = 0;
    virtual HRESULT WINAPI GetWindowColorPreview(struct HBITMAP__**) = 0;
    virtual HRESULT WINAPI GetBackgroundColor(unsigned long*) = 0;
    virtual HRESULT WINAPI GetColor(unsigned int, unsigned long*) = 0;
    virtual HRESULT WINAPI GetBrandLogo(unsigned short**) = 0;
    virtual HRESULT WINAPI SetBrandLogo(unsigned short*) = 0;
    virtual HRESULT WINAPI ClearBrandLogo(void) = 0;
    virtual HRESULT WINAPI GetScreenSaverName(unsigned short**) = 0;
    virtual HRESULT WINAPI GetBackgroundPreview(struct HBITMAP__**) = 0;
    virtual HRESULT WINAPI Copy(struct ITheme**) = 0;
    virtual HRESULT WINAPI SetThemeColor(unsigned short const*, unsigned long) = 0;

    // see "re" folder for full vtables

public:
    HRESULT GetDisplayName(LPWSTR& name)
    {
        LPWSTR lpwstr = nullptr;
        auto hr = get_DisplayName(&lpwstr);
        if (SUCCEEDED(hr) && lpwstr)
        {
            if (lpwstr)
            {
                name = lpwstr;
                SysFreeString(lpwstr);
            }
            else
            {
                hr = E_FAIL;
            }
        }
        return hr;
    }
};

// const CThemeManager2::`vftable'
MIDL_INTERFACE("{c1e8c83e-845d-4d95-81db-e283fdffc000}") IThemeManager2 : IUnknown
{
  virtual HRESULT WINAPI Init(THEME_MANAGER_INITIALIZATION_FLAGS) = 0;
  virtual HRESULT WINAPI InitAsync(HWND, int) = 0;
  virtual HRESULT WINAPI Refresh() = 0;
  virtual HRESULT WINAPI RefreshAsync(HWND, int) = 0;
  virtual HRESULT WINAPI RefreshComplete() = 0;
  virtual HRESULT WINAPI GetThemeCount(int*) = 0;
  virtual HRESULT WINAPI GetTheme(int, ITheme**) = 0;
  virtual HRESULT WINAPI IsThemeDisabled(int, int*) = 0;
  virtual HRESULT WINAPI GetCurrentTheme(int*) = 0;
  virtual HRESULT WINAPI SetCurrentTheme(
    HWND parent,
    int theme_idx,
    int apply_now_not_only_registry, // 1 when called in Windows
    ULONG apply_flags, // 0 when called in Windows
    ULONG pack_flags // 0 when called in Windows
  ) = 0;
  virtual HRESULT WINAPI GetCustomTheme(int*) = 0;
  virtual HRESULT WINAPI GetDefaultTheme(int*) = 0;
  virtual HRESULT WINAPI CreateThemePack(HWND, LPCWSTR, ULONG pack_flags) = 0;
  virtual HRESULT WINAPI CloneAndSetCurrentTheme(HWND, LPCWSTR, LPWSTR*) = 0;
  virtual HRESULT WINAPI InstallThemePack(HWND, LPCWSTR, int, ULONG pack_flags, LPWSTR*, ITheme**) = 0;
  virtual HRESULT WINAPI DeleteTheme(LPCWSTR) = 0;
  virtual HRESULT WINAPI OpenTheme(HWND, LPCWSTR, ULONG pack_flags) = 0;
  virtual HRESULT WINAPI AddAndSelectTheme(HWND, LPCWSTR, ULONG apply_flags, ULONG pack_flags) = 0;
  virtual HRESULT WINAPI SQMCurrentTheme() = 0;
  virtual HRESULT WINAPI ExportRoamingThemeToStream(IStream*, int) = 0;
  virtual HRESULT WINAPI ImportRoamingThemeFromStream(IStream*, int) = 0;
  virtual HRESULT WINAPI UpdateColorSettingsForLogonUI() = 0;
  virtual HRESULT WINAPI GetDefaultThemeId(GUID*) = 0;
  virtual HRESULT WINAPI UpdateCustomTheme() = 0;
};
