#include "viewport.h"

Viewport::Viewport(QWidget *parent) :
    QGLWidget(parent),
    m_loadedMap(false),
    m_camMouseMode(0)
{}

QSize Viewport::sizeHint() const
{   return QSize(800,480);   }

void Viewport::onLoadMap(const QString &mapPath, const QString &stylePath)
{
    // this should be called after initializeGL
//    osg::setNotifyLevel(osg::INFO);

    if(m_loadedMap)
    {   qDebug() << "INFO: Map already loaded!";   return;   }

    // load database
    m_databaseParam = new osmscout::DatabaseParameter;
    m_database = new osmscout::Database(*m_databaseParam);
    if(m_database->Open(mapPath.toStdString()))
    {   qDebug() << "INFO: Opened Database Successfully";   }
    else
    {   qDebug() << "ERROR: Could not open database";   return;   }

    // load style data
    osmscout::RenderStyleConfigReader styleConfigReader(stylePath.toStdString(),
                                                        m_database->GetTypeConfig(),
                                                        m_listStyleConfigs);
    if(styleConfigReader.HasErrors())
    {   qDebug() << "ERROR: Could not read style config";   return;   }
    else
    {   qDebug() << "INFO: Opened Style Configs successfully";   }

    // load map renderer
    m_mapRenderer = new osmscout::MapRendererOSG(m_database,m_osg_viewer);
    m_mapRenderer->SetRenderStyleConfigs(m_listStyleConfigs);

    // init scene
    osmscout::PointLLA camLLA(43.66,-79.377,500);
    m_mapRenderer->InitializeScene(camLLA,30.0,1.67);

    // get osmscout camera
    osmscout::Camera const * myCam = m_mapRenderer->GetCamera();
    osg::Vec3 camEye(myCam->eye.x,myCam->eye.y,myCam->eye.z);
    osg::Vec3 camViewPt(myCam->viewPt.x,myCam->viewPt.y,myCam->viewPt.z);
    osg::Vec3 camUp(myCam->up.x,myCam->up.y,myCam->up.z);

    // update openscenegraph camera
    m_osg_trackballManip = new osgGA::TrackballManipulator;
    m_osg_trackballManip->setAllowThrow(false); // important since we only render on demand
    m_osg_viewer->setCameraManipulator(m_osg_trackballManip);
    m_osg_viewer->getCameraManipulator()->setHomePosition(camEye,camViewPt,camUp);
    m_osg_viewer->getCameraManipulator()->home(0);
    m_osg_viewer->getCamera()->setClearColor(osg::Vec4(0.1,0.1,0.1,1.0));
    m_osg_viewer->realize();
    updateGL();

    // start the camera update timer
    connect(&m_updateTimer,SIGNAL(timeout()),
            this,SLOT(onUpdateScene()));
    m_updateTimer.setInterval(1000);
    m_updateTimer.start();

    m_loadedMap = true;
}

void Viewport::onSetCameraLLA(double camLat, double camLon, double camAlt)
{
    // update scene camera
    osmscout::Camera const * myCam = m_mapRenderer->GetCamera();

    m_mapRenderer->SetCamera(osmscout::PointLLA(camLat,camLon,camAlt),
                             myCam->fovY,
                             myCam->aspectRatio);

    // now update viewer's camera (since the scene cam has been updated)
    // ... this is a little unintuitive
    myCam = m_mapRenderer->GetCamera();
    osg::Vec3d camEye(myCam->eye.x,myCam->eye.y,myCam->eye.z);
    osg::Vec3d camViewPt(myCam->viewPt.x,myCam->viewPt.y,myCam->viewPt.z);
    osg::Vec3d camUp(myCam->up.x,myCam->up.y,myCam->up.z);

    m_osg_trackballManip->setTransformation(camEye,camViewPt,camUp);

    updateGL();
}

void Viewport::onSetCameraMouseMode(int camMode)
{
    //
    if(camMode > 0 && camMode < 3)
    {   m_camMouseMode = camMode;   }

    std::cout << "Set Camera Mouse Mode" << camMode << std::endl;
}

