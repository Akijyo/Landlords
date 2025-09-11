#pragma once

#include <QThread>
class RobotPlayer;

class RobotGrabLord  : public QThread
{
	Q_OBJECT

public:
	RobotGrabLord(RobotPlayer* player,QObject *parent=nullptr);
	~RobotGrabLord();
protected:
	void run() override;//线程运行函数，调用机器人玩家的思考函数
private:
	RobotPlayer* m_player;//机器人玩家对象
};

