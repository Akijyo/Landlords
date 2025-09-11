#pragma once

#include <QObject>
#include<RobotPlayer.h>
#include<UserPlayer.h>
#include<Cards.h>
#include<qtimer.h>
#include<qdebug.h>
#include<PlayHand.h>

class GameControl  : public QObject
{
	Q_OBJECT

public:
	//游戏中会处于的三种状态枚举，发牌，叫地主，出牌
	enum GameState
	{
		DealingCards,//发牌状态
		CallingLord,//叫地主状态
		PlayingCards//出牌状态，有一玩家牌出完游戏结束
	};
	//玩家会处于的三种状态，考虑叫地主，考虑出牌，获胜
	enum PlayerState
	{
		ConsideringCallLord,//考虑叫地主/抢地主
		ConsideringPlayCards,//考虑出牌
		Won//获胜
	};

	//记录抢地主过程中的数据类
	class GrabLordData
	{
	public:
		GrabLordData()
		{
			this->reset();
		}
		void reset()
		{
			this->m_highestBet = 0;
			this->m_highestBetPlayer = nullptr;
			this->m_callRound = 0;
		}
		int m_highestBet;//当前最高叫分
		Player* m_highestBetPlayer;//当前最高叫分的玩家
		int m_callRound;//叫地主轮数
	};

	GameControl(QObject *parent);
	~GameControl();

	void initPlayer();//初始化玩家，设置玩家初始状态

	void setCurrentPlayer(Player* curPlayer);//设置当前正在操作的玩家
	Player* getCurrentPlayer() const;//获取当前正在操作的玩家

	Player* getLordPlayer() const;//获取地主玩家

	RobotPlayer* getRobotLeft() const;//获取左边电脑玩家
	RobotPlayer* getRobotRight() const;//获取右边电脑玩家
	UserPlayer* getUser() const;//获取中间用户玩家

	Player* getPendingPlayer() const;//获取当前待处理牌的玩家
	Cards getPendingCards() const;//获取当前待处理的牌

	void initAllCards();//初始化游戏中所有的牌

	Card takeOneCard();//摸一张牌，内部调用Cards的takeRandomCard函数，随机取一张牌，并在牌堆中移除

	Cards getSurplusCards();//获取剩余的三张地主牌

	void resetCards();//重置游戏，清空玩家手牌，重置玩家状态，重新初始化牌堆

	void startCallLord();//开始叫地主,也就是游戏开始

	void beLord(Player* lord,int baseScore);//设置某玩家为地主

	void resetScore();//重置玩家得分

	//这是玩家抢地主的主要处理函数，里面包含了三个玩家轮流叫地主的逻辑与成为地主的逻辑
	//1.若某个玩家叫了3分，则直接成为地主，结束叫地主
	//2.若三人都为0分，则重新发牌，重新开始叫地主
	//3.若三人都不为0分，则叫分最高的玩家成为地主
	//这个函数是GameControl类的槽函数，接收Player类发出的玩家叫地主信号
	void onPlayerGrabLordBet(Player* player, int bet);//玩家叫地主/抢地主

	int getCurHighestBet() const;

	//这是玩家出牌的主要处理函数，里面包含了玩家轮流出牌的逻辑与判断游戏结束的逻辑
	//1.更新GameControl类和三个玩家的待处理牌和待处理牌玩家
	// 若有炸弹/王炸，则基础分翻倍
	// 2.若玩家手牌出完，游戏结束，判断胜负，计算得分
	// 3.若游戏未结束，切换到下一个玩家，通知主界面当前玩家状态转换为考虑出牌，等待玩家出牌
	//这个函数是GameControl类的槽函数，接收Player类发出的玩家出牌信号
	void onPlayerPlayCards(Player* player, const Cards& cards);//玩家出牌
signals:
	void sigPlayerStateChanged(Player* player, GameControl::PlayerState state);//玩家状态改变信号，这个信号由GameControl类发出，GamePannel类接收

	//通知主界面游戏状态改变
	void sigGameStateChanged(GameControl::GameState state);//游戏状态改变信号，这个信号由GameControl类发出，GamePannel类接收

	//通知主界面玩家叫地主，主要是为了显示叫分
	void sigPlayerGrabLordBet(Player* player, int bet, bool first);//玩家叫地主信号，这个信号由GameControl类发出，GamePannel类接收，first参数表示是否是第一次叫地主

	//通知主界面玩家出牌，负责完成出牌动画，更新玩家手牌显示等
	void sigPlayerPlayCards(Player* player, const Cards& cards);//玩家出牌信号，这个信号由GameControl类发出，GamePannel类接收
private:
	RobotPlayer* m_robotLeft;//左边电脑玩家
	UserPlayer* m_user;//中间用户玩家
	RobotPlayer* m_robotRight;//右边电脑玩家

	Player* m_currentPlayer;//当前正在操作的玩家

	Player* m_pendingPlayer;//当前待处理牌的玩家，即上一个出牌的玩家，如果是自己，则可以出任意牌

	Player* m_lordPlayer;//保存地主玩家
	Cards m_pendingCards;//当前待处理的牌

	Cards m_allCards;//游戏中所有的牌

	GrabLordData m_grabLordData;//叫地主数据

	int m_baseScore;//游戏基础分
};

