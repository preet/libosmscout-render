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

#ifndef OSMSCOUT_MAP_RENDERER_H
#define OSMSCOUT_MAP_RENDERER_H

// sys includes
#include <math.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

// osmscout includes
#include <osmscout/Database.h>
#include <osmscout/ObjectRef.h>
#include <osmscout/Way.h>

// osmscout-render includes
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "SimpleLogger.hpp"
#include "RenderStyleConfig.hpp"


// PI!
#define K_PI 3.141592653589

// epsilon
#define K_EPS 1E-11

// WGS84 ellipsoid parameters
// (http://en.wikipedia.org/wiki/WGS_84)
#define ELL_SEMI_MAJOR 6378137.0            // meters
#define ELL_SEMI_MAJOR_EXP2 40680631590769

#define ELL_SEMI_MINOR 6356752.3142         // meters
#define ELL_SEMI_MINOR_EXP2 40408299984087.1

#define ELL_F 1/298.257223563
#define ELL_ECC_EXP2 6.69437999014e-3
#define ELL_ECC2_EXP2 6.73949674228e-3


namespace osmscout
{

// longitude, latitude, altitude point class
class PointLLA
{
public:
    PointLLA() :
        lon(0),lat(0),alt(0) {}

    PointLLA(double myLat, double myLon) :
        lat(myLat),lon(myLon),alt(0) {}

    PointLLA(double myLat, double myLon, double myAlt) :
        lon(myLon),lat(myLat),alt(myAlt) {}

    double lon;
    double lat;
    double alt;
};

typedef std::pair<NodeRef,unsigned int> NodeRefAndLod;
typedef std::pair<WayRef,unsigned int> WayRefAndLod;
typedef std::pair<Vec2,Vec2> LineVec2;

struct BuildingData
{
    // based on:
    // hxxp://openstreetmap.org/wiki/Simple_3D_Buildings
    BuildingData():height(0) {}
    double height;
//    // TODO
//    double min_height;
//    double levels;
//    double min_levels;
};

struct NodeRenderData
{
    // geometry data
    NodeRef                     nodeRef;
    Vec3                        nodePosn;
    FillRenderStyle const *     fillRenderStyle;
    SymbolRenderStyle const *   symbolRenderStyle;

    // label data
    bool                        hasName;
    std::string                 nameLabel;
    LabelRenderStyle const *    nameLabelRenderStyle;

    // geomPtr points to the engine specific data
    // structure that is used to render this node
    // (such as a node in a scene graph)
    void *geomPtr;
};

struct WayRenderData
{
    // geometry data
    WayRef                  wayRef;
    size_t                  wayLayer;
    std::vector<Vec3>       listWayPoints;
    std::vector<bool>       listSharedNodes;
    LineRenderStyle const*  lineRenderStyle;

    // label data
    bool                        hasName;
    std::string                 nameLabel;
    LabelRenderStyle const *    nameLabelRenderStyle;

    // geomPtr points to the engine specific data
    // structure that is used to render this way
    // (such as a node in a scene graph)
    void *geomPtr;
};

struct AreaRenderData
{
    // geometry data
    WayRef                              areaRef;
    size_t                              areaLayer;
    Vec3                                centerPoint;
    bool                                pathIsCCW;
    std::vector<Vec3>                   listBorderPoints;
    FillRenderStyle const*              fillRenderStyle;

    bool                        isBuilding;
    BuildingData *              buildingData;
    // TODO ensure delete is called on BuildingData

    // label data
    bool                        hasName;
    std::string                 nameLabel;
    LabelRenderStyle const *    nameLabelRenderStyle;

    // geomPtr points to the engine specific data
    // structure that is used to render this area
    // (such as a node in a scene graph)
    void *geomPtr;
};

class Camera
{
public:
    Camera() : fovY(0),aspectRatio(0),nearDist(0),farDist(0),
        minLat(0),minLon(0),maxLat(0),maxLon(0) {}

    PointLLA LLA;
    Vec3 eye;
    Vec3 viewPt;
    Vec3 up;

    double fovY;
    double aspectRatio;
    double nearDist;
    double farDist;

