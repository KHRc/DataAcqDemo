#include "signal_acquisition.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    signal_acquisition window;
    window.show();
    return app.exec();
}
