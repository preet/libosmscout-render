#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QGLWidget>
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

// osmscout
#include <osmscout/Database.h>

// osmscout-render
#include "RenderStyleConfigReader.h"
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

    QSize sizeHint() const;

    //

    
signals:
    
public slots:
    void onLoadMap(QString const &mapPath, QString const &stylePath);
    void onSetCameraLLA(double camLat, double camLon, double camAlt);
    void onUpdateScene();
    
private:
    void initializeGL();
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void debugCamera(osmscout::Camera const * myCam);

    // osmscout
    osmscout::DatabaseParameter * m_databaseParam;
    osmscout::Database * m_database;
    osmscout::MapRendererOSG * m_mapRenderer;
    std::vector<osmscout::RenderStyleConfig*> m_listStyleConfigs;

    // openscenegraph setup
    osgViewer::Viewer * m_osg_viewer;
    osgViewer::GraphicsWindowEmbedded * m_osg_window;
    osg::ref_ptr<osgGA::TrackballManipulator> m_osg_trackballManip;

    QTimer m_updateTimer;

    bool m_loadedMap;
};

#endif // VIEWPORT_H