    double minLat;
    double minLon;
    double maxLat;
    double maxLon;
};

enum CameraMode
{
    CAM_2D,
    CAM_3D,
    CAM_ISO_NE
};

enum IntersectionType
{
    XSEC_FALSE,
    XSEC_TRUE,
    XSEC_COINCIDENT,
    XSEC_PARALLEL
};

class MapRenderer
{
public:
    MapRenderer(Database const *myDatabase);
    virtual ~MapRenderer();

    // SetRenderStyleConfig
    void SetRenderStyleConfigs(std::vector<RenderStyleConfig*> const &listStyleConfigs);

    // GetDebugLog
    void GetDebugLog(std::vector<std::string> &listDebugMessages);

    // InitializeScene
    // *
    void InitializeScene(PointLLA const &camLLA,CameraMode camMode);

    // SetCamera
    // * set the camera up using LLA and a camera mode
    // * updates scene contents if required
    void SetCamera(PointLLA const &camLLA,CameraMode camMode);

    // Camera Manipulators
    // * rotate,pan and zoom camera
    // * updates scene contents if required
    void RotateCamera(Vec3 const &axisVec, double angleDegCCW);
    void PanCamera(Vec3 const &dirnVec, double distMeters);
    void ZoomCamera(double zoomAmount);

    // GetCamera
    Camera const * GetCamera();

    // RenderFrame
    // *
    virtual void RenderFrame() = 0;

private:
    // METHODS
    virtual void initScene() = 0;

    // if the render engine wants to do anything with the
    // new style data (cache certain stuff, etc), it
    // should be done here -- it isn't mandatory to do
    // anything within this function as we send style
    // data over with add[]ToScene regardless
    virtual void rebuildStyleData(std::vector<RenderStyleConfig*> const &listRenderStyles) = 0;

    virtual void addNodeToScene(NodeRenderData &nodeData) = 0;
    virtual void addWayToScene(WayRenderData &wayData) = 0;
    virtual void addAreaToScene(AreaRenderData &areaData) = 0;

    virtual void removeNodeFromScene(NodeRenderData const &nodeData) = 0;
    virtual void removeWayFromScene(WayRenderData const &wayData) = 0;
    virtual void removeAreaFromScene(AreaRenderData const &areaData) = 0;

    virtual void removeAllFromScene() = 0;

    // updateSceneContents
    // * this method uses the active camera's position and
    //   orientation to update the map data that should be
    //   displayed, and calls the renderer driver's functions
    //   to update the scene
    void updateSceneContents();

    // updateSceneBasedOnCamera
    // * compares the last known view extents with the current
    //   camera view extents and calls updateSceneContents()
    //   if there is enough of a difference between the two
    //   (if the overlap of their view extent areas is < 75%)
    void updateSceneBasedOnCamera();

    // * convenience call to calcCameraViewExtents(...)
    //   that implicitly uses m_camera
    bool calcCameraViewExtents();

    // update[]RenderData
    // * removes drawable objects no longer in the scene
    //   and adds drawable objects newly present in the scene
    void updateNodeRenderData(std::vector<std::unordered_map<Id,NodeRef> > &listNodeRefsByLod);
    void updateWayRenderData(std::vector<std::unordered_map<Id,WayRef> > &listWayRefsByLod);
    void updateAreaRenderData(std::vector<std::unordered_map<Id,WayRef> > &listAreaRefsByLod);

    // gen[]RenderData
    // * generates way render data given a []Ref
    //   and its associated RenderStyleConfig
    bool genNodeRenderData(NodeRef const &nodeRef,
                           RenderStyleConfig const *renderStyle,
                           NodeRenderData &nodeRenderData);

    bool genWayRenderData(WayRef const &wayRef,
                          RenderStyleConfig const *renderStyle,
                          WayRenderData &wayRenderData);

    bool genAreaRenderData(WayRef const &areaRef,
                           RenderStyleConfig const *renderStyle,
                           AreaRenderData &areaRenderData);

    // removeWayFromSharedNodes
    // * remove all nodes belonging to way from shared nodes list
    void removeWayFromSharedNodes(WayRef const &wayRef);

    // clearAllRenderData
    // * removes all drawable objects in the scene that
    //   are dynamically updated based on camera position
    void clearAllRenderData();

    // MEMBERS
    Database const *m_database;

