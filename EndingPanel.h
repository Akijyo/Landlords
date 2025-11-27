#pragma once

#include <QWidget>
#include<qpixmap.h>
#include<ScorePanel.h>
#include<qpainter.h>
#include<qpushbutton.h>

class EndingPanel  : public QWidget
{
	Q_OBJECT

public:
	EndingPanel(bool isLord,bool isWin,QWidget *parent=nullptr);
	~EndingPanel();
	//为分数面板设置分数
	void setScores(const int leftScore, const int userScore, const int rightScore);
protected:
	void paintEvent(QPaintEvent* event) override;//绘图事件
private:
	QPixmap m_background;//背景图片
	QLabel* m_title;
	ScorePanel* m_scorePanel;
	QPushButton* m_continueButton;
signals:
	void continueGame();//继续游戏信号
};

