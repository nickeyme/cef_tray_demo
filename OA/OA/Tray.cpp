#include "stdafx.h"
#include "Tray.h"
#include "util.h"
#include <dwmapi.h>
#include <vssym32.h>
#include <conio.h>
#include <stdlib.h>
#include <mutex>
#include "MainFrame.h"
#pragma comment (lib , "dwmapi.lib" ) 




// ��ʼ����̬����
//==================================================================================

int ContextMenu::focsMenu = 0; // 0-�� 1-main 2-exit

WCHAR ContextMenu::contextMenuClass[MAX_LOADSTRING] = L"TrayContextMenu";   // �����Ҽ��˵�
CRect * ContextMenu::mainWindow;
CRect * ContextMenu::exit;
//==================================================================================
WCHAR Tray::szTitle[MAX_LOADSTRING] = L"";									// �������ı�
WCHAR Tray::wClass[MAX_LOADSTRING] = L"tipFrame";						    // ����������
WCHAR Tray::twClass[MAX_LOADSTRING] = L"tipChildFrame";						// ͸����������
HANDLE Tray::flushThread;
NOTIFYICONDATA Tray::m_nid;										// ��������
HICON Tray::hicon = NULL;
TrayWin * Tray::win = new TrayWin();												// ��Ϣ����
HWND Tray::hWnd;												// ���̴���
//HWND Tray::bghWnd;												// ��Ӱ����
HWND Tray::contextMenuHwnd;
HINSTANCE Tray::hInstance;										// �����̾��
BOOL Tray::msgTipFocus = FALSE;									// �Ƿ�ѡ����Ϣ����ʾͼ��
int Tray::selectedIndex = -1;


namespace {
	Tray * g_instance = NULL;
}  // namespace

// static
Tray* Tray::GetInstance() {
	return g_instance;
}

// �ж��Ƿ���������
BOOL Tray::checkInIconRect(HWND hWnd) {
	RECT trayIconRect = util::getTrayRect(hWnd); // ��ǰ����ͼ��λ��
	POINT pos = util::getMousePoint(); // ��ǰ���λ��

	BOOL isInTray = pos.x > trayIconRect.left &&
		pos.x < trayIconRect.right &&
		pos.y > trayIconRect.top &&
		pos.y < trayIconRect.bottom;

	if (isInTray) {
		return true;
	}
	return false;
}

//�ж��Ƿ��ڴ�����
BOOL Tray::checkInHWndRect(HWND hWnd) {
	RECT hWndRect = util::getHWndRect(hWnd); // ��ǰ����λ��
	POINT pos = util::getMousePoint(); // ��ǰ���λ��

	BOOL isInHwnd = pos.x > hWndRect.left &&
		pos.x < hWndRect.right &&
		pos.y > hWndRect.top &&
		pos.y < hWndRect.bottom;

	if (isInHwnd) {
		return true;
	}
	return false;
}

// �ж�����ڵڼ�����Ϣ��
int Tray::checkInMsgs() {
	if (IsWindowVisible(hWnd)) { // ������ʾ�����
		RECT hWndRect = util::getHWndRect(hWnd); // ��ǰ����λ��
		POINT pos = util::getMousePoint(); // ��ǰ���λ��

		int titleHeight = win->getHRBottom();
		int bottomHeight = win->tailblock.height;

		int space = win->msgblock.space;
		hWndRect.top += titleHeight;
		hWndRect.bottom -= bottomHeight;

		int spaceheight;

		// ʣ�¾�������Ϣ��
		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// ����Ϣ��
			spaceheight = pos.y - hWndRect.top;
			return floor(spaceheight / space);
		}
	}
	return -1;
}

