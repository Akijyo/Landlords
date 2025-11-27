#include "EndingPanel.h"

EndingPanel::EndingPanel(bool isLord, bool isWin, QWidget* parent)
	: QWidget(parent)
{
	this->m_background.load(":/Animation/images/gameover.png");
	this->setFixedSize(this->m_background.size());//固定大小为图片大小
	this->m_title = new QLabel(this);
	//分为地主/农民，胜利/失败四种情况组合共四种情况图片
	if (isLord)
	{
		if (isWin)
		{
			this->m_title->setPixmap(QPixmap(":/Animation/images/lord_win.png"));
		}
		else
		{
			this->m_title->setPixmap(QPixmap(":/Animation/images/lord_fail.png"));
		}
	}
	else
	{
		if (isWin)
		{
			this->m_title->setPixmap(QPixmap(":/Animation/images/farmer_win.png"));
		}
		else
		{
			this->m_title->setPixmap(QPixmap(":/Animation/images/farmer_fail.png"));
		}
	}
	this->m_title->move(125, 125);

	//分数面板
	this->m_scorePanel = new ScorePanel(this);
	this->m_scorePanel->move(75,230);
	this->m_scorePanel->setFixedSize(260, 160);
	this->m_scorePanel->setMyFontSize(18);
	this->m_scorePanel->setMyFontColor(ScorePanel::Red);

	//继续游戏按钮
	this->m_continueButton = new QPushButton(this);
	this->m_continueButton->move(84, 429);
	//为按钮编写样式表
	QString buttonStyle = R"(
		QPushButton{border-image: url(:/Button/images/button_normal.png);}
		QPushButton:hover{border-image: url(:/Button/images/button_hover.png);}
		QPushButton:pressed{border-image: url(:/Button/images/button_pressed.png);}
	)";
	this->m_continueButton->setStyleSheet(buttonStyle);
	this->m_continueButton->setFixedSize(231, 48);

	connect(this->m_continueButton, &QPushButton::clicked, this, &EndingPanel::continueGame);
}

EndingPanel::~EndingPanel()
{}

void EndingPanel::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.drawPixmap(this->rect(), this->m_background);
}

void EndingPanel::setScores(const int leftScore, const int userScore, const int rightScore)
{
	this->m_scorePanel->setScores(leftScore, userScore, rightScore);
}

