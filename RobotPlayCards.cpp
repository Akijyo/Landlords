#include "RobotPlayCards.h"
#include<RobotPlayer.h>

RobotPlayCards::RobotPlayCards(RobotPlayer* player, QObject *parent)
	: QThread(parent), m_player(player)
{}

RobotPlayCards::~RobotPlayCards()
{}

void RobotPlayCards::run()
{
	msleep(1500);//模拟思考时间1.5秒
	this->m_player->thinkPlayCards();
}

