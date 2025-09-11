#include "PlayHand.h"

PlayHand::PlayHand(const Cards& cards)
{
	this->m_type = Hand_Unknown;
	this->m_point = cards.minPoint();
	this->m_extraCount = 0;
	//分析卡牌组，确定每张牌的点数
	this->classify(cards);
	//分析卡牌，分析出牌类型
	this->jugdeCardsType();
}

PlayHand::PlayHand(HandType type, Card::CardPoint point, int extra)
	:m_type(type), m_point(point), m_extraCount(extra)
{
	//通过传入的卡牌类型，点数，数量生成一个PlayHand对象
}

void PlayHand::classify(const Cards& cards)
{
	//分析卡牌，对卡牌进行分类
	this->m_cardList = cards.toOrderlyList(Cards::SortType::Asc);
	this->m_singles.clear();
	this->m_pairs.clear();
	this->m_triples.clear();
	this->m_fourples.clear();

	//建立一个哈希表，存储每个点数对应的数量
	QVector<int> pointCount(Card::Point_end, 0);
	for (auto& it : this->m_cardList)
	{
		pointCount[it.getPoint()]++;
	}
	//根据数量分类
	for (int i = 0;i < pointCount.size();i++)
	{
		switch (pointCount[i])
		{
		case 1:
		{
			this->m_singles.push_back((Card::CardPoint)i);
			break;
		}
		case 2:
		{
			this->m_pairs.push_back((Card::CardPoint)i);
			break;
		}
		case 3:
		{
			this->m_triples.push_back((Card::CardPoint)i);
			break;
		}
		case 4:
		{
			this->m_fourples.push_back((Card::CardPoint)i);
			break;
		}
		default:
			break;
		}
	}
	//对点数牌进行排序，方便牌型判断
	std::sort(this->m_singles.begin(), this->m_singles.end());
	std::sort(this->m_pairs.begin(), this->m_pairs.end());
	std::sort(this->m_triples.begin(), this->m_triples.end());
	std::sort(this->m_fourples.begin(), this->m_fourples.end());
}

void PlayHand::jugdeCardsType()
{



	//点数记录规则是：在打出来的牌类型中记录那个点数最小的，那么比如连队334455，记录的点数就是3，4和5通过点数变量增加可以得到
	//额外信息是，组对的数量，比如连队334455，组对的数量是3，那么额外信息就是3
	if (this->isPass())
	{
		this->m_type = Hand_Pass;
	}
	else if (this->isSingle())
	{
		this->m_type = Hand_Single;
		this->m_point = this->m_singles.front();
	}
	else if (this->isPair())
	{
		this->m_type = Hand_Pair;
		this->m_point = this->m_pairs.front();
	}
	else if (this->isTriple())
	{
		this->m_type = Hand_Triple;
		this->m_point = this->m_triples.front();
	}
	else if (this->isTripleSingle())
	{
		this->m_type = Hand_Triple_Single;
		this->m_point = this->m_triples.front();
	}
	else if (this->isTriplePair())
	{
		this->m_type = Hand_Triple_Pair;
		this->m_point = this->m_triples.front();
	}
	// 优先判断飞机带翅膀（有附加单/对的情况），再判断纯飞机
	else if (this->isPlaneWithSingles())
	{
		this->m_type = Hand_Plane_With_Singles;
		this->m_point = this->m_triples.front();
		this->m_extraCount = this->m_triples.size();
	}
	else if (this->isPlaneWithPairs())
	{
		this->m_type = Hand_Plane_With_Pairs;
		this->m_point = this->m_triples.front();
		this->m_extraCount = this->m_triples.size();
	}
	else if (this->isPlane())
	{
		this->m_type = Hand_Plane;
		this->m_point = this->m_triples.front();
		this->m_extraCount = this->m_triples.size();
	}
	else if (this->isSeqPair())
	{
		this->m_type = Hand_Seq_Pair;
		this->m_point = this->m_pairs.front();
		this->m_extraCount = this->m_pairs.size();
	}
	else if (this->isSeqSingle())
	{
		this->m_type = Hand_Seq_Single;
		this->m_point = this->m_singles.front();
		this->m_extraCount = this->m_singles.size();
	}
	// 四带X 放在炸弹之前
	else if (this->isFourWithTwoSingles())
	{
		this->m_type = Hand_Four_Two_Singles;
		this->m_point = this->m_fourples.front();
		//this->m_extraCount = this->m_singles.size();
	}
	else if (this->isFourWithTwoPairs())
	{
		this->m_type = Hand_Four_Two_Pairs;
		this->m_point = this->m_fourples.front();
		//this->m_extraCount = this->m_pairs.size();
	}
	else if (this->isBomb())
	{
		this->m_type = Hand_Bomb;
		this->m_point = this->m_fourples.front();
	}
	else if (this->isBombJokers())
	{
		this->m_type = Hand_Bomb_Jokers;
	}
}


