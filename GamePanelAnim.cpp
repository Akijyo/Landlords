#include "GamePannel.h"
//由于原文件过大，分割为多个文件
//这个文件主要实现主窗口的牌移动动画，已经显示抢地主分数，炸弹特效等动画效果

/////////////////////////////////////////////////////////////////////////////////// <summary>
/// 三个牌移动的动画函数
/////////////////////////////////////////////////////////////////////////////////// </summary>
void GamePannel::startDealCardAnimation()
{
	QPropertyAnimation* ani = new QPropertyAnimation(this->m_moveCardPanel, "pos", this);
	//找到卡牌动画的末尾点
	Player* curPlayer = this->m_gameControl->getCurrentPlayer();
	int idx = this->m_playerList.indexOf(curPlayer);
	QPoint endPos;
	const QRect area = this->m_playerContextMap[curPlayer].cardAreaRect;
	switch (idx)
	{
	case 0: // 左机器人：水平向左
	{
		endPos.setX(area.right() - this->cardSize.width());
		endPos.setY(this->m_baseCardPos.y());
		break;
	}
	case 1: // 用户：垂直向下
	{
		endPos.setX(this->m_baseCardPos.x());
		endPos.setY(area.top() - this->cardSize.height());
		break;
	}
	case 2: // 右机器人：水平向右
	{
		endPos.setX(area.left());
		endPos.setY(this->m_baseCardPos.y());
		break;
	}
	default:
		break;
	}

	//设置动画持续时间
	ani->setDuration(140);
	//设置动画开始位置
	ani->setStartValue(this->m_baseCardPos);
	//设置动画结束位置
	ani->setEndValue(endPos);
	//设置动画曲线
	ani->setEasingCurve(QEasingCurve::Linear);
	//将移动牌放入起始位置
	this->m_moveCardPanel->move(this->m_baseCardPos);
	this->m_moveCardPanel->show();//显示移动牌
	this->m_moveCardPanel->raise();//将移动牌放到最上层

	connect(ani, &QPropertyAnimation::finished, this, [=]()//一段动画完成
		{
			//隐藏移动牌
			this->m_moveCardPanel->hide();
			//从牌堆中取出一张牌
			Card c = this->m_gameControl->takeOneCard();
			//将这张牌存入玩家手牌
			curPlayer->storeDealCard(c);
			//将这张牌对应的卡牌窗口移动到玩家手牌显示区域
			Cards hcs;
			hcs << c;
			this->disposHandCards(curPlayer, hcs);
			//切换玩家
			this->m_gameControl->setCurrentPlayer(curPlayer->getNextPlayer());
			//如果牌已经发完
			if (this->m_gameControl->getSurplusCards().size() == 3)
			{
				//复位移动牌并且隐藏
				this->m_moveCardPanel->move(this->m_baseCardPos);
				this->m_moveCardPanel->hide();
				//完成地主牌移动动画
				//在startBaseToLordAnimation函数中切换到叫地主状态
				this->startBaseToLordAnimation();
				return;
			}
			//继续下一张牌的动画
			this->startDealCardAnimation();
		});
	ani->start(QAbstractAnimation::DeleteWhenStopped);//动画结束后自动删除
}

void GamePannel::startBaseToLordAnimation()
{
	//隐藏基础牌
	this->m_baseCardPanel->hide();
	// 确保有三张临时移动牌
	if (this->m_lordMovingPanels.size() < 3)
	{
		// 懒创建三张背面牌，用于飞行动画
		for (int i = 0; i < 3; ++i)
		{
			CardPanel* cp = new CardPanel(this);
			cp->setImages(this->m_cardBackImage, this->m_cardBackImage);
			cp->setFrontSide(false);
			cp->move(this->m_baseCardPos);
			cp->show();
			cp->raise();
			this->m_lordMovingPanels.append(cp);
		}
	}
	//开始实现三张牌飞行动画

	//计算出从左到右三张牌的目标位置 
	int baseX = (this->width() - (3 * this->cardSize.width() + 2 * 10)) / 2;
	QVector<QPoint> endPosList;
	for (int i = 0;i < 3;i++)
	{
		endPosList.append(QPoint(baseX + i * (this->cardSize.width() + 10), 20));
	}
	//创建并启动三张牌的飞行动画
	QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
	for (int i = 0;i < this->m_lordMovingPanels.size();i++)
	{
		QPropertyAnimation* ani = new QPropertyAnimation(this->m_lordMovingPanels[i], "pos", this);
		ani->setDuration(200);
		ani->setStartValue(this->m_baseCardPos);
		ani->setEndValue(endPosList[i]);
		ani->setEasingCurve(QEasingCurve::Linear);
		group->addAnimation(ani);
	}
	connect(group, &QParallelAnimationGroup::finished, this, [=]()
		{
			//隐藏移动牌
			for (auto* cp : this->m_lordMovingPanels)
			{
				cp->hide();
			}
			//显示三张地主牌的背面
			QVector<Card> lordCards = this->m_gameControl->getSurplusCards().toOrderlyList();
			for (int i = 0;i < lordCards.size();i++)
			{
				this->m_lordCardPanelList[i]->show();
			}
			//等待500毫秒切换到叫地主状态
			QTimer::singleShot(500, this, [=]()
				{
					this->gameStateProcess(GameControl::GameState::CallingLord);
				});
		});
	group->start(QAbstractAnimation::DeleteWhenStopped);//动画结束后自动删除
}

