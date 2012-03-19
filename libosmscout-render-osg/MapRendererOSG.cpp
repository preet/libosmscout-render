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

#include "MapRendererOSG.h"

namespace osmscout
{

MapRendererOSG::MapRendererOSG(const Database *myDatabase) :
    MapRenderer(myDatabase)
{
    m_osg_root = new osg::Group;
    m_osg_osmNodes = new osg::Group;
    m_osg_osmWays = new osg::Group;
    m_osg_osmAreas = new osg::Group;

    m_osg_root->addChild(m_osg_osmNodes);
    m_osg_root->addChild(m_osg_osmWays);
    m_osg_root->addChild(m_osg_osmAreas);
}

MapRendererOSG::~MapRendererOSG() {}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::RenderFrame()
{}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayToScene(WayRenderData &wayData)
{
//    if(wayData.wayRef->GetId() != 24220832)
//    {   return;   }

//    OSRDEBUG << "INFO: Added Way "
//             << wayData.wayRef->GetId() << " to Scene Graph";

    // build up the way geometry (done as a triangle strip)
    osg::ref_ptr<osg::Vec3Array> listWayPts = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listWayTriStripPts = new osg::Vec3Array;
    double lineWidth = wayData.lineRenderStyle->GetLineWidth();

    listWayPts->resize(wayData.listPointData.size());
    for(int i=0; i < listWayPts->size(); i++)
    {
        listWayPts->at(i) = osg::Vec3(wayData.listPointData[i].x,
                                      wayData.listPointData[i].y,
                                      wayData.listPointData[i].z);
    }

    buildWayAsTriStrip(listWayPts,osg::Vec3(0,0,0),
                       lineWidth,listWayTriStripPts);

    // offset listTriWayStripPts to account for
    // OpenGL precision issues at large distances
    osg::Vec3 offsetVec(wayData.listPointData[0].x,
                        wayData.listPointData[0].y,
                        wayData.listPointData[0].z);

    for(int i=0; i < listWayTriStripPts->size(); i++)
    {   listWayTriStripPts->at(i) -= offsetVec;   }

    osg::ref_ptr<osg::Geometry> geomWay = new osg::Geometry;
    geomWay->setVertexArray(listWayTriStripPts.get());
    geomWay->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
                                                 listWayTriStripPts->size()));

    osg::ref_ptr<osg::Geode> nodeWay = new osg::Geode;
    nodeWay->addDrawable(geomWay.get());

    // the geometry needs to be parented with a matrix
    // transform node to implement a floating origin offset
    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(offsetVec.x(),
                                                    offsetVec.y(),
                                                    offsetVec.z()));
    nodeTransform->addChild(nodeWay.get());

    // add the nodes to the scene graph
    m_osg_osmWays->addChild(nodeTransform.get());

    // save a reference to this node
    wayData.geomPtr = nodeTransform.get();
}

void MapRendererOSG::removeWayFromScene(WayRenderData const &wayData)
{
//    OSRDEBUG << "INFO: Removed Way "
//             << wayData.wayRef->GetId() << " from Scene Graph";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::initScene()
{
    // render mode
    osg::PolygonMode *polygonMode = new osg::PolygonMode();
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    m_osg_root->getOrCreateStateSet()->setAttributeAndModes(polygonMode,osg::StateAttribute::ON);
    m_osg_root->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    OSRDEBUG << "INFO: MapRenderOSG Initialized Scene";
}

void MapRendererOSG::buildWayAsTriStrip(const osg::Vec3Array *listWayPoints,
                                        const osg::Vec3 &pointEarthCenter,
                                        double const lineWidth,
                                        osg::Vec3Array *listWayTriStripPts)
{
    osg::Vec3 vecOffset;            // vector in the direction of the line segment's offset

    osg::Vec3 vecWaySurface;        // vector along the current line segment of the way

    osg::Vec3 vecEarthCenter;       // vector from a point on the current line segment to
                                    // earth's center -- note that we're in a different
                                    // reference frame since the geometry as shifted to
                                    // account for position issues

    osg::ref_ptr<osg::Vec3Array> listOffsetPointsA = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listOffsetPointsB = new osg::Vec3Array;

    int listSize = listWayPoints->size();
    listOffsetPointsA->resize(listSize);
    listOffsetPointsB->resize(listSize);

    // offset the first point in the wayPoint list
    // using the normal to the first line segment
    vecEarthCenter = pointEarthCenter-listWayPoints->at(0);
    vecWaySurface = listWayPoints->at(1)-listWayPoints->at(0);

    vecOffset = (vecWaySurface^vecEarthCenter);
    vecOffset.normalize();
    vecOffset *= lineWidth*0.5;

    listOffsetPointsA->at(0) = (listWayPoints->at(0) + vecOffset);
    listOffsetPointsB->at(0) = (listWayPoints->at(0) - vecOffset);

    // points in the middle of the wayPoint list have two
    // offsets -- one for each the preceding segment and
    // one for the following segment
    for(int i=1; i < listSize-1; i++)
    {
        // vecOffset remains the same for the preceding segment
        listOffsetPointsA->at(i) = (listWayPoints->at(i) + vecOffset);
        listOffsetPointsB->at(i) = (listWayPoints->at(i) - vecOffset);

        // vecOffset is different for the following segment
        vecEarthCenter = pointEarthCenter-listWayPoints->at(i);
        vecWaySurface = listWayPoints->at(i+1)-listWayPoints->at(i);

        vecOffset = (vecWaySurface^vecEarthCenter);
        vecOffset.normalize();
        vecOffset *= lineWidth*0.5;

        listOffsetPointsA->at(i) = (listWayPoints->at(i) + vecOffset);
        listOffsetPointsB->at(i) = (listWayPoints->at(i) - vecOffset);
    }

    // offset the last point in the wayPoint list
    // using the normal to the last line segment
    vecEarthCenter = pointEarthCenter-listWayPoints->at(listSize-1);
    vecWaySurface = listWayPoints->at(listSize-1) - listWayPoints->at(listSize-2);

    vecOffset = (vecWaySurface^vecEarthCenter); // '^' is cross product
    vecOffset.normalize();
    vecOffset *= lineWidth*0.5;

    listOffsetPointsA->at(listSize-1) = (listWayPoints->at(listSize-1) + vecOffset);
    listOffsetPointsB->at(listSize-1) = (listWayPoints->at(listSize-1) - vecOffset);

    // build triangle strip
    for(int i=0; i < listSize; i++)
    {
        listWayTriStripPts->push_back(listOffsetPointsA->at(i));
        listWayTriStripPts->push_back(listOffsetPointsB->at(i));
    }

    // debug
//    for(int i=0; i < listWayTriStripPts->size(); i++)
//    {
//        OSRDEBUG << "(" << listWayTriStripPts->at(i).x()
//                 << "," << listWayTriStripPts->at(i).y()
//                 << "," << listWayTriStripPts->at(i).z()
//                 << ")";
//    }
}

}
