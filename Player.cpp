#include "Player.h"

Player::Player(QObject* parent):QObject(parent)
{
	this->m_score = 0;
	this->m_isWin = false;
}

Player::Player(QString name,QObject *parent)
	: QObject(parent), m_name(name)
{
	this->m_score = 0;
	this->m_isWin = false;
}

Player::~Player()
{}

void Player::setName(QString name)
{
	this->m_name = name;
}

QString Player::getName() const
{
	return this->m_name;
}

void Player::setRole(Role role)
{
	this->m_role = role;
}

Player::Role Player::getRole() const
{
	return this->m_role;
}

void Player::setGender(Gender gender)
{
	this->m_gender = gender;
}

Player::Gender Player::getGender() const
{
	return this->m_gender;
}

void Player::setDirection(Direction direction)
{
	this->m_direction = direction;
}

Player::Direction Player::getDirection() const
{
	return this->m_direction;
}

void Player::setType(Type type)
{
	this->m_type = type;
}

Player::Type Player::getType() const
{
	return this->m_type;
}

void Player::setScore(int score)
{
	this->m_score = score;
}

int Player::getScore() const
{
	return this->m_score;
}

void Player::setIsWin(bool isWin)
{
	this->m_isWin = isWin;
}

bool Player::isWin() const
{
	return this->m_isWin;
}

void Player::grabLordBet(int bet)
{
	//TODO:实现叫地主/抢地主逻辑
	emit this->sigGrabLordBet(this, bet);
}

void Player::storeDealCard(const Card& card)
{
	this->m_handCards.add(card);
}

void Player::storeDealCard(const Cards& cards)
{
	this->m_handCards.add(cards);
}

void Player::setPrevPlayer(Player* prevPlayer)
{
	this->m_prevPlayer = prevPlayer;
}

Player* Player::getPrevPlayer() const
{
	return this->m_prevPlayer;
}

void Player::setNextPlayer(Player* nextPlayer)
{
	this->m_nextPlayer = nextPlayer;
}

Player* Player::getNextPlayer() const
{
	return this->m_nextPlayer;
}

Cards Player::getHandCards() const
{
	return this->m_handCards;
}

void Player::clearHandCards()
{
	this->m_handCards.clear();
}

void Player::playCards(const Cards& cards)
{
	this->m_handCards.remove(cards);
	emit this->sigPlayCards(this, cards);
}

void Player::setPending(const Cards& cards, Player* player)
{
	this->m_pendingCards = cards;
	this->m_pengdingPlayer = player;
}

Cards Player::getPendingCards() const
{
	return this->m_pendingCards;
}

Player* Player::getPendingPlayer() const
{
	return this->m_pengdingPlayer;
}

void Player::prepareCallLord()
{
	//TODO:实现叫地主逻辑
}

void Player::preparePlayCards()
{
	//TODO:实现出牌逻辑
}

