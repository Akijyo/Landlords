#pragma once

#include <QWidget>
#include<Card.h>
#include<qpixmap.h>
#include<QPaintEvent>
#include<QMouseEvent>
#include<qpainter.h>
#include<Player.h>

class CardPanel  : public QWidget
{
	Q_OBJECT

public:
	CardPanel(QWidget *parent);
	~CardPanel();

	void setImages(const QPixmap& front, const QPixmap& back);//设置正反面图片
	QPixmap getFrontImage();//获取正面图片，由于反面图片全部单一，没有必要获取

	void setFrontSide(bool isFront);//设置显示正反面
	bool isFrontSide();//获取当前显示的是正反面

	void setSelected(bool isSelected);//设置是否被选中
	bool isSelected();//获取是否被选中

	void setCard(const Card& card);//设置卡牌数据
	Card getCard();//获取卡牌数据

	void setOwner(Player* player);//设置卡牌所属玩家
	Player* getOwner();//获取卡牌所属玩家

protected:
	void paintEvent(QPaintEvent* event) override;//绘图事件
	void mousePressEvent(QMouseEvent* event) override;//鼠标按下事件

private:
	QPixmap m_frontImage;//正面图片
	QPixmap m_backImage;//背面图片
	bool m_isFront;//是否显示正面
	bool m_isSelected;//是否被选中
	Card m_card;//卡牌数据,卡牌花色和点数
	Player* m_owner;//卡牌所属玩家
};

