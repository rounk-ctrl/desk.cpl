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
struct ITheme10 : IUnknown
{
    STDMETHOD(get_DisplayName)(LPWSTR*) PURE;
    STDMETHOD(put_DisplayName)(LPWSTR) PURE;
    STDMETHOD(get_ScreenSaver)(LPWSTR*) PURE;
    STDMETHOD(put_ScreenSaver)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyle)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyle)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleColor)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyleColor)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleSize)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyleSize)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleVersion)(int*) PURE;
    STDMETHOD(put_VisualStyleVersion)(int) PURE;
    STDMETHOD(get_ColorizationColor)(unsigned long*) PURE;
    STDMETHOD(put_ColorizationColor)(unsigned long) PURE;
    STDMETHOD(get_ThemeId)(GUID*) PURE;
    STDMETHOD(put_ThemeId)(GUID const&) PURE;
    STDMETHOD(get_Background)(LPWSTR*) PURE;
    STDMETHOD(put_Background)(LPWSTR) PURE;
    STDMETHOD(get_BackgroundPosition)(DESKTOP_WALLPAPER_POSITION*) PURE;
    STDMETHOD(put_BackgroundPosition)(DESKTOP_WALLPAPER_POSITION) PURE;
    STDMETHOD(get_BackgroundWriteTime)(struct _FILETIME*) PURE;
    STDMETHOD(put_BackgroundWriteTime)(struct _FILETIME const*) PURE;
    STDMETHOD(ClearBackgroundWriteTime)(void) PURE;
    STDMETHOD(get_SlideshowSettings)(ISlideshowSettings**) PURE;
    STDMETHOD(put_SlideshowSettings)(ISlideshowSettings*) PURE;
    STDMETHOD(get_SlideshowSourceDirectory)(LPWSTR*) PURE;
    STDMETHOD(put_SlideshowSourceDirectory)(LPWSTR) PURE;
    STDMETHOD(get_RSSFeed)(LPWSTR*) PURE;
    STDMETHOD(IsSlideshowEnabled)(int*) PURE;
    STDMETHOD(GetSlideshowSettingsWithoutFiles)(ISlideshowSettings**) PURE;
    STDMETHOD(GetPath)(short, LPWSTR*) PURE;
    STDMETHOD(SetPath)(LPWSTR) PURE;
    STDMETHOD(GetCursor)(LPWSTR, LPWSTR*) PURE;
    STDMETHOD(SetCursor)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetSoundSchemeName)(LPWSTR*) PURE;
    STDMETHOD(SetSoundSchemeName)(LPWSTR) PURE;
    STDMETHOD(GetSound)(LPWSTR, unsigned int, LPWSTR*) PURE;
    STDMETHOD(SetSound)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetAllSoundEvents)(LPWSTR*) PURE;
    STDMETHOD(GetDesktopIcon)(LPWSTR, int, LPWSTR*) PURE;
    STDMETHOD(GetDefaultDesktopIcon)(LPWSTR, LPWSTR*) PURE;
    STDMETHOD(SetDesktopIcon)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetCategory)(tagTHEMECAT*) PURE;
    STDMETHOD(GetLogonBackgroundFlag)(int*) PURE;
    STDMETHOD(SetLogonBackgroundFlag)(void) PURE;
    STDMETHOD(ClearLogonBackgroundFlag)(void) PURE;
    STDMETHOD(GetAutoColorization)(int*) PURE;
    STDMETHOD(SetAutoColorization)(int) PURE;
    STDMETHOD(GetMultimonBackgroundsEnabled)(int*) PURE;
    STDMETHOD(SetMultimonBackgroundsEnabled)(int) PURE;
    STDMETHOD(GetMultimonBackground)(unsigned int, LPWSTR*) PURE;
    STDMETHOD(SetMultimonBackground)(unsigned int, LPWSTR) PURE;
    STDMETHOD(GetHighContrast)(int*) PURE;
    STDMETHOD(SetHighContrast)(int) PURE;
    STDMETHOD(GetThemeMagicValue)(LPWSTR*) PURE;
    STDMETHOD(SetThemeMagicValue)(LPWSTR) PURE;
    STDMETHOD(GetThemeColor)(LPCWSTR, LPWSTR*) PURE;
    STDMETHOD(GetThemeImage)(int, HBITMAP*) PURE;
    STDMETHOD(GetWindowColorPreview)(HBITMAP*) PURE;
    STDMETHOD(GetBackgroundColor)(unsigned long*) PURE;
    STDMETHOD(GetColor)(unsigned int, unsigned long*) PURE;
    STDMETHOD(GetBrandLogo)(LPWSTR*) PURE;
    STDMETHOD(SetBrandLogo)(LPWSTR) PURE;
    STDMETHOD(ClearBrandLogo)(void) PURE;
    STDMETHOD(GetScreenSaverName)(LPWSTR*) PURE;
    STDMETHOD(GetBackgroundPreview)(HBITMAP*) PURE;
    STDMETHOD(Stub1)(void) PURE;
    STDMETHOD(SetThemeColor)(LPCWSTR, unsigned long) PURE;
    // see "re" folder for full vtables
};

