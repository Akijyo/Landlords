#pragma once

#include<qobject.h>
#include<Player.h>
#include<Strategy.h>
#include<qrandom.h>
#include<RobotGrabLord.h>
#include<qdebug.h>
#include<RobotPlayCards.h>

class RobotPlayer  : public Player
{
	Q_OBJECT

public:
	using Player::Player;
	RobotPlayer(QObject* parent=nullptr);
	~RobotPlayer();
	void prepareCallLord() override;//准备叫地主/抢地主，机器人类的叫地主实现
	void preparePlayCards() override;//准备出牌，机器人类的出牌实现
	//计算手牌的权重，根据权重决定抢地主的分数
	void thinkLordBet();
	//指定
	void thinkPlayCards();
};

