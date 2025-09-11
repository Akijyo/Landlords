#pragma once

#include <QWidget>
#include<qpainter.h>
#include<qpixmap.h>
#include<qevent.h>
#include<qtimer.h>

class Animation  : public QWidget
{
	Q_OBJECT

public:
	Animation(QWidget *parent);
	~Animation();
	void showBetAnimation(int bet);//显示叫分动画，bet为叫分值
protected:
	void paintEvent(QPaintEvent* event) override;
private:
	QPixmap m_images;
};

