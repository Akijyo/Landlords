#include "GamePannel.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GamePannel window;
    window.show();
    return app.exec();
}