BOOL Tray::checkInTipButton(int index) {
	if (IsWindowVisible(hWnd)) { // ������ʾ�����
		POINT pos = util::getMousePoint(); // ��ǰ���λ��
		RECT hWndRect = util::getHWndRect(hWnd); // ��ǰ����λ��

		hWndRect.left += win->msgblock.hoverTipblock.posx;
		hWndRect.top += win->getHRBottom() + win->msgblock.space*index + win->msgblock.hoverTipblock.posy;
		hWndRect.right = hWndRect.left + win->msgblock.hoverTipblock.block_width;
		hWndRect.bottom = hWndRect.top + win->msgblock.hoverTipblock.block_height;

		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// ����ʾ��ť��
			
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Tray::checkInLeftMenu() {
	if (IsWindowVisible(hWnd)) { // ������ʾ�����
		POINT pos = util::getMousePoint(); // ��ǰ���λ��
		RECT hWndRect = util::getHWndRect(hWnd); // ��ǰ����λ��
		StringFormat stringformat = StringFormat::GenericTypographic();
		FontFamily * leftmenuFamily = util::getFontFamily(win->tailblock.leftmenu.fontfamily);
		Gdiplus::Font leftmenuFont(leftmenuFamily, win->tailblock.leftmenu.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Rect leftmenuRect = win->tailblock.leftmenu.getRect(leftmenuFamily, leftmenuFont, stringformat);


		// ������Χ5����
		hWndRect.left += leftmenuRect.X - 5;
		hWndRect.top += win->getMsgBlockBottom() + leftmenuRect.Y - 5;
		hWndRect.right = hWndRect.left + leftmenuRect.Width + 5;
		hWndRect.bottom = hWndRect.top + leftmenuRect.Height + 5;

		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// ����ʾ��ť��

			return TRUE;
		}
	}
	return FALSE;
}

BOOL Tray::checkInRightMenu() {
	if (IsWindowVisible(hWnd)) { // ������ʾ�����

		POINT pos = util::getMousePoint(); // ��ǰ���λ��
		RECT hWndRect = util::getHWndRect(hWnd); // ��ǰ����λ��
		StringFormat stringformat = StringFormat::GenericTypographic();
		FontFamily * rightmenuFamily = util::getFontFamily(win->tailblock.rightmenu.fontfamily);
		Gdiplus::Font rightmenuFont(rightmenuFamily, win->tailblock.rightmenu.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Rect rightmenuRect = win->tailblock.rightmenu.getRect(rightmenuFamily, rightmenuFont, stringformat);

		// ������Χ5����
		hWndRect.left += rightmenuRect.X - 5;
		hWndRect.top += win->getMsgBlockBottom() + rightmenuRect.Y - 5;
		hWndRect.right = hWndRect.left + rightmenuRect.Width + 5;
		hWndRect.bottom = hWndRect.top + rightmenuRect.Height + 5;

		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// ����ʾ��ť��

			return TRUE;
		}
	}
	return FALSE;
}


VOID Tray::move2Tray(HWND targethWnd, int hWndWidth, int hWndHeight) {
	RECT trayIconRect = util::getTrayRect(Tray::hWnd);   // ��������
	//RECT hWndRect = util::getHWndRect(hWnd);    // ��������
	RECT trayBarRect = util::getHWndRect(util::getSysTrayBarWnd()); // ����������
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // ��Ļ���
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);  // ��Ļ�߶�

	// ���ɴ�����
	//int hWndWidth = win->winWidth;
	//int hWndHeight = win->getTailBlockBottom();

	int x = 0, y = 0;

	UINT flag = 0;
	if (trayBarRect.bottom == screenHeight && trayBarRect.top != 0) {
		flag = ABE_BOTTOM;
	}
	else if (trayBarRect.right == screenWidth && trayBarRect.left != 0) {
		flag = ABE_RIGHT;
	}
	else if (trayBarRect.bottom != screenHeight && trayBarRect.top == 0) {
		flag = ABE_TOP;
	}
	else if (trayBarRect.left == 0 && trayBarRect.right != screenWidth) {
		flag = ABE_LEFT;
	}
	
	// �ж�����������
	// switch (util::checkTaskBarDirection()) �ú����ᷢ����Ϣ�����̣������ѭ����
	switch(flag)
	{
	case ABE_TOP:
		if (trayIconRect.left + hWndWidth / 2 >= screenWidth) {
			x = screenWidth - hWndWidth;
		}
		else {
			x = trayIconRect.left - hWndWidth / 2;
		}
		y = trayBarRect.top - hWndHeight;
		y = trayBarRect.bottom;
		break;
	case ABE_LEFT:
		x = trayBarRect.right;
		if (trayIconRect.top + hWndHeight / 2 >= screenHeight) {
			y = screenHeight - hWndHeight;
		}
		else {
			y = trayIconRect.top - hWndHeight / 2;
		}
		break;
	case ABE_RIGHT:
		x = trayBarRect.left - hWndWidth;
		if (trayIconRect.top + hWndHeight / 2 >= screenHeight) {
			y = screenHeight - hWndHeight;
		}else{
			y = trayIconRect.top - hWndHeight / 2;
		}
		break;
	case  ABE_BOTTOM:
		if (trayIconRect.left + hWndWidth / 2 >= screenWidth) {
			x = screenWidth - hWndWidth;
		}
		else {
			x = trayIconRect.left - hWndWidth / 2;
		}
		y = trayBarRect.top - hWndHeight;
		break;
	}

	//::SetWindowPos(bghWnd, HWND_TOPMOST, x, y, hWndWidth, hWndHeight, SWP_SHOWWINDOW);
	SetWindowPos(targethWnd, HWND_TOPMOST, x, y, hWndWidth, hWndHeight, SWP_SHOWWINDOW);
	ShowWindow(targethWnd, SW_SHOW);
	SetForegroundWindow(targethWnd);

	// ����Բ��
	//CRgn rgn;
	//rgn.CreateRoundRectRgn(0, 0, hWndWidth, hWndHeight, 5, 5);
	//SetWindowRgn(hWnd, rgn, TRUE);
	//DeleteObject(rgn);
}


LRESULT CALLBACK Tray::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_TRAYMSG: {
		if (lParam == WM_LBUTTONDOWN) {  // ����¼�
			if (!IsWindowVisible(MainFrame::getHWND())) {
				ShowWindow(MainFrame::getHWND(), SW_SHOW);
			}
			else if (IsIconic(MainFrame::getHWND())) {  // �Ƿ���С��
				ShowWindow(MainFrame::getHWND(), SW_RESTORE);
			}
		}
		else if (lParam == WM_RBUTTONDOWN) {  // �һ��¼�
			if (!IsWindowVisible(contextMenuHwnd)) {
				ShowWindow(hWnd, SW_HIDE);
				Tray::move2Tray(contextMenuHwnd,util::getPixel(1.25),util::getPixel(1));
				//ShowWindow(contextMenuHwnd, SW_SHOW);
			}
		}
		else if (lParam == WM_MOUSEMOVE && Tray::checkInIconRect(hWnd) && win->getMSGSize() > 0) {
			if (!IsWindowVisible(hWnd) && !IsWindowVisible(contextMenuHwnd)){
				Tray::move2Tray(hWnd, win->winWidth, win->getTailBlockBottom());
				::CloseHandle(::CreateThread(NULL, 0, Tray::watchHwndClosed, NULL, 0, NULL));
			}
		}
	}
	break;
	case WM_MOUSEMOVE: {

		Graphics graphics(hWnd);
		int currentIndex = Tray::checkInMsgs();

		if (selectedIndex != currentIndex) {
			if (-1 != selectedIndex) {
				// ��ԭ֮ǰ��MSG
				setupMsgMark(graphics, selectedIndex, FALSE);
			}
			selectedIndex = currentIndex;
			if (currentIndex >= 0 && currentIndex < win->getMSGSize()) {
				// �ػ��ڼ���MSG
				setupMsgMark(graphics, selectedIndex, TRUE);
			}
		}
		else {
			BOOL focus = FALSE;
			BOOL currstatus = FALSE;
			if(-1 != currentIndex){
				currstatus  = checkInTipButton(currentIndex);
				if (currstatus != msgTipFocus) {
					drawTipButton(graphics, currentIndex, currstatus);
					msgTipFocus = currstatus;
				}
			}

			// �ж����Ҳ˵�
			focus |= currstatus || checkInLeftMenu() || checkInRightMenu();

			// ��ý��㻻����
			if (focus) {
				SetCursor(LoadCursor(NULL, IDC_HAND));
			}
			else {
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
		}

	}
	break;
	case WM_LBUTTONDOWN:{

		int currentIndex = Tray::checkInMsgs();
		if (currentIndex >= 0 && currentIndex<win->getMSGSize()) {  // ����Ϣ����
			if (checkInTipButton(currentIndex)) {
				// MessageBox(NULL, L"���������ȡ����ť", L"Win32_Mouse", MB_OK);
				// ����ո�����Ϣ����������
			}
			else {
				::SendMessage(MainFrame::getHWND(), WM_MSG, NULL, (LPARAM)(LPCTSTR)win->getMsg(currentIndex).objname);
			}
			win->removeMSG(currentIndex); // �����Ϣ
			selectedIndex = -1; // �����ȥ����ǰ����

			if (win->getMSGSize() == 0) { // ���û����Ϣ�ˣ������ش���
				ShowWindow(hWnd, SW_HIDE);
			} 
			else {  // �ػ���ǰ����
				Tray::move2Tray(hWnd, win->winWidth, win->getTailBlockBottom());
			}
		}
		else if (checkInLeftMenu()) {
			// ����������Ϣ�������ش���
			win->clearMSG();
			ShowWindow(hWnd, SW_HIDE);
		}
		else if (checkInRightMenu()) {
			// ���͵�һ����Ϣ�������������Ϣ��
			::SendMessage(MainFrame::getHWND(), WM_MSG, NULL, (LPARAM)(LPCTSTR)win->getMsg(currentIndex).objname);
			win->clearMSG();
			ShowWindow(hWnd, SW_HIDE);
		}

	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hWnd, &ps);
		Graphics graphics(hdc);
			
		Tray::PaintWindow(graphics);
		//Tray::PaintWindowTitle(graphics);
		::EndPaint(hWnd, &ps);

	}
	break;
	case WM_DESTROY:  // ͬCLOSE����
	case WM_CLOSE: {
		if (IsWindowVisible(hWnd)) {
			//MoveWindow(bghWnd, 0, 0, 0, 0, TRUE);
			//MoveWindow(hWnd, 0, 0, 0, 0, FALSE);
			ShowWindow(hWnd, SW_HIDE);
		}
	}
	break;
	case WM_NCCALCSIZE:
	{
		return 0;
	}
	default:
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND Tray::getHWND() {
	return hWnd;
}

VOID Tray::setupTrayIcon(LPCWSTR icon)
{
	m_nid.cbSize = sizeof(NOTIFYICONDATA);// �ṹ��С
	m_nid.hWnd = hWnd;  // ���� ����֪ͨ��Ϣ �Ĵ��ھ��
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE;//��ʾuCallbackMessage ��Ч 
	m_nid.uCallbackMessage = WM_TRAYMSG; // ��Ϣ�����͵��˴��ڹ���
	m_nid.hIcon = (HICON)::LoadImage(NULL, icon, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	hicon = m_nid.hIcon;
	Shell_NotifyIcon(NIM_ADD, &m_nid);
}

DWORD WINAPI Tray::flushingTrayIcon(LPVOID icon) {
	HANDLE hicon_new = NULL;
	bool flag = true;
	
	CString iconpath("");
	TrayMsg * msg;
	while (win->getMSGSize()>0) { // ����������Ϣ�Ͳ�ͣ����
		if (flag) {
			msg = &win->getLastMsg();
			if (msg->icon != iconpath) {  // ��ǰҪ��ȡ��ͼƬ��ַ�ͻ��治ͬ
				if (hicon_new) {  // ���ڻ���Ļ�����ɾ������
					DeleteObject(hicon_new);
					hicon_new = NULL;
				}
				iconpath = msg->icon;
				hicon_new = ::LoadImage(NULL, iconpath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
			}
			m_nid.hIcon = (HICON)hicon_new;
		}
		else {
			m_nid.hIcon = NULL;
		}
		Shell_NotifyIcon(NIM_MODIFY, &m_nid);
	
		flag = !flag;
		Sleep(500);
	}
	if (hicon_new) {  // ���ڻ���Ļ�����ɾ������
		DeleteObject(hicon_new);
		hicon_new = NULL;
	}

	// ��ԭΪ֮ǰ��ͼ��
	m_nid.hIcon = hicon;
	if (Shell_NotifyIcon(NIM_MODIFY, &m_nid)) {
		int s=0;
	}

	CloseHandle(flushThread);
	flushThread = NULL;
	return 0;
}

DWORD WINAPI Tray::watchHwndClosed(LPVOID icon) {
	while (Tray::checkInHWndRect(hWnd) || Tray::checkInIconRect(hWnd)) {
		Sleep(500);
	}

	if (IsWindowVisible(hWnd)) {
		//MoveWindow(bghWnd, 0, 0, 0, 0, TRUE);
		ShowWindow(hWnd, SW_HIDE);
	}
	return 0;
}


ATOM Tray::initWindowClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Tray::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OA));
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_OA);
	wcex.lpszClassName = wClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}

