#include "Strategy.h"

Strategy::Strategy(Player* player)
{
	this->m_player = player;
	this->m_cards = this->m_player->getHandCards();
}

Strategy::Strategy(Player* player, const Cards& cards)
{
	this->m_player = player;
	this->m_cards = cards;
}

Cards Strategy::findSameCards(Card::CardPoint point, int count)
{
	//非法牌数
	if(count<1||count>4)
	{
		return Cards();
	}
	Cards result;
	int foundCount = 0;
	QVector<Card> cardList = this->m_cards.toOrderlyList(Cards::SortType::Asc);
	//遍历手牌，找到指定点数的牌
	//值传递
	for (auto it : cardList)
	{
		if (it.getPoint() == point)
		{
			result.add(it);
			foundCount++;
			//找够了count张，返回结果
			if (foundCount == count)
			{
				return result;
			}
		}
	}
	
	//没找到，返回空Cards类
	return Cards();
}

QVector<Cards> Strategy::findCardsByCount(int count)
{
	QVector<Cards> result;
	if(count<1||count>4)
	{
		return result;
	}
	for (Card::CardPoint point = (Card::CardPoint)(Card::CardPoint::Point_begin + 1);point < Card::CardPoint::Point_end;point = (Card::CardPoint)(point + 1))
	{
		//如果当前点数的牌数量等于count，则找到
		if (this->m_cards.pointCount(point) == count)
		{
			//先判断再调用findSameCards，避免返回空Cards类
			Cards cards = this->findSameCards(point, count);
			result.push_back(cards);
		}
	}
	return result;
}

Cards Strategy::findCardsByPointRange(Card::CardPoint minPoint, Card::CardPoint maxPoint)
{
	Cards result;
	if (minPoint >= maxPoint || minPoint <= Card::CardPoint::Point_begin || maxPoint > Card::CardPoint::Point_end)
	{
		return result;
	}
	for (Card::CardPoint point = minPoint;point < maxPoint;point = (Card::CardPoint)(point + 1))
	{
		int count = this->m_cards.pointCount(point);
		if (count > 0)
		{
			Cards cards = this->findSameCards(point, count);
			result.add(cards);
		}
	}
	return result;
}

QVector<Cards> Strategy::findCardsByType(const PlayHand& ph, bool beat)
{
	PlayHand::HandType type = ph.getType();
	Card::CardPoint point = ph.getPoint();
	int extra = ph.getExtraCount();

	//如果beat是true，那么要找的点数的起始点数比传入的点数大1
	Card::CardPoint beginPoint = beat ? (Card::CardPoint)(point + 1) : point;

	//分牌型查找牌组
	//分析规则：
	//1.正常组合牌，就分析得出满足牌型的所有组合
	//2.需要带的牌，比如三带一，飞机带单这类，带的那个牌只找最小的，对子同理
	//tip:三带和四带使用的是保守的粗略策略，会尽可能的少拆3带和4带，去找最小的额外牌进行组合返回
	switch (type)
	{
	case PlayHand::Hand_Unknown:
		break;
	case PlayHand::Hand_Pass:
		break;
	case PlayHand::Hand_Single://单 双 三 四处理类型类似
		return this->getCards(beginPoint, 1);
	case PlayHand::Hand_Pair:
		return this->getCards(beginPoint, 2);
	case PlayHand::Hand_Triple:
		return this->getCards(beginPoint, 3);
	case PlayHand::Hand_Triple_Single:
		return this->getCardsWithExtras(beginPoint, 3, false);
		break;
	case PlayHand::Hand_Triple_Pair:
		return this->getCardsWithExtras(beginPoint, 3, true);
		break;
	case PlayHand::Hand_Plane://单飞机 顺子 连对处理类似
	{
		if (beat)
		{
			return this->getCardsWithSequences(beginPoint, extra, 3, true);
		}
		else
		{
			return this->getCardsWithSequences(beginPoint, 2, 3, false);
		}
		break;
	}
	case PlayHand::Hand_Plane_With_Singles:
	{
		if (beat)
		{
			return this->getPlaneWithExtras(beginPoint, extra, false, true);
		}
		else
		{
			return this->getPlaneWithExtras(beginPoint, 2, false, false);
		}
		break;
	}
	case PlayHand::Hand_Plane_With_Pairs:
	{
		if (beat)
		{
			return this->getPlaneWithExtras(beginPoint, extra, true, true);
		}
		else
		{
			return this->getPlaneWithExtras(beginPoint, 2, true, false);
		}
		break;
	}
	case PlayHand::Hand_Seq_Pair:
	{
		if (beat)
		{
			return this->getCardsWithSequences(beginPoint, extra, 2, true);
		}
		else
		{
			return this->getCardsWithSequences(beginPoint, 3, 2, false);
		}
		break;
	}
	case PlayHand::Hand_Seq_Single:
	{
		if (beat)
		{
			return this->getCardsWithSequences(beginPoint, extra, 1, true);
		}
		else
		{
			return this->getCardsWithSequences(beginPoint, 5, 1, false);
		}
		break;
	}
	case PlayHand::Hand_Four_Two_Singles:
		return this->getCardsWithExtras(beginPoint, 4, false);
		break;
	case PlayHand::Hand_Four_Two_Pairs:
		return this->getCardsWithExtras(beginPoint, 4, true);
		break;
	case PlayHand::Hand_Bomb://单 双 三 四处理类型类似
		return this->getCards(beginPoint, 4);
		break;
	case PlayHand::Hand_Bomb_Jokers:
		return this->getJokerBombs();
		break;
	default:
		return QVector<Cards>();
		break;
	}
	return QVector<Cards>();
}