struct ITheme1809 : IUnknown
{
    STDMETHOD(get_DisplayName)(LPWSTR*) PURE;
    STDMETHOD(put_DisplayName)(LPWSTR) PURE;
    //STDMETHOD(get_ScreenSaver)(LPWSTR*) PURE;
    //STDMETHOD(put_ScreenSaver)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyle)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyle)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleColor)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyleColor)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleSize)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyleSize)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleVersion)(int*) PURE;
    STDMETHOD(put_VisualStyleVersion)(int) PURE;
    STDMETHOD(get_ColorizationColor)(unsigned long*) PURE;
    STDMETHOD(put_ColorizationColor)(unsigned long) PURE;
    STDMETHOD(get_ThemeId)(GUID*) PURE;
    STDMETHOD(put_ThemeId)(GUID const&) PURE;
    STDMETHOD(get_Background)(LPWSTR*) PURE;
    STDMETHOD(put_Background)(LPWSTR) PURE;
    STDMETHOD(get_BackgroundPosition)(DESKTOP_WALLPAPER_POSITION*) PURE;
    STDMETHOD(put_BackgroundPosition)(DESKTOP_WALLPAPER_POSITION) PURE;
    STDMETHOD(get_BackgroundWriteTime)(struct _FILETIME*) PURE;
    STDMETHOD(put_BackgroundWriteTime)(struct _FILETIME const*) PURE;
    STDMETHOD(ClearBackgroundWriteTime)(void) PURE;
    STDMETHOD(get_SlideshowSettings)(ISlideshowSettings**) PURE;
    STDMETHOD(put_SlideshowSettings)(ISlideshowSettings*) PURE;
    STDMETHOD(get_SlideshowSourceDirectory)(LPWSTR*) PURE;
    STDMETHOD(put_SlideshowSourceDirectory)(LPWSTR) PURE;
    STDMETHOD(get_RSSFeed)(LPWSTR*) PURE;
    STDMETHOD(IsSlideshowEnabled)(int*) PURE;
    STDMETHOD(GetSlideshowSettingsWithoutFiles)(ISlideshowSettings**) PURE;
    STDMETHOD(GetPath)(short, LPWSTR*) PURE;
    STDMETHOD(SetPath)(LPWSTR) PURE;
    STDMETHOD(GetCursor)(LPWSTR, LPWSTR*) PURE;
    STDMETHOD(SetCursor)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetSoundSchemeName)(LPWSTR*) PURE;
    STDMETHOD(SetSoundSchemeName)(LPWSTR) PURE;
    STDMETHOD(GetSound)(LPWSTR, unsigned int, LPWSTR*) PURE;
    STDMETHOD(SetSound)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetAllSoundEvents)(LPWSTR*) PURE;
    STDMETHOD(GetDesktopIcon)(LPWSTR, int, LPWSTR*) PURE;
    STDMETHOD(GetDefaultDesktopIcon)(LPWSTR, LPWSTR*) PURE;
    STDMETHOD(SetDesktopIcon)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetCategory)(tagTHEMECAT*) PURE;
    STDMETHOD(GetLogonBackgroundFlag)(int*) PURE;
    STDMETHOD(SetLogonBackgroundFlag)(void) PURE;
    STDMETHOD(ClearLogonBackgroundFlag)(void) PURE;
    STDMETHOD(GetAutoColorization)(int*) PURE;
    STDMETHOD(SetAutoColorization)(int) PURE;
    STDMETHOD(GetMultimonBackgroundsEnabled)(int*) PURE;
    STDMETHOD(SetMultimonBackgroundsEnabled)(int) PURE;
    STDMETHOD(GetMultimonBackground)(unsigned int, LPWSTR*) PURE;
    STDMETHOD(SetMultimonBackground)(unsigned int, LPWSTR) PURE;
    STDMETHOD(GetHighContrast)(int*) PURE;
    STDMETHOD(SetHighContrast)(int) PURE;
    STDMETHOD(GetThemeMagicValue)(LPWSTR*) PURE;
    STDMETHOD(SetThemeMagicValue)(LPWSTR) PURE;
    STDMETHOD(GetThemeColor)(LPCWSTR, LPWSTR*) PURE;
    STDMETHOD(GetThemeImage)(int, HBITMAP*) PURE;
    STDMETHOD(GetWindowColorPreview)(HBITMAP*) PURE;
    STDMETHOD(GetBackgroundColor)(unsigned long*) PURE;
    STDMETHOD(GetColor)(unsigned int, unsigned long*) PURE;
    STDMETHOD(GetBrandLogo)(LPWSTR*) PURE;
    STDMETHOD(SetBrandLogo)(LPWSTR) PURE;
    STDMETHOD(ClearBrandLogo)(void) PURE;
    STDMETHOD(GetScreenSaverName)(LPWSTR*) PURE;
    STDMETHOD(GetBackgroundPreview)(HBITMAP*) PURE;
    STDMETHOD(Stub1)(void) PURE;
    STDMETHOD(SetThemeColor)(LPCWSTR, unsigned long) PURE;
    // see "re" folder for full vtables
};

