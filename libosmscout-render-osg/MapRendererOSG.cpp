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

void MapRendererOSG::initScene()
{

//    startTiming("MapRendererOSG: initScene()");

    // render mode
//    osg::PolygonMode *polygonMode = new osg::PolygonMode();
//    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
//    m_osg_root->getOrCreateStateSet()->setAttributeAndModes(polygonMode,osg::StateAttribute::ON);

//    OSRDEBUG << "INFO: OpenGL Scene Lighting OFF";
    m_osg_osmWays->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    // build prelim font cache
    std::vector<std::string> listFonts;
    getFontList(listFonts);

    m_fontGeoMap.clear();
    m_fontGeoMap.reserve(listFonts.size());

//    OSRDEBUG << "INFO: Font Types Found: ";
    for(int i=0; i < listFonts.size(); i++)
    {
//        OSRDEBUG << "INFO:   " << listFonts[i];

        CharGeoMap fontChars;
        fontChars.reserve(100);

        // TODO handle multiple locales
        std::string baseCharList("abcdefghijklmnopqrstuvwxyz"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "0123456789 '.-");

        for(int j=0; j < baseCharList.size(); j++)
        {
            std::string charStr = baseCharList.substr(j,1);
            osg::ref_ptr<osgText::Text> textChar = new osgText::Text;

            // note: since space chars return a quad with zero
            // dims, we replace them with hyphens to est. dims
            // and set opacity to zero
            textChar->setAlignment(osgText::Text::CENTER_BASE_LINE);
            textChar->setFont(listFonts[i]);
            textChar->setColor(osg::Vec4(1,1,1,1));
            textChar->setCharacterSize(1.0);

            if(charStr.compare(" ") == 0)
            {   textChar->setText("-");       }
            else
            {   textChar->setText(charStr);   }

            std::pair<std::string,osg::ref_ptr<osgText::Text> > fChar;
            fChar.first = charStr;
            fChar.second = textChar;
            fontChars.insert(fChar);
        }

        // save fontChars in all fonts list
        std::pair<std::string,CharGeoMap> fC(listFonts[i],fontChars);
        m_fontGeoMap.insert(fC);
    }

//    endTiming();
//    OSRDEBUG << "INFO: MapRenderOSG Initialized Scene";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayToScene(WayRenderData &wayData)
{
    return;

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

    if(wayData.hasName)
    {
        if(wayData.nameLabelRenderStyle->GetLabelType() == LABEL_CONTOUR)
        {   this->addContourLabel(wayData,offsetVec,nodeTransform,true);   }
    }

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

    m_osg_osmAreas->removeChild(areaNode->get());
    delete areaNode;

//        OSRDEBUG << "INFO: Removed Area "
//                 << areaData.areaRef->GetId() << " to Scene Graph";
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
//    osg::ref_ptr<osg::Geode> nodeXPoints = new osg::Geode;
//    nodeParent->addChild(nodeXPoints.get());
//    for(int i=0; i < listWayPoints->size(); i++)
//    {
//        if(wayData.listSharedNodes[i])
//        {
//            osg::ref_ptr<osg::ShapeDrawable> xsecMarker = new osg::ShapeDrawable;
//            xsecMarker->setShape(new osg::Box(listWayPoints->at(i)-offsetVec,9.0));
//            xsecMarker->setColor(osg::Vec4d(0,0,0,1));
//            nodeXPoints->addDrawable(xsecMarker.get());
//        }
//    }

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
    // calculate area base (earth surface) normal
    osg::Vec3d areaBaseNormal(areaData.centerPoint.x,
                              areaData.centerPoint.y,
                              areaData.centerPoint.z);
    areaBaseNormal.normalize();

    if(areaData.isBuilding)
    {
        int numBaseVerts = areaData.listBorderPoints.size();
        double const &bHeight = areaData.buildingData->height;

        osg::ref_ptr<osg::Geometry> geomRoof = new osg::Geometry;
        osg::ref_ptr<osg::Geometry> geomSides = new osg::Geometry;

        // add vertices for base, than roof (using Vec3 because
        // osgUtil::Tessellator has an issue with Vec3d?)
        osg::ref_ptr<osg::Vec3Array> listBaseVertices = new osg::Vec3Array(numBaseVerts);
        osg::ref_ptr<osg::Vec3Array> listRoofVertices = new osg::Vec3Array(numBaseVerts);

        for(int i=0; i < numBaseVerts; i++)
        {
            double px = areaData.listBorderPoints[i].x - offsetVec.x();
            double py = areaData.listBorderPoints[i].y - offsetVec.y();
            double pz = areaData.listBorderPoints[i].z - offsetVec.z();

            listBaseVertices->at(i) = osg::Vec3(px,py,pz);
            listRoofVertices->at(i) = osg::Vec3(px,py,pz)+(areaBaseNormal*bHeight);
        }

        // colors
        osg::ref_ptr<osg::Vec4Array> listAreaColors = new osg::Vec4Array;
        listAreaColors->push_back(osg::Vec4(0.5,0.5,0.5,1));

        // build roof
        osg::ref_ptr<osg::Vec3dArray> listRoofNormals = new osg::Vec3dArray;
        listRoofNormals->push_back(areaBaseNormal);

        geomRoof->setVertexArray(listRoofVertices.get());
        geomRoof->setNormalArray(listRoofNormals.get());
        geomRoof->setNormalBinding(osg::Geometry::BIND_OVERALL);
        geomRoof->setColorArray(listAreaColors.get());
        geomRoof->setColorBinding(osg::Geometry::BIND_OVERALL);
        geomRoof->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN,0,
                                                      numBaseVerts));
        osgUtil::Tessellator roofTess;
        roofTess.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        roofTess.retessellatePolygons(*geomRoof);

        // build side walls (2 tris * 3 pts) / edge
        osg::ref_ptr<osg::Vec3dArray> listSideVertices =
                new osg::Vec3dArray(listBaseVertices->size()*6);

        osg::ref_ptr<osg::Vec3dArray> listSideNormals =
                new osg::Vec3dArray(listBaseVertices->size()*6);

        double normalFix = (areaData.pathIsCCW) ? 1 : -1;

        // temporarily increase base/roof verts to go full circle
        listBaseVertices->push_back(listBaseVertices->at(0));
        listRoofVertices->push_back(listRoofVertices->at(0));

        for(int i=0; i < listBaseVertices->size()-1; i++)
        {
            unsigned int n=i*6;
            osg::Vec3d alongSide = listBaseVertices->at(i+1)-listBaseVertices->at(i);
            osg::Vec3d alongHeight = listRoofVertices->at(i)-listBaseVertices->at(i);
            osg::Vec3d sideNormal = (alongSide^alongHeight)*normalFix;
            sideNormal.normalize();

            // triangle 1 vertices
            listSideVertices->at(n) = listBaseVertices->at(i);
            listSideVertices->at(n+1) = listBaseVertices->at(i+1);
            listSideVertices->at(n+2) = listRoofVertices->at(i);

            // triangle 1 normals
            listSideNormals->at(n) = sideNormal;
            listSideNormals->at(n+1) = sideNormal;
            listSideNormals->at(n+2) = sideNormal;

            // triangle 2 vertices
            listSideVertices->at(n+3) = listRoofVertices->at(i);
            listSideVertices->at(n+4) = listBaseVertices->at(i+1);
            listSideVertices->at(n+5) = listRoofVertices->at(i+1);

            // triangle 2 normals
            listSideNormals->at(n+3) = sideNormal;
            listSideNormals->at(n+4) = sideNormal;
            listSideNormals->at(n+5) = sideNormal;
        }
        listBaseVertices->pop_back();
        listRoofVertices->pop_back();

        osg::ref_ptr<osg::DrawElementsUInt> listTriIndex =
                new osg::DrawElementsUInt(GL_TRIANGLES,listSideVertices->size());

        for(int i=0; i < listSideVertices->size(); i++)
        {   listTriIndex->at(i) = i;   }

        geomSides->setVertexArray(listSideVertices.get());
        geomSides->setNormalArray(listSideNormals.get());
        geomSides->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geomSides->setColorArray(listAreaColors.get());
        geomSides->setColorBinding(osg::Geometry::BIND_OVERALL);
        geomSides->addPrimitiveSet(listTriIndex.get());

        // add geometry to parent node
        osg::ref_ptr<osg::Geode> nodeArea = new osg::Geode;
        nodeArea->addDrawable(geomRoof.get());
        nodeArea->addDrawable(geomSides.get());
        nodeParent->addChild(nodeArea.get());
    }
    else
    {
        return; // TODO
        osg::ref_ptr<osg::Geometry> geomArea = new osg::Geometry;

        // using Vec3 because of osgUtil::Tessellator issue
        osg::ref_ptr<osg::Vec3Array> listBorderPoints = new osg::Vec3Array;
        listBorderPoints->resize(areaData.listBorderPoints.size());

        for(int i=0; i < listBorderPoints->size(); i++)
        {
            double px = areaData.listBorderPoints[i].x - offsetVec.x();
            double py = areaData.listBorderPoints[i].y - offsetVec.y();
            double pz = areaData.listBorderPoints[i].z - offsetVec.z();

            listBorderPoints->at(i) = osg::Vec3(px,py,pz);
        }

        // set color
        osg::ref_ptr<osg::Vec4Array> listAreaColors = new osg::Vec4Array;
        listAreaColors->push_back(osg::Vec4(1,1,0,1));

        // save geometry
        geomArea->setVertexArray(listBorderPoints.get());
        geomArea->setColorArray(listAreaColors.get());
        geomArea->setColorBinding(osg::Geometry::BIND_OVERALL);
        geomArea->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN,0,
                                                      listBorderPoints->size()));

        // NOTE: tessellator only supports Vec3 (not Vec3d)?
        osgUtil::Tessellator geomTess;
        geomTess.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        geomTess.retessellatePolygons(*geomArea);

        osg::ref_ptr<osg::Geode> nodeArea = new osg::Geode;
        nodeArea->addDrawable(geomArea.get());

        // add geometry to parent node
        nodeParent->addChild(nodeArea.get());
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addDefaultLabel(const AreaRenderData &areaData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{
    std::string labelName;
    LabelRenderStyle const *labelStyle;

    if(usingName)
    {
        labelName = areaData.nameLabel;
        labelStyle = areaData.nameLabelRenderStyle;
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    double heightBuff = labelStyle->GetOffsetHeight();
    osg::Vec3d btmCenterVec(areaData.centerPoint.x,
                            areaData.centerPoint.y,
                            areaData.centerPoint.z);

    osg::Vec3d heightVec = btmCenterVec;
    heightVec.normalize();

    // adjust heightBuff is area is building
    if(areaData.isBuilding)
    {   heightVec *= (areaData.buildingData->height + heightBuff);   }
    else
    {   heightVec *= heightBuff;   }

    // use the max bounding box length of the base
    // as a rough metric for setting max label width
    double xMin = areaData.listBorderPoints[0].x; double xMax = xMin;
    double yMin = areaData.listBorderPoints[0].y; double yMax = yMin;
    double zMin = areaData.listBorderPoints[0].z; double zMax = zMin;

    for(int i=1; i < areaData.listBorderPoints.size(); i++)
    {
        Vec3 const &areaPoint = areaData.listBorderPoints[i];

        xMin = std::min(xMin,areaPoint.x);
        xMax = std::max(xMax,areaPoint.x);

        yMin = std::min(yMin,areaPoint.y);
        yMax = std::max(yMax,areaPoint.y);

        zMin = std::min(zMin,areaPoint.z);
        zMax = std::max(zMax,areaPoint.z);
    }

    osg::ref_ptr<osgText::Text> labelText = new osgText::Text;
    labelText->setFont(labelStyle->GetFontFamily());
    labelText->setAlignment(osgText::Text::CENTER_BOTTOM);
    labelText->setAxisAlignment(osgText::TextBase::SCREEN);
    labelText->setCharacterSize(labelStyle->GetFontSize());
    labelText->setPosition(btmCenterVec+heightVec-offsetVec);
    labelText->setText(labelName);

    ColorRGBA fontColor = labelStyle->GetFontColor();
    labelText->setColor(osg::Vec4(fontColor.R,
                                  fontColor.G,
                                  fontColor.B,
                                  fontColor.A));

    // add newlines to the text label so it better reflects
    // the shape of the area its attached to -- we use the
    // bounding box computed earlier and insert newlines
    // by comparing the label width, and the max bbox width
    double maxLabelWidth;
    maxLabelWidth = std::max(xMax-xMin,yMax-yMin);
    maxLabelWidth = std::max(maxLabelWidth,zMax-zMin);

    int breakChar = -1;
    while(true)     // NOTE: this expects labelName to initially
    {               //       have NO newlines, "\n", etc!
        double fracLength = (labelText->getBound().xMax()-
                labelText->getBound().xMin()) / maxLabelWidth;

        if(fracLength <= 1)
        {   break;   }

        if(breakChar == -1)
        {   breakChar = ((1/fracLength)*labelName.size())-1;   }

        // find all instances of (" ") in label
        std::vector<unsigned int> listPosSP;
        unsigned int pos = labelName.find(" ",0);
        while(pos != std::string::npos) {
            listPosSP.push_back(pos);
            pos = labelName.find(" ",pos+1);
        }

        if(listPosSP.size() == 0)
        {   break;   }

        // insert a newline at the (" ") closest to breakChar
        unsigned int cPos = 0;
        for(int i=0; i < listPosSP.size(); i++)  {
            if(abs(breakChar-listPosSP[i]) < abs(breakChar-listPosSP[cPos]))
            {   cPos = i;   }
        }

        labelName.replace(listPosSP[cPos],1,"\n");
        labelText->setText(labelName);
    }

    osg::ref_ptr<osg::Geode> labelNode = new osg::Geode;
    labelNode->addDrawable(labelText.get());
    nodeParent->addChild(labelNode.get());
}

void MapRendererOSG::addPlateLabel(const AreaRenderData &areaData,
                                   const osg::Vec3d &offsetVec,
                                   osg::MatrixTransform *nodeParent,
                                   bool usingName)
{
    std::string labelName;
    LabelRenderStyle const *labelStyle;

    if(usingName)
    {
        labelName = areaData.nameLabel;
        labelStyle = areaData.nameLabelRenderStyle;
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    osg::Vec3d btmCenterVec(areaData.centerPoint.x,
                            areaData.centerPoint.y,
                            areaData.centerPoint.z);

    osg::Vec3d surfNorm = btmCenterVec;
    surfNorm.normalize();

    // use the max bounding box length of the base area
    // as a rough metric for setting max label width
    double xMin = areaData.listBorderPoints[0].x; double xMax = xMin;
    double yMin = areaData.listBorderPoints[0].y; double yMax = yMin;
    double zMin = areaData.listBorderPoints[0].z; double zMax = zMin;

    for(int i=1; i < areaData.listBorderPoints.size(); i++)
    {
        Vec3 const &areaPoint = areaData.listBorderPoints[i];
        xMin = std::min(xMin,areaPoint.x);
        xMax = std::max(xMax,areaPoint.x);
        yMin = std::min(yMin,areaPoint.y);
        yMax = std::max(yMax,areaPoint.y);
        zMin = std::min(zMin,areaPoint.z);
        zMax = std::max(zMax,areaPoint.z);
    }

    osg::ref_ptr<osgText::Text> labelText = new osgText::Text;
    labelText->setFont(labelStyle->GetFontFamily());
    labelText->setAlignment(osgText::Text::CENTER_CENTER);
    labelText->setCharacterSize(labelStyle->GetFontSize());
    labelText->setText(labelName);

    ColorRGBA fontColor = labelStyle->GetFontColor();
    labelText->setColor(osg::Vec4(fontColor.R,
                                  fontColor.G,
                                  fontColor.B,
                                  fontColor.A));

    // add newlines to the text label so it better reflects
    // the shape of the area its attached to -- we use the
    // bounding box computed earlier and insert newlines
    // by comparing the label width, and the max bbox width
    double maxLabelWidth;
    maxLabelWidth = std::max(xMax-xMin,yMax-yMin);
    maxLabelWidth = std::max(maxLabelWidth,zMax-zMin);

    int breakChar = -1;
    while(true)     // NOTE: this expects labelName to initially
    {               //       have NO newlines, "\n", etc!
        double fracLength = (labelText->getBound().xMax()-
                labelText->getBound().xMin()) / maxLabelWidth;

        if(fracLength <= 1)
        {   break;   }

        if(breakChar == -1)
        {   breakChar = ((1/fracLength)*labelName.size())-1;   }

        // find all instances of (" ") in label
        std::vector<unsigned int> listPosSP;
        unsigned int pos = labelName.find(" ",0);
        while(pos != std::string::npos) {
            listPosSP.push_back(pos);
            pos = labelName.find(" ",pos+1);
        }

        if(listPosSP.size() == 0)
        {   break;   }

        // insert a newline at the (" ") closest to breakChar
        unsigned int cPos = 0;
        for(int i=0; i < listPosSP.size(); i++)  {
            if(abs(breakChar-listPosSP[i]) < abs(breakChar-listPosSP[cPos]))
            {   cPos = i;   }
        }

        labelName.replace(listPosSP[cPos],1,"\n");
        labelText->setText(labelName);
        labelText->update();
    }

    double platePadding = labelStyle->GetPlatePadding();
    double plateOutlineWidth = labelStyle->GetPlateOutlineWidth();

    // note: yMin and yMax don't have the correct
    // positioning but they have the right relative
    // distance, so only yHeight is a valid metric in y
    xMin = labelText->computeBound().xMin();// - platePadding;
    xMax = labelText->computeBound().xMax();// + platePadding;
    yMin = labelText->computeBound().yMin();// - platePadding;
    yMax = labelText->computeBound().yMax();// + platePadding;
    double yHeight = yMax-yMin;

    // calculate the offsetHeight
    double offsetHeight = labelStyle->GetOffsetHeight()+(yHeight/2.0);

    if(areaData.isBuilding)
    {   offsetHeight += areaData.buildingData->height;   }

    osg::Vec3d shiftVec = btmCenterVec+(surfNorm*offsetHeight)-offsetVec;
//    labelText->setPosition(shiftVec);

    // use the label bounding box+padding to create the plate
    osg::ref_ptr<osg::Geometry> labelPlate = new osg::Geometry;
    osg::ref_ptr<osg::Vec3dArray> pVerts = new osg::Vec3dArray(8);
    osg::ref_ptr<osg::Vec4Array> pColors = new osg::Vec4Array(8);
    osg::ref_ptr<osg::DrawElementsUInt> pIdxs =
            new osg::DrawElementsUInt(GL_TRIANGLES,6);
    osg::ref_ptr<osg::DrawElementsUInt> pIdxsOL =
            new osg::DrawElementsUInt(GL_TRIANGLES,6);

    // build up plate vertices
    yHeight += (2*platePadding);
    xMin -= platePadding; xMax += platePadding;
    pVerts->at(0) = osg::Vec3d(xMin,-1*(yHeight/2),-0.1);   // bl
    pVerts->at(1) = osg::Vec3d(xMax,-1*(yHeight/2),-0.1);   // br
    pVerts->at(2) = osg::Vec3d(xMax,(yHeight/2),-0.1);   // tr
    pVerts->at(3) = osg::Vec3d(xMin,(yHeight/2),-0.1);   // tl

    yHeight += (2*plateOutlineWidth);
    xMin -= plateOutlineWidth; xMax += plateOutlineWidth;
    pVerts->at(4) = osg::Vec3d(xMin,-1*(yHeight/2),-0.15);   // bl
    pVerts->at(5) = osg::Vec3d(xMax,-1*(yHeight/2),-0.15);   // br
    pVerts->at(6) = osg::Vec3d(xMax,(yHeight/2),-0.15);   // tr
    pVerts->at(7) = osg::Vec3d(xMin,(yHeight/2),-0.15);   // tl

    // build up plate tris
    pIdxs->at(0) = 0;   pIdxs->at(1) = 1;   pIdxs->at(2) = 2;
    pIdxs->at(3) = 0;   pIdxs->at(4) = 2;   pIdxs->at(5) = 3;

    pIdxsOL->at(0) = 4;   pIdxsOL->at(1) = 5;   pIdxsOL->at(2) = 6;
    pIdxsOL->at(3) = 4;   pIdxsOL->at(4) = 6;   pIdxsOL->at(5) = 7;

    // set color
    ColorRGBA plateColor = labelStyle->GetPlateColor();
    osg::Vec4 plateColorVec = osg::Vec4(plateColor.R,
                                        plateColor.G,
                                        plateColor.B,
                                        plateColor.A);

    ColorRGBA plateColorOL = labelStyle->GetPlateOutlineColor();
    osg::Vec4 plateOutlineColorVec = osg::Vec4(plateColorOL.R,
                                               plateColorOL.G,
                                               plateColorOL.B,
                                               plateColorOL.A);
    pColors->at(0) = plateColorVec;
    pColors->at(1) = plateOutlineColorVec;

    labelPlate->setVertexArray(pVerts.get());
    labelPlate->addPrimitiveSet(pIdxs.get());
    labelPlate->addPrimitiveSet(pIdxsOL.get());
    labelPlate->setColorArray(pColors.get());
    labelPlate->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);

    osg::ref_ptr<osg::Billboard> labelNode = new osg::Billboard;
    labelNode->setMode(osg::Billboard::POINT_ROT_EYE);
    labelNode->setNormal(osg::Vec3d(0,0,1));
    labelNode->addDrawable(labelPlate.get(),shiftVec);
    labelNode->addDrawable(labelText.get(),shiftVec);

    nodeParent->addChild(labelNode.get());
}

void MapRendererOSG::addPlateLabel(const std::string &labelName,
                                   const LabelRenderStyle *labelRenderStyle,
                                   const osg::Vec3d &centerVec,
                                   const osg::Vec3d &offsetVec,
                                   osg::MatrixTransform *nodeParent)
{}

void MapRendererOSG::addContourLabel(const WayRenderData &wayData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{
    // set predefined vars up based on name or ref
    std::string const *labelText;
    LabelRenderStyle const *labelStyle;
    ColorRGBA fontColor;
    double fontSize;
    double labelPadding;

    if(usingName)
    {
        labelText = &(wayData.nameLabel);
        labelStyle = wayData.nameLabelRenderStyle;
        fontSize = labelStyle->GetFontSize();
        fontColor = labelStyle->GetFontColor();
        labelPadding = labelStyle->GetContourPadding();
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    // look up font char list
    FontGeoMap::iterator fListIt = m_fontGeoMap.find(labelStyle->GetFontFamily());
    CharGeoMap &fontCharsMap = fListIt->second;

    // create osgText::Text objects for each character
    // in the way name, and save their width dims
    unsigned int numChars = labelText->size();
    std::vector<osg::ref_ptr<osgText::Text> > listChars(numChars);
    std::vector<osg::BoundingBox> listCharBounds(numChars);

    double nameLength = 0;
    double maxCHeight = 0;
    double minCHeight = 0;

    CharGeoMap::iterator fCharIt;
    for(int i=0; i < numChars; i++)
    {
        // lookup font character
        std::string charStr = labelText->substr(i,1);
        fCharIt = fontCharsMap.find(charStr);
        if(fCharIt == fontCharsMap.end())
        {
            // character dne in list, add it
            osg::ref_ptr<osgText::Text> textChar = new osgText::Text;
            textChar->setAlignment(osgText::Text::CENTER_BASE_LINE);
            textChar->setFont(labelStyle->GetFontFamily());
            textChar->setColor(osg::Vec4(1,1,1,1));
            textChar->setCharacterSize(1.0);
            textChar->setText(charStr);

            std::pair<std::string,osg::ref_ptr<osgText::Text> > addChar;
            addChar.first = charStr;
            addChar.second = textChar;

            fCharIt = fontCharsMap.insert(addChar).first;

            OSRDEBUG << "INFO: Added char " << charStr
                     << " for font " << labelStyle->GetFontFamily();
        }

        osg::ref_ptr<osgText::Text> textChar = dynamic_cast<osgText::Text*>
                ((fCharIt->second)->clone(osg::CopyOp::DEEP_COPY_ALL));

        textChar->setCharacterSize(fontSize);
        textChar->setColor(osg::Vec4(fontColor.R,
                                     fontColor.G,
                                     fontColor.B,
                                     fontColor.A));

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

    double baselineOffset = (maxCHeight-minCHeight)/2.0 - minCHeight;

    // we need to find suitable lengths along the way to
    // draw the label without interfering with intersections

    // given a way name of length L, the total length taken
    // up by a single label will be (approx)

    // _________123 Sesame St_________
    // <---A---><----1.0----><---A--->
    // (where A is the fractional labelPadding length)

    // we divide the total length of the way by the label
    // length to see how many labels will fit in the path

    std::vector<double> listSegLengths;
    calcWaySegmentLengths(listWayPoints,listSegLengths);
    double labelLength = (nameLength + 2.0*labelPadding*nameLength);
    int numLabelsFit = listSegLengths.back()/labelLength;

    // check that at least one label can fit within the way
    if(listSegLengths.back()/labelLength < 1.0)
    {
//        OSRDEBUG << "WARN: Label " << wayData.nameLabel
//                 << " length exceeds wayLength";
        return;
    }

    // get a list of cumulative lengths for this way's
    // shared nodes (intersection points with other ways)
    std::vector<double> listSharedNodeLengths;
    listSharedNodeLengths.push_back(0);
    for(int i=1; i < wayData.listSharedNodes.size()-1; i++)
    {
        if(wayData.listSharedNodes[i])
        {   listSharedNodeLengths.push_back(listSegLengths[i]);   }
    }
    listSharedNodeLengths.push_back(listSegLengths.back());

    // fit as many labels as possible in the length
    // available on the way between intersection nodes
    for(int i=1; i < listSharedNodeLengths.size(); i++)
    {
        double labelSpace = (listSharedNodeLengths[i]-
                             listSharedNodeLengths[i-1]);

        double labelOffset = listSharedNodeLengths[i-1];

        if(labelSpace > labelLength)
        {
            numLabelsFit = (labelSpace/labelLength);

            // space to leave in between each label
            double tweenSpace = (labelSpace-numLabelsFit*
                                 labelLength)/(numLabelsFit+1);

            for(int j=0; j < numLabelsFit; j++)
            {               
                // define the start and end lengths of the label
                double startLength = labelOffset +
                        (j+1)*tweenSpace + j*labelLength;

                double charWidth = 0;
                double prevCharWidth = 0;
                double lengthAlongPath = startLength;

                osg::Matrixd xformMatrix;
                osg::Vec3d pointAtLength,dirnAtLength,
                           normAtLength,sideAtLength;

                // apply transform to align chars to way
                for(int k=0; k < listChars.size(); k++)
                {
                    prevCharWidth = charWidth;
                    charWidth = listCharBounds[k].xMax()-listCharBounds[k].xMin();
                    lengthAlongPath += ((charWidth+prevCharWidth)/2.0);

                    calcLerpAlongWay(listWayPoints,
                                     listWayPoints,
                                     lengthAlongPath,
                                     pointAtLength,
                                     dirnAtLength,
                                     normAtLength,
                                     sideAtLength);

                    pointAtLength += sideAtLength*baselineOffset;

                    xformMatrix(0,0) = dirnAtLength.x();
                    xformMatrix(0,1) = dirnAtLength.y();
                    xformMatrix(0,2) = dirnAtLength.z();
                    xformMatrix(0,3) = 0;

                    xformMatrix(1,0) = sideAtLength.x()*-1;
                    xformMatrix(1,1) = sideAtLength.y()*-1;
                    xformMatrix(1,2) = sideAtLength.z()*-1;
                    xformMatrix(1,3) = 0;

                    xformMatrix(2,0) = normAtLength.x();
                    xformMatrix(2,1) = normAtLength.y();
                    xformMatrix(2,2) = normAtLength.z();
                    xformMatrix(2,3) = 0;

                    pointAtLength += normAtLength;
                    xformMatrix(3,0) = pointAtLength.x()-offsetVec.x();
                    xformMatrix(3,1) = pointAtLength.y()-offsetVec.y();
                    xformMatrix(3,2) = pointAtLength.z()-offsetVec.z();
                    xformMatrix(3,3) = 1;

                    osg::ref_ptr<osg::Geode> charNode = new osg::Geode;
                    charNode->addDrawable(listChars[k].get());

                    osg::ref_ptr<osg::MatrixTransform> xformNode =
                            new osg::MatrixTransform;
                    xformNode->setMatrix(xformMatrix);
                    xformNode->addChild(charNode.get());

                    nodeParent->addChild(xformNode.get());
                }
            }
        }
    }
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