PlayHand::HandType PlayHand::getType() const
{
	return this->m_type;
}

Card::CardPoint PlayHand::getPoint() const
{
	return this->m_point;
}

int PlayHand::getExtraCount() const
{
	return this->m_extraCount;
}

bool PlayHand::canBeat(const PlayHand& other) const
{
	//自己牌型未定义，或者不出牌，则视为不能大过别人
	if (this->m_type == Hand_Unknown|| this->m_type == Hand_Pass)
	{
		return false;
	}
	//如果别人不出牌，自己出牌肯定大过别人，返回true
	if (other.m_type == Hand_Pass)
	{
		return true;
	}
	//如果自己是王炸，肯定大过别人，返回true
	if (this->m_type == Hand_Bomb_Jokers)
	{
		return true;
	}
	//如果自己是炸弹，别人不是炸弹，肯定大过别人，返回true
	if(this->m_type==Hand_Bomb&&other.getType()!=Hand_Bomb&&other.getType()!= Hand_Bomb_Jokers )
	{
		return true;
	}
	//双方牌型一致
	if (this->m_type == other.getType())
	{
		//如果是连对，顺子，飞机，先比较组数是否相等
		if(this->m_type==Hand_Seq_Single||
			this->m_type==Hand_Seq_Pair||
			this->m_type==Hand_Plane||
			this->m_type==Hand_Plane_With_Singles||
			this->m_type==Hand_Plane_With_Pairs)
		{
			if(this->m_extraCount != other.getExtraCount())
			{
				//组数不等，不能大过对方
				return false;
			}
			else
			{
				//组数相等，比较点数
				return this->m_point > other.getPoint();
			}
		}
		else
		{
			//其他牌型，直接比较点数
			return this->m_point > other.getPoint();
		}
	}
	//其他情况，不能大过对方
	return false;
}









