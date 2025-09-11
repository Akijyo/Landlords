#pragma once

#include <QThread>
class RobotPlayer;

class RobotPlayCards  : public QThread
{
	Q_OBJECT

public:
	RobotPlayCards(RobotPlayer* player, QObject *parent=nullptr);
	~RobotPlayCards();
protected:
	void run() override;
private:
	RobotPlayer* m_player;//机器人玩家对象
};

