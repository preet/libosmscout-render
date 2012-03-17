#ifndef OSMSCOUT_MAP_RENDERER_OSG_H
#define OSMSCOUT_MAP_RENDERER_OSG_H

#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>

#include "MapRenderer.h"

namespace osmscout
{

class MapRendererOSG : public MapRenderer
{
public:
    MapRendererOSG(Database const *myDatabase);
    ~MapRendererOSG();

    // InitializeScene
    // *
    void InitializeScene(PointLLA const &camEye,
                         CameraMode camMode);

    // RenderFrame
    // *
    void RenderFrame();

    osg::ref_ptr<osg::Group> m_osg_root;

protected:
    void AddWayToScene(WayRenderData &wayData);
    void RemoveWayFromScene(WayRenderData const &wayData);

private:
    void buildWayAsTriStrip(osg::Vec3Array const *listWayPoints,
                            osg::Vec3 const &ptEarthCenter,
                            double const lineWidth,
                            osg::Vec3Array *listWayTriStripPts);


    osg::ref_ptr<osg::Group> m_osg_osmNodes;
    osg::ref_ptr<osg::Group> m_osg_osmWays;
    osg::ref_ptr<osg::Group> m_osg_osmAreas;


};

}


#endif
