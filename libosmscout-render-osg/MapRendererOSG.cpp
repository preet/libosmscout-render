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
    m_nodeNodes = new osg::Group;
    m_nodeWays = new osg::Group;
    m_nodeAreas = new osg::Group;

    m_nodeRoot->addChild(m_nodeNodes.get());
    m_nodeRoot->addChild(m_nodeWays.get());
    m_nodeRoot->addChild(m_nodeAreas.get());

    m_nodeNodes->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    m_nodeWays->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    m_nodeAreas->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);

    m_wayBlendFunc = new osg::BlendFunc;
    m_wayBlendFunc->setFunction(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // build geometry used as node symbols
    buildGeomTriangle();
    buildGeomSquare();
    buildGeomCircle();
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

    // get max style id sizes to size material lists
    unsigned int maxLabelStyleId = 0;
    unsigned int maxLineStyleId = 0;
    unsigned int maxFillStyleId = 0;

    for(int i=0; i < listRenderStyles.size(); i++)
    {
        // NODE MATERIAL TYPES
        std::vector<TypeId> listNodeTypes;
        listRenderStyles[i]->GetNodeTypes(listNodeTypes);
        for(int j=0; j < listNodeTypes.size(); j++)
        {
            FillRenderStyle * fillStyle =
                listRenderStyles[i]->GetNodeFillRenderStyle(listNodeTypes[j]);

            LabelRenderStyle * nameStyle =
                listRenderStyles[i]->GetNodeNameLabelRenderStyle(listNodeTypes[j]);

            if(!(fillStyle == NULL))
            {   maxFillStyleId = std::max(maxFillStyleId,fillStyle->GetId());   }

            if(!(nameStyle == NULL))
            {   maxLabelStyleId = std::max(maxLabelStyleId,nameStyle->GetId());   }
        }

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
            {   maxLineStyleId = std::max(maxLineStyleId,lineStyle->GetId());   }

            if(!(nameStyle == NULL))
            {   maxLabelStyleId = std::max(maxLabelStyleId,nameStyle->GetId());   }
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
            {   maxFillStyleId = std::max(maxFillStyleId,fillStyle->GetId());   }

            if(!(nameStyle == NULL))
            {   maxLabelStyleId = std::max(maxLabelStyleId,nameStyle->GetId());   }
        }
    }

    m_listFillMaterials.clear();
    m_listFillMaterials.resize(maxFillStyleId+1);

    m_listLineMaterials.clear();
    m_listLineMaterials.resize(maxLineStyleId+1);

    m_listLabelMaterials.clear();
    m_listLabelMaterials.resize(maxLabelStyleId+1);

    // build material lists
    for(int i=0; i < listRenderStyles.size(); i++)
    {
        // NODE MATERIAL TYPES
        std::vector<TypeId> listNodeTypes;
        listRenderStyles[i]->GetNodeTypes(listNodeTypes);
        for(int j=0; j < listNodeTypes.size(); j++)
        {
            FillRenderStyle * fillStyle =
                listRenderStyles[i]->GetNodeFillRenderStyle(listNodeTypes[j]);

            LabelRenderStyle * nameStyle =
                listRenderStyles[i]->GetNodeNameLabelRenderStyle(listNodeTypes[j]);

            if(!(fillStyle == NULL))
            {
                FillMaterial fillMat;
                fillMat.matId = fillStyle->GetId();

                fillMat.fillColor = new osg::Material;
                fillMat.fillColor->setColorMode(osg::Material::OFF);
                fillMat.fillColor->setEmission(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetFillColor()));
                fillMat.fillColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetFillColor()));

                fillMat.outlineColor = new osg::Material;
                fillMat.outlineColor->setColorMode(osg::Material::OFF);
                fillMat.outlineColor->setEmission(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetOutlineColor()));
                fillMat.outlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetOutlineColor()));

                m_listFillMaterials[fillMat.matId] = fillMat;
            }

            if(!(nameStyle == NULL))
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
                m_listLabelMaterials[labMat.matId] = labMat;
            }
        }

        // WAY MATERIAL TYPES
        std::vector<TypeId> listWayTypes;
        listRenderStyles[i]->GetWayTypes(listWayTypes);
        for(int j=0; j < listWayTypes.size(); j++)
        {
            LineRenderStyle * lineStyle =
                listRenderStyles[i]->GetWayLineRenderStyle(listWayTypes[j]);

            LabelRenderStyle * nameStyle =
                listRenderStyles[i]->GetWayNameLabelRenderStyle(listWayTypes[j]);

            // note: for now, transparency on way LineRenderStyles
            //       (both line fills and outlines is unsupported)
            ColorRGBA lineColor = lineStyle->GetLineColor();
            lineColor.A = 1.0;

            ColorRGBA outlineColor = lineStyle->GetOutlineColor();
            outlineColor.A = 1.0;

            if(!(lineStyle == NULL))
            {
                LineMaterial lineMat;
                lineMat.matId = lineStyle->GetId();

                lineMat.lineColor = new osg::Material;
                lineMat.lineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(lineColor));

                lineMat.outlineColor = new osg::Material;
                lineMat.outlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(outlineColor));

                m_listLineMaterials[lineMat.matId] = lineMat;
            }

            if(!(nameStyle) == NULL)
            {
                LabelMaterial labMat;
                labMat.matId = nameStyle->GetId();

                labMat.fontColor = new osg::Material;
                labMat.fontColor->setColorMode(osg::Material::OFF);
                labMat.fontColor->setEmission(osg::Material::FRONT,
                    colorAsVec4(nameStyle->GetFontColor()));

                labMat.fontOutlineColor = new osg::Material;
                labMat.fontOutlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(nameStyle->GetFontOutlineColor()));

                m_listLabelMaterials[labMat.matId] = labMat;
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
                FillMaterial fillMat;
                fillMat.matId = fillStyle->GetId();

                fillMat.fillColor = new osg::Material;
                fillMat.fillColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetFillColor()));

                fillMat.outlineColor = new osg::Material;
                fillMat.outlineColor->setDiffuse(osg::Material::FRONT,
                    colorAsVec4(fillStyle->GetOutlineColor()));

                m_listFillMaterials[fillMat.matId] = fillMat;
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
                m_listLabelMaterials[labMat.matId] = labMat;
            }
        }
    }

    // build font cache
    std::vector<std::string> listFonts;
    this->getFontList(listFonts);

    m_fontGeoMap.clear();
    m_fontGeoMap.reserve(listFonts.size());

    OSRDEBUG << "INFO: Font Types Found: ";
    for(int i=0; i < listFonts.size(); i++)
    {
        OSRDEBUG << "INFO:   " << listFonts[i];

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

    unsigned int numAreaLayers=this->getMaxAreaLayer()+1;
    unsigned int numWayLayers=this->getMaxWayLayer()+1;

    m_minLayer=0;
    m_layerBaseAreaOLs = m_minLayer;
    m_layerBaseAreas = m_layerBaseAreaOLs+numAreaLayers;
    m_layerBaseWayOLs = m_layerBaseAreas+numAreaLayers;
    m_layerBaseWays = m_layerBaseWayOLs+numWayLayers;
    m_layerBaseWayLabels = m_layerBaseWays+numWayLayers;
    m_depthSortedBin = m_layerBaseWayLabels+10;
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addNodeToScene(NodeRenderData &nodeData)
{
    // use only coordinate as floating origin offset
    osg::Vec3d offsetVec(nodeData.nodePosn.x,
                         nodeData.nodePosn.y,
                         nodeData.nodePosn.z);

    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(offsetVec));

    // build node and add to transform parent
    this->addNodeGeometry(nodeData,offsetVec,nodeTransform.get());

    // build node label (if present)
    if(nodeData.hasName)
    {
        OSRDEBUG << nodeData.nameLabel;
        if(nodeData.nameLabelRenderStyle->GetLabelType() == LABEL_DEFAULT)
        {   this->addDefaultLabel(nodeData,offsetVec,nodeTransform,true);   }

//        else if(nodeData.nameLabelRenderStyle->GetLabelType() == LABEL_PLATE)
//        {   this->addPlateLabel(nodeData,offsetVec,nodeTransform,true);   }
    }

    // add the transform node to the scene graph
    m_nodeNodes->addChild(nodeTransform.get());

    // save a reference to (a reference of) this node
    osg::ref_ptr<osg::Node> * nodeRefPtr = new osg::ref_ptr<osg::Node>;
    (*nodeRefPtr) = nodeTransform;
    nodeData.geomPtr = nodeRefPtr;
}

