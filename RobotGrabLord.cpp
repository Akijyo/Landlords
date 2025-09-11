#include "RobotGrabLord.h"
#include "RobotPlayer.h"

RobotGrabLord::RobotGrabLord(RobotPlayer* player, QObject *parent)
	: m_player(player), QThread(parent)
{}

RobotGrabLord::~RobotGrabLord()
{}

void RobotGrabLord::run()
{
	msleep(1500);//模拟思考时间1.5秒
	this->m_player->thinkLordBet();
}

