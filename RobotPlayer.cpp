#include "RobotPlayer.h"

RobotPlayer::RobotPlayer(QObject* parent)
	: Player(parent)
{
	this->setType(Player::Computer);
}

RobotPlayer::~RobotPlayer()
{}

void RobotPlayer::prepareCallLord()
{
	RobotGrabLord* robotGrabLord = new RobotGrabLord(this);
	robotGrabLord->start();
}

void RobotPlayer::preparePlayCards()
{
	RobotPlayCards* robotPlayCards = new RobotPlayCards(this);
	robotPlayCards->start();
}

void RobotPlayer::thinkLordBet()
{
	//1.有一个王+3分，有王炸再+3分
	//2.有一个炸弹+4分
	//3.把炸弹牌剔除
	//4.剩下的牌计算取出飞机，连对，顺子，按牌长度最长的算分
	//5.剔除那个最长的牌
	//6.剩下的牌每有一个2，+1分
	//7.剔除2
	//8.剩下的牌每有一个3带牌，+2分
	//9.剔除3带牌
	//10.剩下的牌每有一个对，+1分
	int weight = 0;
	Cards backCards = this->m_handCards;
	//1.有一个王+3分，有王炸再+3分
	Cards joker = Strategy(this, backCards).findCardsByPointRange(Card::CardPoint::Card_Joker);
	if (!joker.empty())
	{
		weight += 3;
		if (joker.size() == 2)
		{
			weight += 3;
		}
		backCards.remove(joker);
	}
	qDebug() << "计算王牌后权重值为" << weight;
	//2.炸弹的处理
	QVector<Cards> bombs = Strategy(this, backCards).findCardsByType(PlayHand(PlayHand::Hand_Bomb, Card::CardPoint::Card_3, 0));
	if (!bombs.empty())
	{
		weight += 4 * bombs.size();
		backCards.remove(bombs);//把炸弹都剔除掉
	}
	qDebug() << "炸弹有" << bombs.size();
	qDebug() << "计算炸弹后权重值为" << weight;
	//4.剩下的牌计算取出飞机，连对，顺子，按牌长度最长的算分
	QVector<Cards> cardsArray;
	Cards maxSpecialCards;
	QVector<Cards> planes = Strategy(this, backCards).findCardsByType(PlayHand(PlayHand::Hand_Plane, Card::CardPoint::Card_3, 0));
	planes += Strategy(this, backCards).findCardsByType(PlayHand(PlayHand::Hand_Plane_With_Singles, Card::CardPoint::Card_3, 0));
	planes += Strategy(this, backCards).findCardsByType(PlayHand(PlayHand::Hand_Plane_With_Pairs, Card::CardPoint::Card_3, 0));
	QVector<Cards> seqPairs = Strategy(this, backCards).findCardsByType(PlayHand(PlayHand::Hand_Seq_Pair, Card::CardPoint::Card_3, 0));
	QVector<Cards> seqSingles = Strategy(this, backCards).findCardsByType(PlayHand(PlayHand::Hand_Seq_Single, Card::CardPoint::Card_3, 0));
	cardsArray << planes << seqPairs << seqSingles;
	if (!cardsArray.empty())
	{
		//先对长度进行排序
		std::sort(cardsArray.begin(), cardsArray.end(), [](const Cards& a, const Cards& b) {
			return a.size() > b.size();
			});
		maxSpecialCards = cardsArray.front();
		//把最长的牌型剔除
		weight += maxSpecialCards.size();
		backCards.remove(maxSpecialCards);
	}
	qDebug() << "计算特殊牌型后权重值为" << weight;
	//6.每有一个2，+1分
	Cards twos = Strategy(this, backCards).findCardsByPointRange(Card::CardPoint::Card_2, Card::CardPoint::Card_Joker);
	weight += twos.size();
	backCards.remove(twos);
	qDebug() << "计算2后权重值为" << weight;
	//8.每有一个3带牌，+2分
	QVector<Cards> triples = Strategy(this, backCards).findCardsByCount(3);
	weight += 2 * triples.size();
	backCards.remove(triples);
	qDebug() << "计算三带后权重值为" << weight;
	//10.剩下的牌每有一个对，+1分
	QVector<Cards> pairs = Strategy(this, backCards).findCardsByCount(2);
	weight += pairs.size();
	qDebug() << "计算对子后权重值为" << weight;
	//根据权重值决定叫分大小


	//1.如果weight>=15,叫3分
	//2.如果10<=weight<15,60%叫2分，40%叫3分
	//3.如果5<=weight<10,60%叫1分，40%叫2分
	//4.如果weight<5,80%叫0分，20%叫1分
	qDebug() << "权重值为" << weight;
	int bet = 0;
	if(weight>=15)
	{
		bet = 3;
	}
	else if(weight>=10)
	{
		int randNum = QRandomGenerator().global()->bounded(100);
		if(randNum<80)
		{
			bet = 2;
		}
		else
		{
			bet = 3;
		}
	}
	else if(weight>=5)
	{
		int randNum = QRandomGenerator().global()->bounded(100);
		if (randNum < 80)
		{
			bet = 1;
		}
		else
		{
			bet = 2;
		}
	}
	else
	{
		int randNum = QRandomGenerator().global()->bounded(100);
		if (randNum < 80)
		{
			bet = 0;
		}
		else
		{
			bet = 1;
		}
	}
	//转入叫地主函数
	this->grabLordBet(bet);
}

void RobotPlayer::thinkPlayCards()
{
	Strategy st(this);
	Cards playCards = st.makeStrategy();
	this->playCards(playCards);
}

