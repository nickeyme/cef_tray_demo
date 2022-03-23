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




// 初始化静态变量
//==================================================================================

int ContextMenu::focsMenu = 0; // 0-无 1-main 2-exit

WCHAR ContextMenu::contextMenuClass[MAX_LOADSTRING] = L"TrayContextMenu";   // 托盘右键菜单
CRect * ContextMenu::mainWindow;
CRect * ContextMenu::exit;
//==================================================================================
WCHAR Tray::szTitle[MAX_LOADSTRING] = L"";									// 标题栏文本
WCHAR Tray::wClass[MAX_LOADSTRING] = L"tipFrame";						    // 主窗口类名
WCHAR Tray::twClass[MAX_LOADSTRING] = L"tipChildFrame";						// 透明窗体类名
HANDLE Tray::flushThread;
NOTIFYICONDATA Tray::m_nid;										// 定义托盘
HICON Tray::hicon = NULL;
TrayWin * Tray::win = new TrayWin();												// 消息主体
HWND Tray::hWnd;												// 托盘窗口
//HWND Tray::bghWnd;												// 阴影窗口
HWND Tray::contextMenuHwnd;
HINSTANCE Tray::hInstance;										// 主进程句柄
BOOL Tray::msgTipFocus = FALSE;									// 是否选中消息的提示图标
int Tray::selectedIndex = -1;


namespace {
	Tray * g_instance = NULL;
}  // namespace

// static
Tray* Tray::GetInstance() {
	return g_instance;
}

// 判断是否在托盘内
BOOL Tray::checkInIconRect(HWND hWnd) {
	RECT trayIconRect = util::getTrayRect(hWnd); // 当前托盘图标位置
	POINT pos = util::getMousePoint(); // 当前鼠标位置

	BOOL isInTray = pos.x > trayIconRect.left &&
		pos.x < trayIconRect.right &&
		pos.y > trayIconRect.top &&
		pos.y < trayIconRect.bottom;

	if (isInTray) {
		return true;
	}
	return false;
}

//判断是否在窗体内
BOOL Tray::checkInHWndRect(HWND hWnd) {
	RECT hWndRect = util::getHWndRect(hWnd); // 当前窗体位置
	POINT pos = util::getMousePoint(); // 当前鼠标位置

	BOOL isInHwnd = pos.x > hWndRect.left &&
		pos.x < hWndRect.right &&
		pos.y > hWndRect.top &&
		pos.y < hWndRect.bottom;

	if (isInHwnd) {
		return true;
	}
	return false;
}

// 判断鼠标在第几个消息内
int Tray::checkInMsgs() {
	if (IsWindowVisible(hWnd)) { // 窗体显示的情况
		RECT hWndRect = util::getHWndRect(hWnd); // 当前窗体位置
		POINT pos = util::getMousePoint(); // 当前鼠标位置

		int titleHeight = win->getHRBottom();
		int bottomHeight = win->tailblock.height;

		int space = win->msgblock.space;
		hWndRect.top += titleHeight;
		hWndRect.bottom -= bottomHeight;

		int spaceheight;

		// 剩下就是在消息区
		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// 在消息区
			spaceheight = pos.y - hWndRect.top;
			return floor(spaceheight / space);
		}
	}
	return -1;
}

BOOL Tray::checkInTipButton(int index) {
	if (IsWindowVisible(hWnd)) { // 窗体显示的情况
		POINT pos = util::getMousePoint(); // 当前鼠标位置
		RECT hWndRect = util::getHWndRect(hWnd); // 当前窗体位置

		hWndRect.left += win->msgblock.hoverTipblock.posx;
		hWndRect.top += win->getHRBottom() + win->msgblock.space*index + win->msgblock.hoverTipblock.posy;
		hWndRect.right = hWndRect.left + win->msgblock.hoverTipblock.block_width;
		hWndRect.bottom = hWndRect.top + win->msgblock.hoverTipblock.block_height;

		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// 在提示按钮区
			
			return TRUE;
		}
	}
	return FALSE;
}

