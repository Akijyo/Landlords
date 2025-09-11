#include "GamePannel.h"

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

	//初始发牌步长
	this->m_cardMovePos = 0;
	//初始化定时器
	this->m_dealCardTimer = new QTimer(this);
	this->m_dealCardTimer->setTimerType(Qt::PreciseTimer);
	connect(this->m_dealCardTimer, &QTimer::timeout, this, &GamePannel::onDealCardTimerTimeout);
	//初始化地主牌移动定时器
	this->m_lordCardMoveTimer = new QTimer(this);
	this->m_lordCardMoveTimer->setTimerType(Qt::PreciseTimer);
	connect(this->m_lordCardMoveTimer, &QTimer::timeout, this, &GamePannel::onBaseToLordTimeout);
	this->m_lordCardMoveIndex = 0;

	//初始化动画类
	this->m_animation = new Animation(this);
}

GamePannel::~GamePannel()
{}

void GamePannel::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	painter.drawPixmap(this->rect(), this->m_background);
}

void GamePannel::initGameControl()
{
	this->m_gameControl = new GameControl(this);//实例化游戏控制类对象
	this->m_gameControl->initPlayer();//初始化玩家
	this->m_playerList.append(this->m_gameControl->getRobotLeft());//左机器人
	this->m_playerList.append(this->m_gameControl->getUser());//用户玩家
	this->m_playerList.append(this->m_gameControl->getRobotRight());//右机器人	
	//处理玩家状态变化的信号槽
	connect(this->m_gameControl, &GameControl::sigPlayerStateChanged, this, &GamePannel::playerStateProcess);
	//处理游戏状态变化的信号槽
	connect(this->m_gameControl, &GameControl::sigGameStateChanged, this, &GamePannel::gameStateProcess);
	//处理叫地主时期的窗口变化控制信号槽
	connect(this->m_gameControl, &GameControl::sigPlayerGrabLordBet, this, &GamePannel::onPlayerGrabLordBet);
	//处理出牌时期的窗口变化控制信号槽
	connect(this->m_gameControl, &GameControl::sigPlayerPlayCards, this, &GamePannel::onResponsePlayCards);
}

void GamePannel::updatePlayerScores()
{
	this->ui.scorePanel->setScores(this->m_playerList[0]->getScore(), this->m_playerList[1]->getScore(), this->m_playerList[2]->getScore());
}

void GamePannel::initCardsData()
{
	//加载卡牌图片
	QPixmap pixmap(":/Animation/images/card.png");
	QVector<int> suitIndex = { 2,3,1,4 };//由于卡牌类里的枚举是1-4顺序是红桃方块梅花黑桃，但是图片顺序是方块梅花红桃黑桃，所以需要一个映射数组

	//计算每个卡牌的大小
	this->cardSize.setWidth(pixmap.width() / 13);//宽度为图片宽度的1/13
	this->cardSize.setHeight(pixmap.height() / 5);//高度为图片高度的1/5

	//分割背景图
	this->m_cardBackImage = pixmap.copy(2 * this->cardSize.width(), 4 * this->cardSize.height(), this->cardSize.width(), this->cardSize.height());

	//分割大小王
	Card bigJoker(Card::CardSuit::Suit_end, Card::CardPoint::Card_BigJoker);
	this->cropCardImages(pixmap, this->cardSize.width(), 4 * this->cardSize.height(), bigJoker);

	Card smallJoker(Card::CardSuit::Suit_end, Card::CardPoint::Card_Joker);
	this->cropCardImages(pixmap, 0, 4 * this->cardSize.height(), smallJoker);

	//分割点数牌
	for(int i=0;i<4;i++)//花色循环
	{
		for(int j=0;j<13;j++)//点数循环
		{
			Card c(Card::CardSuit(suitIndex[i]), Card::CardPoint(j + 1));//根据映射数组获取正确的花色
			this->cropCardImages(pixmap, j * this->cardSize.width(), i * this->cardSize.height(), c);
		}
	}
}

void GamePannel::cropCardImages(QPixmap& pixmap, int x, int y, Card& c)
{
	QPixmap sub = pixmap.copy(x, y, this->cardSize.width(), this->cardSize.height());//切割图片
	CardPanel* cardPanel = new CardPanel(this);//创建卡牌窗口
	cardPanel->setImages(sub, this->m_cardBackImage);//设置卡牌窗口的正反面图片
	cardPanel->setCard(c);//设置卡牌数据
	cardPanel->hide();//卡牌窗口最开始隐藏
	this->m_cardMap.insert(c, cardPanel);//将卡牌与卡牌窗口绑定
}

