#include "GameControl.h"

GameControl::GameControl(QObject *parent)
	: QObject(parent)
{
	this->m_currentPlayer = nullptr;
	this->m_pendingPlayer = nullptr;
	this->m_robotLeft = nullptr;
	this->m_robotRight = nullptr;
	this->m_lordPlayer = nullptr;
	this->m_user = nullptr;
	this->m_baseScore = 0;
}

GameControl::~GameControl()
{}

void GameControl::initPlayer()
{
	//初始化三个玩家对象
	this->m_robotLeft = new RobotPlayer("机器人1", this);//左机器人
	this->m_robotRight = new RobotPlayer("机器人2", this);//右机器人
	this->m_user = new UserPlayer("真人玩家", this);//真人玩家

	//设置性别，随机一个性别
	int gender=QRandomGenerator::global()->bounded(2);
	this->m_robotLeft->setGender(gender==0?(Player::Gender::Male):(Player::Gender::Female));
	gender = QRandomGenerator::global()->bounded(2);
	this->m_robotRight->setGender(gender==0?(Player::Gender::Male):(Player::Gender::Female));
	gender = QRandomGenerator::global()->bounded(2);
	this->m_user->setGender(gender == 0 ? (Player::Gender::Male) : (Player::Gender::Female));

	//设置玩家的座位位置
	this->m_robotLeft->setDirection(Player::Direction::Left);//左边机器人的人物位置在卡牌的左边
	this->m_user->setDirection(Player::Direction::Right);//中间用户玩家的人物位置在卡牌的右边
	this->m_robotRight->setDirection(Player::Direction::Right);//右边机器人的人物位置在卡牌的右边

	//设置每个玩家的上下家，总体循序为逆时针
		//左边机器人玩家的上家是右边机器人，下家是用户玩家
	this->m_robotLeft->setPrevPlayer(this->m_robotRight);
	this->m_robotLeft->setNextPlayer(this->m_user);
		//用户玩家的上家是左边机器人，下家是右边机器人
	this->m_user->setPrevPlayer(this->m_robotLeft);
	this->m_user->setNextPlayer(this->m_robotRight);
		//右边机器人玩家的上家是用户玩家，下家是左边机器人
	this->m_robotRight->setPrevPlayer(this->m_user);
	this->m_robotRight->setNextPlayer(this->m_robotLeft);

	//设置最开始的玩家为真人玩家，后可以随机
	this->m_currentPlayer = this->m_user;

	//接收玩家叫地主/抢地主信号，调用抢地主的槽函数
	connect(this->m_robotLeft, &Player::sigGrabLordBet, this, &GameControl::onPlayerGrabLordBet);
	connect(this->m_user, &Player::sigGrabLordBet, this, &GameControl::onPlayerGrabLordBet);
	connect(this->m_robotRight, &Player::sigGrabLordBet, this, &GameControl::onPlayerGrabLordBet);

	//接收玩家出牌信号，调用出牌的槽函数
	connect(this->m_robotLeft, &Player::sigPlayCards, this, &GameControl::onPlayerPlayCards);
	connect(this->m_user, &Player::sigPlayCards, this, &GameControl::onPlayerPlayCards);
	connect(this->m_robotRight, &Player::sigPlayCards, this, &GameControl::onPlayerPlayCards);
}

void GameControl::setCurrentPlayer(Player* curPlayer)
{
	this->m_currentPlayer = curPlayer;
}

Player* GameControl::getCurrentPlayer() const
{
	return this->m_currentPlayer;
}

Player* GameControl::getLordPlayer() const
{
	return this->m_lordPlayer;
}

RobotPlayer* GameControl::getRobotLeft() const
{
	return this->m_robotLeft;
}

RobotPlayer* GameControl::getRobotRight() const
{
	return this->m_robotRight;
}

UserPlayer* GameControl::getUser() const
{
	return this->m_user;
}

Player* GameControl::getPendingPlayer() const
{
	return this->m_pendingPlayer;
}

Cards GameControl::getPendingCards() const
{
	return this->m_pendingCards;
}

