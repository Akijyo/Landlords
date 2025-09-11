#include "Animation.h"

Animation::Animation(QWidget *parent)
	: QWidget(parent)
{}

Animation::~Animation()
{}

void Animation::showBetAnimation(int bet)
{
	switch (bet)
	{
		case 1:
		{
			this->m_images.load(":/Animation/images/score1.png");
			break;
		}
		case 2:
		{
			this->m_images.load(":/Animation/images/score2.png");
			break;
		}
		case 3:
		{
			this->m_images.load(":/Animation/images/score3.png");
			break;
		}
		default:
			break;
	}
	this->update();
}

void Animation::paintEvent(QPaintEvent * event)
{	
	QPainter painter(this);
	painter.drawPixmap(this->rect(), this->m_images);
}

