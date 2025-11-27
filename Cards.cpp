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
	else if (sortType == SortType::Display)
	{
		// 1) 基于 Desc 的基础顺序，保证点力/花色的一致性
		std::sort(list.begin(), list.end(), [](const Card& a, const Card& b)//正常降序
			{
				if (a.getPoint() == b.getPoint()) return a.getSuit() > b.getSuit();
				return a.getPoint() > b.getPoint();
			});

		// 2) 分桶：按点数聚合，记录点数出现顺序（遵循基础顺序）
		QHash<int, QVector<Card>> buckets; // key: CardPoint
		QList<int> order;                  // 点数出现顺序（高到低）
		order.reserve(list.size());

		for (const Card& c : list)
		{
			int key = static_cast<int>(c.getPoint());
			if (!buckets.contains(key))
			{
				buckets.insert(key, {});
				order.push_back(key);// 记录点数出现顺序
			}
			buckets[key].push_back(c);//这里在为每个点数的桶中添加卡牌，实现分桶
		}

		// 3) 组装：4张组 -> 3张组 -> 2张组 -> 1张组；同一张数组内保持点数与花色的基础顺序
		QVector<Card> display;
		display.reserve(list.size());
		for (int cnt = 4; cnt >= 1; --cnt)
		{
			for (int key : order)
			{
				const auto& v = buckets[key];
				if (v.size() == cnt)
				{
					display += v;
				}
				//这样就实现了按组数从多到少的顺序排列
			}
		}
		return display;
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
