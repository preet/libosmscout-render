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

#include <sys/time.h>
#include <string>
#include <sstream>

#include <osg/ref_ptr>
#include <osg/Vec3d>
#include <osg/Geode>
#include <osgText/Text>
#include <osg/Geometry>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osgUtil/Tessellator>
#include <osgUtil/MeshOptimizers>
#include <osg/AutoTransform>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>

#include "MapRenderer.h"
//#include <libosmscout-render/MapRenderer.h>

// todo
// should we ever be using vec3d as opposed to vec3f??

// todo
// maybe rename "addNodeGeometry" type functions
// to "createNode","createNodeLabel", etc?

namespace osmscout
{
typedef TYPE_UNORDERED_MAP<std::string,osg::ref_ptr<osgText::Text> > CharGeoMap;
typedef TYPE_UNORDERED_MAP<std::string,CharGeoMap> FontGeoMap;

struct VxAttributes
{
    osg::ref_ptr<osg::Vec3Array>  listVx;        // position
    osg::ref_ptr<osg::Vec3Array>  listNx;        // normals
    osg::ref_ptr<osg::Vec4Array>  listCx;        // colors
    osg::Vec3d                    centerPt;
    size_t                        layer;         // layered geom only
};
struct AreaDsElement
{
    AreaDsElement(Id mId,size_t mVxCount,bool mIsRelArea) :
        uid(mId),vxCount(mVxCount),isRelArea(mIsRelArea) {}

    Id      uid;
    size_t  vxCount;
    bool    isRelArea;
};

// osm object type id geometry map
typedef TYPE_UNORDERED_MAP<Id,VxAttributes> IdGeoMap;

class UndefinedBoundsCallback : public osg::Drawable::ComputeBoundingBoxCallback
{
public:
    osg::BoundingBox computeBound(osg::Drawable const &drawable)
    {
        osg::BoundingBox new_uninit_bbox;
        return new_uninit_bbox;
    }
};

class MapRendererOSG : public MapRenderer
{
public:
    MapRendererOSG(Database const *myDatabase,
                   osgViewer::Viewer *myViewer,
                   std::string const &pathShaders,
                   std::string const &pathFonts);

    ~MapRendererOSG();

    void startTiming(std::string const &desc);
    void endTiming();

private:
    void initScene();
    void rebuildStyleData(std::vector<RenderStyleConfig*> const &listRenderStyles);
    inline unsigned int getWayOLRenderBin(unsigned int wayLayer);
    inline unsigned int getWayRenderBin(unsigned int wayLayer);
    inline unsigned int getWayLabelRenderBin(unsigned int wayLabel);
    inline unsigned int getAreaRenderBin(unsigned int areaLayer);
    inline unsigned int getTunnelRenderBin();
    inline unsigned int getBridgeRenderBin();

    void addNodeToScene(NodeRenderData &nodeData);
    void removeNodeFromScene(const NodeRenderData &nodeData);

    void addWayToScene(WayRenderData &wayData);
    void removeAreaFromScene(AreaRenderData const &areaData);

    void addAreaToScene(AreaRenderData &areaData);
    void removeWayFromScene(WayRenderData const &wayData);
    void doneUpdatingAreas();

    void addRelAreaToScene(RelAreaRenderData &relAreaData);
    void removeRelAreaFromScene(const RelAreaRenderData &relAreaData);
    void doneUpdatingRelAreas();

    void removeAllFromScene();

    void showCameraViewArea(osmscout::Camera &sceneCam);

    void addEarthGeometryPointCloud();

    void addNodeGeometry(NodeRenderData const &nodeData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent);

    void addWayGeometry(WayRenderData const &wayData,
                        osg::Vec3d const &offsetVec,
                        osg::MatrixTransform *nodeParent);

    void createAreaGeometry(AreaRenderData const &areaData,
                            VxAttributes &vxAttr);

    void addNodeLabel(NodeRenderData const &nodeData,
                      osg::Vec3d const &offsetVec,
                      osg::MatrixTransform *nodeParent,
                      bool usingName);

    void addAreaLabel(AreaRenderData const &areaData,
                      osg::Vec3d const &offsetVec,
                      osg::MatrixTransform *nodeParent,
                      bool usingName);

    void addContourLabel(WayRenderData const &wayData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent,
                         bool usingName);

    // merge depth sorted area (and rel area) geoms
    void addDsAreaGeometries();

    // merge layered area (and rel area) geoms
    void addLyAreaGeometries();

    void buildGeomTriangle();
    void buildGeomSquare();
    void buildGeomCircle();

    void buildGeomTriangleOutline();
    void buildGeomSquareOutline();
    void buildGeomCircleOutline();

    // tessellator callbacks
    // needs to be static so they can act as callbacks
    static void tessBeginCallback(GLenum type);

    static void tessVertexCallback(void * inVx,
                            void * listVx);

    static void tessCombineCallback(GLdouble newVx[3],
                             void * neighbourVx[4],
                             GLfloat neighbourWeight[4],
                             void **dataOut,
                             void * listVx);

