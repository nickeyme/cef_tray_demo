#include "stdafx.h"
#include "CEFWin.h"
#include "JSFunHandler.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "CefHandle.h";
#include "util.h"

HINSTANCE CEFWin::hinstance;
HWND CEFWin::hWnd;

CEFWin::CEFWin(HINSTANCE hinstance,HWND hWnd) {
	CEFWin::hinstance = hinstance;
	CEFWin::hWnd = hWnd;
}

// 初始化
void CEFWin::OnContextInitialized() {
	CEF_REQUIRE_UI_THREAD();

	// SimpleHandler implements browser-level callbacks.
	CefRefPtr<CefHandle> handler(new CefHandle());

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	CString url = L"file:///"+util::getRunDir(L"Resources/html/index.html");

	// Information used when creating the native window.
	CefWindowInfo window_info;

	// On Windows we need to specify certain flags that will be passed to
	// CreateWindowEx().

	RECT hWndRect;
	GetWindowRect(hWnd, &hWndRect);    // 窗口区域

	RECT rect;
	rect.left = 0;
	rect.top = 37;
	rect.right = hWndRect.right;
	rect.bottom = hWndRect.bottom;
	window_info.SetAsChild(hWnd, rect);


	// Create the first browser window.
	CefBrowserHost::CreateBrowser(window_info, handler, url.GetString(), browser_settings,NULL);

}

CEFWin::~CEFWin() {

}

void CEFWin::OnContextCreated(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefV8Context> context) {

	CefRefPtr<CefV8Value> window = context->GetGlobal();
	CefRefPtr<JSFunHandler> pJsHandler(new JSFunHandler());
	CefRefPtr<CefV8Value> myFunc = CefV8Value::CreateFunction(_T("myfun"), pJsHandler);
	window->SetValue(L"myfun", myFunc, V8_PROPERTY_ATTRIBUTE_NONE);

}