void GamePannel::initButtonGroup()
{
	this->ui.buttonGroup->initButton();//初始化按钮组
	this->ui.buttonGroup->setCurrentPage(ButtonGroup::StartGamePage);//设置初始页面为开始游戏页面
	//连接按钮信号与槽，具体做什么待定
	//TODO
	connect(this->ui.buttonGroup,&ButtonGroup::sigStartGame,this,[=]()
		{
			//点击开始游戏后，窗口的初始化
			//隐藏按钮组，切换到stackedWidget的空的那一页
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::EmptyPage);
			//分数清空
			this->m_gameControl->resetScore();
			this->updatePlayerScores();
			//游戏状态->发牌状态
			this->gameStateProcess(GameControl::GameState::DealingCards);
		});
	connect(this->ui.buttonGroup, &ButtonGroup::sigPlayCards, this, [=]()
		{});
	connect(this->ui.buttonGroup, &ButtonGroup::sigRefuseCards, this, [=]()
		{});
	connect(this->ui.buttonGroup, &ButtonGroup::sigGrabLord, this, [=](int score)
		{
			//用户点击抢地主按钮后，调用用户玩家的grabLordBet函数
			this->m_gameControl->getUser()->grabLordBet(score);
			//按钮组切换到等待页面
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::EmptyPage);
		});
}

void GamePannel::initPlayerContext()
{
	//放置玩家扑克牌的区域
	const QRect cardsRect[] =
	{
		// x, y, width, height
		QRect(90, 130, 100, height() - 200),                    // 左侧机器人
		QRect(250, rect().bottom() - 120, width() - 500, 100),   // 当前玩家
		QRect(rect().right() - 190, 130, 100, height() - 200)  // 右侧机器人
	};
	//放置玩家出牌的区域
	const QRect playHandRect[] =
	{
		QRect(260, 150, 100, 100),                              // 左侧机器人
		QRect(150, rect().bottom() - 290, width() - 300, 105),   // 当前玩家
		QRect(rect().right() - 360, 150, 100, 100)           // 右侧机器人
	};
	//玩家头像的显示位置
	const QPoint roleImgPos[] =
	{
		QPoint(cardsRect[0].left() - 80, cardsRect[0].height() / 2 + 20),     // 左侧机器人
		QPoint(cardsRect[1].right() - 10, cardsRect[1].top() - 10) ,           // 当前玩家
		QPoint(cardsRect[2].right() + 10, cardsRect[2].height() / 2 + 20)    // 右侧机器人
	};

	//为三个玩家初始化上下文环境
	int userIndex = this->m_playerList.indexOf(this->m_gameControl->getUser());//获取用户玩家的索引
	for (int i = 0;i < 3;i++)
	{
		PlayerContext context;
		//1.玩家扑克牌显示区域
		context.cardAreaRect = cardsRect[i];
		//2.玩家出牌显示区域
		context.playAreaRect = playHandRect[i];
		//3.玩家扑克牌的对齐方式
		context.cardAlign = (i == userIndex) ? PlayerContext::CardAlign::Horizontal : PlayerContext::CardAlign::Vertical;
		//4.扑克牌显示的正反面
		context.isFrontSide = (i == userIndex) ? true : false;
		//5.游玩过程的提示信息
		context.infoLabel = new QLabel(this);
		context.infoLabel->resize(160, 98);
		context.infoLabel->hide();
		//将提示信息移动到出牌区域的中心
		context.infoLabel->move(playHandRect[i].left() + (playHandRect[i].width() - context.infoLabel->width()) / 2, 
			playHandRect[i].top() + (playHandRect[i].height() - context.infoLabel->height()) / 2);
		//6.玩家的头像
		context.roleImage = new QLabel(this);
		context.roleImage->resize(84, 120);
		context.roleImage->hide();
		context.roleImage->move(roleImgPos[i]);
		//绑定玩家和玩家上下文环境
		this->m_playerContextMap.insert(this->m_playerList[i], context);
	}
}

