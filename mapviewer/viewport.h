#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <sys/time.h>

#include <QGLWidget>
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

// osmscout
#include <osmscout/Database.h>

// osmscout-render
#include "MapRendererOSG.h"

// openscenegraph
#include <osgViewer/Viewer>
#include <osgGA/CameraManipulator>
#include <osgGA/TrackballManipulator>

class Viewport : public QGLWidget
{
    Q_OBJECT

public:
    explicit Viewport(QWidget *parent = 0);
    ~Viewport();
    QSize sizeHint() const;

    //


signals:
    
public slots:
    void onLoadMap(QString const &mapPath, QString const &stylePath);
    void onSetCameraLLA(double camLat, double camLon, double camAlt);
    void onSetCameraMouseMode(int camMode);
    void onUpdateScene();

    void startTiming(std::string const &desc);
    void endTiming();
    
private:
    void initializeGL();
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void debugCamera(osmsrender::Camera const * myCam);

    // osmscout
    osmsrender::DataSetOSM * m_dataset_osm;

    osmscout::DatabaseParameter * m_databaseParam;
    osmscout::Database * m_database;
    osmsrender::MapRendererOSG * m_mapRenderer;

    // openscenegraph setup
    osgViewer::Viewer * m_osg_viewer;
    osgViewer::GraphicsWindowEmbedded * m_osg_window;
    osg::ref_ptr<osgGA::TrackballManipulator> m_osg_trackballManip;

    QTimer m_updateTimer;
    bool m_loadedMap;

    // timing vars
    timeval m_t1,m_t2;
    std::string m_timingDesc;

    int m_camMouseMode;
};

#endif // VIEWPORT_H