void MapRendererOSG::removeNodeFromScene(const NodeRenderData &nodeData)
{}

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

    if(wayData.hasName)  {
        if(wayData.nameLabelRenderStyle->GetLabelType() == LABEL_CONTOUR)
        {   this->addContourLabel(wayData,offsetVec,nodeTransform,true);   }
    }

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
    //             << wayData.wayRef->GetId() << " from Scene Graph";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addAreaToScene(AreaRenderData &areaData)
{
    return;
    // use first center point for floating point offset
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
    m_nodeAreas->addChild(nodeTransform.get());

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

    m_nodeAreas->removeChild(areaNode->get());
    delete areaNode;

//        OSRDEBUG << "INFO: Removed Area "
//                 << areaData.areaRef->GetId() << " from Scene Graph";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addRelAreaToScene(RelAreaRenderData &relAreaData)
{
    return;
    // use average center point for floating origin offset
    Vec3 avCenter;
    unsigned int numAreas = relAreaData.listAreaData.size();
    for(int i=0; i < numAreas; i++)
    {   avCenter = avCenter+(relAreaData.listAreaData[i].centerPoint);   }
    avCenter.ScaledBy(1.0/numAreas);

    osg::Vec3d offsetVec = convVec3ToOsgVec3d(avCenter);
    osg::ref_ptr<osg::MatrixTransform> nodeTransform = new osg::MatrixTransform;
    nodeTransform->setMatrix(osg::Matrix::translate(offsetVec));

    // add area geometry
    for(int i=0; i < numAreas; i++)   {
        this->addAreaGeometry(relAreaData.listAreaData[i],
                              offsetVec,nodeTransform.get());
    }

    // add the transform node to the scene graph
    m_nodeAreas->addChild(nodeTransform.get());

    // save a reference to (a reference of) this node
    osg::ref_ptr<osg::Node> * nodeRefPtr = new osg::ref_ptr<osg::Node>;
    (*nodeRefPtr) = nodeTransform;
    relAreaData.geomPtr = nodeRefPtr;

    OSRDEBUG << "INFO: Added RelArea "
             << relAreaData.relRef->GetId() << " to Scene Graph";
}

void MapRendererOSG::removeRelAreaFromScene(const RelAreaRenderData &relAreaData)
{
    // recast relAreaData void* reference to osg::Node
    osg::ref_ptr<osg::Node> * areaNode =
            reinterpret_cast<osg::ref_ptr<osg::Node>*>(relAreaData.geomPtr);

    m_nodeAreas->removeChild(areaNode->get());
    delete areaNode;

    //        OSRDEBUG << "INFO: Removed RelArea "
    //                 << relAreaData.relRef->GetId() << " from Scene Graph";
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::removeAllFromScene()
{
//    unsigned int numWays = m_osg_osmWays->getNumChildren();

//    if(numWays > 0)
//    {   m_osg_osmWays->removeChild(0,numWays);   }

    m_listFillMaterials.clear();
    m_listLineMaterials.clear();
    m_listLabelMaterials.clear();

}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addNodeGeometry(const NodeRenderData &nodeData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent)
{
    // get materials
    osg::ref_ptr<osg::Material> fillColor =
            m_listFillMaterials[nodeData.fillRenderStyle->GetId()].fillColor;

    osg::ref_ptr<osg::Material> outlineColor =
            m_listFillMaterials[nodeData.fillRenderStyle->GetId()].outlineColor;

    // build geometry
    osg::ref_ptr<osg::Geometry> geomSymbol;
    osg::ref_ptr<osg::Geometry> geomOutline;

    switch(nodeData.symbolRenderStyle->GetSymbolType())
    {
        case SYMBOL_TRIANGLE:
        {
            geomSymbol = m_symbolTriangle;
            break;
        }
        case SYMBOL_SQUARE:
        {
            geomSymbol = m_symbolSquare;
            break;
        }
        case SYMBOL_CIRCLE:
        {
            geomSymbol = m_symbolCircle;
            break;
        }
        default:
            break;
    }

    // calculate the position vector taking offsetHeight into account
    osg::Vec3d surfaceVec(nodeData.nodePosn.x,
                          nodeData.nodePosn.y,
                          nodeData.nodePosn.z);

    double offsetHeight = nodeData.symbolRenderStyle->GetOffsetHeight();
    osg::Vec3d normVec = surfaceVec; normVec.normalize();
    osg::Vec3d shiftVec = surfaceVec+(normVec*offsetHeight)-offsetVec;

    // autotransform (billboard + scale)
    osg::ref_ptr<osg::AutoTransform> symbolXform = new osg::AutoTransform;
    symbolXform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
    symbolXform->setScale(nodeData.symbolRenderStyle->GetSymbolSize()/2);
    symbolXform->setPosition(shiftVec);

    // create outline if required
    double symbolSize = nodeData.symbolRenderStyle->GetSymbolSize();
    double oL = nodeData.fillRenderStyle->GetOutlineWidth();
    if(oL > 0)
    {
        geomOutline = dynamic_cast<osg::Geometry*>
                (geomSymbol->clone(osg::CopyOp::DEEP_COPY_ALL));

        // since we specified the original geometry as a triangle fan,
        // we remove the first vertex in the outline vertex array
        osg::ref_ptr<osg::Vec3dArray> listVerts =
                dynamic_cast<osg::Vec3dArray*>(geomOutline->getVertexArray());
        listVerts->erase(listVerts->begin());

        double outlineFrac = (oL+symbolSize)/symbolSize;
        unsigned int listVertsShSize = listVerts->size();
        for(int i=0; i < listVertsShSize; i++)   {
            osg::Vec3d const &innerVert = listVerts->at(i);
            listVerts->push_back(osg::Vec3d(innerVert.x()*outlineFrac,
                                            innerVert.y()*outlineFrac,
                                            innerVert.z()*outlineFrac));
        }

        // stitch outline faces together
        osg::ref_ptr<osg::DrawElementsUInt> listIdxs = new
                osg::DrawElementsUInt(GL_TRIANGLES,listVertsShSize*6);
        int k=0;

        // the original symbol vertex array wraps around
        // (first vertex == last vertex) so we don't have
        // to 'close off' the triangles in the outline
        for(int i=0; i < listVertsShSize-1; i++)   {
            int inIdx = i; int outIdx = i+listVertsShSize;
            listIdxs->at(k) = inIdx;        k++;
            listIdxs->at(k) = outIdx;       k++;
            listIdxs->at(k) = inIdx+1;      k++;
            listIdxs->at(k) = inIdx+1;      k++;
            listIdxs->at(k) = outIdx;       k++;
            listIdxs->at(k) = outIdx+1;     k++;
        }

        geomOutline->removePrimitiveSet(0,1);
        geomOutline->addPrimitiveSet(listIdxs.get());

        osg::ref_ptr<osg::Geode> symbolOutlineNode = new osg::Geode;
        symbolOutlineNode->addDrawable(geomOutline.get());
        symbolOutlineNode->getOrCreateStateSet()->setAttribute(outlineColor.get());
        symbolXform->addChild(symbolOutlineNode.get());
    }

    // create geode and set material/properties
    osg::ref_ptr<osg::Geode> symbolNode = new osg::Geode;
    symbolNode->addDrawable(geomSymbol.get());
    symbolNode->getOrCreateStateSet()->setAttribute(fillColor.get());
    symbolXform->addChild(symbolNode.get());
    symbolXform->getOrCreateStateSet()->setRenderBinDetails(m_depthSortedBin,
                                                           "DepthSortedBin");
    nodeParent->addChild(symbolXform.get());
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayGeometry(const WayRenderData &wayData,
                                    const osg::Vec3d &offsetVec,
                                    osg::MatrixTransform *nodeParent)
{
    // get material data
    unsigned int lineRenderId = wayData.lineRenderStyle->GetId();

    osg::ref_ptr<osg::Material> lineColor =
            m_listLineMaterials[lineRenderId].lineColor;

    // build vertex data
    std::vector<Vec3> wayVertexArray;
    this->buildPolylineAsTriStrip(wayData.listWayPoints,
                                  wayData.lineRenderStyle->GetLineWidth(),
                                  OL_CENTER,wayVertexArray);

    osg::ref_ptr<osg::Vec3dArray> listWayTriStripPts=
            new osg::Vec3dArray(wayVertexArray.size());

    osg::ref_ptr<osg::Vec3dArray> listWayTriStripNorms=
            new osg::Vec3dArray(wayVertexArray.size());

    for(int i=0; i < wayVertexArray.size(); i++)   {
        listWayTriStripPts->at(i) =
                convVec3ToOsgVec3d(wayVertexArray[i]) - offsetVec;
        listWayTriStripNorms->at(i) =
                convVec3ToOsgVec3d(wayVertexArray[i].Normalized());
    }

    osg::ref_ptr<osg::Geode> nodeWay = new osg::Geode;

    // if a way outline is specified, save it
    if(wayData.lineRenderStyle->GetOutlineWidth() > 0)
    {
        osg::ref_ptr<osg::Material> outlineColor =
                m_listLineMaterials[lineRenderId].outlineColor;

        double extOutlineWidth = wayData.lineRenderStyle->GetLineWidth()+
                wayData.lineRenderStyle->GetOutlineWidth();

        std::vector<Vec3> wayOLVertexArray;
        this->buildPolylineAsTriStrip(wayData.listWayPoints,
                                      extOutlineWidth,OL_CENTER,
                                      wayOLVertexArray);

        osg::ref_ptr<osg::Vec3dArray> listWayOLTriStripPts=
                new osg::Vec3dArray(wayOLVertexArray.size());

        osg::ref_ptr<osg::Vec3dArray> listWayOLTriStripNorms=
                new osg::Vec3dArray(wayOLVertexArray.size());

        for(int i=0; i < wayOLVertexArray.size(); i++)   {
            listWayOLTriStripPts->at(i) =
                    convVec3ToOsgVec3d(wayOLVertexArray[i]) - offsetVec;
            listWayOLTriStripNorms->at(i) =
                    convVec3ToOsgVec3d(wayOLVertexArray[i].Normalized());
        }

        osg::ref_ptr<osg::Geometry> geomLeftOL = new osg::Geometry;
        geomLeftOL->setVertexArray(listWayOLTriStripPts.get());
        geomLeftOL->setNormalArray(listWayOLTriStripNorms.get());
        geomLeftOL->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geomLeftOL->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
            listWayOLTriStripPts->size()));

        osg::StateSet * wayOLStateSet = geomLeftOL->getOrCreateStateSet();
        wayOLStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        wayOLStateSet->setRenderBinDetails(m_layerBaseWayOLs+wayData.wayLayer,"RenderBin");
        wayOLStateSet->setAttribute(outlineColor.get());
        nodeWay->addDrawable(geomLeftOL.get());
    }

    // save geometry
    osg::ref_ptr<osg::Geometry> geomWay = new osg::Geometry;
    geomWay->setVertexArray(listWayTriStripPts.get());
    geomWay->setNormalArray(listWayTriStripNorms.get());
    geomWay->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geomWay->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
                                                 listWayTriStripPts->size()));
    // save style data
    osg::StateSet * wayStateSet = geomWay->getOrCreateStateSet();
    wayStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    wayStateSet->setRenderBinDetails(m_layerBaseWays+wayData.wayLayer,"RenderBin");
    wayStateSet->setAttribute(lineColor.get());

    nodeWay->addDrawable(geomWay.get());
    nodeParent->addChild(nodeWay.get());
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addAreaGeometry(const AreaRenderData &areaData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent)
{
    // calculate area base normal (earth surface)
    osg::Vec3d areaBaseNormal = offsetVec;
    areaBaseNormal.normalize();

    // get material data
    unsigned int fillId = areaData.fillRenderStyle->GetId();

    osg::ref_ptr<osg::Material> fillColor =
            m_listFillMaterials[fillId].fillColor;

    osg::ref_ptr<osg::Material> outlineColor =
            m_listFillMaterials[fillId].outlineColor;

    // build common vertex data
    unsigned int k=0;
    unsigned int numVerts = areaData.listOuterPoints.size();
    for(int i=0; i < areaData.listListInnerPoints.size(); i++)
    {   numVerts += areaData.listListInnerPoints[i].size();   }

    osg::ref_ptr<osg::Vec3Array> areaBaseVerts = new osg::Vec3Array(numVerts);
    osg::ref_ptr<osg::Vec3Array> areaBaseNorms = new osg::Vec3Array(numVerts);
    std::vector<unsigned int> lsPolys;      // stores the vertex idxs for each simple poly
                                            // in the reln -- lsPolys[0] is the outer poly
                                            // and all subsequent entires are inner polys

    // add outer poly points to base verts
    for(int i=0; i < areaData.listOuterPoints.size(); i++)   {
        areaBaseVerts->at(k) =
                convVec3ToOsgVec3(areaData.listOuterPoints[i]); k++;
    } lsPolys.push_back(areaData.listOuterPoints.size());

    // add inner poly points to base verts
    for(int i=0; i < areaData.listListInnerPoints.size(); i++)   {
        for(int j=0; j < areaData.listListInnerPoints[i].size(); j++)   {
        areaBaseVerts->at(k) =
                convVec3ToOsgVec3(areaData.listListInnerPoints[i][j]); k++;
        } lsPolys.push_back(lsPolys.back()+areaData.listListInnerPoints[i].size());
    }

    // normals
    for(int i=0; i < areaBaseVerts->size(); i++)   {
        osg::Vec3 areaBaseNorm = areaBaseVerts->at(i);
        areaBaseNorm.normalize();
        areaBaseNorms->at(i) = areaBaseNorm;
    }

    // offset fix
    for(int i=0; i < areaBaseVerts->size(); i++)
    {   areaBaseVerts->at(i) -= offsetVec;   }

    // build common geometry (building base)
    osg::ref_ptr<osg::Geometry> geomAreaBase = new osg::Geometry;
    geomAreaBase->setVertexArray(areaBaseVerts.get());
    geomAreaBase->setNormalArray(areaBaseNorms.get());
    geomAreaBase->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    if(areaData.isBuilding)
    {   // build building-specific geometry
        double const &bHeight = areaData.buildingData->height;

        // build roof geometry
        osg::ref_ptr<osg::Vec3Array> areaRoofNorms = areaBaseNorms;
        osg::ref_ptr<osg::Vec3Array> areaRoofVerts = new osg::Vec3Array(numVerts);
        for(int i=0; i < numVerts; i++)   {
            areaRoofVerts->at(i) = areaBaseVerts->at(i)+
                    areaBaseNorms->at(i)*bHeight;
        }
        osg::ref_ptr<osg::Geometry> geomAreaRoof = new osg::Geometry;
        geomAreaRoof->setVertexArray(areaRoofVerts.get());
        geomAreaRoof->setNormalArray(areaRoofNorms.get());
        geomAreaRoof->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

        osgUtil::Tessellator areaRoofTess;
        areaRoofTess.setTessellationNormal(offsetVec);
        areaRoofTess.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        for(int i=0; i < lsPolys.size(); i++)   {
            unsigned int vStart,vNum;
            vStart = (i==0) ? 0 : lsPolys[i-1];
            vNum = (lsPolys[i]-1-vStart)+1;
            geomAreaRoof->addPrimitiveSet
                    (new osg::DrawArrays(GL_TRIANGLE_FAN,vStart,vNum));
        }
        areaRoofTess.retessellatePolygons(*geomAreaRoof);

        // build side walls (2 tris * 3 pts) / edge
        osg::ref_ptr<osg::Vec3Array> areaSideVerts = new osg::Vec3Array(numVerts*6);
        osg::ref_ptr<osg::Vec3Array> areaSideNorms = new osg::Vec3Array(numVerts*6);

        osg::ref_ptr<osg::Geometry> geomAreaSides = new osg::Geometry;
        geomAreaSides->setVertexArray(areaSideVerts.get());
        geomAreaSides->setNormalArray(areaSideNorms.get());
        geomAreaSides->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

        for(int i=0; i < lsPolys.size(); i++)
        {   // each polygon within the area should
            // get its own primitive set of side walls
            unsigned int vStart,vEnd,vNum;
            vStart = (i==0) ? 0 : lsPolys[i-1];
            vEnd = lsPolys[i]-1;
            vNum = (vEnd-vStart)+1;

            // create an index for areaBase/Roof vertices
            // that represents each polygon
            std::vector<unsigned int> lsPolyIdxs((vEnd-vStart)+2);
            for(int j=0; j < lsPolyIdxs.size()-1; j++)
            {   lsPolyIdxs[j] = j+vStart;   }

            // wrap around for easy triangle stitching
            lsPolyIdxs[lsPolyIdxs.size()-1] = lsPolyIdxs[0];

            // stitch triangles together between
            // corresponding roof/base vertices
            for(int j=0; j < lsPolyIdxs.size()-1; j++)
            {
                unsigned int n=lsPolyIdxs[j]*6;
                osg::Vec3 alongSide = areaBaseVerts->at(lsPolyIdxs[j+1])-
                        areaBaseVerts->at(lsPolyIdxs[j]);
                osg::Vec3 alongHeight = areaRoofVerts->at(lsPolyIdxs[j])-
                        areaBaseVerts->at(lsPolyIdxs[j]);
                osg::Vec3 sideNormal = (alongSide^alongHeight);
                sideNormal.normalize();

                // triangle 1 vertices,norms
                areaSideVerts->at(n+0) = areaBaseVerts->at(lsPolyIdxs[j]);
                areaSideVerts->at(n+1) = areaBaseVerts->at(lsPolyIdxs[j+1]);
                areaSideVerts->at(n+2) = areaRoofVerts->at(lsPolyIdxs[j]);
                areaSideNorms->at(n+0) = sideNormal;
                areaSideNorms->at(n+1) = sideNormal;
                areaSideNorms->at(n+2) = sideNormal;

                // triangle 2 vertices,norms
                areaSideVerts->at(n+3) = areaRoofVerts->at(lsPolyIdxs[j]);
                areaSideVerts->at(n+4) = areaBaseVerts->at(lsPolyIdxs[j+1]);
                areaSideVerts->at(n+5) = areaRoofVerts->at(lsPolyIdxs[j+1]);
                areaSideNorms->at(n+3) = sideNormal;
                areaSideNorms->at(n+4) = sideNormal;
                areaSideNorms->at(n+5) = sideNormal;
            }
            geomAreaSides->addPrimitiveSet
                    (new osg::DrawArrays(GL_TRIANGLES,vStart*6,
                                         (lsPolyIdxs.size()-1)*6));
        }


        osg::ref_ptr<osg::Geode> geodeArea = new osg::Geode;
        geodeArea->addDrawable(geomAreaRoof.get());
        geodeArea->addDrawable(geomAreaSides.get());

        // set material
        osg::StateSet * stateSet = geodeArea->getOrCreateStateSet();
        stateSet->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin");
        stateSet->setAttribute(fillColor.get());

        // add to scene

        // areas that have coinciding walls with adjacent
        // areas causes z-fighting artifacts (this is esp
        // noticable when transparency is used) -- so we
        // shrink areas by ~small % to compensate
        osg::ref_ptr<osg::MatrixTransform> nodeXform =
                new osg::MatrixTransform(osg::Matrix::scale(0.95,0.95,0.95));
        nodeXform->addChild(geodeArea.get());
        nodeParent->addChild(nodeXform.get());
    }
    else
    {   // use the base as the geometry for a flat area
        osg::ref_ptr<osg::Geode> geodeArea = new osg::Geode;

        osgUtil::Tessellator areaBaseTess;
        areaBaseTess.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        areaBaseTess.setTessellationNormal(offsetVec);

        for(int i=0; i < lsPolys.size(); i++)   {
            unsigned int vStart,vEnd,vNum;
            vStart = (i==0) ? 0 : lsPolys[i-1];
            vEnd = lsPolys[i]-1;
            vNum = (lsPolys[i]-1-vStart)+1;
            geomAreaBase->addPrimitiveSet
                    (new osg::DrawArrays(GL_TRIANGLE_FAN,vStart,vNum));
        }
        areaBaseTess.retessellatePolygons(*geomAreaBase);

        // build area outline if required
        osg::ref_ptr<osg::Geode> geodeOutlines = new osg::Geode;
        double outlineWidth = areaData.fillRenderStyle->GetOutlineWidth();
        if(outlineWidth > 0)   {
            // outer polygon outline
            std::vector<Vec3> outlineArray;

            // we need to wrap the polyline onto
            // itself to 'close' the area polygon
            std::vector<Vec3> listOutlinePts = areaData.listOuterPoints;
            listOutlinePts.push_back(listOutlinePts[0]);
            this->buildPolylineAsTriStrip(listOutlinePts,outlineWidth,
                                          OL_RIGHT,outlineArray);
            // complete wrap around
            outlineArray.push_back(outlineArray[1]);

            unsigned int numOLVerts = outlineArray.size();
            osg::ref_ptr<osg::Vec3Array> outerOutlineVerts = new osg::Vec3Array(numOLVerts);
            osg::ref_ptr<osg::Vec3Array> outerOutlineNorms = new osg::Vec3Array(numOLVerts);
            for(int i=0;i < outlineArray.size(); i++)   {
                outerOutlineVerts->at(i) = convVec3ToOsgVec3(outlineArray[i])-offsetVec;
                outerOutlineNorms->at(i) = convVec3ToOsgVec3(outlineArray[i].Normalized());
            }
            osg::ref_ptr<osg::Geometry> geomOuterOutline = new osg::Geometry;
            geomOuterOutline->setVertexArray(outerOutlineVerts.get());
            geomOuterOutline->setNormalArray(outerOutlineNorms.get());
            geomOuterOutline->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            geomOuterOutline->addPrimitiveSet(new osg::DrawArrays
                                              (GL_TRIANGLE_STRIP,0,numOLVerts));
            geodeOutlines->addDrawable(geomOuterOutline.get());

            for(int i=0; i < areaData.listListInnerPoints.size(); i++)   {
                outlineArray.clear();
                listOutlinePts = areaData.listListInnerPoints[i];
                listOutlinePts.push_back(listOutlinePts[0]);
                this->buildPolylineAsTriStrip(listOutlinePts,outlineWidth,
                                              OL_RIGHT,outlineArray);
                outlineArray.push_back(outlineArray[1]);

                numOLVerts = outlineArray.size();
                osg::ref_ptr<osg::Vec3Array> innerOutlineVerts = new osg::Vec3Array(numOLVerts);
                osg::ref_ptr<osg::Vec3Array> innerOutlineNorms = new osg::Vec3Array(numOLVerts);
                for(int j=0; j < outlineArray.size(); j++)    {
                    innerOutlineVerts->at(j) = convVec3ToOsgVec3(outlineArray[j])-offsetVec;
                    innerOutlineNorms->at(j) = convVec3ToOsgVec3(outlineArray[j].Normalized());
                }
                osg::ref_ptr<osg::Geometry> geomInnerOutline = new osg::Geometry;
                geomInnerOutline->setVertexArray(innerOutlineVerts.get());
                geomInnerOutline->setNormalArray(innerOutlineNorms.get());
                geomInnerOutline->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                geomInnerOutline->addPrimitiveSet(new osg::DrawArrays
                                                  (GL_TRIANGLE_STRIP,0,numOLVerts));
                geodeOutlines->addDrawable(geomInnerOutline.get());
            }
            // set outline material
            osg::StateSet * stateSet = geodeOutlines->getOrCreateStateSet();
            stateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
            stateSet->setRenderBinDetails(m_layerBaseAreaOLs+areaData.areaLayer,"RenderBin");
            stateSet->setAttribute(outlineColor.get());

            // add to scene
            nodeParent->addChild(geodeOutlines.get());
        }

        // set material
        osg::StateSet * stateSet = geomAreaBase->getOrCreateStateSet();
        stateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        stateSet->setRenderBinDetails(m_layerBaseAreas+areaData.areaLayer,"RenderBin");
        stateSet->setAttribute(fillColor.get());

        // add to scene
        geodeArea->addDrawable(geomAreaBase.get());
        nodeParent->addChild(geodeArea.get());
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addDefaultLabel(const NodeRenderData &nodeData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{
    std::string labelText;
    LabelRenderStyle const *labelStyle;

    if(usingName)   {
        labelText = nodeData.nameLabel;
        labelStyle = nodeData.nameLabelRenderStyle;
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    // font material data
    LabelMaterial const & fontMat = m_listLabelMaterials[labelStyle->GetId()];
    osg::ref_ptr<osg::Material> fontColor = fontMat.fontColor;
    osg::ref_ptr<osg::Material> outlineColor = fontMat.fontOutlineColor;

    // text geometry
    osg::ref_ptr<osgText::Text> geomText = new osgText::Text;
    geomText->setFont(labelStyle->GetFontFamily());
    geomText->setAlignment(osgText::Text::CENTER_CENTER);
    geomText->setCharacterSize(labelStyle->GetFontSize());
//    geomText->setPosition(btmCenterVec+heightVec-offsetVec);
    geomText->setText(labelText);

    osg::ref_ptr<osg::Geode> geodeText = new osg::Geode;
    geodeText->addDrawable(geomText.get());

    // calculate the position vector of the
    // node center taking offsetHeight into account
    osg::Vec3d surfaceVec(nodeData.nodePosn.x,
                          nodeData.nodePosn.y,
                          nodeData.nodePosn.z);

    double symOffsetHeight = nodeData.symbolRenderStyle->GetOffsetHeight();
    osg::Vec3d normVec = surfaceVec; normVec.normalize();
    osg::Vec3d shiftVec = surfaceVec+(normVec*symOffsetHeight)-offsetVec;

    // autotransform (billboard + scale)
    osg::ref_ptr<osg::AutoTransform> textXform = new osg::AutoTransform;
    textXform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
    textXform->setPosition(shiftVec);
    textXform->addChild(geodeText.get());

    osg::StateSet * labelStateSet = textXform->getOrCreateStateSet();
    labelStateSet->setAttribute(fontColor.get());
    labelStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
    labelStateSet->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin",
                                       osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

    nodeParent->addChild(textXform.get());
}

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

    double heightBuff = labelStyle->GetOffsetDist();
    osg::Vec3d btmCenterVec(areaData.centerPoint.x,
                            areaData.centerPoint.y,
                            areaData.centerPoint.z);

    osg::Vec3d heightVec = btmCenterVec;
    heightVec.normalize();

    // font material data
    LabelMaterial const & fontMat =
            m_listLabelMaterials[labelStyle->GetId()];
    osg::ref_ptr<osg::Material> fontColor = fontMat.fontColor;
    osg::ref_ptr<osg::Material> outlineColor = fontMat.fontOutlineColor;

    // adjust heightBuff if area is building
    if(areaData.isBuilding)
    {   heightVec *= (areaData.buildingData->height + heightBuff);   }
    else
    {   heightVec *= heightBuff;   }

    // use the max bounding box length of the base
    // as a rough metric for setting max label width
    double xMin = areaData.listOuterPoints[0].x; double xMax = xMin;
    double yMin = areaData.listOuterPoints[0].y; double yMax = yMin;
    double zMin = areaData.listOuterPoints[0].z; double zMax = zMin;

    for(int i=1; i < areaData.listOuterPoints.size(); i++)
    {
        Vec3 const &areaPoint = areaData.listOuterPoints[i];

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
    osg::StateSet * labelStateSet = labelNode->getOrCreateStateSet();
    labelStateSet->setAttribute(fontColor.get());
    labelStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON);
    labelStateSet->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin",
                                       osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

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

    // material data
    LabelMaterial const & plateMat = m_listLabelMaterials[labelStyle->GetId()];
    osg::ref_ptr<osg::Material> fontColor = plateMat.fontColor;
    osg::ref_ptr<osg::Material> outlineColor = plateMat.fontOutlineColor;
    osg::ref_ptr<osg::Material> plateColor = plateMat.plateColor;
    osg::ref_ptr<osg::Material> plateOutlineColor = plateMat.plateOutlineColor;

    // use the max bounding box length of the base area
    // as a rough metric for setting max label width
    double xMin = areaData.listOuterPoints[0].x; double xMax = xMin;
    double yMin = areaData.listOuterPoints[0].y; double yMax = yMin;
    double zMin = areaData.listOuterPoints[0].z; double zMax = zMin;

    for(int i=1; i < areaData.listOuterPoints.size(); i++)
    {
        Vec3 const &areaPoint = areaData.listOuterPoints[i];
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

    osg::StateSet *labelStateSet = labelText->getOrCreateStateSet();
    labelStateSet->setAttribute(fontColor.get());

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
    // positioning (bug) but they have the right relative
    // distance, so only yHeight is a valid metric in y
    xMin = labelText->computeBound().xMin();// - platePadding;
    xMax = labelText->computeBound().xMax();// + platePadding;
    yMin = labelText->computeBound().yMin();// - platePadding;
    yMax = labelText->computeBound().yMax();// + platePadding;
    double yHeight = yMax-yMin;

    // calculate the offsetHeight
    double offsetHeight = labelStyle->GetOffsetDist()+(yHeight/2.0);

    if(areaData.isBuilding)
    {   offsetHeight += areaData.buildingData->height;   }

    osg::Vec3d shiftVec = btmCenterVec+(surfNorm*offsetHeight)-offsetVec;

    // use the label bounding box+padding to create the plate
    osg::ref_ptr<osg::Geometry> labelPlate = new osg::Geometry;
    osg::ref_ptr<osg::Vec3dArray> pVerts = new osg::Vec3dArray(8);
    osg::ref_ptr<osg::Vec3dArray> pNorms = new osg::Vec3dArray(1);
    osg::ref_ptr<osg::DrawElementsUInt> pIdxs =
            new osg::DrawElementsUInt(GL_TRIANGLES,6);

    // build up plate vertices
    yHeight += (2*platePadding);
    xMin -= platePadding; xMax += platePadding;
    pVerts->at(0) = osg::Vec3d(xMin,-1*(yHeight/2),-0.2);   // bl
    pVerts->at(1) = osg::Vec3d(xMax,-1*(yHeight/2),-0.2);   // br
    pVerts->at(2) = osg::Vec3d(xMax,(yHeight/2),-0.2);   // tr
    pVerts->at(3) = osg::Vec3d(xMin,(yHeight/2),-0.2);   // tl

    yHeight += (2*plateOutlineWidth);
    xMin -= plateOutlineWidth; xMax += plateOutlineWidth;
    pVerts->at(4) = osg::Vec3d(xMin,-1*(yHeight/2),-0.2);   // bl
    pVerts->at(5) = osg::Vec3d(xMax,-1*(yHeight/2),-0.2);   // br
    pVerts->at(6) = osg::Vec3d(xMax,(yHeight/2),-0.2);   // tr
    pVerts->at(7) = osg::Vec3d(xMin,(yHeight/2),-0.2);   // tl
    labelPlate->setVertexArray(pVerts.get());

    pNorms->at(0) = osg::Vec3d(0,0,1);
    labelPlate->setNormalArray(pNorms.get());
    labelPlate->setNormalBinding(osg::Geometry::BIND_OVERALL);

    // build up plate tris
    pIdxs->at(0) = 0;   pIdxs->at(1) = 1;   pIdxs->at(2) = 2;
    pIdxs->at(3) = 0;   pIdxs->at(4) = 2;   pIdxs->at(5) = 3;
    labelPlate->addPrimitiveSet(pIdxs.get());

    // set plate material
    labelStateSet = labelPlate->getOrCreateStateSet();
    labelStateSet->setAttribute(plateColor.get());

    osg::ref_ptr<osg::Billboard> labelNode = new osg::Billboard;
    labelNode->setMode(osg::Billboard::POINT_ROT_EYE);
    labelNode->setNormal(osg::Vec3d(0,0,1));
    labelNode->addDrawable(labelPlate.get(),shiftVec);
    labelNode->addDrawable(labelText.get(),shiftVec);

    // build plate outline
    if(plateOutlineWidth > 0)
    {
        osg::ref_ptr<osg::Geometry> labelPlateOutline = new osg::Geometry;
        osg::ref_ptr<osg::DrawElementsUInt> pIdxsOL =
                new osg::DrawElementsUInt(GL_TRIANGLES,6*4);

        // btm
        pIdxsOL->at(0) = 0;    pIdxsOL->at(1) = 4;    pIdxsOL->at(2) = 1;
        pIdxsOL->at(3) = 1;    pIdxsOL->at(4) = 4;    pIdxsOL->at(5) = 5;
        // top
        pIdxsOL->at(6) = 7;    pIdxsOL->at(7) = 3;    pIdxsOL->at(8) = 6;
        pIdxsOL->at(9) = 6;    pIdxsOL->at(10) = 3;   pIdxsOL->at(11) = 2;
        // left
        pIdxsOL->at(12) = 7;   pIdxsOL->at(13) = 4;   pIdxsOL->at(14) = 3;
        pIdxsOL->at(15) = 3;   pIdxsOL->at(16) = 4;   pIdxsOL->at(17) = 0;
        // right
        pIdxsOL->at(18) = 6;   pIdxsOL->at(19) = 1;   pIdxsOL->at(20) = 5;
        pIdxsOL->at(21) = 2;   pIdxsOL->at(22) = 1;   pIdxsOL->at(23) = 6;

        labelPlateOutline->setVertexArray(pVerts.get());
        labelPlateOutline->setNormalArray(pNorms.get());
        labelPlateOutline->setNormalBinding(osg::Geometry::BIND_OVERALL);
        labelPlateOutline->addPrimitiveSet(pIdxsOL.get());

        labelStateSet = labelPlateOutline->getOrCreateStateSet();
        labelStateSet->setAttribute(plateOutlineColor.get());

        labelNode->addDrawable(labelPlateOutline.get(),shiftVec);
    }

    labelStateSet = labelNode->getOrCreateStateSet();
    labelStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::ON); // TODO do I need this?
    labelStateSet->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin",
                                       osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

    nodeParent->addChild(labelNode.get());
}

void MapRendererOSG::addContourLabel(const WayRenderData &wayData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{
    // set predefined vars up based on name or ref
    std::string const *labelText;
    LabelRenderStyle const *labelStyle;
    double fontSize;
    double labelPadding;

    if(usingName)
    {
        labelText = &(wayData.nameLabel);
        labelStyle = wayData.nameLabelRenderStyle;
        fontSize = labelStyle->GetFontSize();
        labelPadding = labelStyle->GetContourPadding();
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    // font material data
    LabelMaterial const & fontMat =
            m_listLabelMaterials[labelStyle->GetId()];
    osg::ref_ptr<osg::Material> fontColor = fontMat.fontColor;
    osg::ref_ptr<osg::Material> outlineColor = fontMat.fontOutlineColor;

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
            textChar->setCharacterSize(1.0);
            textChar->setText(charStr);

            std::pair<std::string,osg::ref_ptr<osgText::Text> > addChar;
            addChar.first = charStr;
            addChar.second = textChar;

            fCharIt = fontCharsMap.insert(addChar).first;

            OSRDEBUG << "INFO: Added char " << charStr
                     << " for font " << labelStyle->GetFontFamily();
        }

        // instance method
        osg::ref_ptr<osgText::Text> textChar = fCharIt->second;

        // save ref and bounds
        listChars[i] = textChar;
        listCharBounds[i] = textChar->getBound();
        listCharBounds[i].xMin() *= fontSize;
        listCharBounds[i].xMax() *= fontSize;
        listCharBounds[i].yMin() *= fontSize;
        listCharBounds[i].yMax() *= fontSize;
        nameLength += (textChar->getBound().xMax() -
                       textChar->getBound().xMin()) * 1.15 * fontSize;

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
    osg::ref_ptr<osg::Group> wayLabel = new osg::Group;
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

                    xformMatrix(0,0) = dirnAtLength.x()*fontSize;
                    xformMatrix(0,1) = dirnAtLength.y()*fontSize;
                    xformMatrix(0,2) = dirnAtLength.z()*fontSize;
                    xformMatrix(0,3) = 0;

                    xformMatrix(1,0) = sideAtLength.x()*-1*fontSize;
                    xformMatrix(1,1) = sideAtLength.y()*-1*fontSize;
                    xformMatrix(1,2) = sideAtLength.z()*-1*fontSize;
                    xformMatrix(1,3) = 0;

                    xformMatrix(2,0) = normAtLength.x()*fontSize;
                    xformMatrix(2,1) = normAtLength.y()*fontSize;
                    xformMatrix(2,2) = normAtLength.z()*fontSize;
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
                    wayLabel->addChild(xformNode.get());
                }
            }
        }
    }

    osg::StateSet * labelStateSet = wayLabel->getOrCreateStateSet();
    labelStateSet->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    labelStateSet->setRenderBinDetails(m_layerBaseWayLabels,"RenderBin");
    labelStateSet->setAttribute(fontColor.get());
    nodeParent->addChild(wayLabel.get());
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::buildGeomTriangle()
{
    m_symbolTriangle = new osg::Geometry;
    osg::ref_ptr<osg::Vec3dArray> listVerts = new osg::Vec3dArray;
    osg::ref_ptr<osg::Vec3dArray> listNorms = new osg::Vec3dArray;
    osg::ref_ptr<osg::DrawElementsUInt> listIdxs =
            new osg::DrawElementsUInt(GL_TRIANGLE_FAN);

    unsigned int numEdges = 3;
    listVerts->push_back(osg::Vec3(0,0,0));
    listNorms->push_back(osg::Vec3(0,0,1));

    // build shape
    for(int i=0; i <= numEdges; i++)
    {
        double cAngle = i*(2*K_PI)/numEdges + K_PI/2;
        listVerts->push_back(osg::Vec3(cos(cAngle),sin(cAngle),0));
        listIdxs->push_back(i);
    }
    listIdxs->push_back(1);

    m_symbolTriangle->setVertexArray(listVerts.get());
    m_symbolTriangle->setNormalArray(listNorms.get());
    m_symbolTriangle->setNormalBinding(osg::Geometry::BIND_OVERALL);
    m_symbolTriangle->addPrimitiveSet(listIdxs.get());
}

void MapRendererOSG::buildGeomSquare()
{
    m_symbolSquare = new osg::Geometry;
    osg::ref_ptr<osg::Vec3dArray> listVerts = new osg::Vec3dArray;
    osg::ref_ptr<osg::Vec3dArray> listNorms = new osg::Vec3dArray;
    osg::ref_ptr<osg::DrawElementsUInt> listIdxs =
            new osg::DrawElementsUInt(GL_TRIANGLE_FAN);

    unsigned int numEdges = 4;
    listVerts->push_back(osg::Vec3(0,0,0));
    listNorms->push_back(osg::Vec3(0,0,1));

    for(int i=0; i <= numEdges; i++)
    {
        double cAngle = i*(2*K_PI)/numEdges + K_PI/4;
        listVerts->push_back(osg::Vec3(cos(cAngle),sin(cAngle),0));
        listIdxs->push_back(i);
    }
    listIdxs->push_back(1);

    m_symbolSquare->setVertexArray(listVerts.get());
    m_symbolSquare->setNormalArray(listNorms.get());
    m_symbolSquare->setNormalBinding(osg::Geometry::BIND_OVERALL);
    m_symbolSquare->addPrimitiveSet(listIdxs.get());
}

void MapRendererOSG::buildGeomCircle()
{
    m_symbolCircle = new osg::Geometry;
    osg::ref_ptr<osg::Vec3dArray> listVerts = new osg::Vec3dArray;
    osg::ref_ptr<osg::Vec3dArray> listNorms = new osg::Vec3dArray;
    osg::ref_ptr<osg::DrawElementsUInt> listIdxs =
            new osg::DrawElementsUInt(GL_TRIANGLE_FAN);

    unsigned int numEdges = 12;
    listVerts->push_back(osg::Vec3(0,0,0));
    listNorms->push_back(osg::Vec3(0,0,1));

    for(int i=0; i <= numEdges; i++)
    {
        double cAngle = i*(2*K_PI)/numEdges;
        listVerts->push_back(osg::Vec3(cos(cAngle)*0.707,sin(cAngle)*0.707,0));
        listIdxs->push_back(i);
    }
    listIdxs->push_back(1);

    m_symbolCircle->setVertexArray(listVerts.get());
    m_symbolCircle->setNormalArray(listNorms.get());
    m_symbolCircle->setNormalBinding(osg::Geometry::BIND_OVERALL);
    m_symbolCircle->addPrimitiveSet(listIdxs.get());
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

osg::Vec3 MapRendererOSG::convVec3ToOsgVec3(const Vec3 &myVector)
{   return osg::Vec3(myVector.x,myVector.y,myVector.z);   }

osg::Vec3d MapRendererOSG::convVec3ToOsgVec3d(const Vec3 &myVector)
{   return osg::Vec3d(myVector.x,myVector.y,myVector.z);   }

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