void GamePannel::initCardsScene()
{
	//游戏中心的初始牌
	//1.基础牌窗口
	this->m_baseCardPanel = new CardPanel(this);
	this->m_baseCardPanel->setImages(this->m_cardBackImage, this->m_cardBackImage);//基础牌正反面都是背面
	//2.发牌过程中移动到玩家手中的牌窗口
	this->m_moveCardPanel = new CardPanel(this);
	this->m_moveCardPanel->setImages(this->m_cardBackImage, this->m_cardBackImage);
	//3.三张地主牌
	for (int i = 0;i <3;i++)
	{
		CardPanel* cp = new CardPanel(this);
		cp->setImages(this->m_cardBackImage, this->m_cardBackImage);
		this->m_lordCardPanelList.append(cp);
		this->m_lordCardPanelList[i]->hide();
		//地主牌最开始隐藏，当牌发完，地主抢完之后再显示正面到主窗口最上方
	}
	//4.每个牌窗口所在的位置
	//4.1基础牌和移动牌初始都在窗口中心偏上50像素的位置
	QPoint baseCardPos((this->width() - this->cardSize.width()) / 2, (this->height() - this->cardSize.height()) / 2-50);
	this->m_baseCardPos = baseCardPos;
	this->m_baseCardPanel->move(baseCardPos);
	this->m_moveCardPanel->move(baseCardPos);
	//4.2地主牌显示的位置是在窗口顶部往下20像素y轴，三张牌间隔10像素x轴中心排放
	int baseX = (this->width() - (3 * this->cardSize.width() + 2 * 10)) / 2;//计算三张牌的最左侧位置
	for (int i = 0;i < 3;i++)
	{
		//隔10像素间隔依次排布
		this->m_lordCardPanelList[i]->move(baseX + i * (this->cardSize.width() + 10), 20);
	}
}

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
	//启动定时器,完成发牌动画
	this->m_dealCardTimer->start(3);
	
	//发牌音效
	//TODO
}

void GamePannel::onDealCardTimerTimeout()
{
	//得到当前被发牌的玩家
	Player* curPlayer = this->m_gameControl->getCurrentPlayer();
	//如果当前这张牌已经发到对应玩家手中
	if (this->m_cardMovePos >= 100)
	{
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
		//重置发牌步长
		this->m_cardMovePos = 0;
		//如果牌已经发完
		if (this->m_gameControl->getSurplusCards().size() == 3)
		{
			//复位移动牌并且隐藏
			this->m_moveCardPanel->move(this->m_baseCardPos);
			this->m_moveCardPanel->hide();
			//停止发牌计时器
			this->m_dealCardTimer->stop();
			//重置牌移动步长
			this->m_cardMovePos = 0;
			//启动地主牌移动定时器，完成地主牌移动动画
			this->m_lordCardMoveTimer->start(3);
			//在onBaseToLordTimeout函数中切换到叫地主状态
			return;
		}
	}
	//移动扑克牌
	this->cardMoveStep(curPlayer);
	this->m_cardMovePos += 2;

}

void GamePannel::cardMoveStep(Player* curPlayer)
{
	// 进度t：0.0 -> 1.0
	const int totalSteps = 100;
	const qreal t = std::min<qreal>(1.0, m_cardMovePos / qreal(totalSteps));

	// 目标区域（玩家手牌显示区域）
	const QRect area = this->m_playerContextMap[curPlayer].cardAreaRect;

	// 三个玩家在 m_playerList 中的顺序：左机器人=0，用户=1，右机器人=2
	const int idx = this->m_playerList.indexOf(curPlayer);

	// 以“卡片外沿贴边”为终点，避免视觉偏差
	// 注意：QRect::right() 是包含性坐标（x+width-1），所以要减去卡片宽再+1
	const int targetX_Left = area.right() - this->cardSize.width() + 1; // 卡片右沿贴 area.right()
	const int targetX_Right = area.left();                                // 卡片左沿贴 area.left()
	const int targetY_User = area.top() - this->cardSize.height();       // 卡片下沿贴 area.top()

	// 基点（发牌起点）
	const QPoint start = this->m_baseCardPos;

	QPoint pos = start;
	switch (idx)
	{
	case 0: // 左机器人：水平向左
	{
		const qreal dx = qreal(targetX_Left - start.x());
		pos.setX(int(std::lround(start.x() + t * dx)));
		pos.setY(start.y()); // 保持水平直线
		break;
	}
	case 1: // 用户：垂直向下
	{
		const qreal dy = qreal(targetY_User - start.y());
		pos.setX(start.x()); // 保持垂直直线
		pos.setY(int(std::lround(start.y() + t * dy)));
		break;
	}
	case 2: // 右机器人：水平向右
	{
		const qreal dx = qreal(targetX_Right - start.x());
		pos.setX(int(std::lround(start.x() + t * dx)));
		pos.setY(start.y()); // 保持水平直线
		break;
	}
	default:
		break;
	}

	// 移动卡片
	this->m_moveCardPanel->move(pos);

	// 显隐控制
	if (this->m_cardMovePos == 0)
		this->m_moveCardPanel->show();
	if (this->m_cardMovePos >= totalSteps)
		this->m_moveCardPanel->hide();
}

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