Cards Strategy::makeStrategy()
{
	Player* pendingPlayer = this->m_player->getPendingPlayer();
	Cards pendingCards = this->m_player->getPendingCards();
	//如果当前没有待处理的牌，或者待处理的牌是自己打出的，那么自己作为第一位出牌玩家
	if (pendingPlayer == this->m_player||pendingPlayer == nullptr)
	{
		return this->firstCards();
	}
	else
	{
		//如果当前待处理的牌是其他玩家打出的，那么需要根据当前牌局情况进行出牌策略
		//取得更大的牌
		PlayHand type(pendingCards);
		Cards beatCards = this->getBeatCards(type);
		//如果找到了更大的牌，那么考虑是否出牌
		bool consider = this->considerToPlay(beatCards);
		if (consider && !beatCards.empty())
		{
			//出牌
			return beatCards;
		}
		else
		{
			//不出牌，过
			return Cards();
		}
	}
	return Cards();
}

Cards Strategy::firstCards()
{
	//如果玩家手中剩下的牌已经是一个完整的牌型，那么就直接出完
	PlayHand ph(this->m_cards);
	if(ph.getType() != PlayHand::Hand_Unknown)
	{
		return this->m_cards;
	}
	//先找一个 最不会拆牌的顺子
	QVector<Cards> seqSingles = this->getCardsWithSequences(Card::Card_3, 5, 1, false);
	if (!seqSingles.empty())
	{
		int beforeSingle = this->findCardsByCount(1).size();
		Cards tempCards = this->m_cards;
		int index = -1;
		for (int i = 0;i < seqSingles.size();i++)
		{
			tempCards.remove(seqSingles[i]);
			int afterSingle = Strategy(m_player, tempCards).findCardsByCount(1).size();
			if (afterSingle < beforeSingle)
			{
				beforeSingle = afterSingle;
				index = i;
			}
			tempCards = this->m_cards;
		}
		if (index != -1)
		{
			return seqSingles[index];
		}
	}
	//先把炸弹都排除掉
	Cards remain = this->m_cards;
	remain.remove(this->findCardsByCount(4));
	//接着的出牌顺序是，飞机-连对-三带-对-单
	QVector<Cards> cardsArray;
	//飞机
	QVector<Cards> planes = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Plane,Card::CardPoint::Card_3,0));
	QVector<Cards> planeWithSingles = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Plane_With_Singles, Card::CardPoint::Card_3, 0));
	QVector<Cards> planeWithPairs = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Plane_With_Pairs, Card::CardPoint::Card_3, 0));
	cardsArray << planes << planeWithSingles << planeWithPairs;
	if(!cardsArray.empty())
	{
		//找到长度最长（消耗牌最多）但是点数最小的飞机
		Cards maxPlane;
		for (int i = 0;i < cardsArray.size();i++)
		{
			if (cardsArray[i].size() > maxPlane.size())
			{
				maxPlane = cardsArray[i];
			}
		}
		return maxPlane;
	}
	//连对
	QVector<Cards> seqPairs = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Seq_Pair, Card::CardPoint::Card_3, 0));
	if(!seqPairs.empty())
	{
		//找到最长的但是点数最小的连对
		Cards maxSeqPair;
		for (int i = 0;i < seqPairs.size();i++)
		{
			if (seqPairs[i].size() > maxSeqPair.size())
			{
				maxSeqPair = seqPairs[i];
			}
		}
		return maxSeqPair;
	}
	//三带
	QVector<Cards> triples = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Triple, Card::CardPoint::Card_3, 0));
	QVector<Cards> tripleWithSingles = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Triple_Single, Card::CardPoint::Card_3, 0));
	QVector<Cards> tripleWithPairs = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Triple_Pair, Card::CardPoint::Card_3, 0));
	cardsArray.clear();
	cardsArray << triples << tripleWithSingles << tripleWithPairs;
	if (!cardsArray.empty())
	{
		//对下一位做判断，如果下一位是敌人且牌小于等于2张，那么就出最小的三带
		Player* nextPlayer = m_player->getNextPlayer();
		if (nextPlayer->getRole() != m_player->getRole() && nextPlayer->getHandCards().size() <= 2)
		{
			return cardsArray.front();
		}
		//如果下一位是对手并且牌大于等于3且小于等于5张，那么就出最大的三带
		else if (nextPlayer->getRole() != m_player->getRole() && nextPlayer->getHandCards().size() >= 3 && nextPlayer->getHandCards().size() <= 5)
		{
			return cardsArray.back();
		}
		//否则就出长度最长但是点数最小的三带，但是这个三带的型号不能是2
		else
		{
			Cards maxTriple;
			for (int i = 0;i < cardsArray.size();i++)
			{
				if (cardsArray[i].size() > maxTriple.size())
				{
					maxTriple = cardsArray[i];
				}
			}
			if (PlayHand(maxTriple).getPoint() != Card::Card_2)
			{
				return maxTriple;
			}
		}
	}
	remain.remove(cardsArray);//把三带都移除掉去找对子和单牌，因为三带可能找到了但是没有打出去
	//对子
	QVector<Cards> pairs = Strategy(m_player, remain).findCardsByType(PlayHand(PlayHand::Hand_Pair, Card::CardPoint::Card_3, 0));
	if (!pairs.empty())
	{
		//如果下一位1是对手并且牌数在2-4张之间，那么就出最大的对子
		Player* nextPlayer = m_player->getNextPlayer();
		if (nextPlayer->getRole() != m_player->getRole() && nextPlayer->getHandCards().size() >= 2 && nextPlayer->getHandCards().size() <= 4)
		{
			return pairs.back();
		}
		//否则就找最小的对子，但是这个对子不能是2
		else
		{
			for (int i = 0;i < pairs.size();i++)
			{
				if (PlayHand(pairs[i]).getPoint() != Card::Card_2)
				{
					return pairs[i];
				}
			}
		}
	}
	remain.remove(pairs);//把对子都移除掉去找单牌，因为对子可能找到了但是没有打出去
	//单牌，换一种招牌算法，到了这里，说明手牌中没有顺子，飞机，连对，合适的三带，合适的对子
	cardsArray.clear();
	for(Card::CardPoint point = Card::Card_3;point < Card::CardPoint::Card_BigJoker;point = (Card::CardPoint)(point + 1))
	{
		Cards singles = this->findSameCards(point, 1);
		if(!singles.empty())
		{
			cardsArray.push_back(singles);
		}
	}
	//如果下一位是对手并且牌数在1-2张之间，那么就出最大的单牌
	if (!cardsArray.empty())
	{
		Player* nextPlayer = m_player->getNextPlayer();
		if (nextPlayer->getRole() != m_player->getRole() && nextPlayer->getHandCards().size() >= 1 && nextPlayer->getHandCards().size() <= 2)
		{
			return cardsArray.back();
		}
		else
		{
			//否则就找最小的单牌
			return cardsArray.front();
		}
	}
	//应该不可能到这里
	return Cards();
}

