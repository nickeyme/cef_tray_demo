#include "stdafx.h"
#include "CefHandle.h"

// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

namespace {
	CefHandle* g_instance = NULL;
}  // namespace

CefHandle::CefHandle() {
	DCHECK(!g_instance);
	g_instance = this;
}

CefHandle::~CefHandle() {
	g_instance = NULL;
	rootBrowser->Release();
}

// static
CefHandle* CefHandle::GetInstance() {
	return g_instance;
}

// 调用单独一个浏览器执行脚本
void CefHandle::runJavaScript(CefString & strCode) {
	if(!rootBrowser->IsLoading() && rootBrowser->HasDocument()){
		rootBrowser->GetMainFrame()->ExecuteJavaScript(strCode, L"", 0);
	}
	else {
		jsList.push_back(strCode);
	}
}

// 只创建一个浏览器，并赋给变量
void CefHandle::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
	CEF_REQUIRE_UI_THREAD();
	rootBrowser = browser;
}

// 刚好碰上没装载完页面，就装载完再执行
void CefHandle::OnLoadEnd(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	int httpStatusCode) {

	// 页面装载完后，执行缓存的js脚本
	for (std::vector<CefString>::iterator bit = jsList.begin(); bit != jsList.end(); ++bit)
	{
		frame->ExecuteJavaScript((*bit), L"", 0);
	}

}

// 禁用右键菜单
void CefHandle::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	CefRefPtr<CefContextMenuParams> params,
	CefRefPtr<CefMenuModel> model) {
	if ((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0) {
		// Add a separator if the menu already has items.
		if (model->GetCount() > 0)
		{
			model->Clear();
		}
	}
}


// 页面装载失败时候
void CefHandle::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl) {
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;

	// Display a load error message.
	std::stringstream ss;
	ss << "<html><body bgcolor=\"white\">"
		"<h2>Failed to load URL "
		<< std::string(failedUrl) << " with error " << std::string(errorText)
		<< " (" << errorCode << ").</h2></body></html>";
	frame->LoadString(ss.str(), failedUrl);
}




