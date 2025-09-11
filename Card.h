#pragma once
class Card
{
public:
	//卡牌花色枚举
	enum CardSuit
	{
		Suit_begin,// 花色起始
		Hearts,   // 红桃
		Diamonds, // 方块
		Clubs,    // 梅花
		Spades,    // 黑桃
		Suit_end// 花色结束
	};
	//卡牌点数枚举
	enum CardPoint
	{
		Point_begin, // 点数起始
		Card_3,   // 3
		Card_4,   // 4
		Card_5,   // 5
		Card_6,   // 6
		Card_7,   // 7
		Card_8,   // 8
		Card_9,   // 9
		Card_10,  // 10
		Card_J,   // J
		Card_Q,   // Q
		Card_K,   // K
		Card_A,   // A
		Card_2,   // 2
		Card_Joker, // 小王
		Card_BigJoker, // 大王
		Point_end // 点数结束
	};
	Card() = default;
	Card(CardSuit suit, CardPoint point);// 构造函数
	void setSuit(CardSuit suit);// 设置花色
	void setPoint(CardPoint point);// 设置点数
	CardSuit getSuit() const; // 获取花色
	CardPoint getPoint() const; // 获取点数

	// QMap 需要可比键：提供严格弱序关系
	bool operator<(const Card& other) const;

private:
	CardSuit m_suit; // 花色
	CardPoint m_point; // 点数
};

