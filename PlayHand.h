#pragma once
#include<Cards.h>
#include<qvector.h>
#include<algorithm>
//游戏规则--玩家出牌规则
class PlayHand
{
public:
	// 出牌组合或者方式
	enum HandType
	{
		Hand_Unknown, // 未知
		Hand_Pass, // 过


		Hand_Single, // 单
		Hand_Pair, // 对


		Hand_Triple, // 三个
		Hand_Triple_Single, // 三带一
		Hand_Triple_Pair, // 三带二


		Hand_Plane, // 飞机，555_666
		Hand_Plane_With_Singles, // 飞机带单，555_666_3_4
		Hand_Plane_With_Pairs, // 飞机带双，555_666_33_44


		Hand_Seq_Pair, // 连对，33_44_55(_66...)
		Hand_Seq_Single, // 顺子，34567(8...)


		// 四带X 的标准命名（放在炸弹之前）
		Hand_Four_Two_Singles, // 四带两单，4带两单
		Hand_Four_Two_Pairs, // 四带两对，4带两对


		Hand_Bomb, // 炸弹（纯四张）
		Hand_Bomb_Jokers // 王炸
	};


	PlayHand() = default;
	//通过构造函数，传入的卡牌组分析出 出牌类型，点数数量
	PlayHand(const Cards& cards);
	//上面的函数反过来，给卡牌类型，点数，数量生成一个PlayHand对象
	PlayHand(HandType type, Card::CardPoint point, int extra);


	//得到私有成员
	HandType getType() const;
	Card::CardPoint getPoint() const;
	int getExtraCount() const;


	//比较两个玩家出的牌的大小
	bool canBeat(const PlayHand& other) const;


private:
	HandType m_type;//出牌类型
	Card::CardPoint m_point;//出牌点数
	int m_extraCount;//附加卡牌数量，比如三带一的1，飞机带单的2等


	//这样定义是方便判断牌型
	QVector<Card::CardPoint> m_singles; // 单牌点数列表
	QVector<Card::CardPoint> m_pairs; // 对牌点数列表
	QVector<Card::CardPoint> m_triples; // 三张点数列表
	QVector<Card::CardPoint> m_fourples; // 四张点数列表
	QVector<Card> m_cardList; //传入的卡牌列表，也就是本次玩家出的牌在，这个变量保存下来也是方便判断牌型


	//分析卡牌组，确定每张牌的点数
	//这函数将卡牌组分类出来，一种点数只有一张的有哪些，两张的有哪些，三张的有哪些，四张的有哪些
	void classify(const Cards& cards);


	//分析卡牌组，分析出出牌类型HandType，点数m_point，附加数量m_extraCount
		//这里分析出的目的是方便后面判断玩家牌的比较
		//点数记录规则是：在打出来的牌类型中记录那个点数最小的，那么比如连队334455，记录的点数就是3，4和5通过点数变量增加可以得到
		//额外信息是，组对的数量，比如连队334455，组对的数量是3，那么额外信息就是3
	void jugdeCardsType();



	// 判断牌的类型，对每一种类型单独写一种判断函数
	//用在jugdeCardsType函数里
	bool isPass(); // 放弃出牌
	bool isSingle(); // 单
	bool isPair(); // 对
	bool isTriple(); // 三个(相同)
	bool isTripleSingle(); // 三带一
	bool isTriplePair(); // 三带二
	bool isPlane(); // 飞机
	bool isPlaneWithSingles(); // 飞机带单（命名已调整）
	bool isPlaneWithPairs(); // 飞机带对（命名已调整）
	bool isSeqPair(); // 连对
	bool isSeqSingle(); // 顺子
	bool isFourWithTwoSingles(); // 四带两单（新命名）
	bool isFourWithTwoPairs(); // 四带两对（新命名）
	bool isBomb(); // 炸弹（纯四张）
	bool isBombJokers(); // 王炸
};