Cards Strategy::getBeatCards(PlayHand& ph)
{
	{
		//1. 出牌玩家和当前玩家不是一伙的
		Player* pendPlayer = m_player->getPendingPlayer();
		//待处理的牌不是队友而且要打完牌了
		if (pendPlayer != nullptr && pendPlayer->getRole() != m_player->getRole() && pendPlayer->getHandCards().size()<=3)
		{
			//那就找炸弹
			QVector<Cards> bombs = findCardsByCount(4);
			for (int i = 0; i < bombs.size(); ++i)
			{
				if (PlayHand(bombs[i]).canBeat(ph))
				{
					return bombs[i];
				}
			}
			// 搜索当前玩家手中有没有王炸
			Cards sj = findSameCards(Card::Card_Joker, 1);
			Cards bj = findSameCards(Card::Card_BigJoker, 1);
			if (!sj.empty() && !bj.empty())
			{
				Cards jokers;
				jokers << sj << bj;
				return jokers;
			}
		}
		//2. 当前玩家和下一个玩家不是一伙的
		Player* nextPlayer = m_player->getNextPlayer();
		// 将玩家手中的最合理的顺子剔除出去
		Cards remain = m_cards;
		remain.remove(Strategy(m_player, remain).pickOptimalSeqSingles());


		auto beatCard = std::bind([=](const Cards& cards)
			{
				//找出能压过对方的牌
				QVector<Cards> beatCardsArray = Strategy(m_player, cards).findCardsByType(ph, true);
				if (!beatCardsArray.empty())
				{
					//如果当前玩家和下一个玩家不是一伙的，并且下一个玩家手中的牌小于等于2张，就找最大的牌压
					if (m_player->getRole() != nextPlayer->getRole() && nextPlayer->getHandCards().size() <= 2)
					{
						return beatCardsArray.back();
					}
					else//否则就找最小的牌压
					{
						return beatCardsArray.front();
					}
				}
				return Cards();
			}, std::placeholders::_1);

		Cards cs;
		if (!(cs = beatCard(remain)).empty())//先用剔除顺子的牌找
		{
			return cs;
		}
		else//如果没找到，那就用全部手牌找
		{
			if (!(cs = beatCard(m_cards)).empty()) return cs;
		}
		return Cards();
	}
}