ATOM Tray::initTWindowClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_NOCLOSE;
	wcex.lpfnWndProc = DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OA));
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_OA);
	wcex.lpszClassName = twClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}

/*
VOID Tray::PaintWindowTitle(HDC &hdc) {
	CDC *pDC = CDC::FromHandle(hdc); // ��ȡ��ǰPDC
	CBrush Brush(RGB(83, 83, 83));
	CBrush* pOldBrush = pDC->SelectObject(&Brush);
	CRect rtWnd, rtTitle, rtButtons;
	GetWindowRect(hWnd, &rtWnd);
	//ȡ�ñ�������λ��
	//SM_CXFRAME ���ڱ߿�ı�Ե���
	//SM_CYFRAME ���ڱ߿�ı�Ե�߶�
	//SM_CXSIZE  ���ڱ��������
	//SM_CYSIZE  ���ڱ������߶�
	rtTitle.left = GetSystemMetrics(SM_CXFRAME);
	rtTitle.top = GetSystemMetrics(SM_CYFRAME);
	rtTitle.right = rtWnd.right - rtWnd.left - GetSystemMetrics(SM_CXFRAME);
	rtTitle.bottom = rtTitle.top + GetSystemMetrics(SM_CYSIZE);
	CPoint point;
	//��䶥�����
	point.x = rtWnd.Width();
	point.y = GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) + 0;
	pDC->PatBlt(0, 0, point.x, point.y, PATCOPY);
	//��������
	point.x = GetSystemMetrics(SM_CXFRAME) - 1;
	point.y = rtWnd.Height() - 1;
	pDC->PatBlt(0, 0, point.x, point.y, PATCOPY);
	//���ײ����
	point.x = rtWnd.Width();
	point.y = GetSystemMetrics(SM_CYFRAME);
	pDC->PatBlt(0, rtWnd.Height() - point.y, point.x, point.y, PATCOPY);
	//����Ҳ���
	point.x = GetSystemMetrics(SM_CXFRAME);
	point.y = rtWnd.Height();
	pDC->PatBlt(rtWnd.Width() - point.x, 0, point.x, point.y, PATCOPY);
}
*/

