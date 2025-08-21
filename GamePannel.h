#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GamePannel.h"

class GamePannel : public QMainWindow
{
    Q_OBJECT

public:
    GamePannel(QWidget *parent = nullptr);
    ~GamePannel();

private:
    Ui::GamePannelClass ui;
};

