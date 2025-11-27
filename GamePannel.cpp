#include "GamePannel.h"

//构造函数
GamePannel::GamePannel(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	//主窗口初始化工作
	//1.设置背景
	int bgIndex = QRandomGenerator::global()->bounded(10);//随机生成0-9的整数，共有10张背景图片
	QString bgPath = QString(":/Background/images/background-%1.png").arg(bgIndex+1);//背景图片路径
	m_background.load(bgPath);//加载背景图片，主窗口初始化完后会触发paintEvent事件，绘制背景图片

	//2.设置标题和窗口大小
	this->setWindowTitle("欢乐斗地主");//设置标题
	this->setFixedSize(1000, 650);//设置窗口大小，固定大小
	this->ui.mainToolBar->hide();//隐藏工具栏

	//3.实例化游戏控制类对象
	this->initGameControl();
	
	//4.玩家得分面板初始化
	this->updatePlayerScores();//初始化为0分

	//5.切割卡牌图片
	this->initCardsData();

	//6.初始化游戏中的按钮组
	this->initButtonGroup();

	//7.初始化玩家的上下文环境
	this->initPlayerContext();

	//8.初始化开始游戏前的卡牌场景
	this->initCardsScene();

	//9.初始化用户倒计时窗口
	this->initUserTicking();

	//初始化动画类
	this->m_animation = new Animation(this);

	//初始化拖选功能的成员变量
	this->m_dragActive = false;
	this->m_dragTargetSelected = false;

	//为主窗口安装事件过滤器
	this->installEventFilter(this);

	this->m_endingPanel = nullptr;
}

GamePannel::~GamePannel()
{}

//绘图事件，主要用于绘制主窗口背景
void GamePannel::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.drawPixmap(this->rect(), this->m_background);
}