VOID Tray::PaintWindow(Graphics &graphics) {
	RECT hWndRect = util::getHWndRect(hWnd);    // ��������
	if (hWndRect.left == 0 && hWndRect.right == 0 && hWndRect.bottom == 0 && hWndRect.top == 0) {
		return;
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));	

	//�������� 
	Tray::drawWinTitle(graphics);
	// hr
	Tray::drawWinHR(graphics);
	// ��Ϣ
	Tray::drawWinMessages(graphics);
	// �ײ�����
	Tray::drawWinMenu(graphics);

}

VOID Tray::setupMsgMark(Graphics &graphics, int index, BOOL selected) {
	int posHeight = win->getHRBottom();
	int space = win->msgblock.space;

	// ˢ�µ�ǰ����
	Region rgn(RectF(0, posHeight + index*space, win->winWidth, space));
	if (selected) { //ѡ��
		graphics.FillRegion(&SolidBrush(Color(255, 
			win->msgblock.hoverPaint.red, 
			win->msgblock.hoverPaint.green,
			win->msgblock.hoverPaint.blue)), &rgn);
	}
	else {// ûѡ��
		graphics.FillRegion(&SolidBrush(Color(255, 
			win->msgblock.blurPaint.red,
			win->msgblock.blurPaint.green,
			win->msgblock.blurPaint.blue)), &rgn);
	}

	Tray::drawWinOneMessage(graphics, index, selected);
}

