#pragma once
#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"
class CEFWin : public CefApp, 
	public CefBrowserProcessHandler,
	public CefRenderProcessHandler
{
public:
	CEFWin(HINSTANCE hinstance, HWND hWnd);
	~CEFWin();

// CefApp methods:
virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
OVERRIDE {
	return this;
}

virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE {
	return this;
};
virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;


// CefBrowserProcessHandler methods:
virtual void OnContextInitialized() OVERRIDE;

 private:
	 static HINSTANCE hinstance;
	 static HWND hWnd;
	 // Include the default reference counting implementation.
	 IMPLEMENT_REFCOUNTING(CEFWin);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