//实现拖选功能
bool GamePannel::eventFilter(QObject* obj, QEvent* event)
{
	// 只关心鼠标相关事件
	if (event->type() != QEvent::MouseButtonPress &&
		event->type() != QEvent::MouseButtonDblClick &&
		event->type() != QEvent::MouseMove &&
		event->type() != QEvent::MouseButtonRelease)
	{
		return QMainWindow::eventFilter(obj, event);
	}
	//如果当前游戏状态不是出牌状态，直接返回
	if (this->m_currentGameState != GameControl::GameState::PlayingCards)
	{
		return QMainWindow::eventFilter(obj, event);
	}
	//如果当前按钮页面不是出牌页面，直接返回
	if (this->ui.buttonGroup->getCurrentPage() != ButtonGroup::OnlyPlayCardsPage &&
		this->ui.buttonGroup->getCurrentPage() != ButtonGroup::PlayAndRefusePage)
	{
		return QMainWindow::eventFilter(obj, event);
	}
	CardPanel* cardPanel = qobject_cast<CardPanel*>(obj);
	const bool isMainWindow = (obj == this);
	
	if (cardPanel != nullptr)//在qobject_cast<CardPanel*>(obj);中，如何事件目标对象不是CardPanel类型，则返回nullptr
	{
		//仅允许用户玩家的手牌窗口可以被选
		if (cardPanel->getOwner() != this->m_gameControl->getUser())
		{
			return QMainWindow::eventFilter(obj, event);
		}
		switch (event->type())
		{
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonDblClick:
			{
				//处理左键按下事件
				auto* me = static_cast<QMouseEvent*>(event);
				if (me->button() != Qt::LeftButton)
				{
					break;
				}
				//这里事件是仅仅点击，不算进入拖选模式
				this->m_curSelCard = cardPanel;//把当前点击的牌保存，用于后面拖选进行辅助判断
				this->m_dragActive = false;
				this->m_dragTargetSelected = !cardPanel->isSelected();
				//处理这张牌
				cardPanel->setSelected(this->m_dragTargetSelected);//设置选中状态
				if (this->m_dragTargetSelected)
				{
					this->m_selectedCardSet.insert(cardPanel);//如果是选中，加入选中集合
				}
				else
				{
					this->m_selectedCardSet.remove(cardPanel);//如果是取消选中，从选中集合移除
				}
				//更新用户玩家手牌窗口
				this->updateHandCardsPanel(this->m_gameControl->getUser());
				return true; // 事件已处理
			}
			case QEvent::MouseMove:
			{
				auto* me = static_cast<QMouseEvent*>(event);
				if ((me->buttons() & Qt::LeftButton) == 0)
				{
					break;
				}
				//在主窗口全局坐标系中获取鼠标位置
				QPoint local = this->mapFromGlobal(me->globalPosition().toPoint());
				//由于卡牌窗口叠加显示在主窗口的最上层，所以按住的那个位置的鼠标在主窗口坐标系中命中的窗口就是卡牌窗口
				auto* over = qobject_cast<CardPanel*>(this->childAt(local));
				if (!this->m_dragActive && over != nullptr && over != this->m_curSelCard)
				{
					//如果当前按住鼠标左键，并且鼠标悬停在某个卡牌窗口上，并且这个卡牌窗口不是当前点击的卡牌窗口
					//进入拖选模式
					this->m_dragActive = true;
				}
				if (this->m_dragActive)//拖选模式逻辑处理
				{
					if (over == nullptr)//如果拖到牌外，直接返回
					{
						break;
					}
					//仅允许用户玩家的手牌窗口可以被选
					if (over->getOwner() != this->m_gameControl->getUser())
					{
						return QMainWindow::eventFilter(obj, event);
					}
					this->m_dragTargetSelected = !over->isSelected();
					//拖选模式
					if (over != this->m_curSelCard)
					{
						//如果上一次处理的牌不是当前悬停的牌
						this->m_curSelCard = over;//更新当前选中的牌
						over->setSelected(this->m_dragTargetSelected);//设置选中状态
						if (this->m_dragTargetSelected)
						{
							this->m_selectedCardSet.insert(over);//如果是选中，加入选中集合
						}
						else
						{
							this->m_selectedCardSet.remove(over);//如果是取消选中，从选中集合移除
						}
						//更新用户玩家手牌窗口
						this->updateHandCardsPanel(this->m_gameControl->getUser());
					}
				}
				return true; // 事件已处理
			}
			case QEvent::MouseButtonRelease:
			{
				//处理鼠标左键释放事件
				auto* me = static_cast<QMouseEvent*>(event);
				if (me->button() != Qt::LeftButton)
				{
					break;
				}
				//结束拖选模式
				this->m_dragActive = false;
				this->m_curSelCard = nullptr;
				return true; // 事件已处理
			}
			default:
				break;
		}
		return QMainWindow::eventFilter(obj, event);
	}
	// 情况二：事件来自主窗口（用于“从牌外拖入牌内”的拖选）
	if (isMainWindow)
	{
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			auto* me = static_cast<QMouseEvent*>(event);
			if (me->button() != Qt::LeftButton)
			{
				break;
			}
			// 在牌外按下：不改变任何牌状态，仅重置拖选状态
			this->m_dragActive = false;
			this->m_curSelCard = nullptr;
			// 不拦截，允许主窗口其它行为（如果有）
			return QMainWindow::eventFilter(obj, event);
		}
		case QEvent::MouseMove://从牌外拖入牌内的核心逻辑
		{
			auto* me = static_cast<QMouseEvent*>(event);
			if ((me->buttons() & Qt::LeftButton) == 0)
			{
				break;
			}

			// 从主窗口坐标命中卡牌
			QPoint local = this->mapFromGlobal(me->globalPosition().toPoint());
			auto* over = qobject_cast<CardPanel*>(this->childAt(local));
			if (over == nullptr)
			{
				break;
			}
			// 仅允许用户玩家的卡牌
			if (over->getOwner() != this->m_gameControl->getUser())
			{
				break;
			}

			if (!this->m_dragActive && over != this->m_curSelCard)
			{
				this->m_dragActive = true; // 进入拖选模式
			}
			if (this->m_dragActive)
			{
				this->m_dragTargetSelected = !over->isSelected();

				if (over != this->m_curSelCard)
				{
					this->m_curSelCard = over;
					over->setSelected(this->m_dragTargetSelected);
					if (this->m_dragTargetSelected)
					{
						this->m_selectedCardSet.insert(over);
					}
					else
					{
						this->m_selectedCardSet.remove(over);
					}
					this->updateHandCardsPanel(this->m_gameControl->getUser());
				}
				return true; // 我们已经处理了拖选
			}
			break;
		}
		case QEvent::MouseButtonRelease:
		{
			auto* me = static_cast<QMouseEvent*>(event);
			if (me->button() != Qt::LeftButton)
			{
				break;
			}
			this->m_dragActive = false;
			this->m_curSelCard = nullptr;
			return true;
		}
		default:
			break;
		}
		// 其它主窗口事件按默认处理
		return QMainWindow::eventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}


