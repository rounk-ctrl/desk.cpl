#include "pch.h"
#include "cscheme.h"

DWORD GetNcSysColor(int nIndex)
{
	if (selectedTheme->selectedScheme)
	{
		return selectedTheme->selectedScheme->rgb[nIndex];
	}
	return GetSysColor(nIndex);
}

HBRUSH GetNcSysColorBrush(int nIndex)
{
	if (selectedTheme->selectedScheme)
	{
		return CreateSolidBrush(selectedTheme->selectedScheme->rgb[nIndex]);
	}
	return GetSysColorBrush(nIndex);
}
