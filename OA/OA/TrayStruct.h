#pragma once
#include "stdafx.h"
#include "util.h"

#ifndef _Vector_H__  
#define _Vector_H__
#include <vector> 
#endif  

using namespace std;


// 定义结构体


typedef struct FontColor {
	int red;
	int green;
	int blue;

	VOID init(int r,int g,int b) {
		red = r;
		green = g;
		blue = b;
	}
};

typedef struct Block {
	int posx;
	int posy;
	int block_width;
	int block_height;
	VOID init(int x,int y,int width,int height) {
		posx = x;
		posy = y;
		block_width = width;
		block_height = height;
	}

	Rect getRect() {
		return Rect(posx, posy, block_width, block_height);
	}
};

typedef struct LetterBlock {
	CString fontfamily;    // 字体
	int fontPixel;         // 字体大小
	FontColor color;
	int posx, posy;
	CString str;
	BOOL containWidthFlg = FALSE; // posx 是否包含字符串长度
	BOOL containCenter = FALSE;  // 是否需要被居中
	int containx, containy, containWidth, containHeight; //设置包含需要被居中显示的矩阵

	VOID init(int red,int green,int blue, int pixel,
		int x,int y, CString strparam = _T(""),
		CString family = L"思源黑体 CN Light") {
		color.init(red,green,blue);
		fontPixel = pixel;
		posx = x;
		posy = y;
		str = strparam;
		fontfamily = family;
	}

	VOID setIsContainWidth(BOOL flg) {
		containWidthFlg = flg;
	}

	VOID setCenterIn(int x, int y, int width, int height) {
		containCenter = TRUE;
		containx = x;
		containy = y;
		containWidth = width;
		containHeight = height;
	}
	VOID cleanCenterIn() {
		containCenter = FALSE;
	}

	CString int2CString(int i) {
		wchar_t str[10] = { 0 };
		_itow_s(i, str, 10);
		return CString(str);
	}

	Rect getRect(FontFamily * fontfamily, Font& font, CString & str ,StringFormat& strFormat = StringFormat()) {
		SizeF rect = util::getTextBounds(fontfamily, font, strFormat, str);
		if (containCenter) { // 如果需要被居中，就不考虑当前的pos和包含宽度标识了
			return Rect(containx + floor((containWidth - rect.Width) / 2) , containy + (containHeight - rect.Height) / 2 - 1,
				rect.Width, rect.Height);
		}
		return Rect(containWidthFlg ? posx - rect.Width : posx, posy, rect.Width, rect.Height);
	}

	Rect getRect(FontFamily * fontfamily, Font& font, StringFormat& strFormat = StringFormat()) {
		SizeF rect = util::getTextBounds(fontfamily, font, strFormat, str);
		if (containCenter) { // 如果需要被居中，就不考虑当前的pos和包含宽度标识了
			return Rect(containx + floor((containWidth - rect.Width)/2), containy + (containHeight - rect.Height) / 2 - 1,
				rect.Width, rect.Height);
		}
		return Rect(containWidthFlg ? posx - rect.Width:posx, posy, rect.Width, rect.Height);
	}
};

typedef struct TrayMsg {
	CString icon;    // 消息图片地址
	CString objname;
	int count = 1;       // 消息数目

	VOID init(CString iconstr, CString obj) {
		icon = iconstr;
		objname = obj;
	}
};

typedef struct TitleBlock  {
	const int padding_top = util::getPixel(0.162);      // 标题块文字padding-top 
	const int padding_left = util::getPixel(0.162);     // 标题块文字padding-left
	const int padding_bottom = util::getPixel(0.157);   // 标题块文字padding-bottom 
	LetterBlock title;									//标题块文字
	int height = util::getPixel(0.162 + 0.157) + 14;    //标题块高度

	VOID init() {
		title.init(153, 153, 153, 14, padding_left, padding_top);
	}
};

typedef struct HRBlock {
	const int padding_left = util::getPixel(0.162);     // 标题块文字padding-left
	const int padding_right = util::getPixel(0.162);   // 标题块文字padding-bottom 
	FontColor color;
	int height = 1;    //HR块高度

	VOID init() {
		color.init(236, 236, 236);
	}
};