    // render style config list (shouldnt this be <RenderStyleConfig const *>)?
    std::vector<RenderStyleConfig*>            m_listRenderStyleConfigs;

    // lists of geometry data lists
    std::vector<std::unordered_map<Id,NodeRenderData> >  m_listNodeData;
    std::vector<std::unordered_map<Id,WayRenderData> >   m_listWayData;
    std::vector<std::unordered_map<Id,AreaRenderData> >  m_listAreaData;

    // important TagIds
    TagId m_tagName;
    TagId m_tagBuilding;
    TagId m_tagHeight;

    // check for intersections <NodeId,WayId>
    std::unordered_multimap<Id,Id> m_listSharedNodes;

    unsigned int m_wayNodeCount;

    // camera vars
    Camera m_camera;
    double m_dataMinLat;
    double m_dataMinLon;
    double m_dataMaxLat;
    double m_dataMaxLon;

protected:
    // METHODS

    // convLLAToECEF
    // * converts point data in Latitude/Longitude/Altitude to
    //   its corresponding X/Y/Z in ECEF coordinates
    void convLLAToECEF(PointLLA const &pointLLA, Vec3 &pointECEF);
    Vec3 convLLAToECEF(PointLLA const &pointLLA);

    // convECEFToLLA
    // * converts point data in ECEF X/Y/Z to its corresponding
    //   Longitude/Latitude/Altitude coordinates
    void convECEFToLLA(Vec3 const &pointECEF, PointLLA &pointLLA);
    PointLLA convECEFToLLA(Vec3 const &pointECEF);

    // convStrToDbl
    double convStrToDbl(std::string const &strNum);

    // calcLTPVectorsNED
    // * calculate direction vectors in ECEF along North,
    //   East and Down given Latitude,Longitude
    void calcECEFNorthEastDown(PointLLA const &pointLLA,
                               Vec3 &vecNorth,
                               Vec3 &vecEast,
                               Vec3 &vecDown);

    // calcQuadraticEquationReal
    // * computes the solutions to a quadratic equation with
    //   parameters a, b and c, and accounts for numerical error
    //   note: doesn't work with complex roots (will save empty vector)
    void calcQuadraticEquationReal(double a, double b, double c,
                                   std::vector<double> &listRoots);

    // calcLinesIntersect
    // * checks whether two 2d lines intersect
    bool calcLinesIntersect(double a_x1, double a_y1,
                            double a_x2, double a_y2,
                            double b_x1, double b_y1,
                            double b_x2, double b_y2);

    // calcLinesIntersect
    // * checks whether two 2d lines intersect and calculates
    //   the point of intersection (i_x1,i_y1)
    // * IntersectionType indicates whether the two lines intersect,
    //   are coincident, or parallel (if they don't intersect,
    //   i_x1 and i_y1 are invalid)
    IntersectionType calcLinesIntersect(double a_x1, double a_y1,
                                        double a_x2, double a_y2,
                                        double b_x1, double b_y1,
                                        double b_x2, double b_y2,
                                        double &i_x1, double &i_y1);

    // calcEstSkewLineProj
    // * given two skew lines, calculates the projection of the
    //   second line onto the first (the intersection point if the
    //   lines were touching) -- used to estimate the int. pt of
    //   two lines in 3d that do int. or are close to intersecting
    bool calcEstSkewLineProj(const Vec3 &a_p1, const Vec3 &a_p2,
                             const Vec3 &b_p1, const Vec3 &b_p2,
                             Vec3 &i_p);
    // calcPolyIsSimple
    // * checks if a given polygon specified as a list of
    //   ordered points is simple (no intersecting edges)
    bool calcPolyIsSimple(std::vector<Vec2> const &listPolyPoints);

    // calcMultiPolyIsSimple
    // * checks if a multi-polygon (polygon with holes in it)
    //   specified as a set of lists of ordered points is simple
    bool calcMultiPolyIsSimple(std::vector<LineVec2> const &listEdges,
                               std::vector<bool> const &edgeStartsNewPoly);

    // calcPolyIsCCW
    // * checks if a given polygon specified as a list of
    //   ordered points has a CCW or CW orientation
    bool calcPolyIsCCW(std::vector<Vec2> const &listPolyPoints);

