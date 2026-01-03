#include "pch.h"
#include "fms.h"

FmsInitializeEnumerator_t FmsInitializeEnumerator;
FmsGetFilteredPropertyList_t FmsGetFilteredPropertyList;
FmsFreeEnumerator_t FmsFreeEnumerator;

void InitFms()
{
	HMODULE hFms = LoadLibrary(L"fms.dll");
	if (hFms)
	{
		FmsInitializeEnumerator = (FmsInitializeEnumerator_t)GetProcAddress(hFms, MAKEINTRESOURCEA(14));
		FmsGetFilteredPropertyList = (FmsGetFilteredPropertyList_t)GetProcAddress(hFms, MAKEINTRESOURCEA(9));
		FmsFreeEnumerator = (FmsFreeEnumerator_t)GetProcAddress(hFms, MAKEINTRESOURCEA(4));

		//FreeLibrary(hFms);
	}
}

HRESULT GetFilteredFontFamilies(_Out_ UINT* cFontFamily, _Out_ wchar_t*** ppFontFamily)
{
	*cFontFamily = 0;
	*ppFontFamily = NULL;

	void* fontEnumerator = NULL;
	FmsInitializeEnumerator(&fontEnumerator, 0);

	unsigned int cProperties, cbProperties;
	HRESULT hr = FmsGetFilteredPropertyList(fontEnumerator, 2, &cProperties, &cbProperties, NULL);
	if (SUCCEEDED(hr))
	{
		int cchMax = cbProperties >> 1;

		wchar_t* arra = (wchar_t*)malloc(sizeof(wchar_t) * cchMax);
		hr = FmsGetFilteredPropertyList(fontEnumerator, 2, &cProperties, &cbProperties, arra);

		wchar_t* p = arra;
		wchar_t** lppFontFamilies = (wchar_t**)malloc(cProperties * sizeof(wchar_t*));
		for (UINT i = 0; i < cProperties; ++i)
		{
			int len = lstrlen(p) + 1;
			lppFontFamilies[i] = (wchar_t*)malloc(len * sizeof(wchar_t));
			StringCchPrintf(lppFontFamilies[i], len, L"%s", p);
			p += len;
		}

		*cFontFamily = cProperties;
		*ppFontFamily = lppFontFamilies;

		free(arra);
	}
	hr = FmsFreeEnumerator(&fontEnumerator);
	return hr;
}