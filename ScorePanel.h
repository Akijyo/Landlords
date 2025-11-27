#pragma once

#include <QWidget>
#include "ui_ScorePanel.h"

class ScorePanel : public QWidget
{
	Q_OBJECT

public:
	enum FontColor
	{
		Black,
		White,
		Red,
		Blue,
		Green
	};
	ScorePanel(QWidget *parent = nullptr);
	~ScorePanel();
	void setScores(int leftScore, int userScore, int rightScore);//设置三个玩家的得分

	//设置字体大小
	void setMyFontSize(int size);

	//设置字体颜色
	void setMyFontColor(FontColor color);

private:
	Ui::ScorePanelClass ui;
	//方便操作的标签数组
	QVector<QLabel*> m_labels;
	//与字体颜色对应的颜色样式表字符串
	QMap<FontColor, QString> m_color;
};

