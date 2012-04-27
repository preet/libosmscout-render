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
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/AutoTransform>
#include <osg/PolygonMode>
#include <osgText/TextBase>
#include <osg/ShapeDrawable>
#include <osgUtil/Tessellator>
#include <osg/MatrixTransform>

#include "MapRenderer.h"

namespace osmscout
{

struct FillMaterial
{
    unsigned int matId;
    osg::ref_ptr<osg::Material> fillColor;
    osg::ref_ptr<osg::Material> outlineColor;
};

struct LineMaterial
{
    unsigned int matId;
    osg::ref_ptr<osg::Material> lineColor;
    osg::ref_ptr<osg::Material> outlineColor;
};

struct LabelMaterial
{
    unsigned int matId;
    osg::ref_ptr<osg::Material> fontColor;
    osg::ref_ptr<osg::Material> fontOutlineColor;
    osg::ref_ptr<osg::Material> plateColor;
    osg::ref_ptr<osg::Material> plateOutlineColor;
};

inline bool FillMaterialCompare(FillMaterial const &fillMat1,
                                FillMaterial const &fillMat2)
{    return (fillMat1.matId < fillMat2.matId);   }

inline bool LineMaterialCompare(LineMaterial const &lineMat1,
                                LineMaterial const &lineMat2)
{    return (lineMat1.matId < lineMat2.matId);   }

inline bool LabelMaterialCompare(LabelMaterial const &labelMat1,
                                 LabelMaterial const &labelMat2)
{    return (labelMat1.matId < labelMat2.matId);   }

typedef std::unordered_map<std::string,osg::ref_ptr<osgText::Text> > CharGeoMap;
typedef std::unordered_map<std::string,CharGeoMap> FontGeoMap;

class MapRendererOSG : public MapRenderer
{
public:
    MapRendererOSG(Database const *myDatabase);
    ~MapRendererOSG();

    // RenderFrame
    void RenderFrame();

    // TEMPORARILY PUBLIC (viewer from main.cpp)
    osg::ref_ptr<osg::Group> m_nodeRoot;


private:
    void initScene();
    void rebuildStyleData(std::vector<RenderStyleConfig*> const &listRenderStyles);

    void addNodeToScene(NodeRenderData &nodeData);
    void removeNodeFromScene(const NodeRenderData &nodeData);

    void addWayToScene(WayRenderData &wayData);
    void removeAreaFromScene(AreaRenderData const &areaData);

    void addAreaToScene(AreaRenderData &areaData);
    void removeWayFromScene(WayRenderData const &wayData);

    void addRelAreaToScene(RelAreaRenderData &relAreaData);
    void removeRelAreaFromScene(const RelAreaRenderData &relAreaData);

    void removeAllFromScene();

    void addNodeGeometry(NodeRenderData const &nodeData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent);

    void addWayGeometry(WayRenderData const &wayData,
                        osg::Vec3d const &offsetVec,
                        osg::MatrixTransform *nodeParent);

    void addAreaGeometry(AreaRenderData const &areaData,
                         osg::Vec3d const &offsetVec,
                         osg::MatrixTransform *nodeParent);

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

    void buildGeomTriangle();
    void buildGeomSquare();
    void buildGeomCircle();

    double calcWayLength(osg::Vec3dArray const *listWayPoints);

    void calcWaySegmentLengths(osg::Vec3dArray const *listWayPoints,
                               std::vector<double> &listSegLengths);

    void calcLerpAlongWay(osg::Vec3dArray const *listWayPoints,
                          osg::Vec3dArray const *listWayNormals,
                          double const lengthAlongWay,
                          osg::Vec3d &pointAtLength,
                          osg::Vec3d &dirnAtLength,
                          osg::Vec3d &normalAtLength,
                          osg::Vec3d &sideAtLength);

    inline osg::Vec4 colorAsVec4(ColorRGBA const &color);
    inline osg::Vec3 convVec3ToOsgVec3(Vec3 const &myVector);
    inline osg::Vec3d convVec3ToOsgVec3d(Vec3 const &myVector);

    void startTiming(std::string const &desc);
    void endTiming();

    timeval m_t1,m_t2;
    std::string m_timingDesc;

    // scene graph vars
    osg::ref_ptr<osg::Group> m_nodeNodes;
    osg::ref_ptr<osg::Group> m_nodeWays;
    osg::ref_ptr<osg::Group> m_nodeAreas;

    osg::ref_ptr<osg::BlendFunc> m_wayBlendFunc;

    FontGeoMap m_fontGeoMap;
    std::vector<FillMaterial> m_listFillMaterials;
    std::vector<LineMaterial> m_listLineMaterials;
    std::vector<LabelMaterial> m_listLabelMaterials;

    // layer defs <-> render bins
    unsigned int m_minLayer;
    unsigned int m_layerBaseAreaOLs;
    unsigned int m_layerBaseAreas;
    unsigned int m_layerBaseWayOLs;
    unsigned int m_layerBaseWays;
    unsigned int m_layerBaseWayLabels;
    unsigned int m_depthSortedBin;

    // symbol geometry to use for instancing
    osg::ref_ptr<osg::Geometry> m_symbolTriangle;
    osg::ref_ptr<osg::Geometry> m_symbolSquare;
    osg::ref_ptr<osg::Geometry> m_symbolCircle;
};

}


#endif