//更新玩家得分面板
void GamePannel::updatePlayerScores()
{
	this->ui.scorePanel->setScores(this->m_playerList[0]->getScore(), this->m_playerList[1]->getScore(), this->m_playerList[2]->getScore());
}

//这个函数发生在发牌动画之后，当一张牌发给某个玩家后，调用这个函数更新玩家手牌
void GamePannel::disposHandCards(Player* player, const Cards& hcards)
{
	//将发出的牌设置给对应拥有者
	QVector<Card> hcList = hcards.toOrderlyList();
	for (int i = 0;i < hcList.size();i++)
	{
		CardPanel* cp = this->m_cardMap[hcList[i]];
		cp->setOwner(player);
	}
	//调用更新玩家手牌窗口位置的通用函数
	this->updateHandCardsPanel(player);
}

//更新玩家手牌窗口位置的通用函数，一更新就是整个窗口
void GamePannel::updateHandCardsPanel(Player* player)
{
	Cards hcs = player->getHandCards();
	QVector<Card> hcList = hcs.toOrderlyList();
	//由于卡牌是堆叠放置，设定一个偏移量
	int space = 20;//卡牌间隔20像素
	QRect cardAreaRect = this->m_playerContextMap[player].cardAreaRect;//玩家手牌显示区域
	for (int i = 0;i < hcList.size();i++)
	{
		CardPanel* cp = this->m_cardMap[hcList[i]];
		cp->setFrontSide(this->m_playerContextMap[player].isFrontSide);//设置卡牌正反面
		//计算水平与重置摆放的移动位置
		if (this->m_playerContextMap[player].cardAlign == PlayerContext::CardAlign::Horizontal)//用户
		{
			//计算x轴起始位置，计算方式是：区域左侧+（区域宽度-（卡牌数-1）*间隔-卡牌宽度）/2
			int x = cardAreaRect.left() + (cardAreaRect.width() - (hcList.size() - 1) * space - this->cardSize.width()) / 2;
			//计算y轴位置，计算方式是：区域顶部+（区域高度-卡牌高度）/2
			int y = cardAreaRect.top() + (cardAreaRect.height() - this->cardSize.height()) / 2;
			//如果这个
			if (cp->isSelected())
			{
				y -= 10;//如果被选中，y轴上移10像素
			}
			cp->move(x + i * space, y);//移动卡牌
		}
		else//机器人
		{
			//计算x轴位置，计算方式是：区域左侧+（区域宽度-卡牌宽度）/2
			int x = cardAreaRect.left() + (cardAreaRect.width() - this->cardSize.width()) / 2;
			//计算y轴起始位置，计算方式是：区域顶部+（区域高度-（卡牌数-1）*间隔-卡牌高度）/2
			int y = cardAreaRect.top() + (cardAreaRect.height() - (hcList.size() - 1) * space - this->cardSize.height()) / 2;
			cp->move(x, y + i * space);//移动卡牌
		}
		//显示卡牌
		cp->show();
		//设置卡牌为最上层
		cp->raise();
	}
}

