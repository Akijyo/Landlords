#pragma once

#include <QtWidgets/QMainWindow>
#include<qpixmap.h>
#include<qpainter.h>
#include<qrandom.h>
#include<GameControl.h>
#include<qvector.h>
#include "ui_GamePannel.h"
#include<qtimer.h>
#include <CardPanel.h>
#include<thread>
#include<Animation.h>
#include<PlayHand.h>

class GamePannel : public QMainWindow
{
    Q_OBJECT

public:
    GamePannel(QWidget *parent = nullptr);
    ~GamePannel();
	//玩家的上下文环境类
	class PlayerContext
	{
	public:
		enum CardAlign
		{
			//水平
			Horizontal,
			//垂直
			Vertical
		};
		//1.玩家扑克牌显示区域
		QRect cardAreaRect;
		//2.玩家出牌显示区域
		QRect playAreaRect;
		//3.玩家扑克牌的对齐方式
		CardAlign cardAlign;
		//4.扑克牌显示的正反面
		bool isFrontSide;
		//5.游玩过程的提示信息
		QLabel* infoLabel;
		//6.玩家的头像
		QLabel* roleImage;
		//7.玩家上次打出过的牌
		Cards lastPlayCards;
	};
	//动画的类型
	enum WindowAnimation
	{
		BetScore,//地主叫分
		Joker,//王炸
		Bomb,//炸弹
		Plane,//飞机
		SeqSingle,//单顺
		SeqPair,//连对
	};
protected:
	//绘图事件，主要用于绘制主窗口背景
	void paintEvent(QPaintEvent* event) override;

private:
	//游戏控制类初始化，实例化游戏控制类对象，并初始化玩家，并且将三个玩家对象保存到类中
		//这个函数初始化了GameControl类成员变量m_gameControl，并且初始化了玩家对象，并且将三个玩家对象保存到m_playerList中
	void initGameControl();

	//更新玩家得分面板
		//这个函数将玩家得分面板窗口中的分数更新为当前玩家的分数
	void updatePlayerScores();

	//初始化卡牌相关内容，包括切割卡牌图片，将卡牌与卡牌窗口绑定
		//这个函数将资源文件中的总卡牌图片个个分割出来，并且逐个调用cropCardImages函数将卡牌与卡牌窗口绑定
	void initCardsData();

	//切割卡牌图片，x,y为切割起始位置，c为切割后卡牌对应的花色和点数，并且实现卡牌窗口的初始化和绑定
		//这个函数主要是实例化CardPanel类对象，将这个卡牌窗口对象设置属性，并且将卡牌窗口对象与对应的卡牌通过成员变量m_cardMap绑定
	void cropCardImages(QPixmap& pixmap, int x, int y, Card& c);

	//初始化按钮组
		//这个函数主要是实例化按钮组窗口，并且将按钮组窗口中的按钮点击信号与槽函数绑定，有开始游戏，地主赌注，出牌，过牌信号
	void initButtonGroup();

	//这个函数通过成员变量m_playerContextMap将玩家对象与玩家上下文环境绑定，并且初始化玩家上下文环境的各个属性，包括
	//1.玩家扑克牌显示区域，2.玩家出牌显示区域，3.玩家扑克牌的对齐方式，4.扑克牌显示的正反面，5.游玩过程的提示信息，6.玩家的头像，7.玩家上次打出过的牌（默认空）
	void initPlayerContext();

	//初始化开始游戏前的卡牌场景，包含初始化在窗口中的基础牌窗口，还有初始化移动到玩家手中的牌窗口，还有地主三张底牌窗口初始化（都是卡牌窗口）
		//这个函数主要是实例化CardPanel类的几个关键卡牌窗口，并且对这些卡牌窗口设置图片、与初始放置位置。
		// 卡牌窗口包括基础牌窗口m_baseCardPanel，移动到玩家手中的牌窗口m_moveCardPanel，地主三张底牌窗口m_lordCardPanelList
	void initCardsScene();

	//游戏状态对窗口的控制函数
		//类似游戏引擎的状态机，这个函数接收参数state并且保存到成员变量m_currentGameState中，然后根据不同的游戏状态调用不同的处理函数，是游戏状态主控函数
		//内部调用了发牌的子函数dealCardsProcess
	void gameStateProcess(GameControl::GameState gameState);

