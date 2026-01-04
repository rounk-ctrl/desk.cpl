#include "pch.h"
#include "cscheme.h"
#include "helper.h"

int cs_dpi;

DWORD NcGetSysColor(int nIndex)
{
	if (selectedTheme->selectedScheme)
	{
		return selectedTheme->selectedScheme->rgb[nIndex];
	}
	return GetSysColor(nIndex);
}

// callee must free the brush, if selectedScheme isnt NULL
HBRUSH NcGetSysColorBrush(int nIndex)
{
	if (selectedTheme->selectedScheme)
	{
		return CreateSolidBrush(selectedTheme->selectedScheme->rgb[nIndex]);
	}
	return GetSysColorBrush(nIndex);
}

__declspec(noinline) BOOL NcDrawFrameControl(HDC hdc, RECT* lprc, UINT uType, int type)
{
	HBRUSH br = NcGetSysColorBrush(COLOR_BTNFACE);
	FillRect(hdc, lprc, br);
	if (selectedTheme->selectedScheme) DeleteBrush(br);

	DrawEdge(hdc, lprc, EDGE_RAISED, BF_RECT | BF_SOFT);
	{
		int fontHeight = RECTHEIGHT(*lprc) - 4;
		HFONT hFont = CreateFont(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
			SYMBOL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Marlett");

		if (hFont)
		{
			HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
			COLORREF oldTextColor = SetTextColor(hdc, NcGetSysColor(COLOR_BTNTEXT));

			const wchar_t* text = L"\0";
			if (uType == DFC_CAPTION)
			{
				if (type == 1) text = L"r";
				else if (type == 2) text = L"1";
				else if (type == 3) text = L"0";
			}
			else if (uType == DFC_SCROLL)
			{
				if (type == 1) text = L"t";
				else if (type == 2) text = L"u";
			}
			DrawText(hdc, text, 1, lprc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

			SetTextColor(hdc, oldTextColor);
			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);
		}
	}
	return TRUE;
}

__declspec(noinline) int NcGetSystemMetrics(int nIndex)
{
	if (selectedTheme->selectedScheme)
	{
		if (!selectedTheme->selectedScheme->dpiScaled)
		{
			ScaleNonClientMetrics(selectedTheme->selectedScheme->ncm, cs_dpi);
			selectedTheme->selectedScheme->dpiScaled = true;
		}

		int iValue;
		NONCLIENTMETRICSW_2k ncm = selectedTheme->selectedScheme->ncm;
		switch (nIndex)
        {
            case SM_CXHSCROLL:  // fall through
            case SM_CXVSCROLL:  iValue = ncm.iScrollWidth;  break;
            case SM_CYHSCROLL:  // fall through
            case SM_CYVSCROLL:  iValue = ncm.iScrollHeight;  break;

            case SM_CXSIZE:     iValue = ncm.iCaptionWidth;  break;
            case SM_CYSIZE:     iValue = ncm.iCaptionHeight;  break;
            case SM_CYCAPTION:  iValue = ncm.iCaptionHeight + 1;  break;
            case SM_CXSMSIZE:   iValue = ncm.iSmCaptionWidth;  break;
            case SM_CYSMSIZE:   iValue = ncm.iSmCaptionHeight;  break;
            case SM_CXMENUSIZE: iValue = ncm.iMenuWidth;  break;
            case SM_CYMENUSIZE: iValue = ncm.iMenuHeight;  break;

			case SM_CXBORDER:
			case SM_CYBORDER:	iValue = ncm.iBorderWidth; break;

			//case SM_CXFRAME:	iValue = GetSystemMetrics(SM_CXEDGE) + ncm.iBorderWidth + 1; break;
            
            default:            iValue = GetSystemMetrics(nIndex); break;
        }

		// dpi scale the value
		//iValue = MulDiv(iValue, cs_dpi, 96);
        return iValue;
	}
	return GetSystemMetrics(nIndex);
}


void NcUpdateSystemMetrics(int nIndex, int value)
{
	if (selectedTheme->selectedScheme)
	{
		NONCLIENTMETRICSW_2k ncm = selectedTheme->selectedScheme->ncm;
		switch (nIndex)
		{
		case SM_CXHSCROLL:  // fall through
		case SM_CXVSCROLL:  ncm.iScrollWidth = value;  break;
		case SM_CYHSCROLL:  // fall through
		case SM_CYVSCROLL:  ncm.iScrollHeight = value;  break;

		case SM_CXSIZE:     ncm.iCaptionWidth = value;  break;
		case SM_CYSIZE:     ncm.iCaptionHeight = value;  break;
		case SM_CYCAPTION:  ncm.iCaptionHeight = value + 1;  break;
		case SM_CXSMSIZE:   ncm.iSmCaptionWidth = value;  break;
		case SM_CYSMSIZE:   ncm.iSmCaptionHeight = value;  break;
		case SM_CXMENUSIZE: ncm.iMenuWidth = value;  break;
		case SM_CYMENUSIZE: ncm.iMenuHeight = value;  break;

		case SM_CXBORDER:
		case SM_CYBORDER:	ncm.iBorderWidth = value; break;
		default: break;
		}

		selectedTheme->selectedScheme->ncm = ncm;
	}
}