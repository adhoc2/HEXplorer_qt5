#include <QApplication>
#ifdef Q_WS_WIN32
    #include "usewinhoard.cpp"
#endif
#include "mdimain.h"
#include <bitset>
#include <stdlib.h>
#include "qdebug.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Ch.Hoël");
    app.setApplicationName("HEXplorer");
    app.setApplicationVersion("0.9.90");

    MDImain w;
    w.show();
    return app.exec();
}
