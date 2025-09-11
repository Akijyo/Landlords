#pragma once
#include<qset.h>
#include<Card.h>
#include<qvector.h>
#include<qrandom.h>
class Cards//卡牌集合，存储多张卡牌的类,玩家手牌
{
public:
	Cards();

	enum SortType
	{
		Asc,
		Desc,
		None
	};

	void add(const Card& card);//添加卡牌
	void add(const Cards& cards);//添加多张卡牌
	Cards& operator<<(const Card& card);//重载<<运算符，添加卡牌
	Cards& operator<<(const Cards& cards);//重载<<运算符，添加多张卡牌

	void remove(const Card& card);//移除卡牌
	void remove(const Cards& cards);//移除多张卡牌
	void remove(const QVector<Cards> cards);//移除多组卡牌
	void removeByPoint(Card::CardPoint point);//移除指定点数的所有卡牌

	int size() const;//获取卡牌数量
	bool empty() const;//判断是否为空
	void clear();//清空卡牌集合

	Card::CardPoint maxPoint() const;//获取最大点数
	Card::CardPoint minPoint() const;//获取最小点数

	int pointCount(Card::CardPoint point) const;//获取指定点数的卡牌数量
	bool contains(const Card& card) const;//判断是否包含指定卡牌
	bool contains(const Cards& cards) const;//判断是否包含指定多张卡牌

	Card takeRandomCard();//随机抽取一张卡牌并在集合中移除，也就是摸牌

	QVector<Card> toOrderlyList(SortType sortType=SortType::Desc) const;
private:
	QSet<Card> m_cardSet;//存储卡牌的集合
};

//修补QSet哈希问题的两个函数
bool operator==(const Card& c1, const Card& c2);
uint qHash(const Card& card);
