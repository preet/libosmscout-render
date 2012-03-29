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

void MapRendererOSG::initScene()
{
    // render mode
//    osg::PolygonMode *polygonMode = new osg::PolygonMode();
//    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
//    m_osg_root->getOrCreateStateSet()->setAttributeAndModes(polygonMode,osg::StateAttribute::ON);
    m_osg_root->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    OSRDEBUG << "INFO: MapRenderOSG Initialized Scene";
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

//    if(!(wayData.labelRenderData.nameLabelRenderStyle == NULL))
//    {   this->addWayNameLabel(wayData,nodeTransform.get());   }

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

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addAreaToScene(AreaRenderData &areaData)
{
    // use first border point for floating point offset
    osg::Vec3d offsetVec(areaData.listBorderPoints[0].x,
                         areaData.listBorderPoints[0].y,
                         areaData.listBorderPoints[0].z);

    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(offsetVec));

    // build area and add to transform node
    this->addAreaGeometry(areaData,offsetVec,nodeTransform.get());

    if(areaData.hasName)
    {
        osg::Vec3d centerVec(areaData.centerPoint.x,
                             areaData.centerPoint.y,
                             areaData.centerPoint.z);

        this->addDefaultLabel(areaData.nameLabel,
                              areaData.nameLabelRenderStyle,
                              centerVec,offsetVec,nodeTransform.get());
    }

    // add the transform node to the scene graph
    m_osg_osmAreas->addChild(nodeTransform.get());

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

    m_osg_osmWays->removeChild(areaNode->get());
    delete areaNode;
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::removeAllFromScene()
{
    unsigned int numWays = m_osg_osmWays->getNumChildren();

    if(numWays > 0)
    {   m_osg_osmWays->removeChild(0,numWays);   }

}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayNameLabel(const WayRenderData &wayData,
                                     osg::MatrixTransform *nodeParent)
{
//    LabelRenderData const &labelRenderData = wayData.labelRenderData;

//    if(labelRenderData.nameLabelRenderStyle->GetLabelType() == LABEL_CONTOUR)
//    {
//        // create osgText::Text objects for each character
//        // in the way name, and save their width dims
//        unsigned int numChars = labelRenderData.nameLabel.size();
//        std::vector<osg::ref_ptr<osgText::Text> > listChars(numChars);
//        std::vector<osg::BoundingBox> listCharBounds(numChars);

//        double nameLength = 0;
//        double maxCHeight = 0;
//        double minCHeight = 0;
//        for(int i=0; i < numChars; i++)
//        {
//            // create the character geometry
//            osg::ref_ptr<osgText::Text> textChar = new osgText::Text;
//            textChar->setFont("res/DroidSans-Bold.ttf");
//            textChar->setCharacterSize(10.0);
//            std::string charStr = labelRenderData.nameLabel.substr(i,1);

//            // since space chars by themselves return a quad
//            // with zero dims, we replace them with hyphens
//            // and set their opacity to zero
//            if(charStr.compare(" ") == 0)
//            {
//                textChar->setText("-");
//                textChar->setColor(osg::Vec4(0,0,0,0));
//            }
//            else
//            {
//                textChar->setText(charStr);
//                textChar->setColor(osg::Vec4(0,0,0,1));
////                textChar->setBackdropType(osgText::Text::OUTLINE);
//            }

//            //
//            textChar->setAlignment(osgText::Text::CENTER_BASE_LINE);

//            // save ref and bounds
//            listChars[i] = textChar;
//            listCharBounds[i] = textChar->getBound();

//            nameLength += (textChar->getBound().xMax() -
//                            textChar->getBound().xMin()) * 1.15;

//            if(i > 0)
//            {
//                maxCHeight = std::max(textChar->getBound().yMax(),
//                                         listCharBounds[i-1].yMax());

//                minCHeight = std::min(textChar->getBound().yMin(),
//                                         listCharBounds[i-1].yMin());
//            }
//        }

//        // get way points
//        osg::ref_ptr<osg::Vec3dArray> listWayPoints = new osg::Vec3dArray;
//        listWayPoints->resize(wayData.listWayPoints.size());
//        for(int i=0; i < listWayPoints->size(); i++)
//        {
//            listWayPoints->at(i) = osg::Vec3d(wayData.listWayPoints[i].second.x,
//                                              wayData.listWayPoints[i].second.y,
//                                              wayData.listWayPoints[i].second.z);
//        }

//        double wayLength = calcWayLength(listWayPoints);

//        if(wayLength < nameLength*1.15)
//        {   return;   }

//        double charWidth = 0;
//        double prevCharWidth = 0;
//        double startLength = (wayLength-nameLength)/2.0;
//        double lengthAlongPath = startLength;
//        double baselineOffset = (maxCHeight-minCHeight)/2.0-minCHeight;

//        osg::Vec3d const &offsetVec = listWayPoints->at(0);
//        osg::Vec3d pointAtLength,dirnAtLength,normAtLength,sideAtLength;

//        // apply transform to get text chars aligned to way
//        for(int i=0; i < listChars.size(); i++)
//        {
//            prevCharWidth = charWidth;
//            charWidth = listCharBounds[i].xMax()-
//                    listCharBounds[i].xMin();

//            lengthAlongPath += ((charWidth+prevCharWidth)/2.0);

//            calcLerpAlongWay(listWayPoints,
//                             listWayPoints,
//                             lengthAlongPath,
//                             pointAtLength,
//                             dirnAtLength,
//                             normAtLength,
//                             sideAtLength);

//            pointAtLength += sideAtLength*baselineOffset;

//            osg::Matrixd xformMatrix;

//            xformMatrix(0,0) = dirnAtLength.x();
//            xformMatrix(0,1) = dirnAtLength.y();
//            xformMatrix(0,2) = dirnAtLength.z();
//            xformMatrix(0,3) = 0;

//            xformMatrix(1,0) = sideAtLength.x()*-1;
//            xformMatrix(1,1) = sideAtLength.y()*-1;
//            xformMatrix(1,2) = sideAtLength.z()*-1;
//            xformMatrix(1,3) = 0;

//            xformMatrix(2,0) = normAtLength.x();
//            xformMatrix(2,1) = normAtLength.y();
//            xformMatrix(2,2) = normAtLength.z();
//            xformMatrix(2,3) = 0;

//            pointAtLength += normAtLength;
//            xformMatrix(3,0) = pointAtLength.x()-offsetVec.x();
//            xformMatrix(3,1) = pointAtLength.y()-offsetVec.y();
//            xformMatrix(3,2) = pointAtLength.z()-offsetVec.z();
//            xformMatrix(3,3) = 1;

//            osg::ref_ptr<osg::Geode> charNode = new osg::Geode;
//            charNode->addDrawable(listChars[i].get());

//            osg::ref_ptr<osg::MatrixTransform> xformNode =
//                    new osg::MatrixTransform;
//            xformNode->setMatrix(xformMatrix);
//            xformNode->addChild(charNode.get());

//            nodeParent->addChild(xformNode.get());
//        }
//    }
//    else if(labelRenderData.nameLabelRenderStyle->GetLabelType() == LABEL_DEFAULT)
//    {
//        double labelWidth = 0;
//        double labelHeight = 0;

//        osg::ref_ptr<osgText::Text> labelText = new osgText::Text;
//        labelText->setFont("res/DroidSans-Bold.ttf");
//        labelText->setCharacterSize(12.0);
//        labelText->setText(labelRenderData.nameLabel);
//        labelText->setColor(osg::Vec4(1,1,1,1));
//        labelText->setAlignment(osgText::Text::CENTER_BASE_LINE);

//        labelWidth = labelText->getBound().xMax()-
//                     labelText->getBound().xMin();

//        labelHeight = labelText->getBound().yMax()-
//                      labelText->getBound().yMin();

//        labelText->setAxisAlignment(osgText::TextBase::SCREEN);

//        // get the center point of the way in ecef coords
//        double cLat,cLon;
//        wayData.wayRef->GetCenter(cLat,cLon);
//        Vec3 labelCenter = convLLAToECEF(PointLLA(cLat,cLon,labelHeight));

//        // get floating point corr. offset vector
//        osg::Vec3d offsetVec(wayData.listWayPoints[0].second.x,
//                             wayData.listWayPoints[0].second.y,
//                             wayData.listWayPoints[0].second.z);

//        labelText->setPosition(osg::Vec3d(labelCenter.x-offsetVec.x(),
//                                          labelCenter.y-offsetVec.y(),
//                                          labelCenter.z-offsetVec.z()));

//        osg::ref_ptr<osg::Geode> labelNode = new osg::Geode;
//        labelNode->addDrawable(labelText.get());

//        nodeParent->addChild(labelNode.get());
//    }
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

    // intersection test
    osg::ref_ptr<osg::Geode> nodeXPoints = new osg::Geode;
    nodeParent->addChild(nodeXPoints.get());
    for(int i=0; i < listWayPoints->size(); i++)
    {
        if(wayData.listSharedNodes[i])
        {
            osg::ref_ptr<osg::ShapeDrawable> xsecMarker = new osg::ShapeDrawable;
            xsecMarker->setShape(new osg::Box(listWayPoints->at(i)-offsetVec,9.0));
            xsecMarker->setColor(osg::Vec4d(0,0,0,1));
            nodeXPoints->addDrawable(xsecMarker.get());
        }
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
//    osg::ref_ptr<osg::Vec3dArray> listBorderPoints = new osg::Vec3dArray;
//    listBorderPoints->resize(areaData.listBorderPoints.size());

//    for(int i=0; i < listWayPoints->size(); i++)
//    {
//        listBorderPoints->at(i) = osg::Vec3d(areaData.listBorderPoints[i].x,
//                                             areaData.listBorderPoints[i].y,
//                                             areaData.listBorderPoints[i].z);
//    }

}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addDefaultLabel(const std::string &labelName,
                                     const LabelRenderStyle *labelRenderStyle,
                                     const osg::Vec3d &centerVec,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent)
{}

void MapRendererOSG::addPlateLabel(const std::string &labelName,
                                   const LabelRenderStyle *labelRenderStyle,
                                   const osg::Vec3d &centerVec,
                                   const osg::Vec3d &offsetVec,
                                   osg::MatrixTransform *nodeParent)
{}

void MapRendererOSG::addContourLabel(const WayRenderData &wayData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent)
{
    // create osgText::Text objects for each character
    // in the way name, and save their width dims
    unsigned int numChars = wayData.nameLabel.size();
    std::vector<osg::ref_ptr<osgText::Text> > listChars(numChars);
    std::vector<osg::BoundingBox> listCharBounds(numChars);

    double nameLength = 0;
    double maxCHeight = 0;
    double minCHeight = 0;
    for(int i=0; i < numChars; i++)
    {
        // create the character geometry
        osg::ref_ptr<osgText::Text> textChar = new osgText::Text;
        textChar->setFont("res/DroidSans-Bold.ttf");
        textChar->setCharacterSize(10.0);
        std::string charStr = wayData.nameLabel.substr(i,1);

        // since space chars by themselves return a quad
        // with zero dims, we replace them with hyphens and
        // set their opacity to zero to estimate dimensions
        if(charStr.compare(" ") == 0)
        {
            textChar->setText("-");
            textChar->setColor(osg::Vec4(0,0,0,0));
        }
        else
        {
            textChar->setText(charStr);
            textChar->setColor(osg::Vec4(0,0,0,1));
            //textChar->setBackdropType(osgText::Text::OUTLINE);
        }

        textChar->setAlignment(osgText::Text::CENTER_BASE_LINE);

        // save ref and bounds
        listChars[i] = textChar;
        listCharBounds[i] = textChar->getBound();
        nameLength += (textChar->getBound().xMax() -
                       textChar->getBound().xMin()) * 1.15;

        if(i > 0)
        {
            maxCHeight = std::max(textChar->getBound().yMax(),
                                  listCharBounds[i-1].yMax());

            minCHeight = std::min(textChar->getBound().yMin(),
                                  listCharBounds[i-1].yMin());
        }
    }

    // get way centerline
    osg::ref_ptr<osg::Vec3dArray> listWayPoints = new osg::Vec3dArray;
    listWayPoints->resize(wayData.listWayPoints.size());
    for(int i=0; i < listWayPoints->size(); i++)
    {
        listWayPoints->at(i) = osg::Vec3d(wayData.listWayPoints[i].x,
                                          wayData.listWayPoints[i].y,
                                          wayData.listWayPoints[i].z);
    }

    std::vector<double> listSegLengths;
    calcWaySegmentLengths(listWayPoints,listSegLengths);
    if(nameLength*1.15 > listSegLengths.back())
    {
        OSRDEBUG << "WARN: Label " << wayData.nameLabel
                 << " length exceeds wayLength";
        return;
    }

    // we need to find suitable lengths along the way to
    // draw the label without interfering with intersections
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
                                           std::vector<double> listSegLengths)
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

}
