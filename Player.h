#pragma once

#include <QObject>
#include<Cards.h>

class Player  : public QObject
{
	Q_OBJECT

public:
	enum Role//玩家扮演的角色类型，有地主和农民两种
	{
		Lord,//地主
		Farmer//农民
	};
	enum Gender//玩家性别，男女
	{
		Male,//男
		Female//女
	};
	enum Direction//玩家的座位位置，左和右，这里的左右指的是扑克牌组的左边或者右边
	{
		Left,//左
		Right//右
	};
	enum Type//玩家类型，有人和电脑，未知三种
	{
		Human,//人
		Computer,//电脑
		Unknown//未知
	};
	Player(QObject* parent = nullptr);
	Player(QString name, QObject* parent = nullptr);
	~Player();

	void setName(QString name);//设置玩家名称
	QString getName() const;//获取玩家名称

	void setRole(Role role);//设置玩家角色
	Role getRole() const;//获取玩家角色

	void setGender(Gender gender);//设置玩家性别
	Gender getGender() const;//获取玩家性别

	void setDirection(Direction direction);//设置玩家座位位置
	Direction getDirection() const;//获取玩家座位位置

	void setType(Type type);//设置玩家类型
	Type getType() const;//获取玩家类型

	void setScore(int score);//设置玩家得分
	int getScore() const;//获取玩家得分

	void setIsWin(bool isWin);//设置玩家是否获胜
	bool isWin() const;//获取玩家是否获胜

	void grabLordBet(int bet);//玩家叫地主/抢地主，bet为叫分或抢分

	void storeDealCard(const Card& card);//玩家收到发牌
	void storeDealCard(const Cards& cards);//玩家收到多张发牌

	void setPrevPlayer(Player* prevPlayer);//设置上家
	Player* getPrevPlayer() const;//获取上家
	void setNextPlayer(Player* nextPlayer);//设置下家
	Player* getNextPlayer() const;//获取下家

	Cards getHandCards() const;//获取玩家手牌

	void clearHandCards();//清空玩家手牌

	void playCards(const Cards& cards);//玩家出牌，返回是否出牌成功

	void setPending(const Cards& cards, Player* player);//设置当前待处理的牌和打出这幅牌的玩家
	Cards getPendingCards() const;//获取当前待处理的牌
	Player* getPendingPlayer() const;//获取打出这幅待处理牌的玩家

	//用户子类不需要去实现这两个函数，因为用户的叫地主和出牌是通过界面交互实现的
	//机器人的之类需要实现这两个函数，但是要新开线程类，不然会阻塞主窗口
	virtual void prepareCallLord();//准备叫地主/抢地主，由于机器人类和人类类实现不同，所以定义为虚函数
	virtual void preparePlayCards();//准备出牌
protected:
	QString m_name;//玩家名称
	Role m_role;//玩家角色，地主或农民
	Gender m_gender;//玩家性别，男或女
	Direction m_direction;//玩家座位方向，左或右
	Type m_type;//玩家类型，人或电脑或未知
	int m_score;//玩家得分
	bool m_isWin;//玩家是否获胜
	Cards m_handCards;//玩家手牌
	Player* m_prevPlayer;//上家
	Player* m_nextPlayer;//下家
	Cards m_pendingCards;//当前待处理的牌，用于判断是否可以出牌，比如此时厂商待处理的是3，那么玩家只能出比3大的牌
	Player* m_pengdingPlayer;//打出这幅待处理牌的玩家，如果是自己，则可以出任意牌。

signals:
	void sigGrabLordBet(Player* player, int bet);//玩家已经叫地主/抢地主信号，这个信号由Player类发出，GameControl类接收
	void sigPlayCards(Player* player, const Cards& cards);//玩家已经出牌信号，这个信号由Player类发出，GameControl类接收
};