void GamePannel::startLordToHandAnimation()
{
	//将要移动的地主牌从左到右依次翻面
	for (int i = 0;i < this->m_lordMovingPanels.size();i++)
	{
		//获取这个位置地主牌的正面图片
		this->m_lordMovingPanels[i]->setFrontSide(true);
		this->m_lordMovingPanels[i]->setImages(this->m_lordCardPanelList[i]->getFrontImage(), this->m_background);
		this->m_lordMovingPanels[i]->show();
		this->m_lordMovingPanels[i]->raise();
	}
	//计算出从左到右三张牌的目标位置
	Player* lordPlayer = this->m_gameControl->getLordPlayer();
	int idx = this->m_playerList.indexOf(lordPlayer);
	QPoint endPos;
	const QRect area = this->m_playerContextMap[lordPlayer].cardAreaRect;

	switch (idx)
	{
	case 0: // 左机器人：水平向左
	{
		endPos.setX(area.right() - this->cardSize.width());
		endPos.setY(this->m_baseCardPos.y());
		break;
	}
	case 1: // 用户：垂直向下
	{
		endPos.setX(this->m_baseCardPos.x());
		endPos.setY(area.top() - this->cardSize.height());
		break;
	}
	case 2: // 右机器人：水平向右
	{
		endPos.setX(area.left());
		endPos.setY(this->m_baseCardPos.y());
		break;
	}
	default:
		break;
	}
	//创建并启动三张牌的飞行动画

	QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
	for (int i = 0;i < this->m_lordMovingPanels.size();i++)
	{
		QPropertyAnimation* ani = new QPropertyAnimation(this->m_lordMovingPanels[i], "pos", this);
		ani->setDuration(250);
		ani->setStartValue(this->m_lordMovingPanels[i]->pos());
		//每张牌依次错开10像素
		ani->setEndValue(endPos);
		ani->setEasingCurve(QEasingCurve::Linear);
		group->addAnimation(ani);
	}
	connect(group, &QParallelAnimationGroup::finished, this, [=]()
		{
			//隐藏移动牌
			for (auto* cp : this->m_lordMovingPanels)
			{
				cp->hide();
			}
			
			//4.更新地主玩家手牌显示,//为地主玩家的三张地主牌设置拥有者
			//this->updateHandCardsPanel(this->m_gameControl->getLordPlayer());
			Cards hcs = this->m_gameControl->getSurplusCards();
			this->disposHandCards(this->m_gameControl->getLordPlayer(), hcs);

			//5.显示各个玩家头像
			this->showPlayerRoleImage();

			//6.切换玩家状态并且让当前玩家出牌
			this->playerStateProcess(this->m_gameControl->getCurrentPlayer(), GameControl::PlayerState::ConsideringPlayCards);
			this->m_gameControl->getCurrentPlayer()->preparePlayCards();
		});
	group->start(QAbstractAnimation::DeleteWhenStopped);//动画结束后自动删除
}
/////////////////////////////////////////////////////////////////////////////////// <summary>
/// 三个牌移动的动画函数
/////////////////////////////////////////////////////////////////////////////////// </summary>

//startLordToHandAnimation函数里调用（子函数），在地主牌移动到地主玩家手牌完成后，显示玩家头像
void GamePannel::showPlayerRoleImage()
{
	QVector<QString> lordMale;
	QVector<QString> lordFemale;
	QVector<QString> farmerMale;
	QVector<QString> farmerFemale;
	lordMale << ":/Player/images/lord_man_1.png" << ":/Player/images/lord_man_2.png";
	lordFemale << ":/Player/images/lord_woman_1.png" << ":/Player/images/lord_woman_2.png";
	farmerMale << ":/Player/images/farmer_man_1.png" << ":/Player/images/farmer_man_2.png";
	farmerFemale << ":/Player/images/farmer_woman_1.png" << ":/Player/images/farmer_woman_2.png";
	for (auto it : this->m_playerList)
	{
		QImage img;
		QPixmap pixmap;
		int rand = QRandomGenerator::global()->bounded(lordMale.size());
		if (it == this->m_gameControl->getLordPlayer())
		{
			if (it->getGender() == Player::Gender::Male)
			{
				img.load(lordMale[rand]);
			}
			else
			{
				img.load(lordFemale[rand]);
			}
		}
		else
		{
			if (it->getGender() == Player::Gender::Male)
			{
				img.load(farmerMale[rand]);
			}
			else
			{
				img.load(farmerFemale[rand]);
			}
		}
		if (it->getDirection() == Player::Direction::Right)
		{
			img = img.mirrored(true, false);
		}
		pixmap = QPixmap::fromImage(img);
		this->m_playerContextMap[it].roleImage->setPixmap(pixmap);
		this->m_playerContextMap[it].roleImage->show();
	}
}





