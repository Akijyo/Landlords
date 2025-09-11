#include "ScorePanel.h"

ScorePanel::ScorePanel(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ScorePanel::~ScorePanel()
{}

void ScorePanel::setScores(int leftScore, int userScore, int rightScore)
{
	this->ui.leftRobotLabel->setText(QString::number(leftScore));
	this->ui.userScoreLabel->setText(QString::number(userScore));
	this->ui.rightRobotLabel->setText(QString::number(rightScore));
}

