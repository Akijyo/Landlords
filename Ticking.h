#pragma once

#include <QWidget>
#include<qpixmap.h>
#include<qevent.h>
#include<qtimer.h>
#include<qpainter.h>

class Ticking  : public QWidget
{
	Q_OBJECT

public:
	Ticking(QWidget *parent);
	~Ticking();
	void startTicking(int count=15);
	void stopTicking();
protected:
	void paintEvent(QPaintEvent* event) override;
private:
	//时钟图片
	QPixmap m_clockPixmap;
	//计时/数字图片
	QPixmap m_numberPixmap;
	QTimer* m_timer;
	//总时间/剩余时间
	int m_count;
signals:
	//发送给主窗口GamePanel类，提醒倒计时警告音
	void sigNoMuchTime();
	//发送给主窗口GamePanel类，提醒时间到
	void sigTimeout();
};