void GamePannel::onBaseToLordTimeout()
{
	//隐藏基础牌
	if (this->m_lordCardMoveIndex == 0)
	{
		this->m_baseCardPanel->hide();
	}
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
			cp->hide();
			this->m_lordMovingPanels.append(cp);
		}
	}
	//移动牌到地主牌框
	this->lordCardMoveStep();
	this->m_cardMovePos += 2;
	//移动到位
	if (this->m_cardMovePos >= 100)
	{
		//隐藏移动牌
		for (auto* cp : this->m_lordMovingPanels)
		{
			cp->hide();
		}
		//停止地主牌移动定时器
		this->m_lordCardMoveTimer->stop();
		this->m_cardMovePos = 0;
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
	}
}

void GamePannel::lordCardMoveStep()
{
	// 进度 t：0.0 -> 1.0
	const int totalSteps = 100;
	const qreal t = std::min<qreal>(1.0, this->m_cardMovePos / qreal(totalSteps));

	// 起点：中间牌堆位置
	const QPoint start = this->m_baseCardPos;

	// 三个目标：三处地主牌框的位置
	for (int i = 0; i < 3; ++i)
	{
		const QPoint target = this->m_lordCardPanelList[i]->pos();
		const int x = int(std::lround(start.x() + (target.x() - start.x()) * t));
		const int y = int(std::lround(start.y() + (target.y() - start.y()) * t));
		this->m_lordMovingPanels[i]->move(QPoint(x, y));
	}

	// 显隐控制（与 cardMoveStep 风格一致）
	if (this->m_cardMovePos == 0)
	{
		for (auto* cp : this->m_lordMovingPanels)
		{
			cp->show();
			cp->raise();
		}
	}
	if (this->m_cardMovePos >= totalSteps)
	{
		for (auto* cp : this->m_lordMovingPanels) cp->hide();
	}
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

void GamePannel::playerStateProcess(Player* player, GameControl::PlayerState playerState)
{
	switch (playerState)
	{
	case GameControl::ConsideringCallLord:
		//如果是用户玩家，则显示按钮组的叫地主页面
		if (player == this->m_gameControl->getUser())
		{
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::CallAndGrabPage,this->m_gameControl->getCurHighestBet());
		}
		break;
	case GameControl::ConsideringPlayCards:
	{
		if (player == this->m_gameControl->getUser())
		{
			Player* pendingPlayer = player->getPrevPlayer();
			if (pendingPlayer == nullptr || player == pendingPlayer)//如果玩家是优先出牌的玩家
			{
				this->ui.buttonGroup->setCurrentPage(ButtonGroup::PageIndex::OnlyPlayCardsPage);
			}
			else
			{
				this->ui.buttonGroup->setCurrentPage(ButtonGroup::PageIndex::PlayAndRefusePage);
			}
		}
		else
		{
			//如果是机器人玩家，则隐藏按钮组
			this->ui.buttonGroup->setCurrentPage(ButtonGroup::EmptyPage);
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
		break;
	}
	default:
		break;
	}
}

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
	this->showAnimation(GamePannel::WindowAnimation::BetScore, bet);
	//叫地主音效
}

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
			break;
		case GamePannel::Bomb:
			break;
		case GamePannel::Plane:
			break;
		case GamePannel::SeqSingle:
			break;
		case GamePannel::SeqPair:
			break;
		default:
			break;
	}
	this->m_animation->show();
	this->m_animation->raise();
}

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
	Player* pendingPlayer = player->getPrevPlayer();

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
	QVector<Card> vcards = playCards.toOrderlyList(Cards::SortType::Desc);
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
			this->showAnimation(GamePannel::WindowAnimation::Bomb);
			break;
		}
		default:
			break;
	}

	//5.TODO:播放音乐
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


	//4.更新地主玩家手牌显示
	this->updateHandCardsPanel(this->m_gameControl->getLordPlayer());

	//5.显示各个玩家头像
	this->showPlayerRoleImage();

	//6.切换玩家状态并且让当前玩家出牌
	this->playerStateProcess(this->m_gameControl->getCurrentPlayer(), GameControl::PlayerState::ConsideringPlayCards);
	this->m_gameControl->getCurrentPlayer()->preparePlayCards();
}

void GamePannel::onLordCardsToLordTimeout()
{
}

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

