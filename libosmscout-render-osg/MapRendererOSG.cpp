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

    // build up earth geometry
    std::vector<Vec3> earthVertices;
    std::vector<Vec3> earthNormals;
    std::vector<Vec2> earthTexCoords;
    std::vector<unsigned int> earthIndices;

    if(!buildEarthSurfaceGeometry(36,48,
                                  earthVertices,
                                  earthNormals,
                                  earthTexCoords,
                                  earthIndices))
    {
        OSRDEBUG << "WARN: Failed to create Earth geometry";
        return;
    }

    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array;

    osg::ref_ptr<osg::DrawElementsUInt> pointCloud =
            new osg::DrawElementsUInt(GL_POINTS,earthIndices.size());

    for(int i=0; i < earthVertices.size(); i++)
    {
        vertexArray->push_back(osg::Vec3(earthVertices.at(i).x,
                                         earthVertices.at(i).y,
                                         earthVertices.at(i).z));
    }

    for(int i=0; i < earthIndices.size(); i++)
    {   pointCloud->push_back(earthIndices.at(i));   }

    osg::ref_ptr<osg::Geometry> earthGeom = new osg::Geometry;
    earthGeom->setVertexArray(vertexArray.get());
    earthGeom->addPrimitiveSet(pointCloud.get());

    m_osg_earth = new osg::Geode;
    m_osg_earth->addDrawable(earthGeom.get());
    //m_osg_root->addChild(m_osg_earth.get());
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
    // build up the way geometry (done as a triangle strip)
    osg::ref_ptr<osg::Vec3Array> listWayPts = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listWayTriStripPts = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> listWayVertColors = new osg::Vec4Array;

    osmscout::ColorRGBA wayColor = wayData.lineRenderStyle->GetLineColor();
    osg::Vec4 colorVec(wayColor.R,wayColor.G,wayColor.B,wayColor.A);

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

    // add color data
    listWayVertColors->push_back(colorVec);

    osg::ref_ptr<osg::Geometry> geomWay = new osg::Geometry;
    geomWay->setColorArray(listWayVertColors.get());
    geomWay->setColorBinding(osg::Geometry::BIND_OVERALL);
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

    // save a reference to (a reference of) this node
    osg::ref_ptr<osg::Node> * nodeRefPtr = new osg::ref_ptr<osg::Node>;
    (*nodeRefPtr) = nodeTransform;
    wayData.geomPtr = nodeRefPtr;

    //    OSRDEBUG << "INFO: Added Way "
    //             << wayData.wayRef->GetId() << " to Scene Graph";
}

void MapRendererOSG::removeWayFromScene(WayRenderData const &wayData)
{
    // recast wayData void* reference to osg::Node
    osg::ref_ptr<osg::Node> * wayNode =
            reinterpret_cast<osg::ref_ptr<osg::Node>*>(wayData.geomPtr);

    m_osg_osmWays->removeChild(wayNode->get());
    delete wayNode;

    //    OSRDEBUG << "INFO: Removed Way "
    //             << wayData.wayRef->GetId() << " to Scene Graph";
}

void MapRendererOSG::removeAllPrimitivesFromScene()
{
    unsigned int numWays = m_osg_osmWays->getNumChildren();

    if(numWays > 0)
    {   m_osg_osmWays->removeChild(0,numWays);   }

}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::initScene()
{
    // render mode
//    osg::PolygonMode *polygonMode = new osg::PolygonMode();
//    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
//    m_osg_root->getOrCreateStateSet()->setAttributeAndModes(polygonMode,osg::StateAttribute::ON);
//    m_osg_root->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

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

    int listSize = listWayPoints->size();
    int numOffsets = (listSize*2)-2;        // two for every point that isn't an endpoint
    int k = 0;                              // current offset index

    osg::ref_ptr<osg::Vec3Array> listOffsetPointsA = new osg::Vec3Array(numOffsets);
    osg::ref_ptr<osg::Vec3Array> listOffsetPointsB = new osg::Vec3Array(numOffsets);

    // offset the first point in the wayPoint list
    // using the normal to the first line segment
    vecEarthCenter = pointEarthCenter-listWayPoints->at(0);
    vecWaySurface = listWayPoints->at(1)-listWayPoints->at(0);

    vecOffset = (vecWaySurface^vecEarthCenter);
    vecOffset.normalize();
    vecOffset *= lineWidth*0.5;

    listOffsetPointsA->at(k) = (listWayPoints->at(0) + vecOffset);
    listOffsetPointsB->at(k) = (listWayPoints->at(0) - vecOffset);
    k++;

    // points in the middle of the wayPoint list have two
    // offsets -- one for each the preceding segment and
    // one for the following segment
    for(int i=1; i < listSize-1; i++)
    {
        // vecOffset remains the same for the preceding segment
        listOffsetPointsA->at(k) = (listWayPoints->at(i) + vecOffset);
        listOffsetPointsB->at(k) = (listWayPoints->at(i) - vecOffset);
        k++;

        // vecOffset is different for the following segment
        vecEarthCenter = pointEarthCenter-listWayPoints->at(i);
        vecWaySurface = listWayPoints->at(i+1)-listWayPoints->at(i);

        vecOffset = (vecWaySurface^vecEarthCenter);
        vecOffset.normalize();
        vecOffset *= lineWidth*0.5;

        listOffsetPointsA->at(k) = (listWayPoints->at(i) + vecOffset);
        listOffsetPointsB->at(k) = (listWayPoints->at(i) - vecOffset);
        k++;
    }

    // offset the last point in the wayPoint list
    // using the normal to the last line segment
    vecEarthCenter = pointEarthCenter-listWayPoints->at(listSize-1);
    vecWaySurface = listWayPoints->at(listSize-1) - listWayPoints->at(listSize-2);

    vecOffset = (vecWaySurface^vecEarthCenter);
    vecOffset.normalize();
    vecOffset *= lineWidth*0.5;

    listOffsetPointsA->at(k) = (listWayPoints->at(listSize-1) + vecOffset);
    listOffsetPointsB->at(k) = (listWayPoints->at(listSize-1) - vecOffset);

    // build triangle strip
    k = 0;
    listWayTriStripPts->resize(numOffsets*2);
    for(int i=0; i < numOffsets*2; i+=2)
    {
        listWayTriStripPts->at(i) = listOffsetPointsA->at(k);
        listWayTriStripPts->at(i+1) = listOffsetPointsB->at(k);
        k++;
    }
}

}