bool Strategy::considerToPlay(Cards& beatCards)
{
	if(beatCards.empty())
	{
		return false;
	}
	Player* pendingPlayer = this->m_player->getPendingPlayer();
	if(pendingPlayer==nullptr)
	{
		return true;
	}
	else if (pendingPlayer->getRole() == this->m_player->getRole())//队友打的牌
	{
		//手牌在出完这次牌之后，剩余的牌是一个完整的牌型，也就是能一次打完，那就压队友
		Cards tempCards = this->m_cards;
		tempCards.remove(beatCards);
		if(PlayHand(tempCards).getType()!=PlayHand::Hand_Unknown)
		{
			return true;
		}
		//如果手牌最小点数的牌大于等于2，那就不出，留着给地主
		if(this->m_cards.minPoint()>=Card::Card_2)
		{
			return false;
		}
	}
	else//敌人打的牌
	{
		PlayHand ph(beatCards);
		//如果是三个2带一或者带对，并且手牌数量大于等于8，那就不出，留着炸
		if((ph.getType()==PlayHand::Hand_Triple_Single||ph.getType()==PlayHand::Hand_Triple_Pair)&& ph.getPoint()==Card::Card_2 && this->m_cards.size() >= 8)
		{
			return false;
		}
		//如果要出的牌是对2，并且当前待处理玩家手中的牌大于等于10，并且手中的牌数量小于等于5，不出
		if(ph.getType()==PlayHand::Hand_Pair&&ph.getPoint()==Card::Card_2&&pendingPlayer->getHandCards().size()>=10&&this->m_cards.size()<=5)
		{
			return false;
		}
	}
	return true;
}







