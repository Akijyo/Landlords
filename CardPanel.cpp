#include "CardPanel.h"

CardPanel::CardPanel(QWidget *parent)
	: QWidget(parent)
{
	this->m_isFront = true;
	this->m_isSelected = false;
	this->m_owner = nullptr;
}

CardPanel::~CardPanel()
{}

void CardPanel::setImages(const QPixmap & front, const QPixmap & back)
{
	this->m_frontImage = front;
	this->m_backImage = back;
	this->setFixedSize(this->m_frontImage.size());//固定大小为图片大小
	update();//刷新绘制事件
}

QPixmap CardPanel::getFrontImage()
{
	return this->m_frontImage;
}

void CardPanel::setFrontSide(bool isFront)
{
	this->m_isFront = isFront;
}

bool CardPanel::isFrontSide()
{
	return this->m_isFront;
}

void CardPanel::setSelected(bool isSelected)
{
	this->m_isSelected = isSelected;
}

bool CardPanel::isSelected()
{
	return this->m_isSelected;
}

void CardPanel::setCard(const Card& card)
{
	this->m_card = card;
}

Card CardPanel::getCard()
{
	return this->m_card;
}

void CardPanel::setOwner(Player* player)
{
	this->m_owner = player;
}

Player* CardPanel::getOwner()
{
	return this->m_owner;
}

void CardPanel::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (this->m_isFront)
	{
		painter.drawPixmap(this->rect(), this->m_frontImage);
	}
	else
	{
		painter.drawPixmap(this->rect(), this->m_backImage);
	}
}

void CardPanel::mousePressEvent(QMouseEvent* event)
{
}

