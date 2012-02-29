#include <QApplication>
#include <QGLWidget>
#include <QDeclarativeView>

#include "qosgdeclarativeviewport.h"

int main(int argc, char * argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<QOSGDeclarativeViewport>("QOSGDeclarative",1,0,"QOSGViewport");

    QGLWidget* glWidget = new QGLWidget;
    QDeclarativeView mainView;
    mainView.setViewport(glWidget);
    mainView.setSource(QString("main.qml"));
    mainView.show();

    return app.exec();
}