	//发牌状态处理函数，是游戏状态主控函数gameStateProcess的子函数。主要操作是重置发牌窗口，重置玩家上下文环境，发牌动画音效等
		//这个函数应该发生在点击开始按钮之后，完成了重置卡牌窗口，重置了玩家上下文环境，隐藏按钮组窗口，启动发牌动画定时器，定时器槽函数是onDealCardTimerTimeout
	void dealCardsProcess();

	//定时器计时的槽函数，发牌动画在这里处理
		//这个函数是发牌的过程和动画的实现函数，分为两个部分，第一部分是实现发牌动画的渲染，通过cardMoveStep函数实现发牌动画
		//第二部分是当动画到达当前玩家手中，应该在牌堆中取出一张牌，存入当前玩家手牌，并且通过disposHandCards更新当前玩家手牌显示，然后切换玩家。
		//当发完所有牌之后，停止定时器，切换游戏状态到叫地主状态
	void onDealCardTimerTimeout();

	//移动扑克牌的函数，实现发牌动画
	void cardMoveStep(Player* curPlayer);
	
	//更新玩家手牌，里面调用更新玩家手牌窗口位置的函数
		//这个函数实现了为发来的牌窗口CardPanel对象设置对应所有者，并且调用updateHandCardsPanel函数更新玩家手牌窗口显示
	void disposHandCards(Player* player,const Cards& hcards);

	//更新玩家手牌窗口位置的通用函数，一更新就是整个窗口
	void updateHandCardsPanel(Player* player);

	//中间牌堆移动到地主牌的动画槽函数
	void onBaseToLordTimeout();
	
	//从中间牌堆到地主牌框的动画实现函数
	void lordCardMoveStep();

	//状态转换抢地主的处理函数
	void callLordProcess();

	//处理玩家状态转换的函数
	void playerStateProcess(Player* player, GameControl::PlayerState playerState);

	//接收来自GameControl的叫地主/抢地主信号的槽函数
	void onPlayerGrabLordBet(Player* player, int bet, bool first);

	//显示叫分/炸弹/飞机等动画的集成函数
	void showAnimation(WindowAnimation type,int bet=0);

	//接收来自GameControl的玩家出牌信号的槽函数，是界面对玩家出牌的响应
	void onResponsePlayCards(Player* player, const Cards& playCards);

	//游戏进入打牌状态的处理函数
	void playCardsProcess();

	//playCardsProcess的子函数，完成地主牌向地主玩家手牌移动的动画
	void onLordCardsToLordTimeout();

	//playCardsProcess的子函数的子函数，实现把玩家头像显示出来的功能
	void showPlayerRoleImage();




    Ui::GamePannelClass ui;
	//主窗口背景图片
	QPixmap m_background;

	//游戏控制类对象指针
	GameControl* m_gameControl;
	//保存三个玩家对象的指针，循序是左机器人，用户玩家，右机器人
	QVector<Player*> m_playerList;
	//保存卡牌与卡牌窗口的绑定关系
	QMap<Card, CardPanel*> m_cardMap;
	//卡牌尺寸
	QSize cardSize;
	//卡牌背面图片
	QPixmap m_cardBackImage;
	//保存玩家与玩家上下文环境的绑定关系
	QMap<Player*, PlayerContext> m_playerContextMap;
	//中心卡牌位置
	QPoint m_baseCardPos;
	//基础牌窗口
	CardPanel* m_baseCardPanel;
	//移动到玩家手中的牌窗口
	CardPanel* m_moveCardPanel;
	//地主三张底牌窗口
	QVector<CardPanel*> m_lordCardPanelList;
	//记录当前游戏状态
	GameControl::GameState m_currentGameState;
	//发牌过程中的定时器
	QTimer* m_dealCardTimer;
	//用于记录发牌的步长
	int m_cardMovePos;
	//地主牌移动定时器
	QTimer* m_lordCardMoveTimer;
	//辅助地主牌移动的索引
	int m_lordCardMoveIndex;
	// 三张同时飞行所用的临时移动牌面（背面图）
	QVector<CardPanel*> m_lordMovingPanels;
	//游戏特效/动画类
	Animation* m_animation;
};


