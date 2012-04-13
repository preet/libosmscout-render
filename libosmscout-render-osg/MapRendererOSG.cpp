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

    m_nodeRoot = new osg::Group;
    m_nodeWays = new osg::Group;
    m_nodeAreas = new osg::Group;
    m_nodeWays->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
}

MapRendererOSG::~MapRendererOSG() {}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::RenderFrame()
{}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::initScene()
{
    startTiming("MapRendererOSG: initScene()");

    osg::StateSet *waysStateSet = m_nodeWays->getOrCreateStateSet();
    waysStateSet->setMode(GL_BLEND,osg::StateAttribute::ON);

    endTiming();
    return;

//    osg::PolygonMode *polygonMode = new osg::PolygonMode();
//    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
//    m_osg_root->getOrCreateStateSet()->setAttributeAndModes(polygonMode,osg::StateAttribute::ON);

//    OSRDEBUG << "INFO: Lighting for Ways OFF";
//    m_osg_osmWays->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

//    OSRDEBUG << "INFO: Lighting for Areas OFF";
//    m_osg_osmAreas->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
}

void MapRendererOSG::rebuildStyleData(const std::vector<RenderStyleConfig*> &listRenderStyles)
{
    for(int i=0; i < listRenderStyles.size(); i++)
    {
        // build way materials list from style data
        std::vector<TypeId> listWayTypes;
        listRenderStyles[i]->GetWayTypes(listWayTypes);
        for(int j=0; j < listWayTypes.size(); j++)
        {
            LineRenderStyle * lineStyle =
                listRenderStyles[i]->GetWayLineRenderStyle(listWayTypes[j]);

            LabelRenderStyle * nameStyle =
                listRenderStyles[i]->GetWayNameLabelRenderStyle(listWayTypes[j]);

            if(!(lineStyle == NULL))
            {
                WayMaterial wayMat;
                wayMat.matId = lineStyle->GetId();

                wayMat.lineColor = new osg::Material;
                osg::Vec4 lineColor(lineStyle->GetLineColor().R,
                                    lineStyle->GetLineColor().G,
                                    lineStyle->GetLineColor().B,
                                    lineStyle->GetLineColor().A);
                wayMat.lineColor->setDiffuse(osg::Material::FRONT,lineColor);

                wayMat.outlineColor = new osg::Material;
                osg::Vec4 outlineColor(lineStyle->GetOutlineColor().R,
                                       lineStyle->GetOutlineColor().G,
                                       lineStyle->GetOutlineColor().B,
                                       lineStyle->GetOutlineColor().A);
                wayMat.outlineColor->setDiffuse(osg::Material::FRONT,outlineColor);

                m_listWayMaterials.push_back(wayMat);
            }

            if(!(nameStyle) == NULL)
            {
                LabelMaterial labMat;
                labMat.matId = nameStyle->GetId();

                labMat.fontColor = new osg::Material;
                osg::Vec4 fontColor(nameStyle->GetFontColor().R,
                                    nameStyle->GetFontColor().G,
                                    nameStyle->GetFontColor().B,
                                    nameStyle->GetFontColor().A);
                labMat.fontColor->setDiffuse(osg::Material::FRONT,fontColor);

                labMat.fontOutlineColor = new osg::Material;
                osg::Vec4 fontOutline(nameStyle->GetFontOutlineColor().R,
                                      nameStyle->GetFontOutlineColor().G,
                                      nameStyle->GetFontOutlineColor().B,
                                      nameStyle->GetFontOutlineColor().A);
                labMat.fontOutlineColor->setDiffuse(osg::Material::FRONT,fontOutline);

                if(nameStyle->GetLabelType() == LABEL_PLATE)
                {
                    labMat.plateColor = new osg::Material;
                    osg::Vec4 plateColor(nameStyle->GetPlateColor().R,
                                         nameStyle->GetPlateColor().G,
                                         nameStyle->GetPlateColor().B,
                                         nameStyle->GetPlateColor().A);
                    labMat.plateColor->setDiffuse(osg::Material::FRONT,plateColor);

                    labMat.plateOutlineColor = new osg::Material;
                    osg::Vec4 plateOutlineColor(nameStyle->GetPlateOutlineColor().R,
                                                nameStyle->GetPlateOutlineColor().G,
                                                nameStyle->GetPlateOutlineColor().B,
                                                nameStyle->GetPlateOutlineColor().A);
                    labMat.plateOutlineColor->setDiffuse(osg::Material::FRONT,plateOutlineColor);
                }
                m_listLabelMaterials.push_back(labMat);
            }
        }
    }

    std::sort(m_listWayMaterials.begin(),
              m_listWayMaterials.end(),
              WayMaterialCompare);

    std::sort(m_listLabelMaterials.begin(),
              m_listLabelMaterials.end(),
              LabelMaterialCompare);
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayToScene(WayRenderData &wayData)
{
    // the geometry needs to be parented with a matrix
    // transform node to implement a floating origin offset;
    // we arbitrarily use the first way point for the offset
    osg::Vec3d offsetVec(wayData.listWayPoints[0].x,
                         wayData.listWayPoints[0].y,
                         wayData.listWayPoints[0].z);

    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(offsetVec));

    // build way and add to transform node
    this->addWayGeometry(wayData,offsetVec,nodeTransform.get());

//    if(wayData.hasName)
//    {
//        if(wayData.nameLabelRenderStyle->GetLabelType() == LABEL_CONTOUR)
//        {   this->addContourLabel(wayData,offsetVec,nodeTransform,true);   }
//    }

    // add the transform node to the scene graph
    m_nodeWays->addChild(nodeTransform.get());

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

//    m_osg_osmWays->removeChild(wayNode->get());
    delete wayNode;

    //    OSRDEBUG << "INFO: Removed Way "
    //             << wayData.wayRef->GetId() << " to Scene Graph";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addAreaToScene(AreaRenderData &areaData)
{
    // use first border point for floating point offset
    osg::Vec3d offsetVec(areaData.centerPoint.x,
                         areaData.centerPoint.y,
                         areaData.centerPoint.z);

    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(offsetVec));

    // add area geometry
    this->addAreaGeometry(areaData,offsetVec,nodeTransform.get());

    // add area label
    if(areaData.hasName)
    {
        if(areaData.nameLabelRenderStyle->GetLabelType() == LABEL_DEFAULT)
        {   this->addDefaultLabel(areaData,offsetVec,nodeTransform.get(),true);   }

        else if(areaData.nameLabelRenderStyle->GetLabelType() == LABEL_PLATE)
        {   this->addPlateLabel(areaData,offsetVec,nodeTransform.get(),true);   }
    }

    // add the transform node to the scene graph
//    m_osg_osmAreas->addChild(nodeTransform.get());

    // save a reference to (a reference of) this node
    osg::ref_ptr<osg::Node> * nodeRefPtr = new osg::ref_ptr<osg::Node>;
    (*nodeRefPtr) = nodeTransform;
    areaData.geomPtr = nodeRefPtr;
}


