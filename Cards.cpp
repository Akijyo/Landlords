#include "Cards.h"

Cards::Cards()
{
}

void Cards::add(const Card& card)
{
	this->m_cardSet.insert(card);
}

void Cards::add(const Cards& cards)
{
	this->m_cardSet.unite(cards.m_cardSet);//求并集
}

Cards& Cards::operator<<(const Card& card)
{
	this->m_cardSet.insert(card);
	return *this;//返回当前对象的引用，以支持链式调用
}

Cards& Cards::operator<<(const Cards& cards)
{
	this->m_cardSet.unite(cards.m_cardSet);
	return *this;
}

void Cards::remove(const Card& card)
{
	this->m_cardSet.remove(card);
}

void Cards::remove(const Cards& cards)
{
	this->m_cardSet.subtract(cards.m_cardSet);//求差集
}

void Cards::remove(const QVector<Cards> cards)
{
	for(auto it : cards)
	{
		this->m_cardSet.subtract(it.m_cardSet);
	}
}

void Cards::removeByPoint(Card::CardPoint point)
{
	for(auto it = this->m_cardSet.begin(); it != this->m_cardSet.end(); )
	{
		if(it->getPoint() == point)
		{
			it = this->m_cardSet.erase(it);//删除后it自动指向下一个元素
		}
		else
		{
			++it;
		}
	}
}

int Cards::size() const
{
	return this->m_cardSet.size();
}

bool Cards::empty() const
{
	return this->m_cardSet.isEmpty();
}

void Cards::clear()
{
	this->m_cardSet.clear();
}

Card::CardPoint Cards::maxPoint() const
{
	Card::CardPoint max = Card::Point_begin;
	if (!this->empty())
	{
		for (auto& it : this->m_cardSet)
		{
			if (it.getPoint() > max)
			{
				max = it.getPoint();
			}
		}
	}
	return max;
}

Card::CardPoint Cards::minPoint() const
{
	Card::CardPoint min = Card::Point_end;
	if (!this->empty())
	{
		for (auto& it : this->m_cardSet)
		{
			if (it.getPoint() < min)
			{
				min = it.getPoint();
			}
		}
	}
	return min;
}

int Cards::pointCount(Card::CardPoint point) const
{
	int count = 0;
	if(!this->empty())
	{
		for (auto& it : this->m_cardSet)
		{
			if (it.getPoint() == point)
			{
				count++;
			}
		}
	}
	return count;
}

bool Cards::contains(const Card& card) const
{
	return this->m_cardSet.contains(card);
}

bool Cards::contains(const Cards& cards) const
{
	return this->m_cardSet.contains(cards.m_cardSet);
}

Card Cards::takeRandomCard()
{
	if(this->empty())
	{
		return Card();
	}
	int rand=QRandomGenerator::global()->bounded(this->size());
	auto it = this->m_cardSet.constBegin();
	std::advance(it, rand);
	Card card = *it;
	this->m_cardSet.erase(it);
	return card;
}

QVector<Card> Cards::toOrderlyList(SortType sortType) const
{
	QVector<Card> list = this->m_cardSet.values().toVector();
	if (sortType == SortType::Asc)
	{
		std::sort(list.begin(), list.end(), [](const Card& a, const Card& b)
			{
				if(a.getPoint() == b.getPoint())
				{
					return a.getSuit() < b.getSuit();
				}
				else
				{
					return a.getPoint() < b.getPoint();
				}
			});
	}
	else if(sortType== SortType::Desc)
	{
		std::sort(list.begin(), list.end(), [](const Card& a, const Card& b)
			{
				if (a.getPoint() == b.getPoint())
				{
					return a.getSuit() > b.getSuit();
				}
				else
				{
					return a.getPoint() > b.getPoint();
				}
			});
	}
	return list;
}

bool operator==(const Card& c1, const Card& c2)
{
	return (c1.getPoint() == c2.getPoint() && c1.getSuit() == c2.getSuit());
}

uint qHash(const Card& card)
{
	return card.getPoint() * 100 + card.getSuit();
}
