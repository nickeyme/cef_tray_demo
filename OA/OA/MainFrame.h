#pragma once
#include "stdafx.h"

#ifndef _GdiPlus_H__  
#define _GdiPlus_H__
#include <GdiPlus.h>
#include <VSStyle.h>
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;
#endif  

class SysButton {
public:
	CRect * outterRect;
	CRect * buttonRect;

	SysButton(int out_left,int out_top,int out_width,int out_height,
		int btn_left,int btn_top,int btn_width,int btn_height) {

		outterRect = new CRect(out_left, out_top, out_left + out_width, out_top + out_height);
		buttonRect = new CRect(btn_left, btn_top, btn_left + btn_width, btn_top + btn_height);
	}

	~SysButton() {
		delete outterRect;
		delete buttonRect;
	}
};

class SysCommand {
public:
	SysButton * close;
	SysButton * max;
	SysButton * min;
	CRect * menubtn; 

	SysCommand() {
		close = new SysButton(901, 0, 35, 35, 912, 10, 12, 12);
		max = new SysButton(865, 0, 35, 35, 876, 11, 14, 10);
		min = new SysButton(829, 0, 35, 35, 840, 16, 12, 0);
		menubtn = new CRect(809, 0, 809 + 10, 35);
	}

	~SysCommand() {
		delete close;
		delete max;
		delete min;
		delete menubtn;
	}
};

class WinMenu {
private:
	static CRect * setting;
	static CRect * version;
	static CRect * feedback;
	static int focsMenu;
	static VOID resetMenuStatus(Graphics & graphics);
public :
	static WCHAR winFrameClass[MAX_LOADSTRING];					 // 主窗口类名;
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	WinMenu(HINSTANCE hInstance);
	HWND createInstance(HINSTANCE hInstance, HWND hWnd);
	static VOID paintTop(Graphics & graphics, BOOL isFocus);
	static VOID paintMiddle(Graphics & graphics, BOOL isFocus);
	static VOID paintBottom(Graphics & graphics, BOOL isFocus);
	~WinMenu();
};


class MainFrame
{
private:
	ATOM initWindowClass(HINSTANCE hInstance);
	static VOID paintTitle(Graphics & graphics);
	static int focsBtn;
	static VOID resetBtnStatus(Graphics & graphics);
	static VOID paintCloseBtn(Graphics & graphics, BOOL isFocus);
	static VOID paintMaxBtn(Graphics & graphics, BOOL isFocus);
	static VOID paintMinBtn(Graphics & graphics, BOOL isFocus);
	static VOID paintMenuBtn(Graphics & graphics, BOOL isFocus);
	static HWND hWnd;
	static HWND menuHWnd;
	WinMenu * menuFrame;
	static SysCommand * menu;

public:
	static WCHAR winFrameClass[MAX_LOADSTRING];					 // 主窗口类名;
	
	ULONG_PTR m_pGdiToken;
	MainFrame(HINSTANCE hInstance);
	BOOL InitInstance(HINSTANCE hInstance);
	static HWND getHWND();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	~MainFrame();
};