VOID Tray::drawWinTitle(Graphics &graphics) {
	FontFamily * fontFamily = util::getFontFamily(win->titleblock.title.fontfamily);
	Gdiplus::Font font(fontFamily, win->titleblock.title.fontPixel, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//�������ƽ������  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	Rect rect = win->titleblock.title.getRect(fontFamily, font, sf);
	
	graphics.DrawString(win->titleblock.title.str , -1, &font,
		PointF(rect.X, rect.Y),
		&sf, &SolidBrush(Color(255, 
			win->titleblock.title.color.red, 
			win->titleblock.title.color.green, 
			win->titleblock.title.color.blue)));
}

VOID Tray::drawWinHR(Graphics &graphics) {
	graphics.DrawLine(&Pen(Color(255, win->hrblock.color.red, win->hrblock.color.green, win->hrblock.color.blue)),
		PointF(win->hrblock.padding_left, win->getTitleBottom()), PointF(win->winWidth - win->hrblock.padding_right, win->getTitleBottom()));
}

VOID Tray::drawWinOneMessage(Graphics &graphics ,int i,BOOL selected) {

	StringFormat stringformat = StringFormat::GenericTypographic();

	TrayMsg msg = win->getMsg(i);
	// ͼƬ
	Tray::drawPic2(graphics, msg.icon,
		win->msgblock.iconblock.posx,
		win->getHRBottom()+win->msgblock.iconblock.posy + win->msgblock.space*i,
		win->msgblock.iconblock.block_width,
		win->msgblock.iconblock.block_height);


	FontFamily * fontFamily = util::getFontFamily(win->msgblock.chatObj.fontfamily);
	Gdiplus::Font font(fontFamily, win->msgblock.chatObj.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	//�������ƽ������  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	Rect rect = win->msgblock.chatObj.getRect(fontFamily, font, stringformat);
	graphics.DrawString(msg.objname, -1, &font,
		PointF(rect.X, win->getHRBottom() + win->msgblock.space*i + rect.Y ),
		&stringformat, &SolidBrush(Color(255, 
			win->msgblock.chatObj.color.red,
			win->msgblock.chatObj.color.green,
			win->msgblock.chatObj.color.blue)));
	
	if (selected) {
		
		drawTipButton(graphics, i , FALSE);

		/* ���ÿؼ��ĳɻ�ͼ
		HWND hButton1 = ::CreateWindow(L"Button", L"ss", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
			util::getPixel(3.611 - 0.28 - 0.139),
			util::getPixel(i*0.667 + (0.667 - 0.30) / 2) + posHeight,
			util::getPixel(0.139), 
			util::getPixel(0.139), hWnd, NULL, Tray::hInstance, NULL);
		*/

	}else{
		
		graphics.SetSmoothingMode(SmoothingModeDefault);
		Gdiplus::SolidBrush redBrush(Color(255, 
			win->msgblock.blurTipPaint.red,
			win->msgblock.blurTipPaint.green,
			win->msgblock.blurTipPaint.blue));
		
		graphics.FillEllipse(&redBrush,
			win->msgblock.blurTipblock.posx,
			win->getHRBottom() + win->msgblock.space*i + win->msgblock.blurTipblock.posy,
			win->msgblock.blurTipblock.block_width,
			win->msgblock.blurTipblock.block_height);

		// �Ǳ�����
		FontFamily * fontFamily2 = util::getFontFamily(win->msgblock.tipnum.fontfamily);
		Gdiplus::Font font(fontFamily2, win->msgblock.tipnum.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		//�������ƽ������  
		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
		CString str = win->msgblock.tipnum.int2CString(msg.count);
		Rect tipRect = win->msgblock.tipnum.getRect(fontFamily2, font,
			str, stringformat);

		graphics.DrawString(str, wcslen(str), &font,
			PointF(tipRect.X, win->getHRBottom() + win->msgblock.space*i + tipRect.Y),
			&stringformat, &SolidBrush(Color(255, 
				win->msgblock.tipnum.color.red,
				win->msgblock.tipnum.color.green, 
				win->msgblock.tipnum.color.blue)));
	}
	
}

VOID Tray::drawTipButton(Graphics &graphics, int index ,BOOL in) {
	Rect hoverRect = win->msgblock.hoverTipblock.getRect();
	VOID * pen;
	if (in) { // �ڰ�ť����
		// �޸ı����ͱ߿�
		pen = new Pen(Color(255,
			win->msgblock.hoverTipFocuPaint.red,
			win->msgblock.hoverTipFocuPaint.green,
			win->msgblock.hoverTipFocuPaint.blue));
		
	}
	else {
		pen = new Pen(Color(255,
			win->msgblock.hoverTipBlurPaint.red,
			win->msgblock.hoverTipBlurPaint.green,
			win->msgblock.hoverTipBlurPaint.blue));

	}


	graphics.DrawLine((Pen *)pen, PointF(hoverRect.X, win->getHRBottom() + win->msgblock.space*index + hoverRect.Y),
		PointF(hoverRect.X + hoverRect.Width, win->getHRBottom() + win->msgblock.space*index + hoverRect.Y + hoverRect.Height));

	graphics.DrawLine((Pen *)pen, PointF(hoverRect.X + hoverRect.Width, win->getHRBottom() + win->msgblock.space*index + hoverRect.Y),
		PointF(hoverRect.X, win->getHRBottom() + win->msgblock.space*index + hoverRect.Y + hoverRect.Height));

	delete (Pen *)pen;
}

VOID Tray::drawWinMessages(Graphics &graphics) {	
	for (int i = 0; i<win->getMSGSize(); i++) {
		drawWinOneMessage(graphics, i, selectedIndex == i);
	}
}

VOID Tray::drawWinMenu(Graphics &graphics) {
	StringFormat stringformat = StringFormat::GenericTypographic();

	//�������ƽ������  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);

	FontFamily * leftmenuFamily = util::getFontFamily(win->tailblock.leftmenu.fontfamily);
	Gdiplus::Font leftmenuFont(leftmenuFamily, win->tailblock.leftmenu.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Rect leftmenuRect = win->tailblock.leftmenu.getRect(leftmenuFamily, leftmenuFont, stringformat);
	graphics.DrawString(win->tailblock.leftmenu.str, -1, &leftmenuFont,
		PointF(leftmenuRect.X, win->getMsgBlockBottom()+leftmenuRect.Y),
		&stringformat,
	&SolidBrush(Color(255, win->tailblock.leftmenu.color.red, win->tailblock.leftmenu.color.green, win->tailblock.leftmenu.color.blue)));


	FontFamily * rightmenuFamily = util::getFontFamily(win->tailblock.rightmenu.fontfamily);
	Gdiplus::Font rightmenuFont(rightmenuFamily, win->tailblock.rightmenu.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	Rect rightmenuRect = win->tailblock.rightmenu.getRect(rightmenuFamily, rightmenuFont, stringformat);
	graphics.DrawString(win->tailblock.rightmenu.str, -1, &rightmenuFont,
		PointF(rightmenuRect.X, win->getMsgBlockBottom() + rightmenuRect.Y),
		&stringformat, 
	&SolidBrush(Color(255, win->tailblock.rightmenu.color.red, win->tailblock.rightmenu.color.green, win->tailblock.rightmenu.color.blue)));
}

VOID Tray::drawPic2(Graphics &graphics,CString pic, int x,int y,int xDest, int yDest) {
	Image image(pic);
	graphics.DrawImage(&image, RectF(x, y, xDest, yDest), 0, 0, image.GetWidth(), image.GetHeight(), Gdiplus::UnitPixel);
}

VOID Tray::appendMsg(TrayMsg msg) {
	win->addMsg(msg);
	FlashWindow(MainFrame::getHWND(),TRUE);
	if (!flushThread) {
		flushThread = ::CreateThread(NULL, 0, flushingTrayIcon, NULL, 0, NULL);
	}
	if (IsWindowVisible(hWnd)) {  // ����Ѿ���ʾ�˾��ػ�
		Tray::move2Tray(hWnd, win->winWidth, win->getTailBlockBottom());
	}
}




Tray::Tray(HINSTANCE hInstance, LPCWSTR icon)
{
	Tray::hInstance = hInstance;
	initWindowClass(hInstance);
	//initTWindowClass(hInstance);

	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE , wClass, szTitle, WS_POPUP | WS_CAPTION,
		CW_USEDEFAULT, 0, util::getPixel(3.611), 0, nullptr, nullptr, hInstance, nullptr);

	//bghWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE , twClass, szTitle, WS_POPUP | WS_CAPTION,
	//	CW_USEDEFAULT, 0, util::getPixel(3.611), 0, nullptr, nullptr, hInstance, nullptr);


	//SetLayeredWindowAttributes(bghWnd, RGB(255, 255, 255), 200, LWA_COLORKEY);

	if (!hWnd)
	{
		AfxMessageBox(L"�������ʧ��");
		ExitProcess(0);
	}

	MARGINS borderless = { 1,1,1,1 };
	DwmExtendFrameIntoClientArea(hWnd, &borderless);

	setupTrayIcon(icon);

	contextMenu = new ContextMenu(hInstance);
	contextMenuHwnd = contextMenu->createInstance(hInstance, hWnd);


	win->init();// ��ʼ����������

	g_instance = this;
}


Tray::~Tray()
{
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	delete contextMenu;
	delete win;
	delete g_instance;
	g_instance = NULL;
}

ContextMenu::ContextMenu(HINSTANCE hInstance){
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ContextMenu::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OA));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_OA);
	wcex.lpszClassName = contextMenuClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	RegisterClassExW(&wcex);

	mainWindow = new CRect(0, 0, util::getPixel(1.25), util::getPixel(0.5));
	exit = new CRect(0, util::getPixel(0.5), util::getPixel(1.25), util::getPixel(1));
}

HWND ContextMenu::createInstance(HINSTANCE hInstance, HWND hWnd) {
	HWND hwnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE, contextMenuClass, WIN_TITLE, WS_CAPTION,
		CW_USEDEFAULT, 0, util::getPixel(1.25), util::getPixel(1), hWnd, nullptr, hInstance, nullptr);
	//RECT hWndRect = util::getHWndRect(hwnd);
	MARGINS borderless = { 1,0,0,1 };
	DwmExtendFrameIntoClientArea(hwnd, &borderless);

	return hwnd;
}


