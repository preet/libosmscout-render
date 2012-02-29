#ifndef QOSGDECLARATIVEVIEWPORT_H
#define QOSGDECLARATIVEVIEWPORT_H

// Qt includes
#include <QTimer>
#include <QPainter>
#include <QDeclarativeItem>
#include <QScriptEngine>

// osg includes
#include <osg/ref_ptr>
#include <osgDB/ReadFile>
#include <osg/AnimationPath>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>

class QOSGDeclarativeViewport : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit QOSGDeclarativeViewport(QDeclarativeItem *parent = 0);

    void paint(QPainter *myPainter,
               const QStyleOptionGraphicsItem *myOpts,
               QWidget *myWidget);
    
public slots:
    void onUpdateView();
    
private:
    void setupView();

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_osg_winEmb;
    osg::ref_ptr<osg::Group> m_osg_root;
    osg::ref_ptr<osg::MatrixTransform> m_osg_node_xform;
    osg::ref_ptr<osg::Node> m_osg_node_shinyCow;
    osgViewer::Viewer m_osg_viewer;
    QTimer m_updateView;

    bool m_initView;
};

#endif // QOSGDECLARATIVEVIEWPORT_H
