#pragma once

#include <QWidget>
#include<qpushbutton.h>
#include<qevent.h>
#include<QMouseEvent>
#include <QEnterEvent>
#include <QPaintEvent>
#include<qpixmap.h>
#include<qpainter.h>

class MyButton  : public QPushButton
{
	Q_OBJECT

public:
	MyButton(QWidget *parent=nullptr);
	~MyButton();
	void setImages(QString normalImage, QString pressImage, QString hoverImage);
protected:
	void enterEvent(QEnterEvent* event) override;//鼠标进入事件，切换悬停图片
	void leaveEvent(QEvent* event) override;//鼠标离开事件，切换常态图片
	void mousePressEvent(QMouseEvent* event) override;//鼠标按下事件，切换按下图片
	void mouseReleaseEvent(QMouseEvent* event) override;//鼠标释放事件，切换常态图片
	void paintEvent(QPaintEvent* event) override;//绘图事件
private:
	QString m_normalImage;//按钮常态图片路径
	QString m_pressImage;//按钮按下图片路径
	QString m_hoverImage;//按钮悬停图片路径
	QPixmap m_pixmap;//按钮图片
};