typedef struct MsgBlock {
	
	Block iconblock,      // 图标模块
		hoverTipblock,    // 鼠标悬浮时提示模块
		blurTipblock;     // 鼠标移开时提示模块
	int space = util::getPixel(0.667);    //消息块高度
	FontColor hoverPaint, blurPaint, blurTipPaint,hoverTipFocuPaint, hoverTipBlurPaint;
	LetterBlock chatObj,tipnum;

	VOID init() {
		
		iconblock.init(util::getPixel(0.28), util::getPixel((0.667 - 0.472)/2),
			util::getPixel(0.472), util::getPixel(0.472));

		hoverTipblock.init(util::getPixel(3.611 - 0.28 - (0.25 - 0.139)/2 - 0.139), util::getPixel((0.667 - 0.139) / 2),
			util::getPixel(0.139), util::getPixel(0.139));

		blurTipblock.init(util::getPixel(3.611 - 0.28 - 0.25), util::getPixel((0.667 - 0.25) / 2),
			util::getPixel(0.25), util::getPixel(0.25));

		

		tipnum.init(255,255,255, 11, -1, -1, L"", L"Arial");
		tipnum.setCenterIn(blurTipblock.posx, blurTipblock.posy, blurTipblock.block_width, blurTipblock.block_height);

		chatObj.init(153, 153, 153, 13, util::getPixel(0.28 + 0.472 + 0.17), (util::getPixel(0.667) - 13) / 2);

		hoverPaint.init(242, 242, 242);
		hoverTipFocuPaint.init(102, 102, 102);
		hoverTipBlurPaint.init(0, 0, 0);
		blurPaint.init(255, 255, 255);
		blurTipPaint.init(255, 0, 0);
	}



};

typedef struct TailBlock  {
	const int padding_top = util::getPixel(0.121);
	const int padding_left = util::getPixel(0.162);
	const int padding_bottom = util::getPixel(0.162);
	LetterBlock leftmenu,rightmenu;									//标题块文字
	int height = util::getPixel(0.121 + 0.162) + 15;

	VOID init() {
		leftmenu.init(87, 202, 255, 15, util::getPixel(0.162), util::getPixel(0.121), L"忽略");

		rightmenu.init(87, 202, 255, 15, util::getPixel(3.611 - 0.162), util::getPixel(0.121), L"查看");
		rightmenu.setIsContainWidth(TRUE);
	}
};

typedef struct TrayWin {
	TrayMsg EMPTY_MSG;
	HANDLE g_lock;										   // 互斥锁
	int winWidth = util::getPixel(3.611);
	vector<TrayMsg> _msgs;
	int size = 0; // 动态计算
	TitleBlock titleblock; // 标题模块 
	HRBlock hrblock;
	MsgBlock msgblock;   // 消息模块
	TailBlock tailblock; // 尾部模块 


	VOID init() {
		titleblock.init();
		hrblock.init();
		msgblock.init();
		tailblock.init();

		g_lock = CreateMutex(NULL, FALSE, NULL);
	}

	int getTitleBottom() {
		return titleblock.height;
	}

	int getHRBottom() {
		return titleblock.height + hrblock.height;
	}

	int getMsgBlockBottom() {
		return getTitleBottom() + size *  msgblock.space;    //标题块高度;
	}

	int getTailBlockBottom() {
		return getMsgBlockBottom() + tailblock.height;    //标题块高度;
	}

	TrayMsg getMsg(int num) {
		if (num < size &&num >=0) {
			WaitForSingleObject(g_lock, INFINITE);
			TrayMsg msg = _msgs[num];
			ReleaseMutex(g_lock);
			return msg;
		}
		return EMPTY_MSG;
	}

	TrayMsg getLastMsg() {
		if (size>0) {
			WaitForSingleObject(g_lock, INFINITE);
			TrayMsg msg = _msgs[size - 1];
			ReleaseMutex(g_lock);
			return msg;
		}
		return EMPTY_MSG;
	}

	int getMSGSize() {
		return size;
	}
	


	VOID clearMSG() {
		WaitForSingleObject(g_lock, INFINITE);
		_msgs.clear();
		size = 0;
		ReleaseMutex(g_lock);
	}

	VOID removeMSG(int num) {
		if (num < size &&num >=0) {
			WaitForSingleObject(g_lock, INFINITE);
			_msgs.erase(_msgs.begin() + num);
			size--;
			ReleaseMutex(g_lock);
		}
	}

	VOID addMsg(TrayMsg msg) {
		WaitForSingleObject(g_lock, INFINITE);
		BOOL found = FALSE;
		// 查找相关的消息人
		//int size = _msgs.size();
		for (int i = 0; i < size; i++) {
			if (_msgs[i].objname == msg.objname) {
				found = TRUE; // 找到相同的消息人
				_msgs[i].count++;
				break;
			}
		}
		if (!found) {
			_msgs.push_back(msg);
			size+=1;
		}
		
		titleblock.title.str = getTitleStr(); // 每设一个值更新一下MSG
		ReleaseMutex(g_lock);
	}

	CString getTitleStr() {
		char buffer[100] = { 0 };
		int total = 0;
		for (int i = 0; i < size; i++)
		{
			total += _msgs[i].count;
		}
		sprintf_s(buffer, "新消息(%d)", total);
		return CString(buffer);
	}
};