    // calcRectOverlap
    // * checks whether or not two rectangles overlap and
    //   returns the area of the overlapping rectangle
    double calcAreaRectOverlap(double r1_bl_x, double r1_bl_y,
                               double r1_tr_x, double r1_tr_y,
                               double r2_bl_x, double r2_bl_y,
                               double r2_tr_x, double r2_tr_y);

    // calcMinPointLineDistance
    // * computes the minimum distance between a given
    //   point and line segment
    double calcMinPointLineDistance(Vec3 const &distalPoint,
                                    Vec3 const &endPointA,
                                    Vec3 const &endPointB);

    // calcMaxPointLineDistnace
    // * computes the maximum distance between a given
    //   point and line segment
    double calcMaxPointLineDistance(Vec3 const &distalPoint,
                                    Vec3 const &endPointA,
                                    Vec3 const &endPointB);

    // calcLinePlaneMinDistance
    // * computes the minimum distance between a given
    //   point and plane
    double calcMinPointPlaneDistance(Vec3 const &distalPoint,
                                     Vec3 const &planePoint,
                                     Vec3 const &planeNormal);

    // calcGeographicDestination
    // * finds the coordinate that is 'distanceMeters' out from
    //   the starting point at a bearing of 'bearingDegrees'
    // * bearing is degrees CW from North
    // * assumes that Earth is a spheroid, should be good
    //   enough for an approximation
    bool calcGeographicDestination(PointLLA const &pointStart,
                                   double bearingDegrees,
                                   double distanceMeters,
                                   PointLLA &pointDest);

    // calcPointLiesAlongRay
    // * check if a given point lies on/in the specified ray
    // * the ray's direction vector is taken into account
    bool calcPointLiesAlongRay(Vec3 const &distalPoint,
                               Vec3 const &rayPoint,
                               Vec3 const &rayDirn);

    // calcRayPlaneIntersection
    // * computes the intersection point between a given
    //   line and plane
    // * returns false if no intersection point exists
    bool calcRayPlaneIntersection(Vec3 const &linePoint,
                                  Vec3 const &lineDirn,
                                  Vec3 const &planePoint,
                                  Vec3 const &planeNormal,
                                  Vec3 &intersectionPoint);

    // calcRayEarthIntersection
    // * computes the nearest intersection point (to the ray's
    //   origin) with the surface of the Earth defined with
    //   ECEF coordinates
    // * return true if at least one intersection point found
    //   else return false
    bool calcRayEarthIntersection(Vec3 const &rayPoint,
                                  Vec3 const &rayDirn,
                                  Vec3 &nearXsecPoint);

    // calcCameraViewExtents
    // * uses the camera's view frustum to find the view
    //   extents of the camera, in terms of a lat/lon box
    // * return true if the Earth is visible with the
    //   camera's parameters, else return false (camNearDist,
    //   camFarDist, camView[]Corner are invalid in this case
    bool calcCameraViewExtents(Vec3 const &camEye,
                               Vec3 const &camViewpoint,
                               Vec3 const &camUp,
                               double const &camFovY,
                               double const &camAspectRatio,
                               double &camNearDist, double &camFarDist,
                               double &camMinLat, double &camMaxLat,
                               double &camMinLon, double &camMaxLon);

    // buildWayAsTriStrip
    void buildWayAsTriStrip(WayRenderData const &wayData,
                            std::vector<Vec3> &vertexArray);

    // buildEarthSurfaceGeometry
    // * build the ellipsoid geometry of the earth
    //   in ECEF coordinate space, corresponding
    //   mesh resolution is based on lat/lon segments
    bool buildEarthSurfaceGeometry(unsigned int latSegments,
                                   unsigned int lonSegments,
                                   std::vector<Vec3> &myVertices,
                                   std::vector<Vec3> &myNormals,
                                   std::vector<Vec2> &myTexCoords,
                                   std::vector<unsigned int> &myIndices);

    // getFontList
    // * get list of unique fonts from style configs
    void getFontList(std::vector<std::string> &listFonts);

    // getMaxWayLayer
    // *
    size_t getMaxWayLayer();
    size_t getMaxAreaLayer();

    // debug - remove later
    void printVector(Vec3 const &myVector);

    // MEMBERS
    std::vector<std::string> m_listMessages;
};

}

#endif
