#include "Card.h"
Card::Card(CardSuit suit, CardPoint point)
{
	this->setPoint(point);
	this->setSuit(suit);
}
void Card::setSuit(CardSuit suit)
{
	m_suit = suit;
}
void Card::setPoint(CardPoint point)
{
	m_point = point;
}
Card::CardSuit Card::getSuit() const
{
	return m_suit;
}
Card::CardPoint Card::getPoint() const
{
	return m_point;
}

bool Card::operator<(const Card& other) const
{
	// 先按点数从小到大，再按花色从小到大
	if (m_point != other.m_point)
		return m_point < other.m_point;
	return m_suit < other.m_suit;
}
