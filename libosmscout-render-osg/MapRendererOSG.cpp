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

    m_nodeRoot->addChild(m_nodeWays.get());
    m_nodeRoot->addChild(m_nodeAreas.get());

    m_nodeWays->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    m_nodeAreas->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);

    m_maxWayLayer = 0;
    m_maxAreaLayer = 0;
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
        // WAY MATERIAL TYPES
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
                wayMat.lineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(lineStyle->GetLineColor()));

                wayMat.outlineColor = new osg::Material;
                wayMat.outlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(lineStyle->GetOutlineColor()));

                m_listWayMaterials.push_back(wayMat);
            }

            if(!(nameStyle) == NULL)
            {
                LabelMaterial labMat;
                labMat.matId = nameStyle->GetId();

                labMat.fontColor = new osg::Material;
                labMat.fontColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(nameStyle->GetFontColor()));

                labMat.fontOutlineColor = new osg::Material;
                labMat.fontOutlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(nameStyle->GetFontOutlineColor()));

                if(nameStyle->GetLabelType() == LABEL_PLATE)
                {
                    labMat.plateColor = new osg::Material;
                    labMat.plateColor->setDiffuse(osg::Material::FRONT,
                        colorAsVec4(nameStyle->GetPlateColor()));

                    labMat.plateOutlineColor = new osg::Material;
                    labMat.plateOutlineColor->setDiffuse(osg::Material::FRONT,
                        colorAsVec4(nameStyle->GetPlateOutlineColor()));
                }
                m_listLabelMaterials.push_back(labMat);
            }
        }

        // AREA MATERIAL TYPES
        std::vector<TypeId> listAreaTypes;
        listRenderStyles[i]->GetAreaTypes(listAreaTypes);
        for(int j=0; j < listAreaTypes.size(); j++)
        {
            FillRenderStyle * fillStyle =
                listRenderStyles[i]->GetAreaFillRenderStyle(listAreaTypes[j]);

            LabelRenderStyle * nameStyle =
                listRenderStyles[i]->GetAreaNameLabelRenderStyle(listAreaTypes[j]);

            if(!(fillStyle == NULL))
            {
                AreaMaterial areaMat;
                areaMat.matId = fillStyle->GetId();

                areaMat.fillColor = new osg::Material;
                areaMat.fillColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetFillColor()));

                areaMat.outlineColor = new osg::Material;
                areaMat.outlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetOutlineColor()));

                m_listAreaMaterials.push_back(areaMat);
            }

            if(!(nameStyle) == NULL)
            {
                LabelMaterial labMat;
                labMat.matId = nameStyle->GetId();

                labMat.fontColor = new osg::Material;
                labMat.fontColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(nameStyle->GetFontColor()));

                labMat.fontOutlineColor = new osg::Material;
                labMat.fontOutlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(nameStyle->GetFontOutlineColor()));

                if(nameStyle->GetLabelType() == LABEL_PLATE)
                {
                    labMat.plateColor = new osg::Material;
                    labMat.plateColor->setDiffuse(osg::Material::FRONT,
                        colorAsVec4(nameStyle->GetPlateColor()));

                    labMat.plateOutlineColor = new osg::Material;
                    labMat.plateOutlineColor->setDiffuse(osg::Material::FRONT,
                        colorAsVec4(nameStyle->GetPlateOutlineColor()));
                }
                m_listLabelMaterials.push_back(labMat);
            }
        }
    }

    std::sort(m_listWayMaterials.begin(),
              m_listWayMaterials.end(),
              WayMaterialCompare);

    std::sort(m_listAreaMaterials.begin(),
              m_listAreaMaterials.end(),
              AreaMaterialCompare);

    std::sort(m_listLabelMaterials.begin(),
              m_listLabelMaterials.end(),
              LabelMaterialCompare);

    m_maxWayLayer = this->getMaxWayLayer();
    m_maxAreaLayer = this->getMaxAreaLayer();
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayToScene(WayRenderData &wayData)
{
//    return;
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

    m_nodeWays->removeChild(wayNode->get());
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
//    if(areaData.hasName)
//    {
//        if(areaData.nameLabelRenderStyle->GetLabelType() == LABEL_DEFAULT)
//        {   this->addDefaultLabel(areaData,offsetVec,nodeTransform.get(),true);   }

//        else if(areaData.nameLabelRenderStyle->GetLabelType() == LABEL_PLATE)
//        {   this->addPlateLabel(areaData,offsetVec,nodeTransform.get(),true);   }
//    }

    // add the transform node to the scene graph
    m_nodeAreas->addChild(nodeTransform.get());

    // save a reference to (a reference of) this node
    osg::ref_ptr<osg::Node> * nodeRefPtr = new osg::ref_ptr<osg::Node>;
    (*nodeRefPtr) = nodeTransform;
    areaData.geomPtr = nodeRefPtr;
}