struct ITheme1903 : IUnknown
{
    STDMETHOD(get_DisplayName)(LPWSTR*) PURE;
    STDMETHOD(put_DisplayName)(LPWSTR) PURE;
    //STDMETHOD(get_ScreenSaver)(LPWSTR*) PURE;
    //STDMETHOD(put_ScreenSaver)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyle)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyle)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleColor)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyleColor)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleSize)(LPWSTR*) PURE;
    STDMETHOD(put_VisualStyleSize)(LPWSTR) PURE;
    STDMETHOD(get_VisualStyleVersion)(int*) PURE;
    STDMETHOD(put_VisualStyleVersion)(int) PURE;
    STDMETHOD(get_ColorizationColor)(unsigned long*) PURE;
    STDMETHOD(put_ColorizationColor)(unsigned long) PURE;
    STDMETHOD(get_ThemeId)(GUID*) PURE;
    STDMETHOD(put_ThemeId)(GUID const&) PURE;
    // 1903+
    STDMETHOD(get_AppMode)(int*) PURE;
    STDMETHOD(put_AppMode)(int) PURE;
    STDMETHOD(get_SystemMode)(int*) PURE;
    STDMETHOD(put_SystemMode)(int*) PURE;

    STDMETHOD(get_Background)(LPWSTR*) PURE;
    STDMETHOD(put_Background)(LPWSTR) PURE;
    STDMETHOD(get_BackgroundPosition)(DESKTOP_WALLPAPER_POSITION*) PURE;
    STDMETHOD(put_BackgroundPosition)(DESKTOP_WALLPAPER_POSITION) PURE;
    STDMETHOD(get_BackgroundWriteTime)(struct _FILETIME*) PURE;
    STDMETHOD(put_BackgroundWriteTime)(struct _FILETIME const*) PURE;
    STDMETHOD(ClearBackgroundWriteTime)(void) PURE;
    STDMETHOD(get_SlideshowSettings)(ISlideshowSettings**) PURE;
    STDMETHOD(put_SlideshowSettings)(ISlideshowSettings*) PURE;
    STDMETHOD(get_SlideshowSourceDirectory)(LPWSTR*) PURE;
    STDMETHOD(put_SlideshowSourceDirectory)(LPWSTR) PURE;
    STDMETHOD(get_RSSFeed)(LPWSTR*) PURE;
    STDMETHOD(IsSlideshowEnabled)(int*) PURE;
    STDMETHOD(GetSlideshowSettingsWithoutFiles)(ISlideshowSettings**) PURE;
    STDMETHOD(GetPath)(short, LPWSTR*) PURE;
    STDMETHOD(SetPath)(LPWSTR) PURE;
    STDMETHOD(GetCursor)(LPWSTR, LPWSTR*) PURE;
    STDMETHOD(SetCursor)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetSoundSchemeName)(LPWSTR*) PURE;
    STDMETHOD(SetSoundSchemeName)(LPWSTR) PURE;
    STDMETHOD(GetSound)(LPWSTR, unsigned int, LPWSTR*) PURE;
    STDMETHOD(SetSound)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetAllSoundEvents)(LPWSTR*) PURE;
    STDMETHOD(GetDesktopIcon)(LPWSTR, int, LPWSTR*) PURE;
    STDMETHOD(GetDefaultDesktopIcon)(LPWSTR, LPWSTR*) PURE;
    STDMETHOD(SetDesktopIcon)(LPWSTR, LPWSTR) PURE;
    STDMETHOD(GetCategory)(tagTHEMECAT*) PURE;
    STDMETHOD(GetLogonBackgroundFlag)(int*) PURE;
    STDMETHOD(SetLogonBackgroundFlag)(void) PURE;
    STDMETHOD(ClearLogonBackgroundFlag)(void) PURE;
    STDMETHOD(GetAutoColorization)(int*) PURE;
    STDMETHOD(SetAutoColorization)(int) PURE;
    STDMETHOD(GetMultimonBackgroundsEnabled)(int*) PURE;
    STDMETHOD(SetMultimonBackgroundsEnabled)(int) PURE;
    STDMETHOD(GetMultimonBackground)(unsigned int, LPWSTR*) PURE;
    STDMETHOD(SetMultimonBackground)(unsigned int, LPWSTR) PURE;
    STDMETHOD(GetHighContrast)(int*) PURE;
    STDMETHOD(SetHighContrast)(int) PURE;
    STDMETHOD(GetThemeMagicValue)(LPWSTR*) PURE;
    STDMETHOD(SetThemeMagicValue)(LPWSTR) PURE;
    STDMETHOD(GetThemeColor)(LPCWSTR, LPWSTR*) PURE;
    STDMETHOD(GetThemeImage)(int, HBITMAP*) PURE;
    STDMETHOD(GetWindowColorPreview)(HBITMAP*) PURE;
    STDMETHOD(GetBackgroundColor)(unsigned long*) PURE;
    STDMETHOD(GetColor)(unsigned int, unsigned long*) PURE;
    STDMETHOD(GetBrandLogo)(LPWSTR*) PURE;
    STDMETHOD(SetBrandLogo)(LPWSTR) PURE;
    STDMETHOD(ClearBrandLogo)(void) PURE;
    STDMETHOD(GetScreenSaverName)(LPWSTR*) PURE;
    STDMETHOD(GetBackgroundPreview)(HBITMAP*) PURE;
    STDMETHOD(Stub1)(void) PURE;
    STDMETHOD(SetThemeColor)(LPCWSTR, unsigned long) PURE;
    // see "re" folder for full vtables
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
  virtual HRESULT WINAPI GetTheme(int, IUnknown**) = 0;
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
  virtual HRESULT WINAPI InstallThemePack(HWND, LPCWSTR, int, ULONG pack_flags, LPWSTR*, IUnknown**) = 0;
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

