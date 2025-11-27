#include "ScorePanel.h"

ScorePanel::ScorePanel(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->m_labels<<this->ui.label1
		<<this->ui.userScoreLabel
		<<this->ui.label4
		<<this->ui.label2
		<<this->ui.leftRobotLabel
		<<this->ui.label5
		<<this->ui.label3
		<<this->ui.rightRobotLabel
		<<this->ui.label6;

	//初始化颜色样式表
	this->m_color[Black] = "black";
	this->m_color[White] = "white";
	this->m_color[Red] = "red";
	this->m_color[Blue] = "blue";
	this->m_color[Green] = "green";
}

ScorePanel::~ScorePanel()
{}

void ScorePanel::setScores(int leftScore, int userScore, int rightScore)
{
	this->ui.leftRobotLabel->setText(QString::number(leftScore));
	this->ui.userScoreLabel->setText(QString::number(userScore));
	this->ui.rightRobotLabel->setText(QString::number(rightScore));
}

void ScorePanel::setMyFontSize(int size)
{
	//微软雅黑加粗
	QFont font("微软雅黑", size, QFont::Bold);
	for (QLabel* label : this->m_labels)
	{
		label->setFont(font);
	}
}

void ScorePanel::setMyFontColor(FontColor color)
{
	QString style = QString("QLabel { color: %1; }").arg(this->m_color[color]);
	for (QLabel* label : this->m_labels)
	{
		label->setStyleSheet(style);
	}
}

