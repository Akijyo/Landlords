#include "Ticking.h"

Ticking::Ticking(QWidget *parent)
	: QWidget(parent)
{
	this->setFixedSize(70, 70);
	this->m_timer = new QTimer(this);
	//响应每秒计时
	connect(this->m_timer, &QTimer::timeout, this, [=]()
		{
			this->m_count--;
			//计时小于10开始显示
			if (this->m_count < 10 && this->m_count>0)
			{
				this->m_clockPixmap.load(":/Animation/images/clock.png");
				this->m_numberPixmap = QPixmap(":/Animation/images/number.png").copy(this->m_count * 40, 0, 30, 42).scaled(20, 30);
			}
			//计时等于5时，发出信号让主窗口提醒警告音
			if (this->m_count == 5)
			{
				emit this->sigNoMuchTime();
			}
			//计时等于0时，发出信号让主窗口提醒时间到
			if (this->m_count <= 0)
			{
				//停止计时
				this->m_timer->stop();
				//清空图片资源
				this->m_clockPixmap = QPixmap();
				this->m_numberPixmap = QPixmap();
				emit this->sigTimeout();
			}
			this->update();
		});
}

Ticking::~Ticking()
{}

void Ticking::startTicking(int count)
{
	this->m_count = count;
	this->m_timer->start(1000);
}

void Ticking::stopTicking()
{
	//停止计时
	this->m_timer->stop();
	//清空图片资源
	this->m_clockPixmap = QPixmap();
	this->m_numberPixmap = QPixmap();
	this->update();
}

void Ticking::paintEvent(QPaintEvent * event)
{
	QPainter p(this);
	p.drawPixmap(this->rect(), this->m_clockPixmap);
	p.drawPixmap(24, 24, this->m_numberPixmap.width(), this->m_numberPixmap.height(), this->m_numberPixmap);
}

