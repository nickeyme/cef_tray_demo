#include "stdafx.h"
#include "util.h"

PrivateFontCollection * util::fontcollection;
int util::nPPI = 0;

// �ж���������λ ������̷�����Ϣ
UINT util::checkTaskBarDirection() {
	APPBARDATA taskbar = { 0 };
	taskbar.cbSize = sizeof(APPBARDATA);
	SHAppBarMessage(ABM_GETTASKBARPOS, &taskbar);
	return taskbar.uEdge;
}

// ��ȡ��ǰ���ھ����Ӧ������ͼ��λ��
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

// ��ȡ��ǰ���ڿ��
RECT util::getHWndRect(HWND hWnd) {
	RECT hWndRect;
	GetWindowRect(hWnd, &hWndRect);
	return hWndRect;
}


HWND util::getSysTrayBarWnd() {
	HWND sysTrayBarWnd = FindWindow(L"shell_traywnd", NULL);
	if (!sysTrayBarWnd)
	{
		AfxMessageBox(L"��ȡ���ʧ��");
		ExitProcess(0);
	}
	return sysTrayBarWnd;
}

int util::getPixel(double inch) {
	return util::getPPI() * inch;
}

int util::getPPI() {
	if (nPPI == 0) {
		// ������Ļ��PPI
		int nXScr = 0;//��ĻX�ֱ���
		int nYScr = 0;//��ĻY�ֱ���
		int nXWidth = 0;//��Ļ�Ŀ��
		int nYHeight = 0; //��Ļ�ĸ߶�
		double dScrLeng = 0;

		nXScr = GetSystemMetrics(SM_CXSCREEN);//��ĻX�ֱ���
		nYScr = GetSystemMetrics(SM_CYSCREEN); //��ĻY�ֱ���

		HDC hdcScreen = GetDC(NULL);   //��ȡHDC���
		nXWidth = GetDeviceCaps(hdcScreen, HORZSIZE);//��Ļ�Ŀ��
		nYHeight = GetDeviceCaps(hdcScreen, VERTSIZE); //��Ļ�ĸ߶�
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


// CRect �Դ���Ϊ��׼
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

	/// ��ȡ�߽緶Χ
	graphicsPathObj.GetBounds(&rcBound);
	/// �����ı��Ŀ��
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
	// �Ȳ鱾���ֿ�
	FontFamily * font = new FontFamily(fontFamily);
	if (font->IsAvailable()) {
		return font;
	}


	// û�б����ֿ�
	int   nFound = 0;
	int   nCount = fontcollection->GetFamilyCount();
	FontFamily*  pFontFamily = new FontFamily[nCount];
	fontcollection->GetFamilies(nCount, pFontFamily, &nFound);

	if (nFound <= 0) {
		AfxMessageBox(L"װ������ʧ��");
	}

	FontFamily * correctFamily;
	WCHAR familyName[100];//����Ϊ�˼򻯳��򣬷����㹻��Ŀռ�  
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
