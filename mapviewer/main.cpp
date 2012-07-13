#include <QApplication>
#include "mapviewer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MapViewer w;
    w.show();
    return a.exec();
}