ContextMenu::~ContextMenu(){
	delete mainWindow;
	delete exit;
}


LRESULT CALLBACK ContextMenu::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hWnd, &ps);
		Graphics graphics(hdc);
		RECT hWndRect = util::getHWndRect(hWnd);
		paintTop(graphics, FALSE);
		paintBottom(graphics, FALSE);
		::EndPaint(hWnd, &ps);
	}
	break;
	case WM_ACTIVATE: {
		if (lParam == WA_INACTIVE || lParam > 2) {  // ���ʧȥ���� ,���߸����ڻ�ȡ������
			if (IsWindowVisible(hWnd)) {
				ShowWindow(hWnd, SW_HIDE);
			}
		}
	}
	break;
	case WM_MOUSELEAVE: {
		if (focsMenu != 0) {
			resetMenuStatus(Graphics(hWnd));
			focsMenu = 0;
		}
	}
						break;
	case WM_MOUSEMOVE: {
		TRACKMOUSEEVENT tme = { 0 };  // ��������¼�
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;  // ֻ�����Ƴ����
		tme.hwndTrack = hWnd;
		tme.dwHoverTime = 50;
		TrackMouseEvent(&tme);  // �������

		BOOL focus = FALSE;
		POINT pos = util::getMousePoint();
		RECT hWndRect = util::getHWndRect(hWnd);

		if ((focus |= util::checkMouseInRect(pos, hWndRect, mainWindow))) {
			if (focsMenu != 1) {
				Graphics graphics(hWnd);
				resetMenuStatus(graphics);
				// �ڽ�������
				paintTop(graphics, TRUE);
				focsMenu = 1;
			}
		}
		else if (focus |= util::checkMouseInRect(pos, hWndRect, exit)) {
			if (focsMenu != 2) {
				Graphics graphics(hWnd);
				resetMenuStatus(graphics);
				// �ڷŴ�����
				paintBottom(graphics, TRUE);
				focsMenu = 2;
			}
		}
		else if (focsMenu != 0) {
			resetMenuStatus(Graphics(hWnd));
			focsMenu = 0;

		}

		if (focus) {
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}
	 break;
	case WM_LBUTTONDOWN:
	{
		if (focsMenu == 1) {  // ��������
			if (!IsWindowVisible(MainFrame::getHWND())) {
				ShowWindow(MainFrame::getHWND(), SW_SHOW);
			}
			else if (IsIconic(MainFrame::getHWND())) {  // �Ƿ���С��
				ShowWindow(MainFrame::getHWND(), SW_RESTORE);
			}
		}
		else if (focsMenu == 2) {  // �˳�
			Shell_NotifyIcon(NIM_DELETE, &Tray::m_nid);
			::exit(0);
		}
		ShowWindow(hWnd, SW_HIDE);
	}

	case WM_NCCALCSIZE:
	{
		return 0;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}

	return 0;
}

