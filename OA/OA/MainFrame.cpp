#include "stdafx.h"
#include "MainFrame.h"
#include "include/cef_app.h";

#include "CefHandle.h";
#include "util.h"
using namespace std;


// 静态变量初始化
SysCommand * MainFrame::menu; // 系统菜单
int MainFrame::focsBtn = 0; // 0-无 1-close 2-max 3-min 4-menu

HWND MainFrame::hWnd;
HWND MainFrame::menuHWnd;

WCHAR MainFrame::winFrameClass[MAX_LOADSTRING] = L"mainFrame";		   // 主窗口类名;
WCHAR WinMenu::winFrameClass[MAX_LOADSTRING] = L"mainMenu";		   // 主窗口类名;

CRect * WinMenu::setting;
CRect * WinMenu::version;
CRect * WinMenu::feedback;
int WinMenu::focsMenu = 0; // 0-无 1-setting 2-version 3-feedback

ATOM MainFrame::initWindowClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainFrame::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OA));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_OA);
	wcex.lpszClassName = winFrameClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK  MainFrame::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_MSG: {
		CString msg;
		msg.Append(L"switch2Msg('");
		msg.Append((LPCWSTR)lParam);
		msg.Append(L"')");
		CefHandle::GetInstance()->runJavaScript(CefString(msg));
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(hWnd, &ps);
		paintTitle(Graphics(hdc));
		::EndPaint(hWnd, &ps);
	}
	break;
	case WM_MOUSELEAVE: {
		if (focsBtn != 0) {
			resetBtnStatus(Graphics(hWnd));
			focsBtn = 0;
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


		if ((focus |= util::checkMouseInRect(pos, hWndRect, menu->close->outterRect))) {
			if(focsBtn !=1){
				Graphics graphics(hWnd);
				resetBtnStatus(graphics);
				// 在结束区内
				paintCloseBtn(graphics, TRUE);
				focsBtn = 1;
			}
		}else if (focus |= util::checkMouseInRect(pos, hWndRect, menu->max->outterRect)) {
			if (focsBtn != 2) {
				Graphics graphics(hWnd);
				resetBtnStatus(graphics);
				// 在放大区内
				paintMaxBtn(graphics, TRUE);
				focsBtn = 2;
			}
		}
		else if (focus |= util::checkMouseInRect(pos, hWndRect, menu->min->outterRect)) {
			if (focsBtn != 3) {
				Graphics graphics(hWnd);
				resetBtnStatus(graphics);
				//在缩小区内
				paintMinBtn(graphics, TRUE);
				focsBtn = 3;
			}
		}
		else if (focus |= util::checkMouseInRect(pos, hWndRect, menu->menubtn)) {
			if (focsBtn != 4) {
				Graphics graphics(hWnd);
				resetBtnStatus(graphics);
				// 在菜单区内
				paintMenuBtn(graphics, TRUE);
				focsBtn = 4;
			}
		}
		else if (focsBtn != 0) {
			resetBtnStatus(Graphics(hWnd));
			focsBtn = 0;
			
		}

		if (focus) {
			SetCursor(LoadCursor(NULL, IDC_HAND));
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}
	break;
	case WM_LBUTTONDOWN: {
		POINT pos = util::getMousePoint();
		RECT hWndRect = util::getHWndRect(hWnd);
		if (util::checkMouseInRect(pos, hWndRect, menu->close->outterRect)) {
			// 在结束区内 -- 隐藏当前窗口
			ShowWindow(hWnd, SW_HIDE);
			ShowWindow(menuHWnd, SW_HIDE);
		}
		else if (util::checkMouseInRect(pos, hWndRect, menu->max->outterRect)) {
			// 在放大区内
		}
		else if (util::checkMouseInRect(pos, hWndRect, menu->min->outterRect)) {
			//在缩小区内 -- 缩小当前窗口
			ShowWindow(menuHWnd, SW_HIDE);
			ShowWindow(hWnd, SW_MINIMIZE);
		}
		else if (util::checkMouseInRect(pos, hWndRect, menu->menubtn)) {
			if (::IsWindowVisible(menuHWnd)) {
				ShowWindow(menuHWnd, SW_HIDE);
			}
			else {
				// 在菜单区内
				// 计算当前菜单区
				RECT menuHWndRect = util::getHWndRect(menuHWnd);
				int left = hWndRect.left + (menu->menubtn->left + menu->menubtn->right) / 2 - (menuHWndRect.right - menuHWndRect.left) / 2;
				int top = hWndRect.top + menu->menubtn->bottom - 10;
				SetWindowPos(menuHWnd, hWnd, left, top, menuHWndRect.right - menuHWndRect.left, menuHWndRect.bottom - menuHWndRect.top, SWP_SHOWWINDOW);
			}
		}
		else {  // 都不在就移动窗体
			if (IsWindowVisible(menuHWnd)) {
				ShowWindow(menuHWnd, SW_HIDE);
			}
			DefWindowProc(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pos.x, pos.y)); // 移动标题栏
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NCCALCSIZE:
	{
		return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

VOID MainFrame::resetBtnStatus(Graphics & graphics) {
	SolidBrush brush(Color(255, 255, 255, 255));
	switch(focsBtn){
		case 1: {
			// 重画画布
			graphics.FillRectangle(&brush, menu->close->outterRect->left, menu->close->outterRect->top,
				menu->close->outterRect->Width(), menu->close->outterRect->Height());
			paintCloseBtn(graphics, FALSE);
		}
		break;
		case 2: {
			// 重画画布
			graphics.FillRectangle(&brush, menu->max->outterRect->left, menu->max->outterRect->top,
				menu->max->outterRect->Width(), menu->max->outterRect->Height());
			paintMaxBtn(graphics, FALSE);
		}
		break;
		case 3: {
			// 重画画布
			graphics.FillRectangle(&brush, menu->min->outterRect->left, menu->min->outterRect->top,
				menu->min->outterRect->Width(), menu->min->outterRect->Height());
			paintMinBtn(graphics, FALSE);
		}
		break;
		case 4: {
			// 重画画布
			graphics.FillRectangle(&brush, menu->menubtn->left, menu->menubtn->top, menu->menubtn->Width(), menu->menubtn->Height());
			paintMenuBtn(graphics, FALSE);
		}
		break;
	}
}

VOID MainFrame::paintCloseBtn(Graphics & graphics,BOOL isFocus) {
	VOID * menuPen;
	if (isFocus) {
		menuPen = new Pen(Color(255, 255, 255, 255));
		SolidBrush brush(Color(255, 255, 84, 84)); 
		graphics.FillRectangle(&brush, menu->close->outterRect->left, menu->close->outterRect->top,
			menu->close->outterRect->Width(), menu->close->outterRect->Height());
	}
	else {
		menuPen = new Pen(Color(255, 153, 153, 153));
	}
	

	graphics.DrawLine((Pen *)menuPen, Point(menu->close->buttonRect->left, menu->close->buttonRect->top),
		Point(menu->close->buttonRect->right, menu->close->buttonRect->bottom));
	graphics.DrawLine((Pen *)menuPen, Point(menu->close->buttonRect->right, menu->close->buttonRect->top),
		Point(menu->close->buttonRect->left, menu->close->buttonRect->bottom));

	delete (Pen *)menuPen;
}

VOID MainFrame::paintMaxBtn(Graphics & graphics, BOOL isFocus) {
	VOID * menuPen;
	if (isFocus) {
		menuPen = new Pen(Color(255, 153, 153, 153));
		SolidBrush brush(Color(255, 240, 246, 250));
		graphics.FillRectangle(&brush, menu->max->outterRect->left, menu->max->outterRect->top,
			menu->max->outterRect->Width(), menu->max->outterRect->Height());
	}
	else {
		menuPen = new Pen(Color(255, 0, 0, 0));
	}
	graphics.DrawRectangle((Pen *)menuPen, menu->max->buttonRect->left, menu->max->buttonRect->top,
		menu->max->buttonRect->Width(), menu->max->buttonRect->Height());
	delete (Pen *)menuPen;
}

VOID MainFrame::paintMinBtn(Graphics & graphics, BOOL isFocus) {
	VOID * menuPen;
	if (isFocus) {
		menuPen = new Pen(Color(255, 153, 153, 153));
		SolidBrush brush(Color(255, 240, 246, 250));
		graphics.FillRectangle(&brush, menu->min->outterRect->left, menu->min->outterRect->top,
			menu->min->outterRect->Width(), menu->min->outterRect->Height());
	}
	else {
		menuPen = new Pen(Color(255, 0, 0, 0));
	}
	graphics.DrawLine((Pen *)menuPen, Point(menu->min->buttonRect->left, menu->min->buttonRect->top), 
		Point(menu->min->buttonRect->right, menu->min->buttonRect->bottom));
	delete (Pen *)menuPen;
}

VOID MainFrame::paintMenuBtn(Graphics & graphics, BOOL isFocus) {
	SolidBrush menuBrush(Color(255, 87, 202, 255));
	int height = 6;
	Point ltp(menu->menubtn->left, (menu->menubtn->bottom + menu->menubtn->top) / 2 - height / 2); // 左上角的点
	Point rtp(menu->menubtn->right, (menu->menubtn->bottom + menu->menubtn->top) / 2 - height / 2); // 右上角的点
	Point mbp((menu->menubtn->right + menu->menubtn->left) / 2, (menu->menubtn->bottom + menu->menubtn->top) / 2 + height / 2); // 中间下面的点
	Point points[] = { ltp,rtp,mbp };
	graphics.FillPolygon(&menuBrush, points, sizeof(points) / sizeof(Point));
}

VOID MainFrame::paintTitle(Graphics & graphics) {
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	graphics.SetSmoothingMode(SmoothingModeDefault);
	// 左边图标
	Image logo(util::getRunDir(L"Resources\\images\\logo.png"));
	graphics.DrawImage(&logo, RectF(15, 9, 71, 17), 0, 0, logo.GetWidth(), logo.GetHeight(), Gdiplus::UnitPixel);



	// 右边功能键
	Pen menuPen(Color(255, 153, 153, 153));
	SolidBrush menuBrush(Color(255, 87, 202, 255));
	

	// 结束键
	paintCloseBtn(graphics, FALSE);

	// 放大键
	paintMaxBtn(graphics, FALSE);

	// 缩小键
	paintMinBtn(graphics, FALSE);
		
	// 菜单键 
	paintMenuBtn(graphics, FALSE);

	// 下方HR
	Pen pen(Color(255,214,218,222));
	graphics.DrawLine(&pen, Point(0,36), Point(935,36));
}

HWND MainFrame::getHWND() {
	return hWnd;
}

MainFrame::MainFrame(HINSTANCE hInstance)
{
	initWindowClass(hInstance);
	hWnd = CreateWindowEx(WS_EX_WINDOWEDGE, winFrameClass,WIN_TITLE, WS_CAPTION,
		CW_USEDEFAULT, 0, 935, 642, nullptr, nullptr, hInstance, nullptr);

	MARGINS borderless = { 1,0,0,1 }; 
	DwmExtendFrameIntoClientArea(hWnd, &borderless);

	menuFrame = new WinMenu(hInstance);
	menuHWnd = menuFrame->createInstance(hInstance, hWnd);

	menu = new SysCommand();
}

MainFrame::~MainFrame()
{
	delete menu;
	delete menuFrame;
}

WinMenu::WinMenu(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WinMenu::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OA));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCEW(IDC_OA);
	wcex.lpszClassName = winFrameClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);
}

HWND WinMenu::createInstance(HINSTANCE hInstance,HWND hWnd) {
	HWND hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_WINDOWEDGE, winFrameClass, WIN_TITLE, WS_POPUP,
		CW_USEDEFAULT, 0, 84, 28 * 3, hWnd, nullptr, hInstance, nullptr);

	setting = new CRect(0, 0, 84, 28);
	version = new CRect(0, 28, 84, 28 * 2);
	feedback = new CRect(0, 28 * 2, 84, 28 * 3);


	// 增加透明
	SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 700, 0x1);  // 只对黑色透明 不对窗体透明 所以700无用

	//MARGINS borderless = { 1,0,0,1 };
	//DwmExtendFrameIntoClientArea(hwnd, &borderless);
	
	return hwnd;
}

