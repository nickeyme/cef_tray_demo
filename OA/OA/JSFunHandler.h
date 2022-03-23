#pragma once
#include "include\cef_v8.h"
#include "Tray.h"
#include "util.h"

class JSFunHandler :
	public CefV8Handler
{
public:
	JSFunHandler();
	virtual bool Execute(const CefString& name,
		CefRefPtr<CefV8Value> object,
		const CefV8ValueList& arguments,
		CefRefPtr<CefV8Value>& retval,
		CefString& exception) OVERRIDE {
		if (name == "myfun") {
			TrayMsg msg;
			msg.init(util::getRunDir(L"Resources\\logo.ico"), L"ÎÒÊÇ");
			Tray::GetInstance()->appendMsg(msg);
			char ch[25];
			itoa(Tray::GetInstance()->win->getMSGSize(), ch, 10);
			retval = CefV8Value::CreateString(ch);
			return true;
		}
		return false;
	}

	IMPLEMENT_REFCOUNTING(JSFunHandler);
};

