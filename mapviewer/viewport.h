/*
    This source is a part of libosmscout-render

    Copyright (C) 2012, Preet Desai

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <sys/time.h>

#include <QGLWidget>
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

// openscenegraph
#include <osgViewer/Viewer>
#include <osgGA/CameraManipulator>
#include <osgGA/TrackballManipulator>

// libosmscout
#include <osmscout/TypeConfigLoader.h>
#include <osmscout/Database.h>

// libosmscout-render
#include <libosmscout-render/RenderStyleReader.h>
#include <libosmscout-render-osg/MapRendererOSG.h>

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

    // data
    osmsrender::DataSetOSM * m_dataset_osm;
    osmsrender::DataSetTemp * m_dataset_temp;
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