VOID ContextMenu::paintTop(Graphics & graphics, BOOL isFocus) {
	if (isFocus) {
		//�ػ�����
		graphics.FillRectangle(&SolidBrush(Color(255, 242, 242, 242)),Rect(mainWindow->left, mainWindow->top, mainWindow->Width(), mainWindow->Height()));
	}

	// ������
	FontFamily * fontFamily = util::getFontFamily(L"˼Դ���� CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//�������ƽ������  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	CString str(L"��������");
	SizeF size = util::getTextBounds(fontFamily, font, sf, str);
	graphics.DrawString(str, -1, &font, PointF((mainWindow->Width()-size.Width)/2, (mainWindow->Height() - size.Height) / 2), &SolidBrush(Color(255,17,17,17)));
}


VOID ContextMenu::paintBottom(Graphics & graphics, BOOL isFocus) {
	if (isFocus) {
		//�ػ�����
		graphics.FillRectangle(&SolidBrush(Color(255, 242, 242, 242)), Rect(exit->left, exit->top, exit->Width(), exit->Height()));
	}

	// ������
	FontFamily * fontFamily = util::getFontFamily(L"˼Դ���� CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//�������ƽ������  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	CString str(L"�˳�");
	SizeF size = util::getTextBounds(fontFamily, font, sf, str);
	graphics.DrawString(str, -1, &font, PointF((exit->Width() - size.Width) / 2, mainWindow->Height()+(exit->Height() - size.Height) / 2), &SolidBrush(Color(255, 255, 84, 84)));
}

VOID ContextMenu::resetMenuStatus(Graphics & graphics) {
	switch (focsMenu) {
		case 1: {
			graphics.FillRectangle(&SolidBrush(Color(255, 255, 255, 255)), Rect(mainWindow->left, mainWindow->top, mainWindow->Width(), mainWindow->Height()));
			paintTop(graphics, FALSE);
		}
		break;
		case 2: {
			graphics.FillRectangle(&SolidBrush(Color(255, 255, 255, 255)), Rect(exit->left, exit->top, exit->Width(), exit->Height()));
			paintBottom(graphics, FALSE);
		}
		break;
	}
}