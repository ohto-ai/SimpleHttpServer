#include "SimpleHttpServer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimpleHttpServer w;
    w.show();
    return a.exec();
}
