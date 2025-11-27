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
	void showSeqSingleAnimation();//显示顺子动画
	void showSeqPairAnimation();//显示连对动画
	void showJokerAnimation();//显示王炸动画
	void showBombAnimation();//显示炸弹动画
	//实现思路为：把窗口区域分为5个区域（5张飞机动画），每个区域内飞机飞行一定距离后，进入下一个区域，就切换飞机图片
	void showPlaneAnimation();//显示飞机动画
protected:
	void paintEvent(QPaintEvent* event) override;
private:
	QPixmap m_images;
	int m_index;//用于动画计数
	int m_x;//x坐标，主要是用给飞机动画坐飞行的
	int m_dist = 0;//飞机在一个区域内的飞行距离
	int m_times = 0;//飞机当前飞过了几个区域
};

