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

namespace osmsrender
{

std::vector<GLdouble*> MapRendererOSG::m_tListNewVx(0);     // for tessellator

MapRendererOSG::MapRendererOSG(osgViewer::Viewer *myViewer,
                               std::string const &pathShaders,
                               std::string const &pathFonts,
                               std::string const &pathCoastGeom) :
    m_pathShaders(pathShaders),
    m_pathFonts(pathFonts),
    m_pathCoastGeom(pathCoastGeom),

    m_countVxLyAreas(0),
    m_countVxDsAreas(0),
    m_limitVxLyAreas(20000),
    m_limitVxDsAreas(30000),
    m_doneUpdDsAreas(false),
    m_modDsAreas(false),
    m_doneUpdLyAreas(false),
    m_modLyAreas(false),
    m_doneUpdDsRelAreas(false),
    m_modDsRelAreas(false),
    m_doneUpdLyRelAreas(false),
    m_modLyRelAreas(false),

    m_showCameraPlane(false)
{

    // fix up path strings
    if(m_pathShaders[m_pathShaders.length()-1] != '/')
    {   m_pathShaders.append("/");   }

    if(m_pathFonts[m_pathFonts.length()-1] != '/')
    {   m_pathFonts.append("/");   }

    // init scene graph nodes
    m_nodeRoot          = new osg::Group;
    m_nodeEarth         = new osg::Group;
    m_nodeNodes         = new osg::Group;
    m_nodeWays          = new osg::Group;
    m_nodeAreaLabels    = new osg::Group;
    m_geodeDsAreas      = new osg::Geode;
    m_geodeLyAreas      = new osg::Geode;
    m_xfDsAreas         = new osg::MatrixTransform;
    m_xfLyAreas         = new osg::MatrixTransform;

    // arrange scene graph
    m_nodeRoot->addChild(m_nodeEarth);
    m_nodeRoot->addChild(m_nodeNodes);
    m_nodeRoot->addChild(m_nodeWays);
    m_nodeRoot->addChild(m_nodeAreaLabels);

    m_nodeRoot->addChild(m_xfDsAreas);
        m_xfDsAreas->addChild(m_geodeDsAreas);

    m_nodeRoot->addChild(m_xfLyAreas);
        m_xfLyAreas->addChild(m_geodeLyAreas);

    // setup shaders
    this->setupShaders();

    // build geometry used as node symbols
    buildGeomTriangle();
    buildGeomSquare();
    buildGeomCircle();

    buildGeomTriangleOutline();
    buildGeomSquareOutline();
    buildGeomCircleOutline();

    // add scene to viewer
    m_viewer = myViewer;
    myViewer->setSceneData(m_nodeRoot);

    // enable matrix uniforms and vertex aliasing for shaders
    // note: osg attribute aliasing uses:
    // 0 - osgVertex
    // 1 - osgNormal
    // 2 - osgColor
    // 3-7 - osgMultiTex01234
    // 6 - osgSecondaryColor
    // 7 - osgFogCoord
    osgViewer::Viewer::Windows windows;
    m_viewer->getWindows(windows);
    for(osgViewer::Viewer::Windows::iterator itr = windows.begin();
        itr != windows.end();
        ++itr)   {
        (*itr)->getState()->setUseModelViewAndProjectionUniforms(true);
        (*itr)->getState()->setUseVertexAttributeAliasing(true);
    }

    // init tess
    m_tobj = osg::gluNewTess();
    osg::gluTessCallback(m_tobj, GLU_TESS_BEGIN,            (void(*)())tessBeginCallback);
    osg::gluTessCallback(m_tobj, GLU_TESS_VERTEX_DATA,      (void(*)())tessVertexCallback);
    osg::gluTessCallback(m_tobj, GLU_TESS_COMBINE_DATA,     (void(*)())tessCombineCallback);
    osg::gluTessCallback(m_tobj, GLU_TESS_EDGE_FLAG_DATA,   (void(*)())tessEdgeCallback);
    osg::gluTessCallback(m_tobj, GLU_TESS_END,              (void(*)())tessEndCallback);
    osg::gluTessCallback(m_tobj, GLU_TESS_ERROR,            (void(*)())tessErrorCallback);
    osg::gluTessProperty(m_tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
}

MapRendererOSG::~MapRendererOSG() {} // todo delete tessellator

void MapRendererOSG::ShowPlanetSurface()
{
    // show planet surface node
    size_t numChildren = m_nodeEarth->getNumChildren();
    for(size_t i=0; i < numChildren; i++)   {
        osg::Node * childNode = m_nodeEarth->getChild(i);
        if(childNode->getName().compare("PlanetSurface") == 0)   {
            childNode->setNodeMask(~0); // bit op NOT(0)
        }
    }
}

void MapRendererOSG::HidePlanetSurface()
{
    // hide planet surface node
    size_t numChildren = m_nodeEarth->getNumChildren();
    for(size_t i=0; i < numChildren; i++)   {
        osg::Node * childNode = m_nodeEarth->getChild(i);
        if(childNode->getName().compare("PlanetSurface") == 0)   {
            childNode->setNodeMask(0);
        }
    }
}

void MapRendererOSG::ShowPlanetCoastlines()
{
    // show planet coastlines node
    size_t numChildren = m_nodeEarth->getNumChildren();
    for(size_t i=0; i < numChildren; i++)   {
        osg::Node * childNode = m_nodeEarth->getChild(i);
        if(childNode->getName().compare("PlanetCoastlines") == 0)   {
            childNode->setNodeMask(~0); // bit op NOT(0)
        }
    }
}

void MapRendererOSG::HidePlanetCoastlines()
{
    // hide planet coastlines node
    size_t numChildren = m_nodeEarth->getNumChildren();
    for(size_t i=0; i < numChildren; i++)   {
        osg::Node * childNode = m_nodeEarth->getChild(i);
        if(childNode->getName().compare("PlanetCoastlines") == 0)   {
            childNode->setNodeMask(0);
        }
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::rebuildStyleData(std::vector<DataSet const *> const &listDataSets)
{
    // build font cache
    std::vector<std::string> listFonts;
    this->getFontList(listFonts);

    m_fontGeoMap.clear();
    m_fontGeoMap.reserve(listFonts.size());

    OSRDEBUG << "INFO: Font Types Found: ";
    for(size_t i=0; i < listFonts.size(); i++)
    {
        OSRDEBUG << "INFO:   " << listFonts[i];

        CharGeoMap fontChars;
        fontChars.reserve(100);

        // TODO handle multiple locales
        std::string baseCharList("abcdefghijklmnopqrstuvwxyz"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "0123456789 '.-");

        for(size_t j=0; j < baseCharList.size(); j++)
        {
            std::string charStr = baseCharList.substr(j,1);
            osg::ref_ptr<osgText::Text> textChar = new osgText::Text;

            // note: since space chars return a quad with zero
            // dims, we replace them with hyphens to est. dims
            // and set opacity to zero
            textChar->setAlignment(osgText::Text::CENTER_BASE_LINE);
            textChar->setFont(m_pathFonts + listFonts[i]);
            textChar->setCharacterSize(1.0);

            // todo fixme spaces
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

    // define layers for rendering order
    unsigned int numAreaLayers=this->getMaxAreaLayer()+1;
    unsigned int numWayLayers=this->getMaxWayLayer()+1;

    m_minLayer=0;

    m_layerPlanetSurface = m_minLayer;
    m_layerPlanetCoastline = m_layerPlanetSurface+1;        // the first two things we render is
                                                            // the planet surface and coastlines

    m_layerBaseAreas = m_layerPlanetCoastline+1;            // areas have two features per layer:
                                                            // 1 - area outline fill
                                                            // 2 - area fill

    m_layerTunnels  = m_layerBaseAreas+numAreaLayers*2;     // tunnels start after areas and have four
                                                            // features per tunnel
                                                            // 1 - tunnel outline fill
                                                            // 2 - tunnel line fill
                                                            // 3 - tunnel oneway fill
                                                            // 4 - tunnel contour label

    m_layerBaseWayOLs = m_layerTunnels+4;                   // way outlines start after tunnels

    m_layerBaseWays = m_layerBaseWayOLs+numWayLayers;       // way line fills start after way outlines

    m_layerBaseWayLabels = m_layerBaseWays+numWayLayers*2;  // way labels start after way line fills

    m_layerBridges = m_layerBaseWayLabels+numWayLayers;     // bridges start after way labels and have
                                                            // four features per bridge
                                                            // 1 - bridge outline fill
                                                            // 2 - bridge line fill
                                                            // 3 - bridge oneway fill
                                                            // 4 - bridge contour label

    m_depthSortedBin = m_layerBridges+4+10;                 // the depth sorted bin is rendered last

    // specify blend function for bridges
    m_blendFunc_bridge = new osg::BlendFunc;
    m_blendFunc_bridge->setFunction(GL_ONE,GL_ONE);

    // enable blending for transparency
    osg::StateSet * rootss = m_nodeRoot->getOrCreateStateSet();
    rootss->setMode(GL_BLEND,osg::StateAttribute::ON);

    // set area shaders
    m_geodeLyAreas->getOrCreateStateSet()->setAttributeAndModes(m_shaderDirectAttr);
    m_geodeLyAreas->getOrCreateStateSet()->setRenderBinDetails(m_layerBaseAreas,"RenderBin");

    m_geodeDsAreas->getOrCreateStateSet()->setAttributeAndModes(m_shaderDiffuseAttr);
    m_geodeDsAreas->getOrCreateStateSet()->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin");

    // add planet geometry if style requires it; planet
    // style data is common across all DataSets and lods
    RenderStyleConfig * rStyle = listDataSets[0]->listStyleConfigs[0];
    if(rStyle->GetPlanetShowSurface())   {
        ColorRGBA surfColor = rStyle->GetPlanetSurfaceColor();
        this->addEarthSurfaceGeometry(surfColor);
    }
    if(rStyle->GetPlanetShowCoastline())   {
        ColorRGBA coastColor = rStyle->GetPlanetCoastlineColor();
        this->addEarthCoastlineGeometry(coastColor);
    }
}

unsigned int MapRendererOSG::getAreaRenderBin(unsigned int areaLayer)
{   return (m_layerBaseAreas + 2*areaLayer);   }

unsigned int MapRendererOSG::getWayOLRenderBin(unsigned int wayLayer)
{   return (m_layerBaseWayOLs + wayLayer);   }

unsigned int MapRendererOSG::getWayRenderBin(unsigned int wayLayer)
{   return (m_layerBaseWays + 2*wayLayer);   }

unsigned int MapRendererOSG::getWayLabelRenderBin(unsigned int wayLayer)
{   return (m_layerBaseWayLabels + wayLayer);   }

unsigned int MapRendererOSG::getTunnelRenderBin()
{   return m_layerTunnels;   }

unsigned int MapRendererOSG::getBridgeRenderBin()
{   return m_layerBridges;   }

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::showCameraViewArea(Camera &sceneCam)
{
    if(!m_showCameraPlane)
    {   return;   }

    //
//    PointLLA pointNW(sceneCam.maxLat,sceneCam.minLon,5);
//    PointLLA pointNE(sceneCam.maxLat,sceneCam.maxLon,5);
//    PointLLA pointSW(sceneCam.minLat,sceneCam.minLon,5);
//    PointLLA pointSE(sceneCam.minLat,sceneCam.maxLon,5);

//    osg::Vec3 vecNW = convVec3ToOsgVec3(convLLAToECEF(pointNW));
//    osg::Vec3 vecNE = convVec3ToOsgVec3(convLLAToECEF(pointNE));
//    osg::Vec3 vecSW = convVec3ToOsgVec3(convLLAToECEF(pointSW));
//    osg::Vec3 vecSE = convVec3ToOsgVec3(convLLAToECEF(pointSE));

//    osg::Array * listVxArray = m_camGeom->getVertexArray();
//    osg::Vec3Array * listVx = dynamic_cast<osg::Vec3Array*>(listVxArray);
//    listVx->at(0) = vecNW;
//    listVx->at(1) = vecNE;
//    listVx->at(2) = vecSW;
//    listVx->at(3) = vecSE;
//    m_camGeom->setVertexArray(listVx);

//    OSRDEBUG << "Debug Camera Plane:";
//    OSRDEBUG << "Eye:"<<sceneCam.eye.x<<","<<sceneCam.eye.y<<","<<sceneCam.eye.z;
//    OSRDEBUG << "ViewPt:"<<sceneCam.viewPt.x<<","<<sceneCam.viewPt.y<<","<<sceneCam.viewPt.z;
//    OSRDEBUG << "Up:"<<sceneCam.up.x<<","<<sceneCam.up.y<<","<<sceneCam.up.z;
//    OSRDEBUG << "LLA:"<<sceneCam.LLA.lat<<","<<sceneCam.LLA.lon<<","<<sceneCam.LLA.alt;

//    OSRDEBUG << "minLat" << sceneCam.minLat;
//    OSRDEBUG << "minLon" << sceneCam.minLon;
//    OSRDEBUG << "maxLat" << sceneCam.maxLat;
//    OSRDEBUG << "maxLon" << sceneCam.maxLon;
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
    if(nodeData.hasName)   {
        this->addNodeLabel(nodeData,offsetVec,nodeTransform,true);
    }

    // add the transform node to the scene graph
    m_nodeNodes->addChild(nodeTransform.get());

    // save a reference to (a reference of) this node
    osg::ref_ptr<osg::Node> * nodeRefPtr = new osg::ref_ptr<osg::Node>;
    (*nodeRefPtr) = nodeTransform;
    nodeData.geomPtr = nodeRefPtr;
}

void MapRendererOSG::removeNodeFromScene(const NodeRenderData &nodeData)
{
    // recast nodeData void* reference to osg::Node
    osg::ref_ptr<osg::Node> * nodeNode =
            reinterpret_cast<osg::ref_ptr<osg::Node>*>(nodeData.geomPtr);

    m_nodeNodes->removeChild(nodeNode->get());
    delete nodeNode;
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
    // [area geometry]
    VxAttributes vxAttr;
    vxAttr.listVx = new osg::Vec3Array;
    vxAttr.listNx = new osg::Vec3Array;
    vxAttr.listCx = new osg::Vec4Array;

    Id areaId = this->getNewAreaId();
    this->createAreaGeometry(areaData,vxAttr);
    std::pair<Id,VxAttributes> insData(areaId,vxAttr);

    if(areaData.isBuilding)   {
        m_modDsAreas = true;
        m_mapDsAreaGeo.insert(insData);
    }
    else   {
        m_modLyAreas = true;
        m_mapLyAreaGeo.insert(insData);
    }

    // [area label]
    if(areaData.hasName)   {
        // create area label geometry
        osg::ref_ptr<osg::MatrixTransform> xfNode = new osg::MatrixTransform;
        xfNode->setMatrix(osg::Matrix::translate(vxAttr.centerPt));
        this->addAreaLabel(areaData,vxAttr.centerPt,xfNode.get(),true);
        m_nodeAreaLabels->addChild(xfNode.get());

        // save reference
        std::pair<Id,osg::Node*> insLabelData(areaId,xfNode);
        m_listAreaLabels.insert(insLabelData);
    }

    // save a key for this data
    size_t * idKey = new size_t(areaId);
    areaData.geomPtr = idKey;
}

void MapRendererOSG::removeAreaFromScene(const AreaRenderData &areaData)
{
    // lookup id
    size_t * pAreaId = static_cast<size_t*>(areaData.geomPtr);

    // [area geometry]
    if(areaData.isBuilding)
    {   m_mapDsAreaGeo.erase((*pAreaId));   }
    else
    {   m_mapLyAreaGeo.erase((*pAreaId));   }

    // [area label]
    IdOsgNodeMap::iterator nIt = m_listAreaLabels.find((*pAreaId));
    if(!(nIt == m_listAreaLabels.end()))   {
        osg::Node * labelNode = nIt->second;
        m_nodeAreaLabels->removeChild(labelNode);
        m_listAreaLabels.erase(nIt);
    }

    // clean up
    delete pAreaId;
}

void MapRendererOSG::doneUpdatingAreas()
{
    // depth sorted
    m_doneUpdDsAreas = true;

    if(m_doneUpdDsAreas && m_doneUpdDsRelAreas)   {
        this->addDsAreaGeometries();
        m_modDsAreas = false;
        m_modDsRelAreas = false;
    }

    // layered
    m_doneUpdLyAreas = true;

    if(m_doneUpdLyAreas && m_doneUpdLyRelAreas)   {
        this->addLyAreaGeometries();
        m_modLyAreas = false;
        m_modLyRelAreas = false;
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addRelAreaToScene(RelAreaRenderData &relAreaData)
{
    // [relation area geometry]
    size_t aCount = relAreaData.listAreaData.size();
    osg::Vec3d mergedCenterPt(0,0,0);

    std::pair<Id,VxAttributes> insData;
    insData.first = this->getNewAreaId();

    // for depth sorted geometry
    osg::ref_ptr<osg::Vec3Array> mergedListDsVx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> mergedListDsNx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> mergedListDsCx = new osg::Vec4Array;

    // for layered geometry
    osg::ref_ptr<osg::Vec3Array> mergedListLyVx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> mergedListLyNx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> mergedListLyCx = new osg::Vec4Array;

    for(size_t i=0; i < aCount; i++)
    {
        AreaRenderData const &areaData =
                relAreaData.listAreaData[i];

        VxAttributes vxAttr;
        vxAttr.listVx = new osg::Vec3Array;
        vxAttr.listNx = new osg::Vec3Array;
        vxAttr.listCx = new osg::Vec4Array;
        this->createAreaGeometry(areaData,vxAttr);
        mergedCenterPt = mergedCenterPt + vxAttr.centerPt;

        if(areaData.isBuilding)   {
            m_modDsRelAreas = true;
            mergedListDsVx->insert(mergedListDsVx->end(),
                vxAttr.listVx->begin(),vxAttr.listVx->end());
            mergedListDsNx->insert(mergedListDsNx->end(),
                vxAttr.listNx->begin(),vxAttr.listNx->end());
            mergedListDsCx->insert(mergedListDsCx->end(),
                vxAttr.listCx->begin(),vxAttr.listCx->end());
        }
        else   {
            m_modLyRelAreas = true;
            mergedListLyVx->insert(mergedListLyVx->end(),
                vxAttr.listVx->begin(),vxAttr.listVx->end());
            mergedListLyNx->insert(mergedListLyNx->end(),
                vxAttr.listNx->begin(),vxAttr.listNx->end());
            mergedListLyCx->insert(mergedListLyCx->end(),
                vxAttr.listCx->begin(),vxAttr.listCx->end());
        }
    }

    // depth sorted
    VxAttributes vxAttr;
    vxAttr.listVx = mergedListDsVx;
    vxAttr.listNx = mergedListDsNx;
    vxAttr.listCx = mergedListDsCx;
    vxAttr.centerPt = mergedCenterPt/aCount;
    insData.second = vxAttr;
    m_mapDsRelAreaGeo.insert(insData);

    // layered
    vxAttr.listVx = mergedListLyVx;
    vxAttr.listNx = mergedListLyNx;
    vxAttr.listCx = mergedListLyCx;
    insData.second = vxAttr;
    m_mapLyRelAreaGeo.insert(insData);

    // [relation area label]
    // we add the label of the first area to the
    // overall center point of the relation

    if(relAreaData.listAreaData[0].hasName)
    {
        osg::ref_ptr<osg::MatrixTransform> xfNode =
                new osg::MatrixTransform;

        this->addAreaLabel(relAreaData.listAreaData[0],
                           vxAttr.centerPt,xfNode,true);

        m_nodeAreaLabels->addChild(xfNode.get());

        // save reference
        std::pair<Id,osg::Node*> insLabelData(insData.first,xfNode);
        m_listAreaLabels.insert(insLabelData);
    }

    // save a key for this data
    size_t * idKey = new size_t(insData.first);
    relAreaData.geomPtr = idKey;
}

void MapRendererOSG::removeRelAreaFromScene(const RelAreaRenderData &relAreaData)
{
    // lookup id
    size_t * pRelAreaId = static_cast<size_t*>(relAreaData.geomPtr);

    // [relation area geometry]
    m_mapDsRelAreaGeo.erase((*pRelAreaId));
    m_mapLyRelAreaGeo.erase((*pRelAreaId));

    // [relation area label]
    IdOsgNodeMap::iterator nIt = m_listAreaLabels.find((*pRelAreaId));
    if(!(nIt == m_listAreaLabels.end()))   {
        osg::Node * labelNode = nIt->second;
        m_nodeAreaLabels->removeChild(labelNode);
        m_listAreaLabels.erase(nIt);
    }

    // clean up
    delete pRelAreaId;
}

void MapRendererOSG::doneUpdatingRelAreas()
{
    m_doneUpdDsRelAreas = true;

    if(m_doneUpdDsAreas && m_doneUpdDsRelAreas)   {
        this->addDsAreaGeometries();
        m_modDsAreas = false;
        m_modDsRelAreas = false;
    }

    // layered
    m_doneUpdLyRelAreas = true;

    if(m_doneUpdLyAreas && m_doneUpdLyRelAreas)   {
        this->addLyAreaGeometries();
        m_modLyAreas = false;
        m_modLyRelAreas = false;
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::removeAllFromScene()
{
    // clear all area data
    m_geodeDsAreas->removeDrawables(0,m_geodeDsAreas->getNumDrawables());
    m_geodeLyAreas->removeDrawables(0,m_geodeLyAreas->getNumDrawables());

    m_mapDsAreaGeo.clear();
    m_mapLyAreaGeo.clear();
    m_mapDsRelAreaGeo.clear();
    m_mapLyRelAreaGeo.clear();

    // clear group nodes
    m_nodeAreaLabels->removeChildren(0,m_nodeAreaLabels->getNumChildren());
    m_nodeEarth->removeChildren(0,m_nodeEarth->getNumChildren());
    m_nodeWays->removeChildren(0,m_nodeWays->getNumChildren());
    m_nodeNodes->removeChildren(0,m_nodeNodes->getNumChildren());
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addEarthSurfaceGeometry(ColorRGBA const &surfColor)
{
    std::vector<Vec3> myVertices;
    std::vector<Vec3> myNormals;
    std::vector<Vec2> myTexCoords;
    std::vector<unsigned int> myIndices;

    // get earth surface geometry (we only use points)
    bool opOk = this->buildEarthSurfaceGeometry(180,360,
                                                myVertices,
                                                myNormals,
                                                myTexCoords,
                                                myIndices);
    if(!opOk)
    {   return;   }

    // convert data to osg geometry
    osg::ref_ptr<osg::Vec3Array> listVx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listNx = new osg::Vec3Array;

    for(size_t i=0; i < myVertices.size(); i++)   {
        osg::Vec3 vx = convVec3ToOsgVec3(myVertices[i]);  // vertex
        listVx->push_back(vx);

        osg::Vec3 nx = convVec3ToOsgVec3(myNormals[i]);   // normal
        listNx->push_back(nx);
    }

    osg::ref_ptr<osg::DrawElementsUInt> listIx =
            new osg::DrawElementsUInt(GL_TRIANGLES);

    for(size_t i=0; i < myIndices.size(); i++)   {
        listIx->push_back(myIndices[i]);
    }

    // planet geometry
    osg::ref_ptr<osg::Geometry> geomEarth = new osg::Geometry;
    geomEarth->setVertexArray(listVx);
    geomEarth->setNormalArray(listNx);
    geomEarth->addPrimitiveSet(listIx);

    // color uniform
    osg::Vec4 planetColor = this->colorAsVec4(surfColor);
    osg::ref_ptr<osg::Uniform> uColor = new osg::Uniform("Color",planetColor);

    // planet geode
    osg::ref_ptr<osg::Geode> geodeEarth = new osg::Geode;
    osg::StateSet *ss = geodeEarth->getOrCreateStateSet();
    ss->setRenderBinDetails(m_layerPlanetSurface,"RenderBin");
    ss->setAttributeAndModes(m_shaderDirect);
    ss->addUniform(uColor);
    geodeEarth->setName("PlanetSurface");
    geodeEarth->addDrawable(geomEarth);
    geodeEarth->setNodeMask(0); // hidden by default

    // add to scene
    m_nodeEarth->addChild(geodeEarth);
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addEarthCoastlineGeometry(const ColorRGBA &coastColor)
{
//    // [point cloud]
//    //
//    std::vector<Vec3> coastVx;
//    if(!buildCoastlinePointCloud(m_pathCoastGeom,coastVx))
//    {   return;   }

//    osg::ref_ptr<osg::Vec3Array> listVx = new osg::Vec3Array;
//    for(size_t i=0; i < coastVx.size(); i+=4)   {
//        listVx->push_back(convVec3ToOsgVec3(coastVx[i]));
//    }

//    // coast geometry
//    osg::ref_ptr<osg::Geometry> geomCoast = new osg::Geometry;
//    geomCoast->setVertexArray(listVx);
//    geomCoast->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,listVx->size()));

    // [lines]
    std::vector<Vec3> coastVx;
    std::vector<unsigned int> coastIx;
    if(!buildCoastlineLines(m_pathCoastGeom,coastVx,coastIx))
    {   return;   }

    osg::ref_ptr<osg::Vec3Array> listVx = new osg::Vec3Array;
    for(size_t i=0; i < coastVx.size(); i++)
    {   listVx->push_back(convVec3ToOsgVec3(coastVx[i]));   }

    osg::ref_ptr<osg::DrawElementsUInt> listIx =
            new osg::DrawElementsUInt(GL_LINES);
    for(size_t i=0; i < coastIx.size(); i++)
    {   listIx->push_back(coastIx[i]);   }

    // coast geometry
    osg::ref_ptr<osg::Geometry> geomCoast = new osg::Geometry;
    geomCoast->setVertexArray(listVx);
    geomCoast->addPrimitiveSet(listIx);

    // color uniform
    osg::Vec4 geomColor = this->colorAsVec4(coastColor);
    osg::ref_ptr<osg::Uniform> uColor = new osg::Uniform("Color",geomColor);

    // camera eye uniform
    osg::ref_ptr<osg::Uniform> uCamEye = new osg::Uniform("ViewDirn",osg::Vec3(0,0,0));
    m_cbEarthCoastlineShader.SetSceneCamera(m_viewer->getCamera());
    uCamEye->setUpdateCallback(&m_cbEarthCoastlineShader);

    // planet geode
    osg::ref_ptr<osg::Geode> geodeCoast = new osg::Geode;
    osg::StateSet *ss = geodeCoast->getOrCreateStateSet();
    ss->setRenderBinDetails(m_layerPlanetCoastline,"RenderBin");
    ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    ss->setAttributeAndModes(m_shaderEarthCoastlineLines);
    ss->addUniform(uColor);
    ss->addUniform(uCamEye);
    geodeCoast->setName("PlanetCoastlines");
    geodeCoast->addDrawable(geomCoast);
    geodeCoast->setNodeMask(0); // hidden by default

    // add to scene
    m_nodeEarth->addChild(geodeCoast);

    // we need to enable variable point sprite sizes explicitly
    ss = m_nodeRoot->getOrCreateStateSet();
    ss->setMode(GL_PROGRAM_POINT_SIZE,osg::StateAttribute::ON);
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addNodeGeometry(const NodeRenderData &nodeData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent)
{

    // get symbol type
    osg::ref_ptr<osg::Geometry> geomSymbol;

    SymbolStyleType sType =
            nodeData.symbolRenderStyle->GetSymbolType();

    if(sType == SYMBOL_TRIANGLE)      {
        geomSymbol = m_symbolTriangle;
    }
    else if(sType == SYMBOL_SQUARE)   {
        geomSymbol = m_symbolSquare;
    }
    else if(sType == SYMBOL_CIRCLE)   {
        geomSymbol = m_symbolCircle;
    }

    // calculate the position vector taking offsetHeight into account
    // note: we use Vec3d to try and improve positional accuracy
    double offsetHeight = nodeData.symbolRenderStyle->GetOffsetHeight();
    osg::Vec3d surfaceVec = offsetVec;
    osg::Vec3d normVec = surfaceVec; normVec.normalize();
    osg::Vec3d shiftVec = surfaceVec+(normVec*offsetHeight)-offsetVec;

    // transform: (billboard + scale)
    osg::ref_ptr<osg::AutoTransform> xfSymbol = new osg::AutoTransform;
    xfSymbol->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
    xfSymbol->setScale(nodeData.symbolRenderStyle->GetSymbolSize()/2);
    xfSymbol->setPosition(shiftVec);

    // add outline if required
    double symbolSize = nodeData.symbolRenderStyle->GetSymbolSize();
    double oL = nodeData.fillRenderStyle->GetOutlineWidth();
    if(oL > 0)
    {
        osg::ref_ptr<osg::Geometry> geomOutline;

        if(sType == SYMBOL_TRIANGLE)      {
            geomOutline = dynamic_cast<osg::Geometry*>
                    (m_symbolTriangleOutline->clone(osg::CopyOp::DEEP_COPY_ALL));
        }
        else if(sType == SYMBOL_SQUARE)   {
            geomOutline = dynamic_cast<osg::Geometry*>
                    (m_symbolSquareOutline->clone(osg::CopyOp::DEEP_COPY_ALL));
        }
        else if(sType == SYMBOL_CIRCLE)   {
            geomOutline = dynamic_cast<osg::Geometry*>
                    (m_symbolCircleOutline->clone(osg::CopyOp::DEEP_COPY_ALL));
        }

        // adjust outline geometry based on outline width
        osg::ref_ptr<osg::Vec3Array> listVerts =
                dynamic_cast<osg::Vec3Array*>(geomOutline->getVertexArray());

        double outlineFrac = (oL+symbolSize)/symbolSize;
        for(size_t i=0; i < listVerts->size(); i+=2)   {
            listVerts->at(i+1) *= outlineFrac;
        }

        // color uniform
        osg::Vec4 outlineColor = colorAsVec4(nodeData.fillRenderStyle->GetOutlineColor());
        osg::ref_ptr<osg::Uniform> uOutlineColor = new osg::Uniform("Color",outlineColor);

        // geode: symbol outline
        osg::ref_ptr<osg::Geode> geodeSymbolOutline = new osg::Geode;
        osg::StateSet *ss = geodeSymbolOutline->getOrCreateStateSet();
        geodeSymbolOutline->addDrawable(geomOutline);
        ss->setAttributeAndModes(m_shaderDirect);
        ss->addUniform(uOutlineColor);

        xfSymbol->addChild(geodeSymbolOutline);
    }

    // color uniform
    osg::Vec4 fillColor = colorAsVec4(nodeData.fillRenderStyle->GetFillColor());
    osg::ref_ptr<osg::Uniform> uFillColor = new osg::Uniform("Color",fillColor);

    // geode: symbol
    osg::ref_ptr<osg::Geode> geodeSymbol = new osg::Geode;
    osg::StateSet *ss = geodeSymbol->getOrCreateStateSet();
    geodeSymbol->addDrawable(geomSymbol);
    ss->setAttributeAndModes(m_shaderDirect);
    ss->addUniform(uFillColor);

    // transform: symbol
    xfSymbol->getOrCreateStateSet()->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin");
    xfSymbol->addChild(geodeSymbol);

    nodeParent->addChild(xfSymbol.get());
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addWayGeometry(const WayRenderData &wayData,
                                    const osg::Vec3d &offsetVec,
                                    osg::MatrixTransform *nodeParent)
{
    osg::StateSet * ss;

    // get layer data
    // note: generic way geometry comprises 2 layers
    //       (way outlines are separate for normal ways)
    //       layer 1: way line
    //       layer 2: oneway markers
    //
    //       contour labels are painted after all other
    //       ways and way oneway arrows are drawn

    unsigned int wayBaseLayer,wayOutlineLayer,wayOnewayLayer;
    if(wayData.wayRef->IsBridge())   {
        wayOutlineLayer = getBridgeRenderBin();
        wayBaseLayer = wayOutlineLayer+1;
        wayOnewayLayer = wayOutlineLayer+2;
    }
    else if(wayData.wayRef->IsTunnel())   {
        wayOutlineLayer = getTunnelRenderBin();
        wayBaseLayer = wayOutlineLayer+1;
        wayOnewayLayer = wayOutlineLayer+2;
    }
    else   {
        wayOutlineLayer = getWayOLRenderBin(wayData.wayLayer);
        wayBaseLayer = getWayRenderBin(wayData.wayLayer);
        wayOnewayLayer = wayBaseLayer+1;
    }

    // way line geometry
    std::vector<Vec3> wayVertexArray;
    this->buildPolylineAsTriStrip(wayData.listWayPoints,
                                  wayData.lineRenderStyle->GetLineWidth(),
                                  OL_CENTER,wayVertexArray);

    osg::ref_ptr<osg::Vec3Array> listWayTriStripPts=
            new osg::Vec3Array(wayVertexArray.size());

    osg::ref_ptr<osg::Vec3Array> listWayTriStripNorms=
            new osg::Vec3Array(wayVertexArray.size());

    for(size_t i=0; i < wayVertexArray.size(); i++)   {
        listWayTriStripPts->at(i) =
                convVec3ToOsgVec3(wayVertexArray[i]) - offsetVec;
        listWayTriStripNorms->at(i) =
                convVec3ToOsgVec3(wayVertexArray[i].Normalized());
    }

    // geometry: way line
    osg::ref_ptr<osg::Geometry> geomWay = new osg::Geometry;
    geomWay->setVertexArray(listWayTriStripPts);
    geomWay->setNormalArray(listWayTriStripNorms);
    geomWay->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geomWay->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
        listWayTriStripPts->size()));

    // color uniform
    osg::Vec4 lineColor = colorAsVec4(wayData.lineRenderStyle->GetLineColor());
    osg::ref_ptr<osg::Uniform> uLineColor = new osg::Uniform("Color",lineColor);

    // geode: way line
    osg::ref_ptr<osg::Geode> geodeWayLine = new osg::Geode;
    ss = geodeWayLine->getOrCreateStateSet();
    ss->addUniform(uLineColor);
    ss->setAttributeAndModes(m_shaderDirect);
    ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    ss->setRenderBinDetails(wayBaseLayer,"RenderBin");
    geodeWayLine->addDrawable(geomWay);
    nodeParent->addChild(geodeWayLine);

    // create way outline
    if(wayData.lineRenderStyle->GetOutlineWidth() > 0)
    {
        double extOutlineWidth = wayData.lineRenderStyle->GetLineWidth()+
                wayData.lineRenderStyle->GetOutlineWidth();

        std::vector<Vec3> wayOLVertexArray;
        this->buildPolylineAsTriStrip(wayData.listWayPoints,
                                      extOutlineWidth,OL_CENTER,
                                      wayOLVertexArray);

        osg::ref_ptr<osg::Vec3Array> listWayOLTriStripPts=
                new osg::Vec3Array(wayOLVertexArray.size());

        osg::ref_ptr<osg::Vec3Array> listWayOLTriStripNorms=
                new osg::Vec3Array(wayOLVertexArray.size());

        for(size_t i=0; i < wayOLVertexArray.size(); i++)   {
            listWayOLTriStripPts->at(i) =
                    convVec3ToOsgVec3(wayOLVertexArray[i]) - offsetVec;
            listWayOLTriStripNorms->at(i) =
                    convVec3ToOsgVec3(wayOLVertexArray[i].Normalized());
        }

        // geometry: way outline
        osg::ref_ptr<osg::Geometry> geomWayOL = new osg::Geometry;
        geomWayOL->setVertexArray(listWayOLTriStripPts.get());
        geomWayOL->setNormalArray(listWayOLTriStripNorms.get());
        geomWayOL->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        geomWayOL->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP,0,
            listWayOLTriStripPts->size()));

        // color uniform
        osg::Vec4 outlineColor = colorAsVec4(wayData.lineRenderStyle->GetOutlineColor());
        osg::ref_ptr<osg::Uniform> uOutlineColor = new osg::Uniform("Color",outlineColor);

        // geode: way outline
        osg::ref_ptr<osg::Geode> geodeWayOL = new osg::Geode;
        ss = geodeWayOL->getOrCreateStateSet();
        ss->addUniform(uOutlineColor);
        ss->setAttributeAndModes(m_shaderDirect);
        ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        ss->setRenderBinDetails(wayOutlineLayer,"RenderBin");
        geodeWayOL->addDrawable(geomWayOL);
        nodeParent->addChild(geodeWayOL);
    }

    // create oneWay markers
    if(wayData.lineRenderStyle->GetOnewayWidth() > 0)
    {
        osg::ref_ptr<osg::Vec3dArray> listWayPoints =
                new osg::Vec3dArray(wayData.listWayPoints.size());

        for(size_t i=0; i < wayData.listWayPoints.size(); i++)
        {   listWayPoints->at(i) = convVec3ToOsgVec3(wayData.listWayPoints[i]);   }

        double onewayWidth = wayData.lineRenderStyle->GetOnewayWidth();
        double onewayPadding = wayData.lineRenderStyle->GetOnewayPadding();
        double paddedLength = ((2*onewayPadding*onewayWidth)+onewayWidth);
        int numSymbols = calcWayLength(listWayPoints)/paddedLength;

        osg::Matrixf xformMatrix;
        osg::Vec3d pointAtLength,dirnAtLength,
                   normAtLength,sideAtLength;

        // color uniform
        osg::Vec4 onewayColor = colorAsVec4(wayData.lineRenderStyle->GetOnewayColor());
        osg::ref_ptr<osg::Uniform> uOnewayColor = new osg::Uniform("Color",onewayColor);

        // group: oneway markers
        osg::ref_ptr<osg::Group> groupOneway = new osg::Group;
        ss = groupOneway->getOrCreateStateSet();
        ss->addUniform(uOnewayColor);
        ss->setAttributeAndModes(m_shaderDirect);
        ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        ss->setRenderBinDetails(wayOnewayLayer,"RenderBin");

        for(int i=1; i <= numSymbols; i++)
        {
            calcLerpAlongWay(listWayPoints,
                             listWayPoints,
                             i*paddedLength,
                             pointAtLength,
                             dirnAtLength,
                             normAtLength,
                             sideAtLength);

            xformMatrix(0,0) = sideAtLength.x()*onewayWidth;
            xformMatrix(0,1) = sideAtLength.y()*onewayWidth;
            xformMatrix(0,2) = sideAtLength.z()*onewayWidth;
            xformMatrix(0,3) = 0;

            xformMatrix(1,0) = dirnAtLength.x()*onewayWidth*2;
            xformMatrix(1,1) = dirnAtLength.y()*onewayWidth*2;
            xformMatrix(1,2) = dirnAtLength.z()*onewayWidth*2;
            xformMatrix(1,3) = 0;

            xformMatrix(2,0) = normAtLength.x()*onewayWidth;
            xformMatrix(2,1) = normAtLength.y()*onewayWidth;
            xformMatrix(2,2) = normAtLength.z()*onewayWidth;
            xformMatrix(2,3) = 0;

            // TODO normAtLength should be smaller?
            pointAtLength += normAtLength;
            xformMatrix(3,0) = pointAtLength.x()-offsetVec.x();
            xformMatrix(3,1) = pointAtLength.y()-offsetVec.y();
            xformMatrix(3,2) = pointAtLength.z()-offsetVec.z();
            xformMatrix(3,3) = 1;

            osg::ref_ptr<osg::Geode> geodeSym = new osg::Geode;
            geodeSym->addDrawable(m_symbolTriangle.get());

            osg::ref_ptr<osg::MatrixTransform> xfMarker =
                    new osg::MatrixTransform;

            xfMarker->setMatrix(xformMatrix);
            xfMarker->addChild(geodeSym);
            groupOneway->addChild(xfMarker);
        }
        nodeParent->addChild(groupOneway);
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::createAreaGeometry(const AreaRenderData &areaData,
                                        VxAttributes &vxAttr)
{
    // calculate base normal from earth's surface
    osg::Vec3d offsetVec = convVec3ToOsgVec3d(areaData.centerPoint);
    osg::Vec3d baseNormal = offsetVec;
    baseNormal.normalize();

    // triangulate building profile
    std::vector<Vec3> listRoofTriVx;
    std::vector<Vec3> listRoofTriNx;
    Vec3 tessNormal = areaData.centerPoint.Normalized();
    this->triangulateContours(areaData.listOuterPoints,
                              areaData.listListInnerPoints,
                              tessNormal,listRoofTriVx);

    if(!areaData.isBuilding)
    {   // if area is flat
        osg::Vec4 colorVec =
            colorAsVec4(areaData.fillRenderStyle->GetFillColor());

        size_t vCount = listRoofTriVx.size();
        vxAttr.listVx->resize(vCount);
        vxAttr.listNx->resize(vCount);
        vxAttr.listCx->resize(vCount);

        for(size_t i=0; i < vCount; i++)   {
            vxAttr.listVx->at(i) = convVec3ToOsgVec3(listRoofTriVx[i]);
            vxAttr.listNx->at(i) = baseNormal;
            vxAttr.listCx->at(i) = colorVec;
        }
        vxAttr.centerPt = offsetVec;
        vxAttr.layer = areaData.areaLayer;
        m_countVxLyAreas += vCount;
    }
    else
    {   // raise the roof
        listRoofTriNx.resize(listRoofTriVx.size());
        double const &bHeight = areaData.buildingHeight;
        Vec3 offsetHeight = tessNormal.ScaledBy(bHeight);
        for(size_t i=0; i < listRoofTriVx.size(); i++)   {
            listRoofTriVx[i] = listRoofTriVx[i] + offsetHeight;
            listRoofTriNx[i] = tessNormal;
        }

        // build sidewall faces
        std::vector<Vec3> listSideTriVx;
        std::vector<Vec3> listSideTriNx;

        // outer sidewall
        this->buildContourSideWalls(areaData.listOuterPoints,offsetHeight,
                                    listSideTriVx,listSideTriNx);
        // inner sidewalls
        for(size_t i=0; i < areaData.listListInnerPoints.size(); i++)   {
            this->buildContourSideWalls(areaData.listListInnerPoints[i],offsetHeight,
                                        listSideTriVx,listSideTriNx);
        }

        std::vector<Vec3> listTriVx;
        listTriVx.reserve(listRoofTriVx.size() + listSideTriVx.size());
        listTriVx.insert(listTriVx.end(),listRoofTriVx.begin(),listRoofTriVx.end());
        listTriVx.insert(listTriVx.end(),listSideTriVx.begin(),listSideTriVx.end());

        std::vector<Vec3> listTriNx;
        listTriNx.reserve(listRoofTriNx.size() + listRoofTriNx.size());
        listTriNx.insert(listTriNx.end(),listRoofTriNx.begin(),listRoofTriNx.end());
        listTriNx.insert(listTriNx.end(),listSideTriNx.begin(),listSideTriNx.end());

        osg::Vec4 colorVec = colorAsVec4(areaData.fillRenderStyle->GetFillColor());

        size_t vCount = listTriVx.size();
        vxAttr.listVx->resize(vCount);
        vxAttr.listNx->resize(vCount);
        vxAttr.listCx->resize(vCount);

        for(size_t i=0; i < listTriVx.size(); i++)   {
            vxAttr.listVx->at(i) = convVec3ToOsgVec3(listTriVx[i]);
            vxAttr.listNx->at(i) = convVec3ToOsgVec3(listTriNx[i]);
            vxAttr.listCx->at(i) = colorVec;
        }
        vxAttr.centerPt = offsetVec;
        m_countVxDsAreas += vCount;
    }
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addNodeLabel(const NodeRenderData &nodeData,
                                  const osg::Vec3d &offsetVec,
                                  osg::MatrixTransform *nodeParent,
                                  bool usingName)
{
    std::string labelText;
    LabelStyle const *labelStyle;
    osg::StateSet * ss;

    if(usingName)   {
        labelText = nodeData.nameLabel;
        labelStyle = nodeData.nameLabelRenderStyle;
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    // geometry: text
    osg::ref_ptr<osgText::Text> geomText = new osgText::Text;
    geomText->setFont(m_pathFonts + labelStyle->GetFontFamily());
    geomText->setAlignment(osgText::Text::CENTER_CENTER);
    geomText->setCharacterSize(labelStyle->GetFontSize());
    geomText->setText(labelText);

    osg::BoundingBox textBounds = geomText->getBound();
    double textHeight = textBounds.yMax()-textBounds.yMin();
    double textWidth = textBounds.xMax()-textBounds.xMin();
    double offsetDist = nodeData.nameLabelRenderStyle->GetOffsetDist();

    osg::Vec3 labelPlaceVec;
    SymbolLabelPos labelPos = nodeData.symbolRenderStyle->GetLabelPos();
    switch(labelPos)
    {
    case SYMBOL_TOP:
        labelPlaceVec.y() = (textHeight/2)+offsetDist;
        break;
    case SYMBOL_TOPRIGHT:
        labelPlaceVec.x() = textWidth/2+offsetDist*0.707;
        labelPlaceVec.y() = textHeight/2+offsetDist*0.707;
        break;
    case SYMBOL_RIGHT:
        labelPlaceVec.x() = textWidth/2+offsetDist;
        break;
    case SYMBOL_BTMRIGHT:
        labelPlaceVec.x() = textWidth/2+offsetDist*0.707;
        labelPlaceVec.y() = -1*(textHeight/2+offsetDist*0.707);
        break;
    case SYMBOL_BTM:
        labelPlaceVec.y() = -1*(textHeight/2+offsetDist);
        break;
    case SYMBOL_BTMLEFT:
        labelPlaceVec.x() = -1*(textWidth/2+offsetDist*0.707);
        labelPlaceVec.y() = -1*(textHeight/2+offsetDist*0.707);
        break;
    case SYMBOL_LEFT:
        labelPlaceVec.x() = -1*(textWidth/2+offsetDist);
        break;
    case SYMBOL_TOPLEFT:
        labelPlaceVec.x() = -1*(textWidth/2+offsetDist*0.707);
        labelPlaceVec.y() = textHeight/2+offsetDist*0.707;
        break;
    default:
        break;
    }
    geomText->setPosition(labelPlaceVec);

    // color uniform
    osg::Vec4 fontColor = colorAsVec4(labelStyle->GetFontColor());
    osg::ref_ptr<osg::Uniform> uTextColor = new osg::Uniform("Color",fontColor);

    // geode: text
    osg::ref_ptr<osg::Geode> geodeText = new osg::Geode;
    ss = geodeText->getOrCreateStateSet();
    ss->addUniform(uTextColor);
    ss->setAttributeAndModes(m_shaderText);
    geodeText->addDrawable(geomText.get());

    // calculate the position vector of the
    // node center taking offsetHeight into account
    osg::Vec3d surfaceVec(nodeData.nodePosn.x,
                          nodeData.nodePosn.y,
                          nodeData.nodePosn.z);

    double symOffsetHeight = nodeData.symbolRenderStyle->GetOffsetHeight();
    osg::Vec3d normVec = surfaceVec; normVec.normalize();
    osg::Vec3d shiftVec = surfaceVec+(normVec*symOffsetHeight)-offsetVec;

    // transform: billboard
    osg::ref_ptr<osg::AutoTransform> xfText = new osg::AutoTransform;
    xfText->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
    xfText->setPosition(shiftVec);
    xfText->addChild(geodeText);

    // if this is a plate label, draw a plate behind the text
    if(labelStyle->GetLabelType() == LABEL_PLATE)
    {
        // geometry: plate
        osg::ref_ptr<osg::Geometry> geomPlate = new osg::Geometry;

        geomPlate = dynamic_cast<osg::Geometry*>
                (m_symbolSquare->clone(osg::CopyOp::DEEP_COPY_ALL));

        osg::ref_ptr<osg::Vec3Array> listPlateVerts =
                dynamic_cast<osg::Vec3Array*>(geomPlate->getVertexArray());

        double widthOff = textWidth/2 + labelStyle->GetPlatePadding();
        double heightOff = textHeight/2 + labelStyle->GetPlatePadding();

        listPlateVerts->at(0) = labelPlaceVec;
        listPlateVerts->at(1) = labelPlaceVec + osg::Vec3(-1*widthOff,-1*heightOff,-0.5);
        listPlateVerts->at(2) = labelPlaceVec + osg::Vec3(widthOff,-1*heightOff,-0.5);
        listPlateVerts->at(3) = labelPlaceVec + osg::Vec3(widthOff,heightOff,-0.5);
        listPlateVerts->at(4) = labelPlaceVec + osg::Vec3(-1*widthOff,heightOff,-0.5);

        // color uniform
        osg::Vec4 plateColor = colorAsVec4(labelStyle->GetPlateColor());
        osg::ref_ptr<osg::Uniform> uPlateColor = new osg::Uniform("Color",plateColor);

        // geode: plate
        osg::ref_ptr<osg::Geode> geodePlate = new osg::Geode;
        ss = geodePlate->getOrCreateStateSet();
        ss->addUniform(uPlateColor);
        ss->setAttributeAndModes(m_shaderDirect);
        geodePlate->addDrawable(geomPlate.get());

        xfText->addChild(geodePlate);

        // create plate border if required
        double olWidth = labelStyle->GetPlateOutlineWidth();
        if(olWidth > 0)
        {
            // geometry: plate outline
            osg::ref_ptr<osg::Geometry> geomPlateOutline = new osg::Geometry;
            geomPlateOutline = dynamic_cast<osg::Geometry*>
                    (m_symbolSquareOutline->clone(osg::CopyOp::DEEP_COPY_ALL));

            osg::ref_ptr<osg::Vec3Array> listPlateOLVerts =
                    dynamic_cast<osg::Vec3Array*>(geomPlateOutline->getVertexArray());

            // reposition inner vertices
            listPlateOLVerts->at(0) = listPlateVerts->at(1);
            listPlateOLVerts->at(2) = listPlateVerts->at(2);
            listPlateOLVerts->at(4) = listPlateVerts->at(3);
            listPlateOLVerts->at(6) = listPlateVerts->at(4);
            listPlateOLVerts->at(8) = listPlateVerts->at(1);

            // reposition outer vertices
            listPlateOLVerts->at(1) = listPlateVerts->at(1)+osg::Vec3(-1*olWidth,-1*olWidth,-0.5);
            listPlateOLVerts->at(3) = listPlateVerts->at(2)+osg::Vec3(olWidth,-1*olWidth,-0.5);
            listPlateOLVerts->at(5) = listPlateVerts->at(3)+osg::Vec3(olWidth,olWidth,-0.5);
            listPlateOLVerts->at(7) = listPlateVerts->at(4)+osg::Vec3(-1*olWidth,olWidth,-0.5);
            listPlateOLVerts->at(9) = listPlateOLVerts->at(1);

            // color uniform
            osg::Vec4 plateOutlineColor =
                    colorAsVec4(labelStyle->GetPlateOutlineColor());

            osg::ref_ptr<osg::Uniform> uPlateColorOutline =
                    new osg::Uniform("Color",plateOutlineColor);

            // geode: plate outline
            osg::ref_ptr<osg::Geode> geodePlateOutline = new osg::Geode;
            ss = geodePlateOutline->getOrCreateStateSet();
            ss->addUniform(uPlateColorOutline);
            ss->setAttributeAndModes(m_shaderDirect);
            geodePlateOutline->addDrawable(geomPlateOutline);

            xfText->addChild(geodePlateOutline);
        }
    }

    // set render bin
    ss = xfText->getOrCreateStateSet();
    ss->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin",
                            osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

    // add label to scene
    nodeParent->addChild(xfText.get());
}

void MapRendererOSG::addAreaLabel(const AreaRenderData &areaData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{
    std::string labelText; double offsetDist;
    LabelStyle const *labelStyle;

    if(usingName)   {
        labelText = areaData.nameLabel;
        labelStyle = areaData.nameLabelRenderStyle;
        offsetDist = areaData.nameLabelRenderStyle->GetOffsetDist();
    }
    else
    {   OSRDEBUG << "WARN: Ref Labels not supported yet!";   return;   }

    osg::StateSet * ss;

    // calculate the position vector of the label
    // center taking offsetDist into account
    osg::Vec3d surfCenterVec(areaData.centerPoint.x,
                             areaData.centerPoint.y,
                             areaData.centerPoint.z);

    osg::Vec3d heightVec = surfCenterVec;
    heightVec.normalize();

    // adjust offsetDist if area is building
    if(areaData.isBuilding)   {
        offsetDist += areaData.buildingHeight;
    }

    // geometry: text
    osg::ref_ptr<osgText::Text> geomText = new osgText::Text;
    geomText->setFont(m_pathFonts + labelStyle->GetFontFamily());
    geomText->setAlignment(osgText::Text::CENTER_CENTER);
    geomText->setCharacterSize(labelStyle->GetFontSize());
    geomText->setText(labelText);

    // color uniform
    osg::Vec4 fontColor =
            colorAsVec4(labelStyle->GetFontColor());

    osg::ref_ptr<osg::Uniform> uFontColor =
            new osg::Uniform("Color",fontColor);

    // geode: text
    osg::ref_ptr<osg::Geode> geodeText = new osg::Geode;
    geodeText->addDrawable(geomText.get());

    ss = geodeText->getOrCreateStateSet();
    ss->addUniform(uFontColor);
    ss->setAttributeAndModes(m_shaderText);

    // use the max bounding box dist along x,y or z
    // as a rough metric for setting max label width
    double xMin = areaData.listOuterPoints[0].x; double xMax = xMin;
    double yMin = areaData.listOuterPoints[0].y; double yMax = yMin;
    double zMin = areaData.listOuterPoints[0].z; double zMax = zMin;
    for(size_t i=1; i < areaData.listOuterPoints.size(); i++)   {
        Vec3 const &areaPoint = areaData.listOuterPoints[i];
        xMin = std::min(xMin,areaPoint.x);
        xMax = std::max(xMax,areaPoint.x);
        yMin = std::min(yMin,areaPoint.y);
        yMax = std::max(yMax,areaPoint.y);
        zMin = std::min(zMin,areaPoint.z);
        zMax = std::max(zMax,areaPoint.z);
    }

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
        double fracLength = (geomText->getBound().xMax()-
                geomText->getBound().xMin()) / maxLabelWidth;

        if(fracLength <= 1)
        {   break;   }

        if(breakChar == -1)
        {   breakChar = ((1/fracLength)*labelText.size())-1;   }

        // find all instances of (" ") in label
        std::vector<int> listPosSP;
        size_t pos = labelText.find(" ",0);     // warning! must use size_t when comparing
        while(pos != std::string::npos) {       // with std::string::npos, NOT int/unsigned int
            listPosSP.push_back(pos);
            pos = labelText.find(" ",pos+1);
        }

        if(listPosSP.size() == 0)
        {   break;   }

        // insert a newline at the (" ") closest to breakChar
        unsigned int cPos = 0;
        for(size_t i=0; i < listPosSP.size(); i++)  {
            if(abs(breakChar-listPosSP[i]) < abs(breakChar-listPosSP[cPos]))
            {   cPos = i;   }
        }

        labelText.replace(listPosSP[cPos],1,"\n");
        geomText->setText(labelText);
    }

    // note: yMin and yMax don't have the correct
    // positioning (bug) but they have the right relative
    // distance, so only yHeight is a valid metric in y
    xMin = geomText->computeBound().xMin();// - platePadding;
    xMax = geomText->computeBound().xMax();// + platePadding;
    yMin = geomText->computeBound().yMin();// - platePadding;
    yMax = geomText->computeBound().yMax();// + platePadding;
    double yHeight = yMax-yMin;

    // calculate final position vectors
    offsetDist += yHeight/2;
    osg::Vec3 shiftVec(surfCenterVec + heightVec*offsetDist - offsetVec);

    // autotransform (billboard)
    osg::ref_ptr<osg::AutoTransform> xfLabel = new osg::AutoTransform;
    xfLabel->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_CAMERA);
    xfLabel->setPosition(shiftVec);
    xfLabel->addChild(geodeText);


    // if this is a plate label, draw a plate behind the text
    if(labelStyle->GetLabelType() == LABEL_PLATE)
    {
        double widthOff = (xMax-xMin)/2 + labelStyle->GetPlatePadding();
        double heightOff = yHeight/2 + labelStyle->GetPlatePadding();

        // geometry: plate
        osg::ref_ptr<osg::Geometry> geomPlate = new osg::Geometry;

        geomPlate = dynamic_cast<osg::Geometry*>
                (m_symbolSquare->clone(osg::CopyOp::DEEP_COPY_ALL));

        osg::ref_ptr<osg::Vec3Array> listPlateVerts =
                dynamic_cast<osg::Vec3Array*>(geomPlate->getVertexArray());

        listPlateVerts->at(1) = osg::Vec3(-1*widthOff,-1*heightOff,-0.5);
        listPlateVerts->at(2) = osg::Vec3(widthOff,-1*heightOff,-0.5);
        listPlateVerts->at(3) = osg::Vec3(widthOff,heightOff,-0.5);
        listPlateVerts->at(4) = osg::Vec3(-1*widthOff,heightOff,-0.5);

        // color uniform
        osg::Vec4 plateColor = colorAsVec4(labelStyle->GetPlateColor());
        osg::ref_ptr<osg::Uniform> uPlateColor = new osg::Uniform("Color",plateColor);

        // geode: plate
        osg::ref_ptr<osg::Geode> geodePlate = new osg::Geode;
        ss = geodePlate->getOrCreateStateSet();
        ss->addUniform(uPlateColor);
        ss->setAttributeAndModes(m_shaderDirect);
        geodePlate->addDrawable(geomPlate.get());

        xfLabel->addChild(geodePlate.get());

        // border if specified
        double olWidth = labelStyle->GetPlateOutlineWidth();
        if(olWidth > 0)
         {
             // geometry: plate outline
             osg::ref_ptr<osg::Geometry> geomPlateOutline = new osg::Geometry;
             geomPlateOutline = dynamic_cast<osg::Geometry*>
                     (m_symbolSquareOutline->clone(osg::CopyOp::DEEP_COPY_ALL));

             osg::ref_ptr<osg::Vec3Array> listPlateOLVerts =
                     dynamic_cast<osg::Vec3Array*>(geomPlateOutline->getVertexArray());

             // reposition inner vertices
             listPlateOLVerts->at(0) = listPlateVerts->at(1);
             listPlateOLVerts->at(2) = listPlateVerts->at(2);
             listPlateOLVerts->at(4) = listPlateVerts->at(3);
             listPlateOLVerts->at(6) = listPlateVerts->at(4);
             listPlateOLVerts->at(8) = listPlateVerts->at(1);

             // reposition outer vertices
             listPlateOLVerts->at(1) = listPlateVerts->at(1)+osg::Vec3(-1*olWidth,-1*olWidth,-0.5);
             listPlateOLVerts->at(3) = listPlateVerts->at(2)+osg::Vec3(olWidth,-1*olWidth,-0.5);
             listPlateOLVerts->at(5) = listPlateVerts->at(3)+osg::Vec3(olWidth,olWidth,-0.5);
             listPlateOLVerts->at(7) = listPlateVerts->at(4)+osg::Vec3(-1*olWidth,olWidth,-0.5);
             listPlateOLVerts->at(9) = listPlateOLVerts->at(1);

             // color uniform
             osg::Vec4 plateOutlineColor =
                     colorAsVec4(labelStyle->GetPlateOutlineColor());

             osg::ref_ptr<osg::Uniform> uPlateColorOutline =
                     new osg::Uniform("Color",plateOutlineColor);

             // geode: plate outline
             osg::ref_ptr<osg::Geode> geodePlateOutline = new osg::Geode;
             ss = geodePlateOutline->getOrCreateStateSet();
             ss->addUniform(uPlateColorOutline);
             ss->setAttributeAndModes(m_shaderDirect);
             geodePlateOutline->addDrawable(geomPlateOutline);

             xfLabel->addChild(geodePlateOutline);
         }
    }

    // set render bin
    ss = xfLabel->getOrCreateStateSet();
    ss->setRenderBinDetails(m_depthSortedBin,"DepthSortedBin",
                            osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);

    // add label to scene
    nodeParent->addChild(xfLabel);
}

void MapRendererOSG::addContourLabel(const WayRenderData &wayData,
                                     const osg::Vec3d &offsetVec,
                                     osg::MatrixTransform *nodeParent,
                                     bool usingName)
{
    // set predefined vars up based on name or ref
    std::string const *labelText;
    LabelStyle const *labelStyle;
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
    for(size_t i=0; i < numChars; i++)
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

        // get font character instance
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
    for(size_t i=0; i < listWayPoints->size(); i++)
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

    for(size_t i=1; i < wayData.listSharedNodes.size()-1; i++)
    {
        if(wayData.listSharedNodes[i])
        {   listSharedNodeLengths.push_back(listSegLengths[i]);   }
    }
    listSharedNodeLengths.push_back(listSegLengths.back());

    // fit as many labels as possible in the length
    // available on the way between intersection nodes
    osg::ref_ptr<osg::Group> wayLabel = new osg::Group;
    for(size_t i=1; i < listSharedNodeLengths.size(); i++)
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

            for(size_t j=0; j < numLabelsFit; j++)
            {
                // define the start and end lengths of the label
                double startLength = labelOffset +
                        (j+1)*tweenSpace + j*labelLength;

                double charWidth = 0;
                double prevCharWidth = 0;
                double lengthAlongPath = startLength;

                osg::Matrixf xformMatrix;
                osg::Vec3d pointAtLength,dirnAtLength,
                           normAtLength,sideAtLength;

                // apply transform to align chars to way
                for(size_t k=0; k < listChars.size(); k++)
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

    unsigned int wayLabelLayer;
    if(wayData.wayRef->IsBridge())
    {   wayLabelLayer = getBridgeRenderBin()+3;   }
    else if(wayData.wayRef->IsTunnel())
    {   wayLabelLayer = getTunnelRenderBin()+3;   }
    else
    {   wayLabelLayer = getWayLabelRenderBin(wayData.wayLayer);   }

    // fontcolor uniform
    osg::Vec4 fontColor = colorAsVec4(labelStyle->GetFontColor());
    osg::ref_ptr<osg::Uniform> uFontColor = new osg::Uniform("Color",fontColor);

    osg::StateSet * ss = wayLabel->getOrCreateStateSet();
    ss->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    ss->setRenderBinDetails(wayLabelLayer,"RenderBin");
    ss->addUniform(uFontColor);
    ss->setAttributeAndModes(m_shaderText);
    nodeParent->addChild(wayLabel.get());
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::addDsAreaGeometries()
{
    // if no depth sorted areas were added or
    // removed, this function should return
    if((!m_modDsAreas) && (!m_modDsRelAreas))
    {   return;   }

    Camera const * myCamera = this->GetCamera();
    osg::Vec3d offsetVec(myCamera->eye.x,
                         myCamera->eye.y,
                         myCamera->eye.z);

    TYPE_UNORDERED_MAP<Id,VxAttributes>::iterator bIt;
    std::map<double,AreaDsElement>::iterator mIt;

    // remove all geometry from merged geode
    m_geodeDsAreas->removeDrawables(0,
        m_geodeDsAreas->getNumDrawables());

    // create merged geometry with combined vertex attributes
    osg::ref_ptr<osg::Vec3Array> mListVx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> mListNx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> mListCx = new osg::Vec4Array;
    mListVx->reserve(m_countVxDsAreas);
    mListNx->reserve(m_countVxDsAreas);
    mListCx->reserve(m_countVxDsAreas);

    // depth sort using center point of individual areas
    std::map<double,AreaDsElement> mapObjectViewDist;

    // [1 - normal areas]
    for(bIt = m_mapDsAreaGeo.begin();
        bIt != m_mapDsAreaGeo.end(); ++bIt)
    {
        // get the negative distance2 to sort objects
        // highest to lowest distance from camera
        osg::Vec3d const &centerPt = bIt->second.centerPt;
        Vec3 objCenter(centerPt.x(),centerPt.y(),centerPt.z());
        double distToObj = objCenter.Distance2To(myCamera->eye)*-1;

        AreaDsElement areaInfo(bIt->first,bIt->second.listVx->size(),false);
        std::pair<double,AreaDsElement> insData(distToObj,areaInfo);
        mapObjectViewDist.insert(insData);
    }

    // [2 - relation areas]
    for(bIt = m_mapDsRelAreaGeo.begin();
        bIt != m_mapDsRelAreaGeo.end(); ++bIt)
    {
        // get the negative distance2 to sort objects
        // highest to lowest distance from camera
        osg::Vec3d const &centerPt = bIt->second.centerPt;
        Vec3 objCenter(centerPt.x(),centerPt.y(),centerPt.z());
        double distToObj = objCenter.Distance2To(myCamera->eye)*-1;

        AreaDsElement areaInfo(bIt->first,bIt->second.listVx->size(),true);
        std::pair<double,AreaDsElement> insData(distToObj,areaInfo);
        mapObjectViewDist.insert(insData);
    }

    // note: mapObjectViewDist now contains a list of both
    // normal and relation areas sorted by depth, with the
    // AreaDsElement::isRelArea flag indicating type

    // we cull area geometries that are furthest
    // from the camera according to m_limitVxDsAreas
    size_t vCount = 0; bool cullList = false;
    // only run this check if we have many buildings
    if(mapObjectViewDist.size() > 20)   {
        for(mIt = mapObjectViewDist.end();
            mIt != mapObjectViewDist.begin();)
        {
            --mIt;
            vCount += mIt->second.vxCount;
            if(vCount > m_limitVxDsAreas)   {
                cullList = true;
                OSRDEBUG << "Depth Sorted Areas limit reached!";
                break;
            }
        }
        if(cullList)   {
            mapObjectViewDist.erase(mapObjectViewDist.begin(),mIt);
        }
    }

    // add sorted objects to the vertex buffer
    for(mIt = mapObjectViewDist.begin();
        mIt != mapObjectViewDist.end(); ++mIt)
    {
        if(mIt->second.isRelArea)
        {   bIt = m_mapDsRelAreaGeo.find(mIt->second.uid);   }
        else
        {   bIt = m_mapDsAreaGeo.find(mIt->second.uid);   }

        VxAttributes const &vxAttr = bIt->second;
        mListVx->insert(mListVx->end(),vxAttr.listVx->begin(),vxAttr.listVx->end());
        mListNx->insert(mListNx->end(),vxAttr.listNx->begin(),vxAttr.listNx->end());
        mListCx->insert(mListCx->end(),vxAttr.listCx->begin(),vxAttr.listCx->end());
    }

    // apply an offset using current camera's eye
    // coordinates to address precision issues
    for(size_t i=0; i < mListVx->size(); i++)   {
        mListVx->at(i) -= offsetVec;
    }

    osg::ref_ptr<osg::Geometry> geomBuildings = new osg::Geometry;
    geomBuildings->setVertexArray(mListVx);
    geomBuildings->setNormalArray(mListNx);
    geomBuildings->setColorArray(mListCx);
    geomBuildings->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geomBuildings->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geomBuildings->setUseVertexBufferObjects(true);
    geomBuildings->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES,0,mListVx->size()));

    // add to scene
    m_xfDsAreas->setMatrix(osg::Matrixd::translate(offsetVec));
    m_geodeDsAreas->addDrawable(geomBuildings);
}

void MapRendererOSG::addLyAreaGeometries()
{
    // if no layered areas were added or
    // removed, this function should return
    if((!m_modLyAreas) && (!m_modLyRelAreas))
    {   return;   }

    // remove all geometry from merged geode
    m_geodeLyAreas->removeDrawables(0,
        m_geodeLyAreas->getNumDrawables());

    Camera const * myCamera = this->GetCamera();
    osg::Vec3d offsetVec(myCamera->eye.x,
                         myCamera->eye.y,
                         myCamera->eye.z);

    TYPE_UNORDERED_MAP<Id,VxAttributes>::iterator bIt;
    std::multimap<size_t,AreaDsElement>::iterator mIt;

    // create merged geometry with combined vertex attributes
    osg::ref_ptr<osg::Vec3Array> mListVx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> mListNx = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> mListCx = new osg::Vec4Array;
    mListVx->reserve(m_countVxLyAreas);
    mListNx->reserve(m_countVxLyAreas);
    mListCx->reserve(m_countVxLyAreas);

    // sort using layer of individual areas
    std::multimap<size_t,AreaDsElement> mapObjectByLayer;
    size_t vCount = 0;

    // [1 - normal areas]
    for(bIt = m_mapLyAreaGeo.begin();
        bIt != m_mapLyAreaGeo.end(); ++bIt)
    {
        vCount += bIt->second.listVx->size();
        if(vCount > m_limitVxLyAreas)   {
            OSRDEBUG << "Layered Areas limit reached!";
            break;
        }

        AreaDsElement areaInfo(bIt->first,bIt->second.listVx->size(),false);
        std::pair<size_t,AreaDsElement> insData(bIt->second.layer,areaInfo);
        mapObjectByLayer.insert(insData);
    }

    // add sorted objects to the vertex buffer
    for(mIt = mapObjectByLayer.begin();
        mIt != mapObjectByLayer.end(); ++mIt)
    {
        if(mIt->second.isRelArea)
        {   bIt = m_mapLyRelAreaGeo.find(mIt->second.uid);   }
        else
        {   bIt = m_mapLyAreaGeo.find(mIt->second.uid);   }

        VxAttributes const &vxAttr = bIt->second;
        mListVx->insert(mListVx->end(),vxAttr.listVx->begin(),vxAttr.listVx->end());
        mListNx->insert(mListNx->end(),vxAttr.listNx->begin(),vxAttr.listNx->end());
        mListCx->insert(mListCx->end(),vxAttr.listCx->begin(),vxAttr.listCx->end());
    }

    // apply an offset using current camera's eye
    // coordinates to address precision issues
    for(size_t i=0; i < mListVx->size(); i++)   {
        mListVx->at(i) -= offsetVec;
    }

    osg::ref_ptr<osg::Geometry> geomAreas = new osg::Geometry;
    geomAreas->setVertexArray(mListVx);
    geomAreas->setNormalArray(mListNx);
    geomAreas->setColorArray(mListCx);
    geomAreas->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geomAreas->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geomAreas->setUseVertexBufferObjects(true);
    geomAreas->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES,0,mListVx->size()));

    // add to scene
    m_xfLyAreas->setMatrix(osg::Matrixd::translate(offsetVec));
    m_geodeLyAreas->addDrawable(geomAreas);
}

Id MapRendererOSG::getNewAreaId()
{
    m_lk_areaId++;

    if(m_lk_areaId > 32000)   {   // arbitrary upper limit
        m_lk_areaId = 0;
    }

    return m_lk_areaId;
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::buildGeomTriangle()
{
    m_symbolTriangle = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> listVerts = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listNorms = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUByte> listIdxs =
            new osg::DrawElementsUByte(GL_TRIANGLE_FAN);

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
    osg::ref_ptr<osg::Vec3Array> listVerts = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listNorms = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUByte> listIdxs =
            new osg::DrawElementsUByte(GL_TRIANGLE_FAN);

    unsigned int numEdges = 4;
    listVerts->push_back(osg::Vec3(0,0,0));
    listNorms->push_back(osg::Vec3(0,0,1));

    for(size_t i=0; i <= numEdges; i++)
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
    osg::ref_ptr<osg::Vec3Array> listVerts = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> listNorms = new osg::Vec3Array;
    osg::ref_ptr<osg::DrawElementsUByte> listIdxs =
            new osg::DrawElementsUByte(GL_TRIANGLE_FAN);

    unsigned int numEdges = 12;
    listVerts->push_back(osg::Vec3(0,0,0));
    listNorms->push_back(osg::Vec3(0,0,1));

    for(size_t i=0; i <= numEdges; i++)
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

void MapRendererOSG::buildGeomTriangleOutline()
{
    m_symbolTriangleOutline = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> listVerts = new osg::Vec3Array;

    unsigned int numEdges = 3;

    // vertices
    // notes: - we stagger the outline vertices
    //        - the vertices wrap around
    for(size_t i=0; i <= numEdges; i++)   {
        double cAngle = i*(2*K_PI)/numEdges + K_PI/2;
        listVerts->push_back(osg::Vec3(cos(cAngle),sin(cAngle),0));
        listVerts->push_back(osg::Vec3(cos(cAngle),sin(cAngle),0));
    }

    // stitch outline faces together
    osg::ref_ptr<osg::DrawElementsUByte> listIdxs = new
            osg::DrawElementsUByte(GL_TRIANGLES,numEdges*6);
    unsigned int k=0;

    // the original symbol vertex array wraps around
    // (first vertex == last vertex) so we don't have
    // to 'close off' the triangles in the outline
    for(size_t i=0; i < numEdges; i++)   {
        int idx = i*2;
        // triangle 1
        listIdxs->at(k) = idx+0;     k++;
        listIdxs->at(k) = idx+1;     k++;
        listIdxs->at(k) = idx+3;     k++;
        // triangle 2
        listIdxs->at(k) = idx+0;     k++;
        listIdxs->at(k) = idx+3;     k++;
        listIdxs->at(k) = idx+2;     k++;
    }

    // set default normal
    osg::ref_ptr<osg::Vec3Array> listNorms = new osg::Vec3Array;
    listNorms->push_back(osg::Vec3(0,0,1));

    m_symbolTriangleOutline->setVertexArray(listVerts);
    m_symbolTriangleOutline->setNormalArray(listNorms);
    m_symbolTriangleOutline->setNormalBinding(osg::Geometry::BIND_OVERALL);
    m_symbolTriangleOutline->addPrimitiveSet(listIdxs);
}

void MapRendererOSG::buildGeomSquareOutline()
{
    m_symbolSquareOutline = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> listVerts = new osg::Vec3Array;

    unsigned int numEdges = 4;

    // vertices
    // notes: - we stagger the outline vertices
    //        - the vertices wrap around
    for(size_t i=0; i <= numEdges; i++)   {
        double cAngle = i*(2*K_PI)/numEdges + K_PI/4;
        listVerts->push_back(osg::Vec3(cos(cAngle),sin(cAngle),0));
        listVerts->push_back(osg::Vec3(cos(cAngle),sin(cAngle),0));
    }

    // stitch outline faces together
    osg::ref_ptr<osg::DrawElementsUByte> listIdxs = new
            osg::DrawElementsUByte(GL_TRIANGLES,numEdges*6);
    unsigned int k=0;

    // the original symbol vertex array wraps around
    // (first vertex == last vertex) so we don't have
    // to 'close off' the triangles in the outline
    for(size_t i=0; i < numEdges; i++)   {
        int idx = i*2;
        // triangle 1
        listIdxs->at(k) = idx+0;     k++;
        listIdxs->at(k) = idx+1;     k++;
        listIdxs->at(k) = idx+3;     k++;
        // triangle 2
        listIdxs->at(k) = idx+0;     k++;
        listIdxs->at(k) = idx+3;     k++;
        listIdxs->at(k) = idx+2;     k++;
    }

    // set default normal
    osg::ref_ptr<osg::Vec3Array> listNorms = new osg::Vec3Array;
    listNorms->push_back(osg::Vec3(0,0,1));

    m_symbolSquareOutline->setVertexArray(listVerts);
    m_symbolSquareOutline->setNormalArray(listNorms);
    m_symbolSquareOutline->setNormalBinding(osg::Geometry::BIND_OVERALL);
    m_symbolSquareOutline->addPrimitiveSet(listIdxs);
}

void MapRendererOSG::buildGeomCircleOutline()
{
    m_symbolCircleOutline = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> listVerts = new osg::Vec3Array;

    unsigned int numEdges = 12;

    // vertices
    // notes: - we stagger the outline vertices
    //        - the vertices wrap around
    for(size_t i=0; i <= numEdges; i++)   {
        double cAngle = i*(2*K_PI)/numEdges;
        listVerts->push_back(osg::Vec3(cos(cAngle)*0.707,sin(cAngle)*0.707,0));
        listVerts->push_back(osg::Vec3(cos(cAngle)*0.707,sin(cAngle)*0.707,0));
    }

    // stitch outline faces together
    osg::ref_ptr<osg::DrawElementsUByte> listIdxs = new
            osg::DrawElementsUByte(GL_TRIANGLES,numEdges*6);
    unsigned int k=0;

    // the original symbol vertex array wraps around
    // (first vertex == last vertex) so we don't have
    // to 'close off' the triangles in the outline
    for(size_t i=0; i < numEdges; i++)   {
        int idx = i*2;
        // triangle 1
        listIdxs->at(k) = idx+0;     k++;
        listIdxs->at(k) = idx+1;     k++;
        listIdxs->at(k) = idx+3;     k++;
        // triangle 2
        listIdxs->at(k) = idx+0;     k++;
        listIdxs->at(k) = idx+3;     k++;
        listIdxs->at(k) = idx+2;     k++;
    }

    // set default normal
    osg::ref_ptr<osg::Vec3Array> listNorms = new osg::Vec3Array;
    listNorms->push_back(osg::Vec3(0,0,1));

    m_symbolCircleOutline->setVertexArray(listVerts);
    m_symbolCircleOutline->setNormalArray(listNorms);
    m_symbolCircleOutline->setNormalBinding(osg::Geometry::BIND_OVERALL);
    m_symbolCircleOutline->addPrimitiveSet(listIdxs);
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::tessBeginCallback(GLenum type) {}

void MapRendererOSG::tessVertexCallback(void *inVx,
                                        void *listVx)
{
    // cast the incoming vertex
    GLdouble const * vxPtr;
    vxPtr = (GLdouble *)inVx;

    // save the vertex to our list
    std::vector<Vec3> * myListVx = (std::vector<Vec3>*)listVx;
    myListVx->push_back(Vec3(vxPtr[0],vxPtr[1],vxPtr[2]));
}

void MapRendererOSG::tessCombineCallback(GLdouble newVx[3],
                                         void *neighbourVx[4],
                                         GLfloat neighbourWeight[4],
                                         void **outVx,
                                         void *listVx)
{
    // copy new vertex resulting from intersection
    // we need to manually assign memory to store it
    GLdouble * vxPtr = (GLdouble *) malloc(3*sizeof(GLdouble));
    vxPtr[0] = newVx[0];
    vxPtr[1] = newVx[1];
    vxPtr[2] = newVx[2];

    // this memory can only be cleaned up after
    // gluTessEndPolygon() is called so save a ref
    m_tListNewVx.push_back(vxPtr);

    // glu will pass this back to tessVertexCallback
    *outVx = vxPtr;
}

void MapRendererOSG::tessEdgeCallback() {}

void MapRendererOSG::tessEndCallback() {}

void MapRendererOSG::tessErrorCallback(GLenum errorCode)
{
    const GLubyte *errString;
    errString = osg::gluErrorString(errorCode);

    std::cout << "Tessellation Error Code: "
             << int(errorCode);
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::triangulateContours(const std::vector<Vec3> &outerContour,
                                         const std::vector<std::vector<Vec3> > &innerContours,
                                         Vec3 const &vecNormal,
                                         std::vector<Vec3> &listTriVx)
{
    // begin polygon and pass listTriVx as user data
    osg::gluTessBeginPolygon(m_tobj,&listTriVx);
    osg::gluTessNormal(m_tobj,vecNormal.x,vecNormal.y,vecNormal.z);

    // note: need to keep the GLdouble arrays alive
    // for as long as we have gluTessEndPolygon

    // total number of inner contour vertices
    size_t innerVxCount = 0;
    for(size_t i=0; i < innerContours.size(); i++)   {
        innerVxCount += innerContours[i].size();
    }

    // apparently arrays of size 0 are valid in C?
    GLdouble outerVx[outerContour.size()][3];
    GLdouble innerVx[innerVxCount][3];
    size_t kIdx = 0;

    // outer contour
    osg::gluTessBeginContour(m_tobj);
    for(size_t i=0; i < outerContour.size(); i++)
    {
        Vec3 const &myVx = outerContour[i];
        outerVx[i][0] = myVx.x;
        outerVx[i][1] = myVx.y;
        outerVx[i][2] = myVx.z;
        osg::gluTessVertex(m_tobj,outerVx[i],outerVx[i]);
    }
    osg::gluTessEndContour(m_tobj);

    // inner contours
    for(size_t i=0; i < innerContours.size(); i++)
    {
        std::vector<Vec3> const &innerContour = innerContours[i];

        osg::gluTessBeginContour(m_tobj);
        for(size_t j=0; j < innerContour.size(); j++)
        {
            Vec3 const &myVx = innerContour[j];
            innerVx[kIdx][0] = myVx.x;
            innerVx[kIdx][1] = myVx.y;
            innerVx[kIdx][2] = myVx.z;
            osg::gluTessVertex(m_tobj,innerVx[kIdx],innerVx[kIdx]);

            kIdx++;
        }
        osg::gluTessEndContour(m_tobj);
    }

    // end
    osg::gluTessEndPolygon(m_tobj);

    // cleanup memory
    for(size_t i=0; i < m_tListNewVx.size(); i++)   {
        free(m_tListNewVx[i]);
    }
    m_tListNewVx.clear();
}

// ========================================================================== //
// ========================================================================== //

void MapRendererOSG::setupShaders()
{
    // create shader programs
    std::string vShader,fShader;

    OSRDEBUG << "Loading Shaders in " << m_pathShaders;

    m_shaderDirect = new osg::Program;
    m_shaderDirect->setName("ShaderDirect");
    vShader = this->readFileAsString(m_pathShaders + "Direct_unif_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "Direct_unif_frag.glsl");
    m_shaderDirect->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderDirect->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));

    m_shaderDiffuse = new osg::Program;
    m_shaderDiffuse->setName("ShaderDiffuse");
    vShader = this->readFileAsString(m_pathShaders + "Diffuse_unif_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "Diffuse_unif_frag.glsl");
    m_shaderDiffuse->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderDiffuse->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));

    m_shaderText = new osg::Program;
    m_shaderText->setName("ShaderText");
    vShader = this->readFileAsString(m_pathShaders + "Text_unif_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "Text_unif_frag.glsl");
    m_shaderText->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderText->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));

    m_shaderDiffuseAttr = new osg::Program;
    m_shaderDiffuseAttr->setName("ShaderDiffuseAttr");
    vShader = this->readFileAsString(m_pathShaders + "Diffuse_attr_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "Diffuse_attr_frag.glsl");
    m_shaderDiffuseAttr->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderDiffuseAttr->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));

    m_shaderDirectAttr = new osg::Program;
    m_shaderDirectAttr->setName("ShaderDirectAttr");
    vShader = this->readFileAsString(m_pathShaders + "Direct_attr_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "Direct_attr_frag.glsl");
    m_shaderDirectAttr->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderDirectAttr->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));

    m_shaderEarthCoastlinePCL = new osg::Program;
    m_shaderEarthCoastlinePCL->setName("EarthCoastlinePCL");
    vShader = this->readFileAsString(m_pathShaders + "EarthCoastlinePCL_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "EarthCoastlinePCL_frag.glsl");
    m_shaderEarthCoastlinePCL->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderEarthCoastlinePCL->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));

    m_shaderEarthCoastlineLines = new osg::Program;
    m_shaderEarthCoastlineLines->setName("EarthCoastlineLines");
    vShader = this->readFileAsString(m_pathShaders + "EarthCoastlineLines_vert.glsl");
    fShader = this->readFileAsString(m_pathShaders + "EarthCoastlineLines_frag.glsl");
    m_shaderEarthCoastlineLines->addShader(new osg::Shader(osg::Shader::VERTEX,vShader));
    m_shaderEarthCoastlineLines->addShader(new osg::Shader(osg::Shader::FRAGMENT,fShader));
}

double MapRendererOSG::calcWayLength(const osg::Vec3dArray *listWayPoints)
{
    double totalDist = 0;
    for(size_t i=1; i < listWayPoints->size(); i++)
    {   totalDist += (listWayPoints->at(i)-listWayPoints->at(i-1)).length();   }

    return totalDist;
}

void MapRendererOSG::calcWaySegmentLengths(const osg::Vec3dArray *listWayPoints,
                                           std::vector<double> &listSegLengths)
{
    double totalDist = 0;
    listSegLengths.resize(listWayPoints->size(),0);
    for(size_t i=1; i < listWayPoints->size(); i++)
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
