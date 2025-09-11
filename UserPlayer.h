#pragma once

#include<qobject.h>
#include<Player.h>

class UserPlayer  : public Player
{
	Q_OBJECT

public:
	using Player::Player;
	UserPlayer(QObject *parent=nullptr);
	~UserPlayer();
	void prepareCallLord() override;//准备叫地主/抢地主，人类类的叫地主实现
	void preparePlayCards() override;//准备出牌，人类类的出牌实现
};

