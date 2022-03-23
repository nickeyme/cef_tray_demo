// OA.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "MainFrame.h"
#include "Tray.h"
#include "OA.h"
#include "include/cef_sandbox_win.h"
#include "include/cef_app.h"
#include "CEFWin.h";



// 全局变量: 
HINSTANCE *hInst = NULL;                                // 当前实例

int initCEF(HINSTANCE hInstance,HWND hWnd) {
	void* sandbox_info = NULL;
	#if defined(CEF_USE_SANDBOX)
		// Manage the life span of the sandbox information object. This is necessary
		// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
		CefScopedSandboxInfo scoped_sandbox;
		sandbox_info = scoped_sandbox.sandbox_info();
	#endif

	CefEnableHighDPISupport();
	// Provide CEF with command-line arguments.
	CefMainArgs main_args(hInstance);

	// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
	// that share the same executable. This function checks the command-line and,
	// if this is a sub-process, executes the appropriate logic.
	//int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
	//if (exit_code >= 0) {
		// The sub-process has completed so return here.
	//	return exit_code;
	//}


	CefRefPtr<CEFWin> app(new CEFWin(hInstance, hWnd));

	// Specify CEF global settings here.
	CefSettings settings;
	settings.no_sandbox = true;
	//settings.multi_threaded_message_loop = true;
//#ifdef _DEBUG   // 单进程模式，避免使用共享内存传递
	settings.single_process = true;
//#endif
	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), sandbox_info);

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is
	// called.
	//
	return 0;

}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    //LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadStringW(hInstance, IDC_OA, szWindowClass, MAX_LOADSTRING);

	// 开启GDI+
	ULONG_PTR m_pGdiToken;
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);

	// 初始化GDI+ 字体
	util::addFont(util::getRunDir(L"Resources\\font\\font.ttf"));

	// 初始化窗体
	MainFrame main(hInstance);
	initCEF(hInstance, main.getHWND());
	Tray tray(hInstance, util::getRunDir(L"Resources/logo.ico"));

	// 显示主窗口必须要放在CEF初始化之后，不然消息会错乱
	ShowWindow(main.getHWND(), SW_SHOW);
	UpdateWindow(main.getHWND());

	// 快捷键
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OA));

	// 调用CEF消息循环代替Win32消息循环
	CefRunMessageLoop();
	/*
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	*/
	// 释放GDI+
	Gdiplus::GdiplusShutdown(m_pGdiToken);

	// Shut down CEF.
	CefShutdown();

	return 0;
	//return (int)msg.wParam;
}


