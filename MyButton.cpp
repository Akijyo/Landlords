#include "MyButton.h"

MyButton::MyButton(QWidget *parent)
	: QPushButton(parent)
{}

MyButton::~MyButton()
{}

void MyButton::setImages(QString normalImage, QString pressImage, QString hoverImage)
{
	this->m_normalImage = normalImage;
	this->m_pressImage = pressImage;
	this->m_hoverImage = hoverImage;
	this->m_pixmap.load(this->m_normalImage);
	this->update();
}

void MyButton::enterEvent(QEnterEvent* event)
{
	this->m_pixmap.load(this->m_hoverImage);
	this->update();
	QPushButton::enterEvent(event);
}

void MyButton::leaveEvent(QEvent* event)
{
	this->m_pixmap.load(this->m_normalImage);
	this->update();
	QPushButton::leaveEvent(event);
}

void MyButton::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->m_pixmap.load(this->m_pressImage);
		this->update();
	}
	QPushButton::mousePressEvent(event);
}

void MyButton::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->m_pixmap.load(this->m_normalImage);
		this->update();
	}
	QPushButton::mouseReleaseEvent(event);
}

void MyButton::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawPixmap(this->rect(), this->m_pixmap);
}

