/*
    libosmscout-render

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

#ifndef OSMSCOUT_MAP_RENDERER_OSG_H
#define OSMSCOUT_MAP_RENDERER_OSG_H

#include <sys/time.h>
#include <string>
#include <sstream>

#include <osg/ref_ptr>
#include <osg/Vec3d>
#include <osg/Geode>
#include <osgText/Text>
#include <osg/Geometry>
#include <osg/Billboard>
#include <osg/LineWidth>
#include <osg/PolygonMode>
#include <osgText/TextBase>
#include <osg/ShapeDrawable>
#include <osgUtil/Tessellator>
#include <osg/MatrixTransform>

#include "MapRenderer.h"

//#include "poly2tri/poly2tri.h"

namespace osmscout
{

typedef std::unordered_map<std::string,osg::ref_ptr<osgText::Text> > CharGeoMap;
typedef std::unordered_map<std::string,CharGeoMap> FontGeoMap;

class MapRendererOSG : public MapRenderer
{
public:
    MapRendererOSG(Database const *myDatabase);
    ~MapRendererOSG();

    // RenderFrame
    // *
    void RenderFrame();

    // todo: why are all these members public again?
    osg::ref_ptr<osg::Group> m_osg_root;
    osg::ref_ptr<osg::Group> m_osg_osmNodes;
    osg::ref_ptr<osg::Group> m_osg_osmWays;
    osg::ref_ptr<osg::Group> m_osg_osmAreas;



    osg::ref_ptr<osg::Geode> m_osg_earth;

private:
    void initScene();

    void addWayToScene(WayRenderData &wayData);
    void removeAreaFromScene(AreaRenderData const &areaData);

    void addAreaToScene(AreaRenderData &areaData);
    void removeWayFromScene(WayRenderData const &wayData);

    void removeAllFromScene();

    void addWayGeometry(WayRenderData const &wayData,
                        osg::Vec3d const &offsetVec,
                        osg::MatrixTransform *nodeParent);

    void addAreaGeometry(AreaRenderData const &areaData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent);

    void addDefaultLabel(AreaRenderData const &areaData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent,
                         bool usingName);

    void addPlateLabel(AreaRenderData const &areaData,
                       osg::Vec3d const &offsetVec,
                       osg::MatrixTransform *nodeParent,
                       bool usingName);

    void addContourLabel(WayRenderData const &wayData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent,
                         bool usingName);

    double calcWayLength(osg::Vec3dArray const *listWayPoints);

    void calcWaySegmentLengths(osg::Vec3dArray const *listWayPoints,
                               std::vector<double> &listSegLengths);

    void calcLerpAlongWay(osg::Vec3dArray const *listWayPoints,
                          osg::Vec3dArray const *listWayNormals,
                          double const lengthAlongWay,
                          osg::Vec3d &pointAtLength,
                          osg::Vec3d &dirnAtLength,
                          osg::Vec3d &normalAtLength,
                          osg::Vec3d &sideAtLength);

    void startTiming(std::string const &desc);
    void endTiming();

    //
    FontGeoMap m_fontGeoMap;

    timeval m_t1,m_t2;
    std::string m_timingDesc;
};

}


#endif
