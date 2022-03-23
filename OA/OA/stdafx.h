// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
//#include <windows.h>
#include <afxwin.h>
#include <WinUser.h>

// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifndef _Resource_H__  
#define _Resource_H__
#include "resource.h"
#endif  



// TODO:  在此处引用程序需要的其他头文件
#define MAX_LOADSTRING 100

#define WIN_TITLE L""										// 标题栏文本;

#define WM_TRAYMSG WM_USER + 305                            //自定义托盘消息，用于监听托盘鼠标
#define WM_MSG WM_USER + 306                                //自定义显示消息，用于点击托盘的消息后回显