void GameControl::initAllCards()
{
	this->m_allCards.clear();
	//初始化非大小王牌
	for(Card::CardPoint point=Card::CardPoint::Card_3;point<=Card::CardPoint::Card_2;point=Card::CardPoint(point+1))
	{
		for(Card::CardSuit suit=Card::CardSuit::Hearts;suit<=Card::CardSuit::Spades;suit=Card::CardSuit(suit+1))
		{
			Card card(suit, point);
			this->m_allCards.add(card);
		}
	}
	//初始化大小王牌
	this->m_allCards.add(Card(Card::CardSuit::Suit_end, Card::CardPoint::Card_Joker));//小王
	this->m_allCards.add(Card(Card::CardSuit::Suit_end, Card::CardPoint::Card_BigJoker));//大王
}

Card GameControl::takeOneCard()
{
	return this->m_allCards.takeRandomCard();
}

Cards GameControl::getSurplusCards()
{
	if (this->m_allCards.size() != 3)
	{
		//TODO:抛出异常
	}
	return this->m_allCards;
}

void GameControl::resetCards()
{
	//清空玩家手牌
	this->m_robotLeft->clearHandCards();
	this->m_user->clearHandCards();
	this->m_robotRight->clearHandCards();
	//重新初始化牌堆
	this->initAllCards();
	//重置玩家状态
	this->m_pendingCards.clear();
	this->m_pendingPlayer = nullptr;
	this->m_currentPlayer = this->m_user;
}

void GameControl::startCallLord()
{
	//开始叫地主
	//这里在GamePannel类中初始化了最开始的当前玩家为用户玩家
	//用户玩家类没有实现这个函数，因为用户玩家的叫地主是通过界面交互实现的
	this->m_currentPlayer->prepareCallLord();
	//发出当前玩家状态转换为考虑叫地主信号
	emit this->sigPlayerStateChanged(this->m_currentPlayer, GameControl::PlayerState::ConsideringCallLord);
}

void GameControl::beLord(Player* lord,int baseScore)
{
	//设置游戏基础分
	this->m_baseScore = baseScore;
	//为地主和农民设置角色
	lord->setRole(Player::Role::Lord);
	lord->getPrevPlayer()->setRole(Player::Role::Farmer);
	lord->getNextPlayer()->setRole(Player::Role::Farmer);
	//将游戏控制的当前玩家设置为地主
	this->m_currentPlayer = lord;
	this->m_lordPlayer = lord;
	//地主获得三张地主牌，并准备出牌
	lord->storeDealCard(this->getSurplusCards());
	//等待1秒钟，完成由斗地主状态向出牌状态的转换
	QTimer::singleShot(2000, this, [=]()
		{
			emit this->sigGameStateChanged(GameControl::GameState::PlayingCards);
			//由于要做出牌动画，所以这里下面两行移植到GamePannel类中
			//emit this->sigPlayerStateChanged(this->m_currentPlayer, GameControl::PlayerState::ConsideringPlayCards);
			//this->m_currentPlayer->preparePlayCards();
		});
}

void GameControl::resetScore()
{
	this->m_robotLeft->setScore(0);
	this->m_user->setScore(0);
	this->m_robotRight->setScore(0);
}

void GameControl::onPlayerGrabLordBet(Player* player, int bet)
{
	//通知主界面玩家叫地主，主要是为了显示叫分
	//如果玩家放弃抢地主，也就是传入参数bet为0，或者赌注小于最高赌注，则判定为放弃抢地主
	if (bet == 0 || bet < this->m_grabLordData.m_highestBet)
	{
		emit this->sigPlayerGrabLordBet(player, 0,false);
	}
	else if (this->m_grabLordData.m_highestBet == 0 && bet > 0)//如果当前最高叫分为0，且玩家叫分大于0，则判定为第一次叫地主
	{
		emit this->sigPlayerGrabLordBet(player, bet, true);
	}
	else//第二第三次抢地主
	{
		emit this->sigPlayerGrabLordBet(player, bet, false);
	}

	this->m_grabLordData.m_callRound++;//叫地主轮数加1
	//如果叫了3分，直接成为地主
	if(bet==3)
	{
		this->beLord(player,bet);
		this->m_grabLordData.reset();
		return;
	}
	//如果叫分大于当前最高叫分，更新最高叫分和最高叫分玩家
	if(bet>this->m_grabLordData.m_highestBet)
	{
		this->m_grabLordData.m_highestBet = bet;
		this->m_grabLordData.m_highestBetPlayer = player;
	}
	//如果叫地主轮数小于3，继续下一个玩家叫地主
	if (this->m_grabLordData.m_callRound < 3)
	{
		//切换到下一个玩家
		this->m_currentPlayer = player->getNextPlayer();
		//通知主界面玩家状态转换为考虑叫地主
		emit this->sigPlayerStateChanged(this->m_currentPlayer, GameControl::PlayerState::ConsideringCallLord);
		//下一个玩家准备叫地主
		this->m_currentPlayer->prepareCallLord();
	}
	else
	{
		//如果三人都是零分，则重新发牌
		if (this->m_grabLordData.m_highestBet == 0)
		{
			this->resetCards();
			emit this->sigGameStateChanged(GameControl::GameState::DealingCards);
		}
		else
		{
			//否则最高叫分玩家成为地主
			this->beLord(this->m_grabLordData.m_highestBetPlayer, this->m_grabLordData.m_highestBet);
		}
		//重置叫地主数据
		this->m_grabLordData.reset();
	}
}

