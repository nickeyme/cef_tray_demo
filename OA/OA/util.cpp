#include "stdafx.h"
#include "util.h"

PrivateFontCollection * util::fontcollection;
int util::nPPI = 0;

// 判断任务栏方位 会给托盘发送信息
UINT util::checkTaskBarDirection() {
	APPBARDATA taskbar = { 0 };
	taskbar.cbSize = sizeof(APPBARDATA);
	SHAppBarMessage(ABM_GETTASKBARPOS, &taskbar);
	return taskbar.uEdge;
}

// 获取当前窗口句柄对应任务栏图标位置
RECT util::getTrayRect(HWND hWnd) {
	NOTIFYICONIDENTIFIER identifier = { 0 };
	identifier.cbSize = sizeof(NOTIFYICONIDENTIFIER);
	identifier.uID = IDR_MAINFRAME;
	identifier.hWnd = hWnd;
	RECT trayRect;
	Shell_NotifyIconGetRect(&identifier, &trayRect);
	return trayRect;
}

POINT util::getMousePoint() {
	POINT p;
	GetCursorPos(&p);
	return p;
}

// 获取当前窗口宽高
RECT util::getHWndRect(HWND hWnd) {
	RECT hWndRect;
	GetWindowRect(hWnd, &hWndRect);
	return hWndRect;
}


HWND util::getSysTrayBarWnd() {
	HWND sysTrayBarWnd = FindWindow(L"shell_traywnd", NULL);
	if (!sysTrayBarWnd)
	{
		AfxMessageBox(L"获取句柄失败");
		ExitProcess(0);
	}
	return sysTrayBarWnd;
}

int util::getPixel(double inch) {
	return util::getPPI() * inch;
}

int util::getPPI() {
	if (nPPI == 0) {
		// 计算屏幕的PPI
		int nXScr = 0;//屏幕X分辨率
		int nYScr = 0;//屏幕Y分辨率
		int nXWidth = 0;//屏幕的宽度
		int nYHeight = 0; //屏幕的高度
		double dScrLeng = 0;

		nXScr = GetSystemMetrics(SM_CXSCREEN);//屏幕X分辨率
		nYScr = GetSystemMetrics(SM_CYSCREEN); //屏幕Y分辨率

		HDC hdcScreen = GetDC(NULL);   //获取HDC句柄
		nXWidth = GetDeviceCaps(hdcScreen, HORZSIZE);//屏幕的宽度
		nYHeight = GetDeviceCaps(hdcScreen, VERTSIZE); //屏幕的高度
		ReleaseDC(NULL, hdcScreen);

		dScrLeng = sqrt((double)(nXWidth * nXWidth + nYHeight * nYHeight));
		nPPI = (int)(sqrt(nXScr *nXScr + nYScr *nYScr) / (dScrLeng *0.03937007));
	}
	return nPPI;
}

CString util::getRunDir(LPWSTR str) {
	CString path = util::getRunDir();
	path.Append(L"\\");
	path.Append(str);
	return path;
}


CString util::getRunDir() {
	CString path;
	GetModuleFileName(NULL, path.GetBufferSetLength(MAX_PATH + 1), MAX_PATH);
	path.ReleaseBuffer();
	int pos = path.ReverseFind('\\');
	path = path.Left(pos);
	return path;
}


// CRect 以窗体为基准
BOOL util::checkMouseInRect(POINT & pos, RECT & hWndRect, CRect *  rect) {
	return (pos.x - hWndRect.left >= rect->left &&
		pos.x - hWndRect.left <= rect->right &&
		pos.y - hWndRect.top >= rect->top &&
		pos.y - hWndRect.top <= rect->bottom);
}

SizeF util::getTextBounds(FontFamily * fontfamily, Font& font, StringFormat& strFormat, CString& szText)
{
	GraphicsPath graphicsPathObj;
	graphicsPathObj.AddString(szText, -1, fontfamily, font.GetStyle(), font.GetSize(), Gdiplus::PointF(0, 0), &strFormat);
	Gdiplus::RectF rcBound;

	/// 获取边界范围
	graphicsPathObj.GetBounds(&rcBound);
	/// 返回文本的宽高
	return SizeF(rcBound.Width, rcBound.Height);
}

BOOL util::addFont(CString &fontPath) {
	if(!fontcollection){
		fontcollection = new PrivateFontCollection();
	}
	Gdiplus::Status nResults = fontcollection->AddFontFile(fontPath);
	return nResults != Gdiplus::Ok;
}

FontFamily * util::getFontFamily(LPCTSTR fontFamily) {
	// 先查本地字库
	FontFamily * font = new FontFamily(fontFamily);
	if (font->IsAvailable()) {
		return font;
	}


	// 没有本地字库
	int   nFound = 0;
	int   nCount = fontcollection->GetFamilyCount();
	FontFamily*  pFontFamily = new FontFamily[nCount];
	fontcollection->GetFamilies(nCount, pFontFamily, &nFound);

	if (nFound <= 0) {
		AfxMessageBox(L"装载字体失败");
	}

	FontFamily * correctFamily;
	WCHAR familyName[100];//这里为了简化程序，分配足够大的空间  
	for (int j = 0; j < nFound; ++j)
	{
		pFontFamily[j].GetFamilyName(familyName);
		if (_tcscmp(familyName, fontFamily) == 0) {
			correctFamily = &pFontFamily[j];
			break;
		}
	}
	delete[] pFontFamily;
	return correctFamily;
}

util::util()
{
	
}


util::~util()
{
	delete fontcollection;
}