QVector<Cards> Strategy::getCards(Card::CardPoint beginPoint, int count)
{
	QVector<Cards> result;
	if (beginPoint <= Card::CardPoint::Point_begin || beginPoint >= Card::CardPoint::Point_end || count < 1 || count > 4)
	{
		return result;
	}
	for (Card::CardPoint point = beginPoint;point < Card::CardPoint::Point_end;point = (Card::CardPoint)(point + 1))
	{
		//多加一个if判断，尽量不去拆牌
		if (this->m_cards.pointCount(point)==count)
		{
			Cards cs = this->findSameCards(point, count);
			result.push_back(cs);
		}
	}
	return result;
}

QVector<Cards> Strategy::getCards(Card::CardPoint beginPoint, int count, Cards& cards)
{
	QVector<Cards> result;
	if (beginPoint <= Card::CardPoint::Point_begin || beginPoint >= Card::CardPoint::Point_end || count < 1 || count > 4)
	{
		return result;
	}
	QVector<Card> cardList = cards.toOrderlyList(Cards::SortType::Asc);
	for (Card::CardPoint point = beginPoint;point < Card::CardPoint::Point_end;point = (Card::CardPoint)(point + 1))
	{
		//多加一个if判断，尽量不去拆牌
		if (cards.pointCount(point) == count)
		{
			Cards foundCards;
			int foundCount = 0;
			//遍历手牌，找到指定点数的牌
			//值传递
			for (auto it : cardList)
			{
				if (it.getPoint() == point)
				{
					foundCards.add(it);
					foundCount++;
					//找够了count张，返回结果
					if (foundCount == count)
					{
						result.push_back(foundCards);
						break;
					}
				}
			}
		}
	}
	return result;
}

QVector<Cards> Strategy::getJokerBombs()
{
	QVector<Cards> result;
	QVector<Card> cardList = this->m_cards.toOrderlyList();
	if (cardList.size() >= 2)
	{
		Card::CardPoint firstPoint = cardList[0].getPoint();
		Card::CardPoint secondPoint = cardList[1].getPoint();
		if (firstPoint == Card::Card_BigJoker && secondPoint == Card::Card_Joker)
		{
			Cards temp;
			temp.add(cardList[0]);
			temp.add(cardList[1]);
			result.push_back(temp);
		}
	}
	return result;
}

QVector<Cards> Strategy::getCardsWithExtras(Card::CardPoint beginPoint, int typeCount, bool pair)
{
	// typeCount是三带和四带的那个数量，pair表示是否是带对
	QVector<Cards> result;
	if (beginPoint <= Card::CardPoint::Point_begin || beginPoint >= Card::CardPoint::Point_end || (typeCount != 3 && typeCount != 4))
	{
		return result;
	}
	//将手牌复制一份
	Cards hcards = this->m_cards;
	//先找出所有满足三带或四带的牌组
	QVector<Cards> mainCards = this->getCards(beginPoint, typeCount, hcards);
	//如果没有找到，直接返回空
	if (mainCards.isEmpty())
	{
		return result;
	}
	//找出所有满足附加牌的牌组
	QVector<Cards> extraCards;
	//删除主牌，如果是三带的情况，手牌中有4张相同点数，也全删了，避免出现333带3的情况
	for (auto& mc : mainCards)
	{
		Card::CardPoint mpoint = mc.minPoint();
		//删除所有这个点数的牌
		//把花色全部遍历
		for(Card::CardSuit suit = (Card::CardSuit)(Card::CardSuit::Suit_begin + 1);suit < Card::CardSuit::Suit_end;suit = (Card::CardSuit)(suit + 1))
		{
			Card c(suit, mpoint);
			if (hcards.contains(c))
			{
				hcards.remove(c);
			}
		}
		
	}
	//对三带一/三带一对 四带二/四带两对的附加牌进行查找
	if (pair)
	{
		extraCards = this->getCards(Card::Card_3, 2, hcards);//找对子
	}
	else
	{
		extraCards = this->getCards(Card::Card_3, 1, hcards);//找单
	}
	//分三带和四带
	if (typeCount == 3)
	{
		//如果附加牌为空
		if (extraCards.isEmpty())
		{
			return result;
		}
		//将附加牌的最小牌和每个主牌组合合并
		for (auto &mc : mainCards)
		{
			mc.add(extraCards[0]);
			result.push_back(mc);
		}
	}
	else
	{
		//四带的附加牌至少要两组
		if (extraCards.size() < 2)
		{
			return result;
		}
		//将附加牌的最小两组和每个主牌组合合并
		Cards ec;
		ec.add(extraCards[0]);
		ec.add(extraCards[1]);
		for (auto &mc : mainCards)
		{
			mc.add(ec);
			result.push_back(mc);
		}
	}
	return result;
}

