#include "GamePannel.h"
//由于原文件过大，分割为多个文件
//这个文件主要实现GamePannel类的初始化工作
//1.初始化了游戏控制类
//2.初始化了卡牌数据
//3.初始化了按钮组
//4.初始化了玩家上下文环境
//5.初始化了开始游戏前的卡牌场景
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
	for (int i = 0;i < 4;i++)//花色循环
	{
		for (int j = 0;j < 13;j++)//点数循环
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
	//安装事件过滤器，用于实现卡牌的单选/拖选功能
	cardPanel->installEventFilter(this);
}

void GamePannel::initButtonGroup()
{
	this->ui.buttonGroup->initButton();//初始化按钮组
	this->ui.buttonGroup->setCurrentPage(ButtonGroup::StartGamePage);//设置初始页面为开始游戏页面
	//连接按钮信号与槽，具体做什么待定
	//TODO
	connect(this->ui.buttonGroup, &ButtonGroup::sigStartGame, this, [=]()
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
	connect(this->ui.buttonGroup, &ButtonGroup::sigPlayCards, this, &GamePannel::onUserPlayCards);
	connect(this->ui.buttonGroup, &ButtonGroup::sigRefuseCards, this, &GamePannel::onUserPassCards);
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
	for (int i = 0;i < 3;i++)
	{
		CardPanel* cp = new CardPanel(this);
		cp->setImages(this->m_cardBackImage, this->m_cardBackImage);
		this->m_lordCardPanelList.append(cp);
		this->m_lordCardPanelList[i]->hide();
		//地主牌最开始隐藏，当牌发完，地主抢完之后再显示正面到主窗口最上方
	}
	//4.每个牌窗口所在的位置
	//4.1基础牌和移动牌初始都在窗口中心偏上50像素的位置
	QPoint baseCardPos((this->width() - this->cardSize.width()) / 2, (this->height() - this->cardSize.height()) / 2 - 50);
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

void GamePannel::initUserTicking()
{
	this->m_userTicking = new Ticking(this);
	//移动到窗口中央偏下30像素
	this->m_userTicking->move((this->width() - this->m_userTicking->width()) / 2, (this->height() - this->m_userTicking->height())/2 + 30);
	//响应sigNoMuchTime时间仅剩5秒的信号，播放警告音
	connect(this->m_userTicking, &Ticking::sigNoMuchTime, this, [=]()
		{
			//TODO:播放警告音
		});
	//响应sigTimeout时间到的信号，直接让用户过牌
	connect(this->m_userTicking, &Ticking::sigTimeout, this, &GamePannel::onUserPassCards);

	//启动计时在玩家状态切换函数playerStateProcess中开启，但是关闭计时会在两个用户响应牌操作的函数onUserPlayCards和onUserPassCards中关闭
}
