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
	static WCHAR contextMenuClass[MAX_LOADSTRING];					 // ����������;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	HWND createInstance(HINSTANCE hInstance, HWND hWnd);
	ContextMenu(HINSTANCE hInstance);
	~ContextMenu();
};


class Tray
{
	private:
		//=================================================================================
		// ���������Ϣ
		static WCHAR szTitle[MAX_LOADSTRING];						  // �������ı�
		static WCHAR wClass[MAX_LOADSTRING];						  // ����������
		static WCHAR twClass[MAX_LOADSTRING];						  // ͸����������

		//===============================================================================

		static HWND hWnd;											  // ��ǰ���
		//static HWND bghWnd;											  // ��ǰ�����������
		static HINSTANCE hInstance;										// ������
		static HANDLE flushThread;
		static HICON hicon;										      // ����ͼ��
		
		static BOOL msgTipFocus;
		static HWND contextMenuHwnd;
		ContextMenu* contextMenu;
		// ===============================================================================
		static VOID move2Tray(HWND hWnd, int hWndWidth, int hWndHeight);                             // �������ƶ���Ŀ�����̴�
		ATOM initWindowClass(HINSTANCE hInstance);
		ATOM initTWindowClass(HINSTANCE hInstance);
		VOID setupTrayIcon(LPCWSTR icon);
		static int selectedIndex;																		// �Ƿ�ѡ����Ϣ
		static VOID drawWinTitle(Graphics &graphics);													// ���������
		static VOID drawWinHR(Graphics &graphics);														// ������HR
		static VOID drawWinMessages(Graphics &graphics);												// ��������Ϣ
		static VOID drawWinMenu(Graphics &graphics);
		static VOID drawPic2(Graphics &graphics, CString pic, int x, int y, int xDest, int yDest);
		static BOOL checkInHWndRect(HWND hWnd);
		static BOOL checkInIconRect(HWND hWnd);
		static BOOL checkInLeftMenu();
		static BOOL checkInRightMenu();
		static VOID PaintWindow(Graphics &graphics);														// ���ƴ���
		static VOID setupMsgMark(Graphics &graphics, int index, BOOL selected);								// ���õڼ�����Ϣ��ѡ��
		static int checkInMsgs();																			// �ж��Ƿ�����Ϣ��
		static BOOL checkInTipButton(int index);															// �ж��Ƿ�����Ϣ�İ�ť����
		static VOID drawTipButton(Graphics &graphics, int index ,BOOL in);
		static VOID drawWinOneMessage(Graphics &graphics, int i, BOOL selected);		// ��������Ϣ
		//static VOID PaintWindowTitle(Graphics &graphics);
		
		// ϵͳ�̻߳ص�  
		static DWORD WINAPI flushingTrayIcon(LPVOID icon);
		static DWORD WINAPI watchHwndClosed(LPVOID icon);
		

	public:
		static NOTIFYICONDATA m_nid;								  // ��������
		static TrayWin * win;											  // ��������
		static Tray* GetInstance();
		Tray(HINSTANCE hInstance,LPCWSTR icon);
		HWND getHWND();
		static LRESULT CALLBACK Tray::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static VOID appendMsg(TrayMsg msg);
		~Tray();
};
