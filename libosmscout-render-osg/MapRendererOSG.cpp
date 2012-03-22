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

    osg::ref_ptr<osg::Vec3dArray> vertexArray = new osg::Vec3dArray;

    osg::ref_ptr<osg::DrawElementsUInt> pointCloud =
            new osg::DrawElementsUInt(GL_POINTS,earthIndices.size());

    for(int i=0; i < earthVertices.size(); i++)
    {
        vertexArray->push_back(osg::Vec3d(earthVertices.at(i).x,
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
    // the geometry needs to be parented with a matrix
    // transform node to implement a floating origin offset
    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(wayData.listPointData[0].x,
                                                    wayData.listPointData[0].y,
                                                    wayData.listPointData[0].z));

    // add way attributes to the transform node
    this->addWayGeometry(wayData,nodeTransform.get());

    if(!(wayData.nameLabelRenderStyle == NULL))
    {   this->addWayNameLabel(wayData,nodeTransform.get());   }

    // add the transform node to the scene graph
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
    m_osg_root->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    OSRDEBUG << "INFO: MapRenderOSG Initialized Scene";
}


void MapRendererOSG::addWayGeometry(const WayRenderData &wayData,
                                    osg::MatrixTransform *nodeParent)
{
    // get preliminary way data
    double lineWidth = wayData.lineRenderStyle->GetLineWidth();

    osg::ref_ptr<osg::Vec3dArray> listWayPoints = new osg::Vec3dArray;
    listWayPoints->resize(wayData.listPointData.size());
    for(int i=0; i < listWayPoints->size(); i++)
    {
        listWayPoints->at(i) = osg::Vec3d(wayData.listPointData[i].x,
                                          wayData.listPointData[i].y,
                                          wayData.listPointData[i].z);
    }

    osg::Vec3d vecOffset;            // vector in the direction of the line segment's offset

    osg::Vec3d vecWaySurface;        // vector along the current line segment of the way

    osg::Vec3d vecEarthCenter;       // vector from a point on the current line segment to
                                     // earth's center -- note that we're in a different
                                     // reference frame since the geometry as shifted to
                                     // account for position issues

    // represents Earth's center in (x,y,z)
    osg::Vec3d pointEarthCenter(0,0,0);

    int listSize = listWayPoints->size();
    int numOffsets = (listSize*2)-2;        // two for every point that isn't an endpoint
    int k = 0;                              // current offset index

    osg::ref_ptr<osg::Vec3dArray> listOffsetPointsA =
            new osg::Vec3dArray(numOffsets);

    osg::ref_ptr<osg::Vec3dArray> listOffsetPointsB =
            new osg::Vec3dArray(numOffsets);

    osg::ref_ptr<osg::Vec3dArray> listWayTriStripPts=
            new osg::Vec3dArray(numOffsets*2);

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
    for(int i=0; i < numOffsets*2; i+=2)
    {
        listWayTriStripPts->at(i) = listOffsetPointsA->at(k);
        listWayTriStripPts->at(i+1) = listOffsetPointsB->at(k);
        k++;
    }

    // offset listTriWayStripPts to account for
    // OpenGL precision issues at large distances
    osg::Vec3d offsetVec(wayData.listPointData[0].x,
                         wayData.listPointData[0].y,
                         wayData.listPointData[0].z);

    for(int i=0; i < listWayTriStripPts->size(); i++)
    {   listWayTriStripPts->at(i) -= offsetVec;   }

    // set color data
    osmscout::ColorRGBA wayColor = wayData.lineRenderStyle->GetLineColor();
    osg::ref_ptr<osg::Vec4Array> listWayVertColors = new osg::Vec4Array;
    listWayVertColors->push_back(osg::Vec4(wayColor.R,
                                           wayColor.G,
                                           wayColor.B,
                                           wayColor.A));

    // save geometry
    osg::ref_ptr<osg::Geometry> geomWay = new osg::Geometry;
    geomWay->setColorArray(listWayVertColors.get());
    geomWay->setColorBinding(osg::Geometry::BIND_OVERALL);
    geomWay->setVertexArray(listWayTriStripPts.get());
    geomWay->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
                                                 listWayTriStripPts->size()));

    osg::ref_ptr<osg::Geode> nodeWay = new osg::Geode;
    nodeWay->addDrawable(geomWay.get());

    // add geometry to parent node
    nodeParent->addChild(nodeWay.get());
}

void MapRendererOSG::addWayNameLabel(const WayRenderData &wayData,
                                     osg::MatrixTransform *nodeParent)
{
    std::string labelType;
    if(wayData.nameLabelRenderStyle->GetLabelType() == LABEL_DEFAULT)
    {   labelType = "default";   }
    else if(wayData.nameLabelRenderStyle->GetLabelType() == LABEL_CONTOUR)
    {   labelType = "contour";   }

    OSRDEBUG << "INFO: Added " << labelType << " label type";
}

double MapRendererOSG::calcWayLength(const osg::Vec3dArray *listWayPoints)
{
    double totalDist = 0;
    for(int i=1; i < listWayPoints->size(); i++)
    {   totalDist += (listWayPoints->at(i)-listWayPoints->at(i-1)).length();   }

    return totalDist;
}

void MapRendererOSG::calcLerpAlongWay(const osg::Vec3dArray *listWayPoints,
                                      const osg::Vec3dArray *listWayNormals,
                                      const double lengthAlongWay,
                                      osg::Vec3d &pointAtLength,
                                      osg::Vec3d &dirnAtLength,
                                      osg::Vec3d &normalAtLength)
{
    int i = 0; osg::Vec3d distVec;
    double lengthAlongWayPrev = 0;
    double lengthAlongWayNext = 0;
    for(i=1; i < listWayPoints->size(); i++)
    {
        distVec = (listWayPoints->at(i)-listWayPoints->at(i-1));
        lengthAlongWayPrev = lengthAlongWayNext;
        lengthAlongWayNext += distVec.length();

        if(lengthAlongWayNext >= lengthAlongWay)
        {   break;   }
    }

    double fAlongSegment = (lengthAlongWay-lengthAlongWayPrev)/
            (lengthAlongWayNext-lengthAlongWayPrev);

    dirnAtLength = distVec;

    pointAtLength = listWayPoints->at(i-1) +
            (distVec)*fAlongSegment;

    normalAtLength = listWayNormals->at(i-1) +
            (listWayNormals->at(i) - listWayNormals->at(i-1))*fAlongSegment;

}

}