BOOL Tray::checkInLeftMenu() {
	if (IsWindowVisible(hWnd)) { // 窗体显示的情况
		POINT pos = util::getMousePoint(); // 当前鼠标位置
		RECT hWndRect = util::getHWndRect(hWnd); // 当前窗体位置
		StringFormat stringformat = StringFormat::GenericTypographic();
		FontFamily * leftmenuFamily = util::getFontFamily(win->tailblock.leftmenu.fontfamily);
		Gdiplus::Font leftmenuFont(leftmenuFamily, win->tailblock.leftmenu.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Rect leftmenuRect = win->tailblock.leftmenu.getRect(leftmenuFamily, leftmenuFont, stringformat);


		// 缓冲周围5像素
		hWndRect.left += leftmenuRect.X - 5;
		hWndRect.top += win->getMsgBlockBottom() + leftmenuRect.Y - 5;
		hWndRect.right = hWndRect.left + leftmenuRect.Width + 5;
		hWndRect.bottom = hWndRect.top + leftmenuRect.Height + 5;

		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// 在提示按钮区

			return TRUE;
		}
	}
	return FALSE;
}

BOOL Tray::checkInRightMenu() {
	if (IsWindowVisible(hWnd)) { // 窗体显示的情况

		POINT pos = util::getMousePoint(); // 当前鼠标位置
		RECT hWndRect = util::getHWndRect(hWnd); // 当前窗体位置
		StringFormat stringformat = StringFormat::GenericTypographic();
		FontFamily * rightmenuFamily = util::getFontFamily(win->tailblock.rightmenu.fontfamily);
		Gdiplus::Font rightmenuFont(rightmenuFamily, win->tailblock.rightmenu.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		Rect rightmenuRect = win->tailblock.rightmenu.getRect(rightmenuFamily, rightmenuFont, stringformat);

		// 缓冲周围5像素
		hWndRect.left += rightmenuRect.X - 5;
		hWndRect.top += win->getMsgBlockBottom() + rightmenuRect.Y - 5;
		hWndRect.right = hWndRect.left + rightmenuRect.Width + 5;
		hWndRect.bottom = hWndRect.top + rightmenuRect.Height + 5;

		if (pos.x > hWndRect.left &&
			pos.x < hWndRect.right &&
			pos.y > hWndRect.top &&
			pos.y < hWndRect.bottom) {
			// 在提示按钮区

			return TRUE;
		}
	}
	return FALSE;
}


VOID Tray::move2Tray(HWND targethWnd, int hWndWidth, int hWndHeight) {
	RECT trayIconRect = util::getTrayRect(Tray::hWnd);   // 托盘区域
	//RECT hWndRect = util::getHWndRect(hWnd);    // 窗口区域
	RECT trayBarRect = util::getHWndRect(util::getSysTrayBarWnd()); // 任务栏区域
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // 屏幕宽度
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);  // 屏幕高度

	// 生成窗体宽高
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
	
	// 判断任务栏方向
	// switch (util::checkTaskBarDirection()) 该函数会发送信息给托盘，造成死循环。
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

	// 构造圆角
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
		if (lParam == WM_LBUTTONDOWN) {  // 左击事件
			if (!IsWindowVisible(MainFrame::getHWND())) {
				ShowWindow(MainFrame::getHWND(), SW_SHOW);
			}
			else if (IsIconic(MainFrame::getHWND())) {  // 是否最小化
				ShowWindow(MainFrame::getHWND(), SW_RESTORE);
			}
		}
		else if (lParam == WM_RBUTTONDOWN) {  // 右击事件
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
				// 还原之前的MSG
				setupMsgMark(graphics, selectedIndex, FALSE);
			}
			selectedIndex = currentIndex;
			if (currentIndex >= 0 && currentIndex < win->getMSGSize()) {
				// 重画第几个MSG
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

			// 判断左右菜单
			focus |= currstatus || checkInLeftMenu() || checkInRightMenu();

			// 获得焦点换手型
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
		if (currentIndex >= 0 && currentIndex<win->getMSGSize()) {  // 在消息区内
			if (checkInTipButton(currentIndex)) {
				// MessageBox(NULL, L"鼠标左键点击取消按钮", L"Win32_Mouse", MB_OK);
				// 就清空该条消息，不做处理
			}
			else {
				::SendMessage(MainFrame::getHWND(), WM_MSG, NULL, (LPARAM)(LPCTSTR)win->getMsg(currentIndex).objname);
			}
			win->removeMSG(currentIndex); // 清除消息
			selectedIndex = -1; // 清除后去掉当前索引

			if (win->getMSGSize() == 0) { // 如果没有消息了，就隐藏窗体
				ShowWindow(hWnd, SW_HIDE);
			} 
			else {  // 重画当前窗体
				Tray::move2Tray(hWnd, win->winWidth, win->getTailBlockBottom());
			}
		}
		else if (checkInLeftMenu()) {
			// 忽略所有消息，并隐藏窗体
			win->clearMSG();
			ShowWindow(hWnd, SW_HIDE);
		}
		else if (checkInRightMenu()) {
			// 发送第一条消息，并清空所有消息。
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
	case WM_DESTROY:  // 同CLOSE方法
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
	m_nid.cbSize = sizeof(NOTIFYICONDATA);// 结构大小
	m_nid.hWnd = hWnd;  // 接收 托盘通知消息 的窗口句柄
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE;//表示uCallbackMessage 有效 
	m_nid.uCallbackMessage = WM_TRAYMSG; // 消息被发送到此窗口过程
	m_nid.hIcon = (HICON)::LoadImage(NULL, icon, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	hicon = m_nid.hIcon;
	Shell_NotifyIcon(NIM_ADD, &m_nid);
}

DWORD WINAPI Tray::flushingTrayIcon(LPVOID icon) {
	HANDLE hicon_new = NULL;
	bool flag = true;
	
	CString iconpath("");
	TrayMsg * msg;
	while (win->getMSGSize()>0) { // 当存在有消息就不停闪动
		if (flag) {
			msg = &win->getLastMsg();
			if (msg->icon != iconpath) {  // 当前要读取的图片地址和缓存不同
				if (hicon_new) {  // 存在缓存的话，先删除缓存
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
	if (hicon_new) {  // 存在缓存的话，先删除缓存
		DeleteObject(hicon_new);
		hicon_new = NULL;
	}

	// 还原为之前的图标
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
	CDC *pDC = CDC::FromHandle(hdc); // 获取当前PDC
	CBrush Brush(RGB(83, 83, 83));
	CBrush* pOldBrush = pDC->SelectObject(&Brush);
	CRect rtWnd, rtTitle, rtButtons;
	GetWindowRect(hWnd, &rtWnd);
	//取得标题栏的位置
	//SM_CXFRAME 窗口边框的边缘宽度
	//SM_CYFRAME 窗口边框的边缘高度
	//SM_CXSIZE  窗口标题栏宽度
	//SM_CYSIZE  窗口标题栏高度
	rtTitle.left = GetSystemMetrics(SM_CXFRAME);
	rtTitle.top = GetSystemMetrics(SM_CYFRAME);
	rtTitle.right = rtWnd.right - rtWnd.left - GetSystemMetrics(SM_CXFRAME);
	rtTitle.bottom = rtTitle.top + GetSystemMetrics(SM_CYSIZE);
	CPoint point;
	//填充顶部框架
	point.x = rtWnd.Width();
	point.y = GetSystemMetrics(SM_CYSIZE) + GetSystemMetrics(SM_CYFRAME) + 0;
	pDC->PatBlt(0, 0, point.x, point.y, PATCOPY);
	//填充左侧框架
	point.x = GetSystemMetrics(SM_CXFRAME) - 1;
	point.y = rtWnd.Height() - 1;
	pDC->PatBlt(0, 0, point.x, point.y, PATCOPY);
	//填充底部框架
	point.x = rtWnd.Width();
	point.y = GetSystemMetrics(SM_CYFRAME);
	pDC->PatBlt(0, rtWnd.Height() - point.y, point.x, point.y, PATCOPY);
	//填充右侧框架
	point.x = GetSystemMetrics(SM_CXFRAME);
	point.y = rtWnd.Height();
	pDC->PatBlt(rtWnd.Width() - point.x, 0, point.x, point.y, PATCOPY);
}
*/

VOID Tray::PaintWindow(Graphics &graphics) {
	RECT hWndRect = util::getHWndRect(hWnd);    // 窗口区域
	if (hWndRect.left == 0 && hWndRect.right == 0 && hWndRect.bottom == 0 && hWndRect.top == 0) {
		return;
	}

	SetCursor(LoadCursor(NULL, IDC_ARROW));	

	//设置字体 
	Tray::drawWinTitle(graphics);
	// hr
	Tray::drawWinHR(graphics);
	// 消息
	Tray::drawWinMessages(graphics);
	// 底部字体
	Tray::drawWinMenu(graphics);

}

VOID Tray::setupMsgMark(Graphics &graphics, int index, BOOL selected) {
	int posHeight = win->getHRBottom();
	int space = win->msgblock.space;

	// 刷新当前区域
	Region rgn(RectF(0, posHeight + index*space, win->winWidth, space));
	if (selected) { //选中
		graphics.FillRegion(&SolidBrush(Color(255, 
			win->msgblock.hoverPaint.red, 
			win->msgblock.hoverPaint.green,
			win->msgblock.hoverPaint.blue)), &rgn);
	}
	else {// 没选中
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
	//字体边沿平滑处理  
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
	// 图片
	Tray::drawPic2(graphics, msg.icon,
		win->msgblock.iconblock.posx,
		win->getHRBottom()+win->msgblock.iconblock.posy + win->msgblock.space*i,
		win->msgblock.iconblock.block_width,
		win->msgblock.iconblock.block_height);


	FontFamily * fontFamily = util::getFontFamily(win->msgblock.chatObj.fontfamily);
	Gdiplus::Font font(fontFamily, win->msgblock.chatObj.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	//字体边沿平滑处理  
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

		/* 不用控件改成画图
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

		// 角标文字
		FontFamily * fontFamily2 = util::getFontFamily(win->msgblock.tipnum.fontfamily);
		Gdiplus::Font font(fontFamily2, win->msgblock.tipnum.fontPixel, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		//字体边沿平滑处理  
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
	if (in) { // 在按钮区内
		// 修改背景和边框
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

	//字体边沿平滑处理  
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
	if (IsWindowVisible(hWnd)) {  // 如果已经显示了就重画
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
		AfxMessageBox(L"开启句柄失败");
		ExitProcess(0);
	}

	MARGINS borderless = { 1,1,1,1 };
	DwmExtendFrameIntoClientArea(hWnd, &borderless);

	setupTrayIcon(icon);

	contextMenu = new ContextMenu(hInstance);
	contextMenuHwnd = contextMenu->createInstance(hInstance, hWnd);


	win->init();// 初始化窗体数据

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
		if (lParam == WA_INACTIVE || lParam > 2) {  // 如果失去焦点 ,或者父窗口获取到焦点
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
		TRACKMOUSEEVENT tme = { 0 };  // 跟踪鼠标事件
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;  // 只监听移出鼠标
		tme.hwndTrack = hWnd;
		tme.dwHoverTime = 50;
		TrackMouseEvent(&tme);  // 跟踪鼠标

		BOOL focus = FALSE;
		POINT pos = util::getMousePoint();
		RECT hWndRect = util::getHWndRect(hWnd);

		if ((focus |= util::checkMouseInRect(pos, hWndRect, mainWindow))) {
			if (focsMenu != 1) {
				Graphics graphics(hWnd);
				resetMenuStatus(graphics);
				// 在结束区内
				paintTop(graphics, TRUE);
				focsMenu = 1;
			}
		}
		else if (focus |= util::checkMouseInRect(pos, hWndRect, exit)) {
			if (focsMenu != 2) {
				Graphics graphics(hWnd);
				resetMenuStatus(graphics);
				// 在放大区内
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
		if (focsMenu == 1) {  // 打开主界面
			if (!IsWindowVisible(MainFrame::getHWND())) {
				ShowWindow(MainFrame::getHWND(), SW_SHOW);
			}
			else if (IsIconic(MainFrame::getHWND())) {  // 是否最小化
				ShowWindow(MainFrame::getHWND(), SW_RESTORE);
			}
		}
		else if (focsMenu == 2) {  // 退出
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
		//重画背景
		graphics.FillRectangle(&SolidBrush(Color(255, 242, 242, 242)),Rect(mainWindow->left, mainWindow->top, mainWindow->Width(), mainWindow->Height()));
	}

	// 画字体
	FontFamily * fontFamily = util::getFontFamily(L"思源黑体 CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//字体边沿平滑处理  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	CString str(L"打开主界面");
	SizeF size = util::getTextBounds(fontFamily, font, sf, str);
	graphics.DrawString(str, -1, &font, PointF((mainWindow->Width()-size.Width)/2, (mainWindow->Height() - size.Height) / 2), &SolidBrush(Color(255,17,17,17)));
}


VOID ContextMenu::paintBottom(Graphics & graphics, BOOL isFocus) {
	if (isFocus) {
		//重画背景
		graphics.FillRectangle(&SolidBrush(Color(255, 242, 242, 242)), Rect(exit->left, exit->top, exit->Width(), exit->Height()));
	}

	// 画字体
	FontFamily * fontFamily = util::getFontFamily(L"思源黑体 CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//字体边沿平滑处理  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	CString str(L"退出");
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