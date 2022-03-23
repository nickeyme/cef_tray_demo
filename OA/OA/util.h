#pragma once
#ifndef _GdiPlus_H__  
#define _GdiPlus_H__
#include <GdiPlus.h>
#include <VSStyle.h>
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;
#endif  

class util
{
private :
	static PrivateFontCollection * fontcollection;
	static int util::getPPI();
public:
	
	static BOOL addFont(CString &fontPath);
	static FontFamily * getFontFamily(LPCTSTR fontFamily);
	static int nPPI;
	static BOOL checkMouseInRect(POINT & pos, RECT & hWndRect, CRect * rect);
	static SizeF getTextBounds(FontFamily * fontfamily, Font& font, StringFormat& strFormat, CString& szText);
	static CString getRunDir();
	static CString getRunDir(LPWSTR str);
	static UINT checkTaskBarDirection();
	static RECT getTrayRect(HWND hWnd);
	static RECT getHWndRect(HWND hWnd);
	static HWND getSysTrayBarWnd();
	static POINT getMousePoint();
	static int getPixel(double inch);
	util();
	~util();
};

