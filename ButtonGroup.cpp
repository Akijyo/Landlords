#include "ButtonGroup.h"

ButtonGroup::ButtonGroup(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

ButtonGroup::~ButtonGroup()
{}

void ButtonGroup::initButton()
{
	//设置开始游戏按钮图片
	this->ui.starGameButton->setImages(":/Button/images/start-1.png", 
		":/Button/images/start-2.png", ":/Button/images/start-3.png");
	//设置优先出牌按钮图片
	this->ui.onlyPlayCardsButton->setImages(":/Button/images/chupai_btn-1.png", 
		":/Button/images/chupai_btn-2.png", ":/Button/images/chupai_btn-3.png");
	//设置对抗出牌按钮图片
	this->ui.playCardsButton->setImages(":/Button/images/chupai_btn-1.png",
		":/Button/images/chupai_btn-2.png", ":/Button/images/chupai_btn-3.png");
	//设置不出按钮图片
	this->ui.refuseCardsButton->setImages(":/Button/images/pass_btn-1.png", 
		":/Button/images/pass_btn-2.png", ":/Button/images/pass_btn-3.png");
	//设置不抢地主按钮图片
	this->ui.refuseLordButton->setImages(":/Button/images/buqiang-1.png",
		":/Button/images/buqiang-2.png", ":/Button/images/buqiang-3.png");
	//设置抢地主按钮图片，分为一分，二分，三分
	this->ui.oneScoreLordButton->setImages(":/Button/images/1fen-1.png",
		":/Button/images/1fen-2.png", ":/Button/images/1fen-3.png");
	this->ui.twoScoreLordButton->setImages(":/Button/images/2fen-1.png",
		":/Button/images/2fen-2.png", ":/Button/images/2fen-3.png");
	this->ui.threeScoreLordButton->setImages(":/Button/images/3fen-1.png",
		":/Button/images/3fen-2.png", ":/Button/images/3fen-3.png");

	//为按钮设置大小
	//所有按钮放在一个QVector中，便于统一设置大小
	QVector<MyButton*> allButtons = { this->ui.starGameButton,
		this->ui.onlyPlayCardsButton, this->ui.playCardsButton,
		this->ui.refuseCardsButton, this->ui.refuseLordButton,
		this->ui.oneScoreLordButton, this->ui.twoScoreLordButton,
		this->ui.threeScoreLordButton };
	for(auto button : allButtons)
	{
		//button->resize(90, 45);
		button->setFixedSize(90, 45);
	}
	//为按钮的clicked信号绑定对应的槽函数
	connect(this->ui.starGameButton, &MyButton::clicked, this, &ButtonGroup::sigStartGame);
	connect(this->ui.onlyPlayCardsButton, &MyButton::clicked, this, &ButtonGroup::sigPlayCards);
	connect(this->ui.playCardsButton, &MyButton::clicked, this, &ButtonGroup::sigPlayCards);
	connect(this->ui.refuseCardsButton, &MyButton::clicked, this, &ButtonGroup::sigRefuseCards);
	connect(this->ui.refuseLordButton, &MyButton::clicked, this, [=]()
		{
			emit sigGrabLord(0);
		});
	connect(this->ui.oneScoreLordButton, &MyButton::clicked, this,[=]()
		{
			emit sigGrabLord(1);
		});
	connect(this->ui.twoScoreLordButton, &MyButton::clicked, this, [=]()
		{
			emit sigGrabLord(2);
		});
	connect(this->ui.threeScoreLordButton, &MyButton::clicked, this, [=]()
		{
			emit sigGrabLord(3);
		});
}

void ButtonGroup::setCurrentPage(PageIndex index,int bet)
{
	this->ui.stackedWidget->setCurrentIndex(index);
	//抢地主过程按钮状态区分
	if (index == PageIndex::CallAndGrabPage)
	{
		switch (bet)
		{
		case 0://当前最高叫分为0，显示全部按钮
			this->ui.oneScoreLordButton->setVisible(true);
			this->ui.twoScoreLordButton->setVisible(true);
			this->ui.threeScoreLordButton->setVisible(true);
			break;
		case 1://当前最高叫分为1，不显示一分按钮
			this->ui.oneScoreLordButton->setVisible(false);
			this->ui.twoScoreLordButton->setVisible(true);
			this->ui.threeScoreLordButton->setVisible(true);
			break;
		case 2://当前最高叫分为2，不显示一分和二分按钮
			this->ui.oneScoreLordButton->setVisible(false);
			this->ui.twoScoreLordButton->setVisible(false);
			this->ui.threeScoreLordButton->setVisible(true);
			break;
		default:
			break;
			//当前最高叫分为3，不会进入到这个函数中，当出现最高分为3时，直接成为地主
		}
	}
}

ButtonGroup::PageIndex ButtonGroup::getCurrentPage() const
{
	return static_cast<PageIndex>(this->ui.stackedWidget->currentIndex());
}