void MapRendererOSG::removeAreaFromScene(const AreaRenderData &areaData)
{
    return;
    // recast areaData void* reference to osg::Node
    osg::ref_ptr<osg::Node> * areaNode =
            reinterpret_cast<osg::ref_ptr<osg::Node>*>(areaData.geomPtr);

    m_nodeAreas->removeChild(areaNode->get());
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

    osg::ref_ptr<osg::Material> lineColor =
            m_listWayMaterials[wayData.lineRenderStyle->GetId()].lineColor;

    osg::ref_ptr<osg::Material> outlineColor =
            m_listWayMaterials[wayData.lineRenderStyle->GetId()].outlineColor;

    double lineWidth = wayData.lineRenderStyle->GetLineWidth();

    // compensate layer to render flat areas before ways
    unsigned int effectiveLayer = wayData.wayLayer+m_maxAreaLayer+1;

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

    osg::ref_ptr<osg::Vec3dArray> listWayTriStripNorms=
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

        osg::Vec3d wayPtNorm = (listOffsetPointsA->at(k) +
                                listOffsetPointsB->at(k))*0.5;
        wayPtNorm.normalize();
        listWayTriStripNorms->at(i) = wayPtNorm;
        listWayTriStripNorms->at(i+1) = wayPtNorm;

        k++;
    }

    // save geometry
    osg::ref_ptr<osg::Geometry> geomWay = new osg::Geometry;
    geomWay->setVertexArray(listWayTriStripPts.get());
    geomWay->setNormalArray(listWayTriStripNorms.get());
    geomWay->setNormalBinding(osg::Geometry::BIND_OVERALL);
    geomWay->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
                                                 listWayTriStripPts->size()));
    // save style data
    osg::StateSet * wayStateSet = geomWay->getOrCreateStateSet();
    wayStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    wayStateSet->setRenderBinDetails(effectiveLayer,"RenderBin");
    wayStateSet->setAttribute(lineColor.get());

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

    // area material data
    AreaMaterial const & areaMat =
            m_listAreaMaterials[areaData.fillRenderStyle->GetId()];
    osg::ref_ptr<osg::Material> fillColor = areaMat.fillColor;
    osg::ref_ptr<osg::Material> outlineColor = areaMat.outlineColor;

    if(areaData.isBuilding)
    {
        int numBaseVerts = areaData.listBorderPoints.size();
        double const &bHeight = areaData.buildingData->height;

        // compensate layer to render height areas after ways
        // note: +10 seems to work better than +1?
        unsigned int effectiveLayer = m_maxAreaLayer+m_maxWayLayer+10;

        osg::ref_ptr<osg::Geometry> geomRoof = new osg::Geometry;
        osg::ref_ptr<osg::Geometry> geomSides = new osg::Geometry;

        // add vertices for base, than roof (using Vec3 because
        // osgUtil::Tessellator has an issue with Vec3d?)
        osg::ref_ptr<osg::Vec3Array> listBaseVertices = new osg::Vec3Array(numBaseVerts);
        osg::ref_ptr<osg::Vec3Array> listRoofVertices = new osg::Vec3Array(numBaseVerts);

        for(int i=0; i < numBaseVerts; i++)  {
            double px = areaData.listBorderPoints[i].x - offsetVec.x();
            double py = areaData.listBorderPoints[i].y - offsetVec.y();
            double pz = areaData.listBorderPoints[i].z - offsetVec.z();
            listBaseVertices->at(i) = osg::Vec3(px,py,pz);
            listRoofVertices->at(i) = osg::Vec3(px,py,pz)+(areaBaseNormal*bHeight);
        }

        // build roof
        osg::ref_ptr<osg::Vec3dArray> listRoofNormals = new osg::Vec3dArray;
        listRoofNormals->push_back(areaBaseNormal);

        geomRoof->setVertexArray(listRoofVertices.get());
        geomRoof->setNormalArray(listRoofNormals.get());
        geomRoof->setNormalBinding(osg::Geometry::BIND_OVERALL);
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

        // save geometry
        geomSides->setVertexArray(listSideVertices.get());
        geomSides->setNormalArray(listSideNormals.get());
        geomSides->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geomSides->addPrimitiveSet(listTriIndex.get());

        // save style data
        osg::StateSet * areaStateSet;
        areaStateSet = geomSides->getOrCreateStateSet();
        areaStateSet->setRenderBinDetails(effectiveLayer,"DepthSortedBin");
        areaStateSet->setAttribute(fillColor.get());
        areaStateSet = geomRoof->getOrCreateStateSet();
        areaStateSet->setRenderBinDetails(effectiveLayer,"DepthSortedBin");
        areaStateSet->setAttribute(fillColor.get());

        // add geometry to parent node
        osg::ref_ptr<osg::Geode> nodeArea = new osg::Geode;
        nodeArea->addDrawable(geomRoof.get());
        nodeArea->addDrawable(geomSides.get());

        // transparent areas that have a wall coinciding
        // with an adjacent area causes z-fighting artifacts
        // so we shrink the current area by ~% to compensate
        osg::ref_ptr<osg::MatrixTransform> nodeXform = new osg::MatrixTransform;
        nodeXform->setMatrix(osg::Matrix::scale(0.97,0.97,0.97));
        nodeXform->addChild(nodeArea.get());
        nodeParent->addChild(nodeXform.get());
        nodeParent->addChild(nodeXform.get());
    }
    else
    {
        osg::ref_ptr<osg::Geometry> geomArea = new osg::Geometry;

        // using Vec3 because of osgUtil::Tessellator
        // requires it (as opposed to Vec3d)
        osg::ref_ptr<osg::Vec3Array> listBorderPoints = new osg::Vec3Array;
        listBorderPoints->resize(areaData.listBorderPoints.size());

        for(int i=0; i < listBorderPoints->size(); i++)  {
            double px = areaData.listBorderPoints[i].x - offsetVec.x();
            double py = areaData.listBorderPoints[i].y - offsetVec.y();
            double pz = areaData.listBorderPoints[i].z - offsetVec.z();
            listBorderPoints->at(i) = osg::Vec3(px,py,pz);
        }

        // set normals
        osg::ref_ptr<osg::Vec3dArray> listAreaNorms = new osg::Vec3dArray(1);
        listAreaNorms->at(0) = areaBaseNormal;

        // save geometry
        geomArea->setVertexArray(listBorderPoints.get());
        geomArea->setNormalArray(listAreaNorms.get());
        geomArea->setNormalBinding(osg::Geometry::BIND_OVERALL);
        geomArea->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN,0,
                                                      listBorderPoints->size()));
        osgUtil::Tessellator geomTess;
        geomTess.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        geomTess.retessellatePolygons(*geomArea);

        // save style data
        // note: since this a flat (0 height) area, we explicitly
        //       control rendering order using layers
        osg::StateSet * areaStateSet = geomArea->getOrCreateStateSet();
        areaStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        areaStateSet->setRenderBinDetails(areaData.areaLayer,"RenderBin");
        areaStateSet->setAttribute(fillColor.get());

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

osg::Vec4 MapRendererOSG::colorAsVec4(const ColorRGBA &color)
{   return osg::Vec4(color.R,color.G,color.B,color.A);   }

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