#define THEMETOOL_APPLY_FLAG_IGNORE_BACKGROUND    (ULONG)(1 << 0)
#define THEMETOOL_APPLY_FLAG_IGNORE_CURSOR        (ULONG)(1 << 1)
#define THEMETOOL_APPLY_FLAG_IGNORE_DESKTOP_ICONS (ULONG)(1 << 2)
#define THEMETOOL_APPLY_FLAG_IGNORE_COLOR         (ULONG)(1 << 3)
#define THEMETOOL_APPLY_FLAG_IGNORE_SOUND         (ULONG)(1 << 4)
#define THEMETOOL_APPLY_FLAG_IGNORE_SCREENSAVER   (ULONG)(1 << 5)
#define THEMETOOL_APPLY_FLAG_UNKNOWN              (ULONG)(1 << 6)
#define THEMETOOL_APPLY_FLAG_UNKNOWN2             (ULONG)(1 << 7)
#define THEMETOOL_APPLY_FLAG_NO_HOURGLASS         (ULONG)(1 << 8)
#define THEMETOOL_PACK_FLAG_UNKNOWN1              (ULONG)(1 << 0)
#define THEMETOOL_PACK_FLAG_UNKNOWN2              (ULONG)(1 << 1)
#define THEMETOOL_PACK_FLAG_SILENT                (ULONG)(1 << 2)
#define THEMETOOL_PACK_FLAG_ROAMED                (ULONG)(1 << 3)
