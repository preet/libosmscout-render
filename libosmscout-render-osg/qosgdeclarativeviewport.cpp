#include "qosgdeclarativeviewport.h"

QOSGDeclarativeViewport::QOSGDeclarativeViewport(QDeclarativeItem *parent) :
    QDeclarativeItem(parent)
{
    // set flag ItemHasNoContents to false to force draw
    this->setFlag(QGraphicsItem::ItemHasNoContents, false);
}

void QOSGDeclarativeViewport::paint(QPainter *myPainter,
                                    const QStyleOptionGraphicsItem *myOpts,
                                    QWidget *myWidget)
{
    Q_UNUSED(myOpts);
    Q_UNUSED(myWidget);

    if(!m_initView)
    {   setupView();   }

    myPainter->beginNativePainting();
    m_osg_viewer.frame();
    myPainter->endNativePainting();
}

void QOSGDeclarativeViewport::onUpdateView()
{
    this->update(this->x(),this->y(),
                 this->width(),this->height());
}

void QOSGDeclarativeViewport::setupView()
{
    // node: all hail the cow
    m_osg_node_shinyCow =
            osgDB::readNodeFile("/home/preet/Downloads/Reference/eBooks/osgdata/cow.osg");

    // node: rotation transform animation
    m_osg_node_xform = new osg::MatrixTransform;
    m_osg_node_xform->addChild(m_osg_node_shinyCow);
    m_osg_node_xform->addUpdateCallback(new osg::AnimationPathCallback(osg::Vec3(0.0f,0.0f,0.0f),
                                                                       osg::Z_AXIS,
                                                                       osg::inDegrees(45.0f)));
    // node: root
    m_osg_root = new osg::Group;
    m_osg_root->addChild(m_osg_node_xform);

    // viewer setup
    m_osg_viewer.setSceneData(m_osg_root);
    m_osg_viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3d(-20.0, 0.0, 10.0),
                                                    osg::Vec3d(0.0, 0.0, 0.0),
                                                    osg::Vec3d(0.0, 0.0, 1.0));

    QScriptEngine scriptEngine;
    QScriptValue absXY = mapToItem(scriptEngine.nullValue(),0,0);
    qreal absX = absXY.property("x").toNumber();
    qreal absY = absXY.property("y").toNumber();

    // graphics window embedded [x,y,w,h parameters don't seem to do anything??]
    m_osg_winEmb = new osgViewer::GraphicsWindowEmbedded(int(absX),int(absY),this->width(),this->height());

    // sets the viewport as described here http://www.opengl.org/sdk/docs/man/xhtml/glViewport.xml
    // but (x=0 and y=0) correspond to (0,0) of the entire window and not just our local item bounds
    m_osg_viewer.getCamera()->setViewport(new osg::Viewport(int(absX),int(absY),this->width(),this->height()));
    m_osg_viewer.getCamera()->setGraphicsContext(m_osg_winEmb.get());
    m_osg_viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    connect(&m_updateView, SIGNAL(timeout()),
            this, SLOT(onUpdateView()));

    m_updateView.setInterval(66);
    m_updateView.start();

    m_initView = true;
}
