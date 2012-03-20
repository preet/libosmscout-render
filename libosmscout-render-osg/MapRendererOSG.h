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

#include <string>

#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PolygonMode>
#include <osg/MatrixTransform>

#include "MapRenderer.h"

namespace osmscout
{

class MapRendererOSG : public MapRenderer
{
public:
    MapRendererOSG(Database const *myDatabase);
    ~MapRendererOSG();

    // RenderFrame
    // *
    void RenderFrame();

    osg::ref_ptr<osg::Group> m_osg_root;
    osg::ref_ptr<osg::Group> m_osg_osmNodes;
    osg::ref_ptr<osg::Group> m_osg_osmWays;
    osg::ref_ptr<osg::Group> m_osg_osmAreas;

    osg::ref_ptr<osg::Geode> m_osg_earth;

private:
    void initScene();
    void addWayToScene(WayRenderData &wayData);
    void removeWayFromScene(WayRenderData const &wayData);

    void removeAllPrimitivesFromScene();

    void buildWayAsTriStrip(osg::Vec3Array const *listWayPoints,
                            osg::Vec3 const &ptEarthCenter,
                            double const lineWidth,
                            osg::Vec3Array *listWayTriStripPts);

};

}


#endif
