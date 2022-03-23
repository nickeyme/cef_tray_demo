#pragma once
#include <shellapi.h>
#include "TrayStruct.h"
using namespace Gdiplus;


class ContextMenu {

private:
	static CRect * mainWindow;
	static CRect * exit;
	static int focsMenu;
	static VOID resetMenuStatus(Graphics & graphics);
	static VOID paintTop(Graphics & graphics, BOOL isFocus);
	static VOID paintBottom(Graphics & graphics, BOOL isFocus);

public:
	static WCHAR contextMenuClass[MAX_LOADSTRING];					 // 主窗口类名;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	HWND createInstance(HINSTANCE hInstance, HWND hWnd);
	ContextMenu(HINSTANCE hInstance);
	~ContextMenu();
};


class Tray
{
	private:
		//=================================================================================
		// 窗体基本信息
		static WCHAR szTitle[MAX_LOADSTRING];						  // 标题栏文本
		static WCHAR wClass[MAX_LOADSTRING];						  // 主窗口类名
		static WCHAR twClass[MAX_LOADSTRING];						  // 透明窗体类名

		//===============================================================================

		static HWND hWnd;											  // 当前句柄
		//static HWND bghWnd;											  // 当前句柄背景窗口
		static HINSTANCE hInstance;										// 主进程
		static HANDLE flushThread;
		static HICON hicon;										      // 托盘图标
		
		static BOOL msgTipFocus;
		static HWND contextMenuHwnd;
		ContextMenu* contextMenu;
		// ===============================================================================
		static VOID move2Tray(HWND hWnd, int hWndWidth, int hWndHeight);                             // 将窗口移动到目标托盘处
		ATOM initWindowClass(HINSTANCE hInstance);
		ATOM initTWindowClass(HINSTANCE hInstance);
		VOID setupTrayIcon(LPCWSTR icon);
		static int selectedIndex;																		// 是否选中消息
		static VOID drawWinTitle(Graphics &graphics);													// 画窗体标题
		static VOID drawWinHR(Graphics &graphics);														// 画窗体HR
		static VOID drawWinMessages(Graphics &graphics);												// 画窗体消息
		static VOID drawWinMenu(Graphics &graphics);
		static VOID drawPic2(Graphics &graphics, CString pic, int x, int y, int xDest, int yDest);
		static BOOL checkInHWndRect(HWND hWnd);
		static BOOL checkInIconRect(HWND hWnd);
		static BOOL checkInLeftMenu();
		static BOOL checkInRightMenu();
		static VOID PaintWindow(Graphics &graphics);														// 绘制窗体
		static VOID setupMsgMark(Graphics &graphics, int index, BOOL selected);								// 设置第几个消息被选中
		static int checkInMsgs();																			// 判断是否在消息内
		static BOOL checkInTipButton(int index);															// 判断是否在消息的按钮区内
		static VOID drawTipButton(Graphics &graphics, int index ,BOOL in);
		static VOID drawWinOneMessage(Graphics &graphics, int i, BOOL selected);		// 画单个消息
		//static VOID PaintWindowTitle(Graphics &graphics);
		
		// 系统线程回调  
		static DWORD WINAPI flushingTrayIcon(LPVOID icon);
		static DWORD WINAPI watchHwndClosed(LPVOID icon);
		

	public:
		static NOTIFYICONDATA m_nid;								  // 定义托盘
		static TrayWin * win;											  // 窗体数据
		static Tray* GetInstance();
		Tray(HINSTANCE hInstance,LPCWSTR icon);
		HWND getHWND();
		static LRESULT CALLBACK Tray::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static VOID appendMsg(TrayMsg msg);
		~Tray();
};
