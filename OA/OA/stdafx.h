// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
// Windows ͷ�ļ�: 
//#include <windows.h>
#include <afxwin.h>
#include <WinUser.h>

// C ����ʱͷ�ļ�
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#ifndef _Resource_H__  
#define _Resource_H__
#include "resource.h"
#endif  



// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
#define MAX_LOADSTRING 100

#define WIN_TITLE L""										// �������ı�;

#define WM_TRAYMSG WM_USER + 305                            //�Զ���������Ϣ�����ڼ����������
#define WM_MSG WM_USER + 306                                //�Զ�����ʾ��Ϣ�����ڵ�����̵���Ϣ�����