//特效动画
void GamePannel::showAnimation(WindowAnimation type, int bet)
{
	switch (type)
	{
	case GamePannel::BetScore:
	{
		//固定为分数图片的大小
		this->m_animation->setFixedSize(160, 98);
		//确定地主分数显示位置
		this->m_animation->move((this->width() - this->m_animation->width()) / 2, (this->height() - this->m_animation->height()) / 2 - 65);
		//显示
		this->m_animation->showBetAnimation(bet);
		break;
	}
	case GamePannel::Joker:
	{
		//固定为王炸图片的大小
		this->m_animation->setFixedSize(250, 200);
		//确定王炸显示位置
		this->m_animation->move((this->width() - this->m_animation->width()) / 2, (this->height() - this->m_animation->height()) / 2 - 70);
		//显示
		this->m_animation->showJokerAnimation();
	}
		break;
	case GamePannel::Bomb:
	{
		//固定为炸弹图片的大小
		this->m_animation->setFixedSize(180, 200);
		//确定炸弹显示位置
		this->m_animation->move((this->width() - this->m_animation->width()) / 2, (this->height() - this->m_animation->height()) / 2 - 70);
		//显示
		this->m_animation->showBombAnimation();
	}
		break;
	case GamePannel::Plane:
	{
		//固定为飞机图片的大小
		this->m_animation->setFixedSize(800, 75);
		//确定飞机显示位置
		this->m_animation->move((this->width() - this->m_animation->width()) / 2, 200);
		//显示
		this->m_animation->showPlaneAnimation();
	}
		break;
	case GamePannel::SeqSingle:
	{
		//固定位顺子图片的大小
		this->m_animation->setFixedSize(250, 150);
		//确定顺子显示位置
		this->m_animation->move((this->width() - this->m_animation->width()) / 2, 200);
		//显示
		this->m_animation->showSeqSingleAnimation();
	}
		break;
	case GamePannel::SeqPair:
	{
		//固定为连对图片的大小
		this->m_animation->setFixedSize(250, 150);
		//确定连对显示位置
		this->m_animation->move((this->width() - this->m_animation->width()) / 2, 200);
		//显示
		this->m_animation->showSeqPairAnimation();
	}
		break;
	default:
		break;
	}
	this->m_animation->show();
	this->m_animation->raise();
}

void GamePannel::showEndingPanel()
{
	Player* user = this->m_gameControl->getUser();
	this->m_endingPanel = new EndingPanel(user->getRole() == Player::Role::Lord, user->isWin(), this);

	//移动到窗口上面先隐藏，方便动画
	this->m_endingPanel->move((this->width() - this->m_endingPanel->width()) / 2, -this->m_endingPanel->height());
	this->m_endingPanel->show();
	this->m_endingPanel->raise();
	this->m_endingPanel->setScores(user->getPrevPlayer()->getScore(), user->getScore(), user->getNextPlayer()->getScore());
	//创建动画
	QPropertyAnimation* anim = new QPropertyAnimation(this->m_endingPanel, "pos", this);
	anim->setDuration(1500);
	anim->setStartValue(QPoint((this->width() - this->m_endingPanel->width()) / 2, -this->m_endingPanel->height()));
	//结束位置是窗口正中央
	anim->setEndValue(QPoint((this->width() - this->m_endingPanel->width()) / 2, (this->height() - this->m_endingPanel->height()) / 2));
	//弹跳曲线
	anim->setEasingCurve(QEasingCurve::OutBounce);
	//开启动画
	anim->start();
	//连接信号槽，点击重新开始按钮
	connect(this->m_endingPanel, &EndingPanel::continueGame, this, [=]()
		{
			//销毁结算面板
			this->m_endingPanel->deleteLater();
			this->m_endingPanel = nullptr;
			//销毁动画对象
			anim->deleteLater();
			//重新开始游戏
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::EmptyPage);
			this->gameStateProcess(GameControl::GameState::DealingCards);
		});
}