//这个抢地主过程的提示函数发生在GameControl类发出叫地主/抢地主信号后
//这个过程的connect在GamePanelInit文件中的initGameControl函数里
void GamePannel::onPlayerGrabLordBet(Player* player, int bet, bool first)
{
	//如果赌注为0，窗口提示不抢
	if (bet == 0)
	{
		this->m_playerContextMap[player].infoLabel->setPixmap(QPixmap(":/Animation/images/buqinag.png"));
	}
	else
	{
		if (first)
		{
			this->m_playerContextMap[player].infoLabel->setPixmap(QPixmap(":/Animation/images/jiaodizhu.png"));
		}
		else
		{
			this->m_playerContextMap[player].infoLabel->setPixmap(QPixmap(":/Animation/images/qiangdizhu.png"));
		}
	}
	this->m_playerContextMap[player].infoLabel->show();

	//TODO
	//显示叫地主分数
	if (bet != 0)
	{
		this->showAnimation(GamePannel::WindowAnimation::BetScore, bet);
	}
	//叫地主音效
}

//同上面函数，GameControl类发出玩家出牌信号后，会对主窗口进行一系列处理
//这个过程的connect在GamePanelInit文件中的initGameControl函数里
void GamePannel::onResponsePlayCards(Player* player, const Cards& playCards)
{
	//1.隐藏上一位玩家不出提示
	Player* prePlayer = player->getPrevPlayer();
	this->m_playerContextMap[prePlayer].infoLabel->hide();
	//2.如果出牌为空，提示不出
	if (playCards.empty())
	{
		this->m_playerContextMap[player].infoLabel->setPixmap(QPixmap(":/Animation/images/pass.png"));
		this->m_playerContextMap[player].infoLabel->show();
		return;
	}
	//下面是出牌不为空的处理
	Player* pendingPlayer = player->getPendingPlayer();

	//隐藏当前待处理玩家的出牌提示
	if (pendingPlayer!=nullptr&&!this->m_playerContextMap[pendingPlayer].lastPlayCards.empty())
	{
		QVector<Card> cardList = this->m_playerContextMap[pendingPlayer].lastPlayCards.toOrderlyList();
		for (int i = 0;i < cardList.size();i++)
		{
			CardPanel* cp = this->m_cardMap[cardList[i]];
			cp->hide();
		}
	}



	//3.移动出牌到出牌区域，并且更新玩家手牌
	this->m_playerContextMap[player].lastPlayCards = playCards;
	
	QRect playAreaRect = this->m_playerContextMap[player].playAreaRect;//玩家出牌显示区域
	QVector<Card> vcards = playCards.toOrderlyList(Cards::SortType::Display);
	//由于卡牌是堆叠放置，设定一个偏移量
	int space = 24;//卡牌间隔20像素
	for (int i = 0;i < vcards.size();i++)
	{
		CardPanel* cp = this->m_cardMap[vcards[i]];
		cp->setFrontSide(true);//出牌都是正面
		if(this->m_playerContextMap[player].cardAlign==PlayerContext::CardAlign::Horizontal)
		{
			int x = playAreaRect.left() + (playAreaRect.width() - (vcards.size() - 1) * space - this->cardSize.width()) / 2;
			int y = playAreaRect.top() + (playAreaRect.height() - this->cardSize.height()) / 2;
			cp->move(x + i * space, y);//移动卡牌
		}
		else
		{
			int x = playAreaRect.left() + (playAreaRect.width() - this->cardSize.width()) / 2;
			int y = playAreaRect.top() + (playAreaRect.height() - (vcards.size() - 1) * space - this->cardSize.height()) / 2;
			cp->move(x, y + i * space);//移动卡牌
		}
		cp->show();
		cp->raise();
	}

	//更新玩家手牌
	this->updateHandCardsPanel(player);

	//4.实现炸弹等动画
	PlayHand ph(playCards);
	PlayHand::HandType type = ph.getType();
	switch (type)
	{
		case PlayHand::Hand_Plane:
		case PlayHand::Hand_Plane_With_Singles:
		case PlayHand::Hand_Plane_With_Pairs:
		{
			this->showAnimation(GamePannel::WindowAnimation::Plane);
			break;
		}
		case PlayHand::Hand_Seq_Pair:
		{
			this->showAnimation(GamePannel::WindowAnimation::SeqPair);
			break;
		}
		case PlayHand::Hand_Seq_Single:
		{
			this->showAnimation(GamePannel::WindowAnimation::SeqSingle);
			break;
		}
		case PlayHand::Hand_Bomb:
		{
			this->showAnimation(GamePannel::WindowAnimation::Bomb);
			break;
		}
		case PlayHand::Hand_Bomb_Jokers:
		{
			this->showAnimation(GamePannel::WindowAnimation::Joker);
			break;
		}
		default:
			break;
	}

	//5.TODO:播放音乐
}

