#include "GamePannel.h"
//由于原文件过大，分割为多个文件
//这个文件主要负责游戏的状态机，搭配游戏控制类GameControl实现游戏的流程控制
//分为游戏的状态切换，和玩家状态切换
//游戏状态包括发牌，叫地主，出牌
//玩家状态包括考虑叫地主，考虑出牌和胜利

void GamePannel::gameStateProcess(GameControl::GameState gameState)
{
	this->m_currentGameState = gameState;
	switch (gameState)
	{
	case GameControl::DealingCards:
		this->dealCardsProcess();
		break;
	case GameControl::CallingLord:
		this->callLordProcess();
		break;
	case GameControl::PlayingCards:
		this->playCardsProcess();
		break;
	default:
		break;
	}
}

void GamePannel::dealCardsProcess()
{
	//重置每张卡牌窗口的属性
	for (auto it : this->m_cardMap)
	{
		it->setFrontSide(true);
		it->setSelected(false);
		it->hide();
	}
	//隐藏三张地主牌
	for (auto& it : this->m_lordCardPanelList)
	{
		it->hide();
	}
	//重置玩家上下文环境
	int userIndex = this->m_playerList.indexOf(this->m_gameControl->getUser());//获取用户玩家的索引
	for (int i = 0;i < this->m_playerList.size();i++)
	{
		//重置手牌朝向
		this->m_playerContextMap[this->m_playerList[i]].isFrontSide = (i == userIndex) ? true : false;
		//隐藏玩家提示信息
		this->m_playerContextMap[this->m_playerList[i]].infoLabel->hide();
		//隐藏玩家头像
		this->m_playerContextMap[this->m_playerList[i]].roleImage->hide();
		//清空玩家上次打出的牌
		this->m_playerContextMap[this->m_playerList[i]].lastPlayCards.clear();
	}
	//重置所有玩家卡牌数据
	this->m_gameControl->resetCards();
	//显示底牌
	this->m_baseCardPanel->show();
	//隐藏按钮组
	this->ui.buttonGroup->setCurrentPage(ButtonGroup::EmptyPage);
	//完成发牌动画
	this->startDealCardAnimation();

	//发牌音效
	//TODO
}

void GamePannel::callLordProcess()
{
	//给地主牌设置牌属性和正面图片
	QVector<Card> lordCards = this->m_gameControl->getSurplusCards().toOrderlyList();
	for (int i = 0;i < lordCards.size();i++)
	{
		this->m_lordCardPanelList[i]->setCard(lordCards[i]);
		this->m_lordCardPanelList[i]->setFrontSide(false);
		this->m_lordCardPanelList[i]->setImages(this->m_cardMap[lordCards[i]]->getFrontImage(), this->m_cardBackImage);
	}
	//开始叫地主
	this->m_gameControl->startCallLord();
}

void GamePannel::playCardsProcess()
{
	//1.隐藏窗口的所有提示信息
	//隐藏叫分动画
	this->m_animation->hide();
	//隐藏玩家叫地主提示
	for (auto it : this->m_playerContextMap)
	{
		it.infoLabel->hide();
	}

	//2.将地主牌翻为正面
	for (auto& it : this->m_lordCardPanelList)
	{
		it->setFrontSide(true);
		it->update();
	}

	//3.完成地主牌移向玩家手牌的动画
	this->startLordToHandAnimation();
	//下面的环节已经改为在startLordToHandAnimation函数中的动画完成槽函数中处理


	//4.更新地主玩家手牌显示
	//this->updateHandCardsPanel(this->m_gameControl->getLordPlayer());

	//5.显示各个玩家头像
	//this->showPlayerRoleImage();

	//6.切换玩家状态并且让当前玩家出牌
	//this->playerStateProcess(this->m_gameControl->getCurrentPlayer(), GameControl::PlayerState::ConsideringPlayCards);
	//this->m_gameControl->getCurrentPlayer()->preparePlayCards();
}

void GamePannel::playerStateProcess(Player* player, GameControl::PlayerState playerState)
{
	switch (playerState)
	{
	case GameControl::ConsideringCallLord:
		//如果是用户玩家，则显示按钮组的叫地主页面
		if (player == this->m_gameControl->getUser())
		{
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::CallAndGrabPage, this->m_gameControl->getCurHighestBet());
		}
		break;
	case GameControl::ConsideringPlayCards:
	{
		if (player == this->m_gameControl->getUser())
		{
			Player* pendingPlayer = player->getPendingPlayer();
			if (pendingPlayer == nullptr || player == pendingPlayer)//如果玩家是优先出牌的玩家
			{
				this->ui.buttonGroup->setCurrentPage(ButtonGroup::PageIndex::OnlyPlayCardsPage);
			}
			else
			{
				this->ui.buttonGroup->setCurrentPage(ButtonGroup::PageIndex::PlayAndRefusePage);
				//启动用户出牌倒计时
				this->m_userTicking->startTicking();
			}
		}
		else
		{
			//如果是机器人玩家，则隐藏按钮组
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::EmptyPage);
			//TODO:做完出牌后再完成这部分
		}
		break;
	}
	case GameControl::Won:
	{
		//把全场玩家牌面显示出来
		this->m_playerContextMap[this->m_gameControl->getRobotLeft()].isFrontSide = true;
		this->m_playerContextMap[this->m_gameControl->getRobotRight()].isFrontSide = true;
		this->updateHandCardsPanel(this->m_gameControl->getRobotLeft());
		this->updateHandCardsPanel(this->m_gameControl->getRobotRight());
		this->updatePlayerScores();
		this->m_gameControl->setCurrentPlayer(player);
		//显示胜利动画
		this->showEndingPanel();
		break;
	}
	default:
		break;
	}
}