WinMenu::~WinMenu(){
	delete setting;
	delete version;
	delete feedback;
}

VOID WinMenu::paintTop(Graphics & graphics, BOOL isFocus) {
	SolidBrush * color;
	Image * bg;
	Image * icon;
	if (isFocus) {  // 如果选择变色
					//SolidBrush brush(Color(255, 240, 246, 250));
					//graphics.FillRectangle(&brush, version->left, version->top, version->Width(), version->Height());
		bg = new Image(util::getRunDir(L"Resources\\images\\1.png"));
		icon = new Image(util::getRunDir(L"Resources\\images\\设置.png"));
		color = new SolidBrush(Color(255, 87, 202, 255));
	}
	else {
		bg = new Image(util::getRunDir(L"Resources\\images\\01.png"));
		icon = new Image(util::getRunDir(L"Resources\\images\\设置0.png"));
		color = new SolidBrush(Color(255, 188, 200, 209));
	}
	// 画背景图
	graphics.DrawImage(bg, RectF(setting->left, setting->top, setting->Width(), setting->Height()), 0, 0, bg->GetWidth(), bg->GetHeight(), Gdiplus::UnitPixel);

	
	// 画图片
	graphics.DrawImage(icon, RectF(11, setting->top + (setting->Height() - icon->GetHeight() )/2, icon->GetWidth(), icon->GetHeight()), 0, 0, icon->GetWidth(), icon->GetHeight(), Gdiplus::UnitPixel);

	// 画字体
	FontFamily * fontFamily = util::getFontFamily(L"思源黑体 CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//字体边沿平滑处理  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	graphics.DrawString(L"设置", -1, &font, PointF(38, setting->top + (setting->Height() - 14) / 2), color);
	delete color;
	delete icon;
	delete bg;
}

VOID WinMenu::paintMiddle(Graphics & graphics, BOOL isFocus) {
	SolidBrush * color;
	Image * bg;
	Image * icon;
	if (isFocus) {  // 如果选择变色
		//SolidBrush brush(Color(255, 240, 246, 250));
		//graphics.FillRectangle(&brush, version->left, version->top, version->Width(), version->Height());
		bg = new Image(util::getRunDir(L"Resources\\images\\2.png"));
		icon = new Image(util::getRunDir(L"Resources\\images\\版本.png"));
		color = new SolidBrush(Color(255, 87, 202, 255));
	}
	else {
		bg = new Image(util::getRunDir(L"Resources\\images\\02.png"));
		icon = new Image(util::getRunDir(L"Resources\\images\\版本0.png"));
		color = new SolidBrush(Color(255, 188, 200, 209));
	}
	// 画背景图
	graphics.DrawImage(bg, RectF(version->left, version->top, version->Width(), version->Height()), 0, 0, bg->GetWidth(), bg->GetHeight(), Gdiplus::UnitPixel);

	// 画图片
	graphics.DrawImage(icon, RectF(14, version->top + (version->Height() - icon->GetHeight()) / 2, icon->GetWidth(), icon->GetHeight()), 0, 0, icon->GetWidth(), icon->GetHeight(), Gdiplus::UnitPixel);

	// 画字体
	FontFamily * fontFamily = util::getFontFamily(L"思源黑体 CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//字体边沿平滑处理  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	graphics.DrawString(L"版本", -1, &font, PointF(38, version->top + (version->Height() - 14) / 2), color);
	delete color;
	delete bg;
	delete icon;
}

VOID WinMenu::paintBottom(Graphics & graphics, BOOL isFocus) {
	SolidBrush * color;
	Image * icon;
	Image * bg;
	if (isFocus) {  // 如果选择变色
					//SolidBrush brush(Color(255, 240, 246, 250));
					//graphics.FillRectangle(&brush, version->left, version->top, version->Width(), version->Height());
		bg = new Image(util::getRunDir(L"Resources\\images\\3.png"));
		icon = new Image(util::getRunDir(L"Resources\\images\\反馈.png"));
		color = new SolidBrush(Color(255, 87, 202, 255));
	}
	else {
		bg = new Image(util::getRunDir(L"Resources\\images\\03.png"));
		icon = new Image(util::getRunDir(L"Resources\\images\\反馈0.png"));
		color = new SolidBrush(Color(255, 188, 200, 209));
	}
	// 画背景图
	graphics.DrawImage(bg, RectF(feedback->left, feedback->top, feedback->Width(), feedback->Height()), 0, 0, bg->GetWidth(), bg->GetHeight(), Gdiplus::UnitPixel);


	// 画图片
	graphics.DrawImage(icon, RectF(12, feedback->top + (feedback->Height() - icon->GetHeight()) / 2, icon->GetWidth(), icon->GetHeight()), 0, 0, icon->GetWidth(), icon->GetHeight(), Gdiplus::UnitPixel);

	// 画字体
	FontFamily * fontFamily = util::getFontFamily(L"思源黑体 CN Light");
	Gdiplus::Font font(fontFamily, 12, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
	//字体边沿平滑处理  
	graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);
	StringFormat sf = StringFormat::GenericTypographic();
	graphics.DrawString(L"反馈", -1, &font, PointF(38, feedback->top + (feedback->Height() - 14) / 2), color);
	delete color;
	delete bg;
	delete icon;
}

VOID WinMenu::resetMenuStatus(Graphics & graphics) {
	switch (focsMenu) {
		case 1: {
			paintTop(graphics, FALSE);
		}
		break;
		case 2: {
			paintMiddle(graphics, FALSE);
		}
		break;
		case 3: {
			paintBottom(graphics, FALSE);
		}
		break;
	}
}

LRESULT CALLBACK WinMenu::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hWnd, &ps);
			Graphics graphics(hdc);
			RECT hWndRect = util::getHWndRect(hWnd);
			// 先透明
			graphics.FillRectangle(&SolidBrush(Color(255, 0, 0, 0)), 0, 0, hWndRect.right - hWndRect.left, hWndRect.bottom - hWndRect.top);

			paintTop(graphics, FALSE);
			paintMiddle(graphics, FALSE);
			paintBottom(graphics, FALSE);
			::EndPaint(hWnd,&ps);
		}
		break;
		case WM_ACTIVATE: {
			if (lParam == WA_INACTIVE) {  // 如果失去焦点
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

			if ((focus |= util::checkMouseInRect(pos, hWndRect, setting))) {
				if (focsMenu != 1) {
					Graphics graphics(hWnd);
					resetMenuStatus(graphics);
					// 在结束区内
					paintTop(graphics, TRUE);
					focsMenu = 1;
				}
			}
			else if (focus |= util::checkMouseInRect(pos, hWndRect, version)) {
				if (focsMenu != 2) {
					Graphics graphics(hWnd);
					resetMenuStatus(graphics);
					// 在放大区内
					paintMiddle(graphics, TRUE);
					focsMenu = 2;
				}
			}
			else if (focus |= util::checkMouseInRect(pos, hWndRect, feedback)) {
				if (focsMenu != 3) {
					Graphics graphics(hWnd);
					resetMenuStatus(graphics);
					//在缩小区内
					paintBottom(graphics, TRUE);
					focsMenu = 3;
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
		case WM_LBUTTONDOWN: {
			if (focsMenu != 1) {
				CefHandle::GetInstance()->runJavaScript(CefString(L"switch2Setting()"));
			}
			else if (focsMenu != 2) {
			}
			else if (focsMenu != 3) {
				CefHandle::GetInstance()->runJavaScript(CefString(L"switch2Feedback()"));
			}
		}
		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		
	}

	return 0;
}