void GamePannel::onUserPlayCards()
{
	//如果当前游戏状态不是出牌状态，直接返回
	//如果当前玩家不是用户玩家，直接返回
	//如果出牌为空，直接返回
	if (this->m_currentGameState != GameControl::GameState::PlayingCards
		|| this->m_gameControl->getCurrentPlayer() != this->m_gameControl->getUser()
		|| this->m_selectedCardSet.isEmpty())
	{
		return;
	}
	Cards playcards;
	for (auto it : this->m_selectedCardSet)
	{
		playcards.add(it->getCard());
	}
	//判断出牌是否是合法牌型
	PlayHand ph(playcards);
	PlayHand::HandType type = ph.getType();
	if(type==PlayHand::Hand_Unknown)
	{
		//TODO:可以做一个出牌不合法的特效提示
		return;
	}
	//判断出牌能否大过上家
	Player* user = this->m_gameControl->getUser();
	Player* pendingPlayer = this->m_gameControl->getPendingPlayer();
	Cards pendingCards = this->m_gameControl->getPendingCards();
	if (pendingPlayer != user && !ph.canBeat(pendingCards))
	{
		//TODO:可以做一个大不过上家的提示
		return;
	}
	//关闭用户出牌倒计时
	this->m_userTicking->stopTicking();

	//合法出牌，调用玩家类出牌函数
	//玩家的出牌类会清除手牌中已经打出去的牌，并且发射出牌信号给GameControl类，里面的onPlayerPlayCards函数被调用，
	// 然后又会发射出玩家出牌信号给主窗口的onResponsePlayCards函数，还会发射玩家状态变更信号给主窗口的onPlayerStateChanged函数
	user->playCards(playcards);
	//清空选中集合
	this->m_selectedCardSet.clear();
}

void GamePannel::onUserPassCards()
{
	//如果当前游戏状态不是出牌状态，直接返回
	//如果当前玩家不是用户玩家，直接返回
	if (this->m_currentGameState != GameControl::GameState::PlayingCards
		|| this->m_gameControl->getCurrentPlayer() != this->m_gameControl->getUser())
	{
		return;
	}
	//关闭用户出牌倒计时
	this->m_userTicking->stopTicking();
	//打出一个空Cards，表示不出
	Cards temp;
	Player* user = this->m_gameControl->getUser();
	user->playCards(temp);
	//清空用户手牌的选中状态，并且刷新用户手牌窗口
	for (auto it : this->m_selectedCardSet)
	{
		it->setSelected(false);
	}
	this->m_selectedCardSet.clear();
	this->updateHandCardsPanel(user);
}

