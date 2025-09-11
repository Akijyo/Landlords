#pragma once
#include<Player.h>
#include<RobotPlayer.h>
#include<PlayHand.h>

// 游戏策略类，是机器人的出牌算法定义类，需要完成机器人对当前牌局出牌的决策
class Strategy
{
public:
	Strategy(Player* player);
	Strategy(Player* player, const Cards& cards);

	//给定指定的牌的点数，和牌的张数count，找到当前玩家的手牌中是否存在这count张这个点数，如果存在则新建一个Cards类新存储并返回，若没找到则返回空Cards类
	Cards findSameCards(Card::CardPoint point, int count);

	//根据传入的牌数量，找到手牌中所有数量等于count的牌，并返回一个Cards类型的QVector
	QVector<Cards> findCardsByCount(int count);

	//找到点数大于等于minPoint，小于maxPoint的所有牌，并返回一个Cards类型的QVector
	Cards findCardsByPointRange(Card::CardPoint minPoint= Card::CardPoint::Card_3, Card::CardPoint maxPoint=Card::CardPoint::Point_end);

	//根据牌型查找牌，找到所有符合牌型的牌组组合，也就是返回一个Cards类型的QVector，传入参数为PlayHand类和bool类型beat
		//如果beat为true，则表示要找出比当前牌型对应点数更大的牌组组合，同时类似顺子的张数也从自由的变为固定
	QVector<Cards> findCardsByType(const PlayHand& handType, bool beat = false);


	//核心函数，指定策略，与指定策略的相关子函数
	Cards makeStrategy();
	//自己作为第一位（最大）出牌玩家
	Cards firstCards();
	//找到能压住当前牌型的牌
	Cards getBeatCards(PlayHand& ph);
	//找到了能压住当前牌型的牌之后，考虑是否出牌
	bool considerToPlay(Cards& beatCards);

private:
	//该策略类服务的机器人玩家
	Player* m_player;
	//机器人玩家所拥有的手牌
	Cards m_cards;


	//应用在findCardsByType中，参数分别是起始点数和这个点数的数量
		//返回的是所有满足获取条件的组合
	QVector<Cards> getCards(Card::CardPoint beginPoint, int count);
	QVector<Cards> getCards(Card::CardPoint beginPoint, int count, Cards& cards);//为getCardsWithExtras新增的重载函数，方便传入指定的Cards类
	//应用在findCardsByType中，专门处理王炸
	QVector<Cards> getJokerBombs();
	//应用在findCardsByType中，处理三带和四带，这两处理逻辑比较类似
		//参数分别是起始点数，类型数量（三带或四带），和是否是对子
	QVector<Cards> getCardsWithExtras(Card::CardPoint beginPoint,int typeCount,bool pair);
	//应用在findCardsByType中，处理单飞机，顺子，连对，这三种处理逻辑比较类似
		//参数分别是起始点数，类型的最小长度（飞机是2，连对是3，顺子是5），牌数（飞机3，连对2，顺子1），和是否是beat
		//对于beat，如果为true，那么连续数量是固定为typeCount的，否则是以typeCount为最小值的
	QVector<Cards> getCardsWithSequences(Card::CardPoint beginPoint, int minLen,int typeCount,bool beat);
	//处理飞机带单/带对的情况
	QVector<Cards> getPlaneWithExtras(Card::CardPoint beginPoint, int minLen, bool pair,bool beat);

	//在getBeatCards，用于找到一个最合理的顺子
	Cards pickOptimalSeqSingles();
};