void Viewport::onUpdateScene()
{
    // get current openscenegraph camera
    osg::Vec3d eye,viewPt,up;
    m_osg_trackballManip->getTransformation(eye,viewPt,up);

    // tell osmscout-render about the camera
    osmscout::Vec3 myEye(eye.x(),eye.y(),eye.z());
    osmscout::Vec3 myViewPt(viewPt.x(),viewPt.y(),viewPt.z());
    osmscout::Vec3 myUp(up.x(),up.y(),up.z());
    m_mapRenderer->UpdateCameraLookAt(myEye,myViewPt,myUp);

    updateGL();
}

void Viewport::startTiming(const std::string &desc)
{
    m_timingDesc = desc;
    gettimeofday(&m_t1,NULL);
}

void Viewport::endTiming()
{
    gettimeofday(&m_t2,NULL);
    double timeTaken = 0;
    timeTaken += (m_t2.tv_sec - m_t1.tv_sec) * 1000.0 * 1000.0;
    timeTaken += (m_t2.tv_usec - m_t1.tv_usec);
    std::cout << "INFO: " << m_timingDesc << ": \t\t"
              << timeTaken << " microseconds\n";
}

void Viewport::initializeGL()
{
    // setup openscenegraph viewport
    m_osg_viewer = new osgViewer::Viewer;
    m_osg_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    m_osg_window = m_osg_viewer->setUpViewerAsEmbeddedInWindow(0,0,this->width(),this->height());
}

void Viewport::paintGL()
{
//    this->startTiming("Rendering Frame");
    m_osg_viewer->frame();
//    this->endTiming();
}

void Viewport::mousePressEvent(QMouseEvent *event)
{
//    qDebug() << "Press";

    // we disable updating during mouse movement
    m_updateTimer.stop();

    int nx = event->pos().x();
    int ny = event->pos().y();
    int button = event->button();

    // osgGA mouse button numbering:
    // 1 for left, 2 for middle, 3 for right
    unsigned int osgMouseBtn = 0;
    switch(button)
    {
    case Qt::LeftButton:
        osgMouseBtn = 1;
        break;
    case Qt::MiddleButton:
        osgMouseBtn = 2;
        break;
    case Qt::RightButton:
        osgMouseBtn = 3;
        break;
    default:
        break;
    }

    // add event to osg manipulator
    m_osg_window->getEventQueue()->mouseButtonPress(nx,ny,osgMouseBtn);
    updateGL();
}

void Viewport::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug() << "Move";
    int nx = event->pos().x();
    int ny = event->pos().y();
    int button = event->button();

    // add event to osg manipulator
    Q_UNUSED(button);
    m_osg_window->getEventQueue()->mouseMotion(nx,ny);
    updateGL();
}

void Viewport::mouseReleaseEvent(QMouseEvent *event)
{
//    qDebug() << "Release";

    // reenable updates on mouse up
    this->onUpdateScene();
    m_updateTimer.start();

    int nx = event->pos().x();
    int ny = event->pos().y();
    int button = event->button();

    // osgGA mouse button numbering:
    // 1 for left, 2 for middle, 3 for right
    unsigned int osgMouseBtn = 0;
    switch(button)
    {
    case Qt::LeftButton:
        osgMouseBtn = 1;
        break;
    case Qt::MiddleButton:
        osgMouseBtn = 2;
        break;
    case Qt::RightButton:
        osgMouseBtn = 3;
        break;
    default:
        break;
    }

    // add event to osg manipulator
    m_osg_window->getEventQueue()->mouseButtonRelease(nx,ny,osgMouseBtn);
    updateGL();
}

void Viewport::debugCamera(const osmscout::Camera * myCam)
{
    qDebug() << "Debug Camera:";
    qDebug() << "Eye:"<<myCam->eye.x<<","<<myCam->eye.y<<","<<myCam->eye.z;
    qDebug() << "ViewPt:"<<myCam->viewPt.x<<","<<myCam->viewPt.y<<","<<myCam->viewPt.z;
    qDebug() << "Up:"<<myCam->up.x<<","<<myCam->up.y<<","<<myCam->up.z;
    qDebug() << "LLA:"<<myCam->LLA.lat<<","<<myCam->LLA.lon<<","<<myCam->LLA.alt;
}
