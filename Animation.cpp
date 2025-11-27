#include "Animation.h"

Animation::Animation(QWidget *parent)
	: QWidget(parent)
{}

Animation::~Animation()
{}

void Animation::showBetAnimation(int bet)
{
	this->m_x = 0;
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

void Animation::showSeqSingleAnimation()
{
	this->m_x = 0;
	this->m_images.load(":/Animation/images/shunzi.png");
	this->update();
	QTimer::singleShot(2000, this, &Animation::hide);
}

void Animation::showSeqPairAnimation()
{
	this->m_x = 0;
	this->m_images.load(":/Animation/images/liandui.png");
	this->update();
	QTimer::singleShot(2000, this, &Animation::hide);
}

void Animation::showJokerAnimation()
{
	this->m_x = 0;
	this->m_index = 0;
	//使用定时器实现动画
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [=]()
		{
			this->m_index++;
			//一共八张图片，播放完后停止定时器
			if (this->m_index > 8)
			{
				timer->stop();
				timer->deleteLater();
				this->hide();
				this->m_index = 8;
			}
			QString path = QString(":/Animation/images/joker_bomb_%1.png").arg(this->m_index);
			this->m_images.load(path);
			this->update();
		});
	//每60毫秒切换一张图片
	timer->start(60);
}

void Animation::showBombAnimation()
{
	this->m_x = 0;
	this->m_index = 0;
	//使用定时器实现动画
	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, [=]()
		{
			this->m_index++;
			//一共十二张图片，播放完后停止定时器
			if (this->m_index > 12)
			{
				timer->stop();
				timer->deleteLater();
				this->hide();
				this->m_index = 12;
			}
			QString path = QString(":/Animation/images/bomb_%1.png").arg(this->m_index);
			this->m_images.load(path);
			this->update();
		});
	//每60毫秒切换一张图片
	timer->start(60);
}

void Animation::showPlaneAnimation()
{
	//初始绘制位置是窗口的最右边
	this->m_x = this->width();
	this->m_images.load(":/Animation/images/plane_1.png");
	this->setFixedHeight(this->m_images.height());
	this->update();

	int step = this->width() / 5;//每个区域的宽度
	QTimer* timer = new QTimer(this);
	this->m_times = 0;
	this->m_dist = 0;
	connect(timer, &QTimer::timeout, this, [=]()
		{
			this->m_dist += 5;//每次移动5个像素
			if (this->m_dist >= step)
			{
				//重置m_dist，进入下一个区域
				this->m_dist = 0;
				//区域数加1，用于加载图片
				this->m_times++;
				//切换图片
				QString path = QString(":/Animation/images/plane_%1.png").arg(this->m_times + 1);
				this->m_images.load(path);
			}
			//飞机图片完全飞出左边窗口边界
			if (this->m_x <= -110)
			{
				timer->stop();
				timer->deleteLater();
				this->hide();
			}
			this->m_x -= 5;
			this->update();
		});
	timer->start(16);//大约60帧每秒
}

void Animation::paintEvent(QPaintEvent * event)
{	
	QPainter painter(this);
	//因为飞机动画引入m_x坐标变量，所以这里改动了
	painter.drawPixmap(this->m_x, 0, this->m_images.width(), this->m_images.height(), this->m_images);
}

