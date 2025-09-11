#include "UserPlayer.h"

UserPlayer::UserPlayer(QObject *parent)
	: Player(parent)
{
	this->setType(Player::Human);
}

UserPlayer::~UserPlayer()
{}

void UserPlayer::prepareCallLord()
{
}

void UserPlayer::preparePlayCards()
{
}

