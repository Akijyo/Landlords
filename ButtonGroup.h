#pragma once

#include <QWidget>
#include "ui_ButtonGroup.h"
#include<qvector.h>
#include<MyButton.h>

class ButtonGroup : public QWidget
{
	Q_OBJECT

public:
	enum PageIndex//stackedWidget中的页面索引
	{
		StartGamePage,//开始游戏页面
		CallAndGrabPage,//叫抢地主页面
		PlayAndRefusePage,//出牌和不出页面
		OnlyPlayCardsPage,//优先出牌页面
		EmptyPage//空页面
	};

	ButtonGroup(QWidget *parent = nullptr);
	~ButtonGroup();
	void initButton();//初始化stackWidget中的所有按钮，包括出牌，抢地主等。

	//bet参数用在抢地主那一页，用于判断是否显示部分按钮，若当前最高叫分为0，则显示全部按钮，若当前最高叫分为1，则不显示一分按钮，依此类推
	void setCurrentPage(PageIndex index,int bet=0);//设置当前显示的页面，用于在不同状态下切换页面显示不同的按钮

	//获取当前页面索引
	PageIndex getCurrentPage() const;

private:
	Ui::ButtonGroupClass ui;

signals:
	//这些为当按钮clicked时发出的信号
	void sigStartGame();//开始游戏按钮
	void sigPlayCards();//出牌按钮
	void sigRefuseCards();//不出牌按钮
	void sigGrabLord(int score);//抢地主按钮，分为一分，二分，三分，还有零分，零分就是不抢
};

