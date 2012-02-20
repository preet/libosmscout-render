#include <QGuiApplication>
#include "qglosmviewport.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc,argv);
    QGLOSMViewport myView;
    myView.setWidth(800);
    myView.setHeight(480);
    myView.show();

    return app.exec();
}