int GameControl::getCurHighestBet() const
{
	return this->m_grabLordData.m_highestBet;
}

void GameControl::onPlayerPlayCards(Player* player, const Cards& cards)
{
	//1.发送出牌信号给主界面，让主界面完成出牌动画，更新玩家手牌显示等
	emit this->sigPlayerPlayCards(player, cards);
	//2.如果不是过牌，则更新当前待处理牌和待处理牌玩家
	if (!cards.empty())
	{
		//更新游戏控制类的当前待处理牌和待处理牌玩家
		this->m_pendingCards = cards;
		this->m_pendingPlayer = player;
		//更新玩家类的当前待处理牌和待处理牌玩家
		this->m_robotLeft->setPending(this->m_pendingCards, this->m_pendingPlayer);
		this->m_user->setPending(this->m_pendingCards, this->m_pendingPlayer);
		this->m_robotRight->setPending(this->m_pendingCards, this->m_pendingPlayer);
	}
	//出的牌每有一个炸弹/王炸，基础分翻倍
	PlayHand ph(cards);
	if(ph.getType()==PlayHand::Hand_Bomb||ph.getType()==PlayHand::Hand_Bomb_Jokers)
	{
		this->m_baseScore *= 2;
	}
	//3.判断玩家是否出完牌，若出完牌则游戏结束
	if (player->getHandCards().empty())
	{
		Player* prevPlayer = player->getPrevPlayer();
		Player* nextPlayer = player->getNextPlayer();
		if (player->getRole() == Player::Role::Lord)
		{
			//地主获胜
			player->setIsWin(true);
			prevPlayer->setIsWin(false);
			nextPlayer->setIsWin(false);
			//地主得分增加，农民得分减少
			player->setScore(player->getScore() + this->m_baseScore * 2);
			prevPlayer->setScore(prevPlayer->getScore() - this->m_baseScore);
			nextPlayer->setScore(nextPlayer->getScore() - this->m_baseScore);
		}
		else
		{
			//农民获胜
			if(prevPlayer->getRole()==Player::Role::Lord)
			{
				player->setIsWin(true);
				nextPlayer->setIsWin(true);
				prevPlayer->setIsWin(false);

				player->setScore(player->getScore() + this->m_baseScore);
				nextPlayer->setScore(nextPlayer->getScore() + this->m_baseScore);
				prevPlayer->setScore(prevPlayer->getScore() - this->m_baseScore * 2);
			}
			else
			{
				player->setIsWin(true);
				prevPlayer->setIsWin(true);
				nextPlayer->setIsWin(false);

				player->setScore(player->getScore() + this->m_baseScore);
				prevPlayer->setScore(prevPlayer->getScore() + this->m_baseScore);
				nextPlayer->setScore(nextPlayer->getScore() - this->m_baseScore * 2);
			}
		}
		//清空当前待处理牌和待处理牌玩家
		this->m_pendingCards.clear();
		this->m_pendingPlayer = nullptr;
		this->m_robotLeft->setPending(this->m_pendingCards, this->m_pendingPlayer);
		this->m_user->setPending(this->m_pendingCards, this->m_pendingPlayer);
		this->m_robotRight->setPending(this->m_pendingCards, this->m_pendingPlayer);
		emit this->sigPlayerStateChanged(player, GameControl::PlayerState::Won);
		return;
	}
	//4.切换到下一个玩家，通知主界面玩家状态转换为考虑出牌
	this->m_currentPlayer = player->getNextPlayer();
	this->m_currentPlayer->preparePlayCards();
	emit this->sigPlayerStateChanged(this->m_currentPlayer, GameControl::PlayerState::ConsideringPlayCards);
}

