#pragma once
#include "pch.h"

typedef HRESULT(WINAPI* FmsInitializeEnumerator_t)(void** pFontEnumerator, char InitFlags);
typedef HRESULT(WINAPI* FmsGetFilteredPropertyList_t)(void* FontEnumerator, int PropertyType, unsigned int* pPropertyCount, unsigned int* pListSize, wchar_t* PropertyList);
typedef HRESULT(WINAPI* FmsFreeEnumerator_t)(void** pFontEnumerator);
//typedef HRESULT(WINAPI* FmsGetGDILogFont_t)(void* FontEnumerator, unsigned int FontID, unsigned short CharSet, ENUMLOGFONTEXW* pEnumLogFont, NEWTEXTMETRICEXW* pTextMetric);
//typedef HRESULT(WINAPI* FmsGetFilteredFontList_t)(void* FontEnumerator, unsigned int* pListSize, unsigned int* FontIDList);

extern FmsInitializeEnumerator_t FmsInitializeEnumerator;
extern FmsGetFilteredPropertyList_t FmsGetFilteredPropertyList;
extern FmsFreeEnumerator_t FmsFreeEnumerator;

void InitFms();
HRESULT GetFilteredFontFamilies(_Out_ UINT* cFontFamily, _Out_ wchar_t*** ppFontFamily);
