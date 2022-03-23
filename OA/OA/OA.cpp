// OA.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "MainFrame.h"
#include "Tray.h"
#include "OA.h"
#include "include/cef_sandbox_win.h"
#include "include/cef_app.h"
#include "CEFWin.h";



// ȫ�ֱ���: 
HINSTANCE *hInst = NULL;                                // ��ǰʵ��

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
//#ifdef _DEBUG   // ������ģʽ������ʹ�ù����ڴ洫��
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

    // TODO: �ڴ˷��ô��롣

    // ��ʼ��ȫ���ַ���
    //LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    //LoadStringW(hInstance, IDC_OA, szWindowClass, MAX_LOADSTRING);

	// ����GDI+
	ULONG_PTR m_pGdiToken;
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);

	// ��ʼ��GDI+ ����
	util::addFont(util::getRunDir(L"Resources\\font\\font.ttf"));

	// ��ʼ������
	MainFrame main(hInstance);
	initCEF(hInstance, main.getHWND());
	Tray tray(hInstance, util::getRunDir(L"Resources/logo.ico"));

	// ��ʾ�����ڱ���Ҫ����CEF��ʼ��֮�󣬲�Ȼ��Ϣ�����
	ShowWindow(main.getHWND(), SW_SHOW);
	UpdateWindow(main.getHWND());

	// ��ݼ�
    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OA));

	// ����CEF��Ϣѭ������Win32��Ϣѭ��
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
	// �ͷ�GDI+
	Gdiplus::GdiplusShutdown(m_pGdiToken);

	// Shut down CEF.
	CefShutdown();

	return 0;
	//return (int)msg.wParam;
}