QVector<Cards> Strategy::getCardsWithSequences(Card::CardPoint beginPoint,int minLen, int typeCount, bool beat)
{
	QVector<Cards> result;
	if (beginPoint <= Card::CardPoint::Point_begin || beginPoint >= Card::CardPoint::Point_end || minLen<2||typeCount<1||typeCount>3)
	{
		return result;
	}
	//将手牌去重，以哈希表构建 点数-数量 映射 
	//3-A，数组仅需12长度
	QVector<int> uniquePoints(12, 0);
	QVector<Card> hcardList = this->m_cards.toOrderlyList(Cards::SortType::Asc);
	for (auto& c : hcardList)
	{
		Card::CardPoint p = c.getPoint();
		if (p >= Card::Card_2)
		{
			continue;
		}
		else
		{
			uniquePoints[(int)(p - 1)]++;
		}
	}
	//开始初定窗口，初始窗口大小为minLen，窗口最大值为12-(beginPoint-1)
	for (int window = minLen; window <= uniquePoints.size()-(int)(beginPoint-1); window++)
	{
		//从起始点数开始
		for (int left = (int)(beginPoint - 1);left <= uniquePoints.size() - window;left++)
		{
			//逐个检查窗口内点数是否满足要求
			bool valid = true;
			for (int i = left;i < left + window;i++)
			{
				if (uniquePoints[i] < typeCount || uniquePoints[i] == 4)//如果点数数量小于typeCount，或者等于4（炸弹），都不满足要求
				{
					valid = false;
					break;
				}
			}
			//如果窗口内点数都满足要求
			if (valid)
			{
				//将窗口内的点数和数量构建成Cards类
				Cards cs;
				for (int i = left;i < left + window;i++)
				{
					Card::CardPoint p = (Card::CardPoint)(i + 1);
					Cards sameCards = this->findSameCards(p, typeCount);
					cs.add(sameCards);
				}
				result.push_back(cs);
			}
		}
		//如果beat为true，那在window=minLen时走一遍就立刻返回结果
		if (beat)
		{
			return result;
		}
	}
	return result;
}

QVector<Cards> Strategy::getPlaneWithExtras(Card::CardPoint beginPoint, int minLen, bool pair, bool beat)
{
	QVector<Cards> result;
	if (beginPoint <= Card::CardPoint::Point_begin || beginPoint >= Card::CardPoint::Point_end || minLen < 2)
	{
		return result;
	}
	//先得到所有的飞机组合
	QVector<Cards> planes = this->getCardsWithSequences(beginPoint, minLen, 3, beat);
	//复制一份手牌并且删除飞机组合的牌
	Cards hcards = this->m_cards;
	hcards.remove(planes);
	//找出所有满足附加牌的牌组
	QVector<Cards> extraCards;
	if(pair)
	{
		extraCards = this->getCards(Card::Card_3, 2, hcards);//找对子
	}
	else
	{
		extraCards = this->getCards(Card::Card_3, 1, hcards);//找单
	}
	for (auto& p : planes)
	{
		int needed = p.size() / 3;
		if(extraCards.size()>=needed)
		{
			Cards ec;
			for(int i=0;i<needed;i++)
			{
				ec.add(extraCards[i]);
			}
			p.add(ec);
			result.push_back(p);
		}
	}
	return result;
}

Cards Strategy::pickOptimalSeqSingles()
{
	//找到手牌中最合理的顺子

	QVector<Cards> seqSingles = this->findCardsByType(PlayHand(PlayHand::Hand_Seq_Single,Card::CardPoint::Card_3,0), false);
	if (seqSingles.empty())
	{
		return Cards();
	}
	else
	{
		return seqSingles.back();//返回最长也是最大的顺子，这是由findCardsByType的实现决定的
	}
}
