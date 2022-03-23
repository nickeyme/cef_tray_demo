#pragma once
// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_client.h"
#include "include/cef_render_process_handler.h"

class CefHandle : public CefClient,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefContextMenuHandler,
	public CefLoadHandler {
public:
	CefHandle();
	~CefHandle();

	// Provide access to the single global instance of this object.
	static CefHandle* GetInstance();

	// CefClient methods:
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE {
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { 
		return this; 
	}
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() OVERRIDE {
		return this;
	}
	void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		CefRefPtr<CefMenuModel> model) OVERRIDE;
	
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		int httpStatusCode) OVERRIDE;

	// CefLoadHandler methods:
	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) OVERRIDE;

	// Request that all existing browser windows close.
	void CefHandle::runJavaScript(CefString & strCode);
private:

	CefRefPtr<CefBrowser> rootBrowser;
	std::vector<CefString> jsList;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(CefHandle);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