    static void tessEdgeCallback();
    static void tessEndCallback();
    static void tessErrorCallback(GLenum errorCode);

    void triangulateContours(std::vector<Vec3> const &outerContour,                 // const
                             std::vector<std::vector<Vec3> > const &innerContours,
                             Vec3 const &vecNormal,
                             std::vector<Vec3> &listTriVx);


    // helpers
    void setupShaders();

    double calcWayLength(osg::Vec3dArray const *listWayPoints);         // const

    void calcWaySegmentLengths(osg::Vec3dArray const *listWayPoints,    // const
                               std::vector<double> &listSegLengths);

    void calcLerpAlongWay(osg::Vec3dArray const *listWayPoints,     // const
                          osg::Vec3dArray const *listWayNormals,
                          double const lengthAlongWay,
                          osg::Vec3d &pointAtLength,
                          osg::Vec3d &dirnAtLength,
                          osg::Vec3d &normalAtLength,
                          osg::Vec3d &sideAtLength);

    inline osg::Vec4 colorAsVec4(ColorRGBA const &color);       // const
    inline osg::Vec3 convVec3ToOsgVec3(Vec3 const &myVector);   // const
    inline osg::Vec3d convVec3ToOsgVec3d(Vec3 const &myVector); // const

    // timing vars
    timeval m_t1,m_t2;
    std::string m_timingDesc;

    // paths
    std::string m_pathFonts;
    std::string m_pathShaders;

    // opts
    bool m_showPlanetSurface;
    bool m_showPlanetCoastline;
    bool m_showLocalCoastline;

    // scene graph vars
    osgViewer::Viewer * m_viewer;
    osg::ref_ptr<osg::Group> m_nodeRoot;
    osg::ref_ptr<osg::Group> m_nodeNodes;
    osg::ref_ptr<osg::Group> m_nodeWays;
    osg::ref_ptr<osg::Group> m_nodeEarth;
    osg::ref_ptr<osg::Group> m_nodeAreaLabels;

    // area (depth sorted) specific
    IdGeoMap                            m_mapDsAreaGeo;
    bool                                m_doneUpdDsAreas;
    bool                                m_modDsAreas;

    // area (layered) specific
    IdGeoMap                            m_mapLyAreaGeo;
    bool                                m_doneUpdLyAreas;
    bool                                m_modLyAreas;

    // relation area (depth sorted) specific
    IdGeoMap                            m_mapDsRelAreaGeo;
    bool                                m_doneUpdDsRelAreas;
    bool                                m_modDsRelAreas;

    // relation area (layered) specific
    IdGeoMap                            m_mapLyRelAreaGeo;
    bool                                m_doneUpdLyRelAreas;
    bool                                m_modLyRelAreas;

    // common
    size_t                              m_countVxDsAreas;
    size_t                              m_limitVxDsAreas;
    osg::ref_ptr<osg::Geode>            m_geodeDsAreas;
    osg::ref_ptr<osg::MatrixTransform>  m_xfDsAreas;

    size_t                              m_countVxLyAreas;
    size_t                              m_limitVxLyAreas;
    osg::ref_ptr<osg::Geode>            m_geodeLyAreas;
    osg::ref_ptr<osg::MatrixTransform>  m_xfLyAreas;


    osg::ref_ptr<osg::Geode> m_nodeCam;
    osg::ref_ptr<osg::Geometry> m_camGeom;
    bool m_showCameraPlane;

    FontGeoMap m_fontGeoMap;

    // layer defs <-> render bins
    unsigned int m_minLayer;
    unsigned int m_layerBaseAreas;
    unsigned int m_layerTunnels;

    unsigned int m_layerBaseWayOLs;
    unsigned int m_layerBaseWays;
    unsigned int m_layerBaseWayLabels;

    unsigned int m_layerBridges;
    unsigned int m_depthSortedBin;

    osg::ref_ptr<osg::BlendFunc> m_blendFunc_bridge;

    // shaders
    osg::ref_ptr<osg::Program> m_shaderDirect;
    osg::ref_ptr<osg::Program> m_shaderDiffuse;
    osg::ref_ptr<osg::Program> m_shaderText;

    osg::ref_ptr<osg::Program> m_shaderDirectAttr;
    osg::ref_ptr<osg::Program> m_shaderDiffuseAttr;
    osg::ref_ptr<osg::Program> m_shaderPoints;

    // symbol geometry
    osg::ref_ptr<osg::Geometry> m_symbolTriangle;
    osg::ref_ptr<osg::Geometry> m_symbolSquare;
    osg::ref_ptr<osg::Geometry> m_symbolCircle;

    osg::ref_ptr<osg::Geometry> m_symbolTriangleOutline;
    osg::ref_ptr<osg::Geometry> m_symbolSquareOutline;
    osg::ref_ptr<osg::Geometry> m_symbolCircleOutline;

    // earth geometry
    osg::ref_ptr<osg::Geometry> m_earthPointCloud;

    // tessellator
    osg::GLUtesselator * m_tobj;
    static std::vector<GLdouble *> m_tListNewVx;
};

}


#endif
