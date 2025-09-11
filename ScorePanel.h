#pragma once

#include <QWidget>
#include "ui_ScorePanel.h"

class ScorePanel : public QWidget
{
	Q_OBJECT

public:
	ScorePanel(QWidget *parent = nullptr);
	~ScorePanel();
	void setScores(int leftScore, int userScore, int rightScore);//设置三个玩家的得分

private:
	Ui::ScorePanelClass ui;
};

