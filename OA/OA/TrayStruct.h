#pragma once
#include "stdafx.h"
#include "util.h"

#ifndef _Vector_H__  
#define _Vector_H__
#include <vector> 
#endif  

using namespace std;


// ����ṹ��


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
	CString fontfamily;    // ����
	int fontPixel;         // �����С
	FontColor color;
	int posx, posy;
	CString str;
	BOOL containWidthFlg = FALSE; // posx �Ƿ�����ַ�������
	BOOL containCenter = FALSE;  // �Ƿ���Ҫ������
	int containx, containy, containWidth, containHeight; //���ð�����Ҫ��������ʾ�ľ���

	VOID init(int red,int green,int blue, int pixel,
		int x,int y, CString strparam = _T(""),
		CString family = L"˼Դ���� CN Light") {
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
		if (containCenter) { // �����Ҫ�����У��Ͳ����ǵ�ǰ��pos�Ͱ�����ȱ�ʶ��
			return Rect(containx + floor((containWidth - rect.Width) / 2) , containy + (containHeight - rect.Height) / 2 - 1,
				rect.Width, rect.Height);
		}
		return Rect(containWidthFlg ? posx - rect.Width : posx, posy, rect.Width, rect.Height);
	}

	Rect getRect(FontFamily * fontfamily, Font& font, StringFormat& strFormat = StringFormat()) {
		SizeF rect = util::getTextBounds(fontfamily, font, strFormat, str);
		if (containCenter) { // �����Ҫ�����У��Ͳ����ǵ�ǰ��pos�Ͱ�����ȱ�ʶ��
			return Rect(containx + floor((containWidth - rect.Width)/2), containy + (containHeight - rect.Height) / 2 - 1,
				rect.Width, rect.Height);
		}
		return Rect(containWidthFlg ? posx - rect.Width:posx, posy, rect.Width, rect.Height);
	}
};

typedef struct TrayMsg {
	CString icon;    // ��ϢͼƬ��ַ
	CString objname;
	int count = 1;       // ��Ϣ��Ŀ

	VOID init(CString iconstr, CString obj) {
		icon = iconstr;
		objname = obj;
	}
};

typedef struct TitleBlock  {
	const int padding_top = util::getPixel(0.162);      // ���������padding-top 
	const int padding_left = util::getPixel(0.162);     // ���������padding-left
	const int padding_bottom = util::getPixel(0.157);   // ���������padding-bottom 
	LetterBlock title;									//���������
	int height = util::getPixel(0.162 + 0.157) + 14;    //�����߶�

	VOID init() {
		title.init(153, 153, 153, 14, padding_left, padding_top);
	}
};

typedef struct HRBlock {
	const int padding_left = util::getPixel(0.162);     // ���������padding-left
	const int padding_right = util::getPixel(0.162);   // ���������padding-bottom 
	FontColor color;
	int height = 1;    //HR��߶�

	VOID init() {
		color.init(236, 236, 236);
	}
};

typedef struct MsgBlock {
	
	Block iconblock,      // ͼ��ģ��
		hoverTipblock,    // �������ʱ��ʾģ��
		blurTipblock;     // ����ƿ�ʱ��ʾģ��
	int space = util::getPixel(0.667);    //��Ϣ��߶�
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
	LetterBlock leftmenu,rightmenu;									//���������
	int height = util::getPixel(0.121 + 0.162) + 15;

	VOID init() {
		leftmenu.init(87, 202, 255, 15, util::getPixel(0.162), util::getPixel(0.121), L"����");

		rightmenu.init(87, 202, 255, 15, util::getPixel(3.611 - 0.162), util::getPixel(0.121), L"�鿴");
		rightmenu.setIsContainWidth(TRUE);
	}
};

typedef struct TrayWin {
	TrayMsg EMPTY_MSG;
	HANDLE g_lock;										   // ������
	int winWidth = util::getPixel(3.611);
	vector<TrayMsg> _msgs;
	int size = 0; // ��̬����
	TitleBlock titleblock; // ����ģ�� 
	HRBlock hrblock;
	MsgBlock msgblock;   // ��Ϣģ��
	TailBlock tailblock; // β��ģ�� 


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
		return getTitleBottom() + size *  msgblock.space;    //�����߶�;
	}

	int getTailBlockBottom() {
		return getMsgBlockBottom() + tailblock.height;    //�����߶�;
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
		// ������ص���Ϣ��
		//int size = _msgs.size();
		for (int i = 0; i < size; i++) {
			if (_msgs[i].objname == msg.objname) {
				found = TRUE; // �ҵ���ͬ����Ϣ��
				_msgs[i].count++;
				break;
			}
		}
		if (!found) {
			_msgs.push_back(msg);
			size+=1;
		}
		
		titleblock.title.str = getTitleStr(); // ÿ��һ��ֵ����һ��MSG
		ReleaseMutex(g_lock);
	}

	CString getTitleStr() {
		char buffer[100] = { 0 };
		int total = 0;
		for (int i = 0; i < size; i++)
		{
			total += _msgs[i].count;
		}
		sprintf_s(buffer, "����Ϣ(%d)", total);
		return CString(buffer);
	}
};