void MapRendererOSG::removeAreaFromScene(const AreaRenderData &areaData)
{
    // recast areaData void* reference to osg::Node
    osg::ref_ptr<osg::Node> * areaNode =
            reinterpret_cast<osg::ref_ptr<osg::Node>*>(areaData.geomPtr);

//    m_osg_osmAreas->removeChild(areaNode->get());
    delete areaNode;

//        OSRDEBUG << "INFO: Removed Area "
//                 << areaData.areaRef->GetId() << " to Scene Graph";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::removeAllFromScene()
{
//    unsigned int numWays = m_osg_osmWays->getNumChildren();

//    if(numWays > 0)
//    {   m_osg_osmWays->removeChild(0,numWays);   }

    m_listWayMaterials.clear();
    m_listAreaMaterials.clear();
    m_listLabelMaterials.clear();

}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayGeometry(const WayRenderData &wayData,
                                    const osg::Vec3d &offsetVec,
                                    osg::MatrixTransform *nodeParent)
{
    osg::ref_ptr<osg::Vec3dArray> listWayPoints = new osg::Vec3dArray;
    listWayPoints->resize(wayData.listWayPoints.size());

    for(int i=0; i < listWayPoints->size(); i++)
    {
        listWayPoints->at(i) = osg::Vec3d(wayData.listWayPoints[i].x,
                                          wayData.listWayPoints[i].y,
                                          wayData.listWayPoints[i].z);
    }

    double lineWidth = wayData.lineRenderStyle->GetLineWidth();

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

    // build triangle strip with offset (offsetVec is the 'global' offset)
    k = 0;
    for(int i=0; i < numOffsets*2; i+=2)
    {
        listWayTriStripPts->at(i) = listOffsetPointsA->at(k) - offsetVec;
        listWayTriStripPts->at(i+1) = listOffsetPointsB->at(k) - offsetVec;
        k++;
    }

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

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addAreaGeometry(const AreaRenderData &areaData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent)
{

}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addDefaultLabel(const AreaRenderData &areaData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{

}

void MapRendererOSG::addPlateLabel(const AreaRenderData &areaData,
                                   const osg::Vec3d &offsetVec,
                                   osg::MatrixTransform *nodeParent,
                                   bool usingName)
{

}

void MapRendererOSG::addContourLabel(const WayRenderData &wayData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{

}


// ========================================================================== //
// ========================================================================== //

double MapRendererOSG::calcWayLength(const osg::Vec3dArray *listWayPoints)
{
    double totalDist = 0;
    for(int i=1; i < listWayPoints->size(); i++)
    {   totalDist += (listWayPoints->at(i)-listWayPoints->at(i-1)).length();   }

    return totalDist;
}

void MapRendererOSG::calcWaySegmentLengths(const osg::Vec3dArray *listWayPoints,
                                           std::vector<double> &listSegLengths)
{
    double totalDist = 0;
    listSegLengths.resize(listWayPoints->size(),0);
    for(int i=1; i < listWayPoints->size(); i++)
    {
        totalDist += (listWayPoints->at(i)-listWayPoints->at(i-1)).length();
        listSegLengths[i] = totalDist;
    }
}

void MapRendererOSG::calcLerpAlongWay(const osg::Vec3dArray *listWayPoints,
                                      const osg::Vec3dArray *listWayNormals,
                                      const double lengthAlongWay,
                                      osg::Vec3d &pointAtLength,
                                      osg::Vec3d &dirnAtLength,
                                      osg::Vec3d &normalAtLength,
                                      osg::Vec3d &sideAtLength)
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

    sideAtLength = dirnAtLength^normalAtLength;

    dirnAtLength.normalize();
    normalAtLength.normalize();
    sideAtLength.normalize();
}

void MapRendererOSG::startTiming(const std::string &desc)
{
    m_timingDesc = desc;
    gettimeofday(&m_t1,NULL);
}

void MapRendererOSG::endTiming()
{
    gettimeofday(&m_t2,NULL);
    double timeTaken = 0;
    timeTaken += (m_t2.tv_sec - m_t1.tv_sec) * 1000.0 * 1000.0;
    timeTaken += (m_t2.tv_usec - m_t1.tv_usec);
    OSRDEBUG << "INFO: " << m_timingDesc << ": \t\t"
              << timeTaken << " microseconds";
}

}