bool PlayHand::isPass()
{
	//如果四个分类的牌都为空，则表示玩家选择了不出牌
	if (this->m_singles.size() == 0 && this->m_pairs.size() == 0 && this->m_triples.size() == 0 && this->m_fourples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isSingle()
{
	//如果单牌分类的牌有且仅有一张，其他分类的牌都为空，则表示玩家出的牌是单张
	if (this->m_singles.size() == 1 && this->m_pairs.size() == 0 && this->m_triples.size() == 0 && this->m_fourples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isPair()
{
	//如果对牌分类的牌有且仅有一张，其他分类的牌都为空，则表示玩家出的牌是对子
	if (this->m_singles.size() == 0 && this->m_pairs.size() == 1 && this->m_triples.size() == 0 && this->m_fourples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isTriple()
{
	//如果三张分类的牌有且仅有一张，其他分类的牌都为空，则表示玩家出的牌是三个相同的牌
	if (this->m_singles.size() == 0 && this->m_pairs.size() == 0 && this->m_triples.size() == 1 && this->m_fourples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isTripleSingle()
{
	//如果三张分类的牌有且仅有一张，单牌分类的牌有且仅有一张，其他分类的牌都为空，则表示玩家出的牌是三带一
	if (this->m_singles.size() == 1 && this->m_pairs.size() == 0 && this->m_triples.size() == 1 && this->m_fourples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isTriplePair()
{
	//如果三张分类的牌有且仅有一张，对牌分类的牌有且仅有一张，其他分类的牌都为空，则表示玩家出的牌是三带二
	if (this->m_singles.size() == 0 && this->m_pairs.size() == 1 && this->m_triples.size() == 1 && this->m_fourples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isPlane()
{
	//飞机：连续的三张数量 >= 2，且没有附带的单/对（纯飞机）
	if (this->m_singles.size() == 0 && this->m_pairs.size() == 0 && this->m_triples.size() >= 2 && this->m_fourples.size() == 0)
	{
		//判断三张的点数是否连续，同时不能有2或王
		if (this->m_triples.back() - this->m_triples.front() + 1 == this->m_triples.size() &&
			this->m_triples.front() >= Card::Card_3 && this->m_triples.back() <= Card::Card_A)
		{
			return true;
		}
	}
	return false;
}

bool PlayHand::isPlaneWithSingles()
{
	//飞机带单：连续三张数量 >=2，且附带单牌数量恰好等于三张组数
	if (this->m_triples.size() >= 2 && this->m_singles.size() == this->m_triples.size() && this->m_pairs.size() == 0 && this->m_fourples.size() == 0)
	{
		//判断三张的点数是否连续，同时不能有2或王
		if (this->m_triples.back() - this->m_triples.front() + 1 == this->m_triples.size() &&
			this->m_triples.front() >= Card::Card_3 && this->m_triples.back() <= Card::Card_A)
		{
			//确保附带的单牌可以有王但是不能有王炸
			int jokerCount = 0;
			for(auto& it : this->m_singles)
			{
				if(it==Card::Card_Joker||it==Card::Card_BigJoker)
				{
					jokerCount++;
				}
			}
			if (jokerCount < 2)
			{
				return true;
			}
		}
	}
	return false;
}

bool PlayHand::isPlaneWithPairs()
{
	//飞机带对：连续三张数量 >=2，且附带对子数量恰好等于三张组数
	if (this->m_triples.size() >= 2 && this->m_pairs.size() == this->m_triples.size() && this->m_singles.size() == 0 && this->m_fourples.size() == 0)
	{
		//判断三张的点数是否连续，同时不能有2或王
		if (this->m_triples.back() - this->m_triples.front() + 1 == this->m_triples.size() &&
			this->m_triples.front() >= Card::Card_3 && this->m_triples.back() <= Card::Card_A)
		{
			if (this->m_pairs.front() >= Card::Card_3 && this->m_pairs.back() <= Card::Card_2)
			{
				return true;
			}
		}
	}
	return false;
}

bool PlayHand::isSeqPair()
{
	//如果对牌分类的牌多于两张，其他分类的牌都为空，则表示玩家出的牌是连对
	if (this->m_singles.size() == 0 && this->m_pairs.size() >= 3 && this->m_triples.size() == 0 && this->m_fourples.size() == 0)
	{
		//判断对牌的点数是否连续，同时不能有2或王
		if (this->m_pairs.back() - this->m_pairs.front() + 1 == this->m_pairs.size() &&
			this->m_pairs.front() >= Card::Card_3 && this->m_pairs.back() <= Card::Card_A)
		{
			return true;
		}
	}
	return false;
}

bool PlayHand::isSeqSingle()
{
	//如果单牌分类的牌多于四张，其他分类的牌都为空，则表示玩家出的牌是顺子
	if (this->m_singles.size() >= 5 && this->m_pairs.size() == 0 && this->m_triples.size() == 0 && this->m_fourples.size() == 0)
	{
		//判断对牌的点数是否连续，同时不能有2或王
		if (this->m_singles.back() - this->m_singles.front() + 1 == this->m_singles.size() &&
			this->m_singles.front() >= Card::Card_3 && this->m_singles.back() <= Card::Card_A)
		{
			return true;
		}
	}
	return false;
}

bool PlayHand::isFourWithTwoSingles()
{
	// 四带两单（4张 + 两张单牌），标准斗地主允许
	if (this->m_fourples.size() == 1 && this->m_singles.size() == 2 && this->m_pairs.size() == 0 && this->m_triples.size() == 0)
	{
		//确保附带的单牌可以有王但是不能有王炸
		int jokerCount = 0;
		for (auto& it : this->m_singles)
		{
			if (it == Card::Card_Joker || it == Card::Card_BigJoker)
			{
				jokerCount++;
			}
		}
		if (jokerCount < 2)
		{
			return true;
		}
	}
	return false;
}

bool PlayHand::isFourWithTwoPairs()
{
	// 四带两对（4张 + 两对），标准斗地主允许
	if (this->m_fourples.size() == 1 && this->m_pairs.size() == 2 && this->m_singles.size() == 0 && this->m_triples.size() == 0)
	{
		return true;
	}
	return false;
}

bool PlayHand::isBomb()
{
	//如果四张分类的牌有且仅有一张，其他分类的牌都为空，则表示玩家出的牌是炸弹（纯炸弹）
	if (this->m_singles.size() == 0 && this->m_pairs.size() == 0 && this->m_triples.size() == 0 && this->m_fourples.size() == 1)
	{
		return true;
	}
	return false;
}

bool PlayHand::isBombJokers()
{
	//如果玩家出的牌有且仅有两张，且这两张牌分别是小王和大王，则表示玩家出的牌是王炸
	if (this->m_singles.size() == 2 && this->m_pairs.size() == 0 && this->m_triples.size() == 0 && this->m_fourples.size() == 0)
	{
		//大小王的判断
		if (this->m_singles.front() == Card::Card_Joker && this->m_singles.back() == Card::Card_BigJoker)
		{
			return true;
		}
	}
	return false;
}
