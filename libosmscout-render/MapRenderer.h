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

// std includes
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef USE_BOOST
    #include <boost/unordered_map.hpp>
    #define TYPE_UNORDERED_MAP boost::unordered::unordered_map
    #define TYPE_UNORDERED_MULTIMAP boost::unordered::unordered_multimap
#else
    #include <unordered_map>
    #define TYPE_UNORDERED_MAP std::unordered_map
    #define TYPE_UNORDERED_MULTIMAP std::unordered_multimap
#endif

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

// epsilon error
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
typedef std::pair<RelationRef,unsigned int> RelRefAndLod;
typedef std::pair<Vec2,Vec2> LineVec2;


// note: reminder to implement constructor,
// destructor and assignment operator if we
// start using pointers where memory is locally
// allocated (like the old style BuildingData)

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
    AreaRenderData() : buildingHeight(20) {}

    // geometry data
    WayRef                              areaRef;
    size_t                              areaLayer;
    Vec3                                centerPoint;
    bool                                pathIsCCW;

    std::vector<Vec3>                   listOuterPoints;
    std::vector<std::vector<Vec3> >     listListInnerPoints;

    FillRenderStyle const*              fillRenderStyle;

    bool                        isBuilding;
    double                      buildingHeight;

//    // should eventually be based on:
//    // hxxp://openstreetmap.org/wiki/Simple_3D_Buildings
//    double                      buildingMinHeight;
//    unsigned int                buildingLevel;
//    unsigned int                buildingMinLevels;

    // label data
    bool                        hasName;
    std::string                 nameLabel;
    LabelRenderStyle const *    nameLabelRenderStyle;

    // geomPtr points to the engine specific data
    // structure that is used to render this area
    // (such as a node in a scene graph)
    void *geomPtr;
};

struct RelAreaRenderData
{
    RelationRef                 relRef;
    std::vector<AreaRenderData> listAreaData;

    // geomPtr points to the engine specific data
    // structure that is used to render this area
    // (such as a node in a scene graph)
    void *geomPtr;
};

typedef std::vector<WayRenderData> RelWayRenderData;

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
    CAM_ISO_NE,
    CAM_ISO_NW,
    CAM_ISO_SE,
    CAM_ISO_SW
};

enum IntersectionType
{
    XSEC_FALSE,
    XSEC_TRUE,
    XSEC_COINCIDENT,
    XSEC_PARALLEL
};

enum OutlineType
{
    OL_CENTER,
    OL_RIGHT,
    OL_LEFT
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
    // * initializes scene using camera
    // * if no camera is specified, the default camera looks
    //   down at the center of the dataset from an alt of 500m
    void InitializeScene();
    void InitializeScene(PointLLA const &camLLA,CameraMode camMode,
                         double fovy, double aspectRatio);

    // SetCamera
    // * set the camera directly
    // * updates scene contents if required
    void SetCamera(PointLLA const &camLLA,CameraMode camMode,
                   double fovy, double aspectRatio);

    // UpdateCameraLookAt
    // * updates the current camera using eye,viewPt,up vectors
    // * meant to be called for incremental updates so that
    //   the scene is updated as the camera moves -- however, do not
    //   call this function every time the camera moves slightly as
    //   its expensive to check if the scene needs to be updated
    // * its more efficient to call this function at a fixed rate
    //   like once every one or two seconds
    void UpdateCameraLookAt(Vec3 const &eye,
                            Vec3 const &viewPt,
                            Vec3 const &up);

    // GetCamera
    Camera const * GetCamera();

private:
    // METHODS

    // todo: do we need this? is it even called at the right location?
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
    virtual void addRelAreaToScene(RelAreaRenderData &relAreaData) = 0;

    virtual void removeNodeFromScene(NodeRenderData const &nodeData) = 0;
    virtual void removeWayFromScene(WayRenderData const &wayData) = 0;
    virtual void removeAreaFromScene(AreaRenderData const &areaData) = 0;
    virtual void removeRelAreaFromScene(RelAreaRenderData const &relAreaData) = 0;

    virtual void removeAllFromScene() = 0;
    virtual void showCameraViewArea(Camera &sceneCam) = 0;

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
    void updateNodeRenderData(std::vector<TYPE_UNORDERED_MAP<Id,NodeRef> > &listNodeRefsByLod);
    void updateWayRenderData(std::vector<TYPE_UNORDERED_MAP<Id,WayRef> > &listWayRefsByLod);
    void updateAreaRenderData(std::vector<TYPE_UNORDERED_MAP<Id,WayRef> > &listAreaRefsByLod);
    void updateRelWayRenderData(std::vector<TYPE_UNORDERED_MAP<Id,RelationRef> > &listRelRefsByLod);
    void updateRelAreaRenderData(std::vector<TYPE_UNORDERED_MAP<Id,RelationRef> > &listRelRefsByLod);

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

    bool genRelWayRenderData(RelationRef const &relRef,
                             RenderStyleConfig const *renderStyle,
                             RelWayRenderData &relRenderData);

    bool genRelAreaRenderData(RelationRef const &relRef,
                              RenderStyleConfig const *renderStyle,
                              RelAreaRenderData &relRenderData);

    // getListOfSharedWayNodes
    void getListOfSharedWayNodes(WayRef const &wayRef,
                                 std::vector<bool> &listSharedNodes);

    // removeWayFromSharedNodes
    // * remove all nodes belonging to way from shared nodes list
    void removeWayFromSharedNodes(WayRef const &wayRef);

    // clearAllRenderData
    // * removes all drawable objects in the scene that
    //   are dynamically updated based on camera position
    void clearAllRenderData();

    // MEMBERS
    Database const *m_database;

    // render style config list (todo shouldnt this be <RenderStyleConfig const *>)?
    std::vector<RenderStyleConfig*>            m_listRenderStyleConfigs;

    // lists of geometry data lists
    std::vector<TYPE_UNORDERED_MAP<Id,NodeRenderData> >    m_listNodeData;
    std::vector<TYPE_UNORDERED_MAP<Id,WayRenderData> >     m_listWayData;
    std::vector<TYPE_UNORDERED_MAP<Id,AreaRenderData> >    m_listAreaData;
    std::vector<TYPE_UNORDERED_MAP<Id,RelWayRenderData> >  m_listRelWayData;
    std::vector<TYPE_UNORDERED_MAP<Id,RelAreaRenderData> > m_listRelAreaData;

    // important TagIds
    TagId m_tagName;
    TagId m_tagBuilding;
    TagId m_tagHeight;

    // check for intersections <NodeId,WayId>
    TYPE_UNORDERED_MULTIMAP<Id,Id> m_listSharedNodes;

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
    // * checks if a polygon (polygons with holes are allowed)
    //   specified as a set of edges is simple (returns true)
    bool calcPolyIsSimple(std::vector<LineVec2> const &listEdges,
                          std::vector<bool> const &edgeStartsNewPoly);

    // calcPolyIsCCW
    // * checks if a simple polygon specified as a list of
    //   ordered points has a CCW or CW orientation
    // * expects Vec2.x as longitude and Vec2.y as latitude
    bool calcPolyIsCCW(std::vector<Vec2> const &listPolyPoints);

    // calcAreaIsValid
    // * checks if an area is valid by verifying it consists
    //   solely of non-intersecting simple polys
    // * will also flip vertex ordering to CCW for outer points
    //   and CW for inner points if required
    bool calcAreaIsValid(std::vector<Vec2> &listOuterPoints);
    bool calcAreaIsValid(std::vector<Vec2> &listOuterPoints,
                         std::vector<std::vector<Vec2> > &listListInnerPoints);

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

    // calcEstBuildingHeight
    // * uses the building's area to estimate a height;
    //   grossly inaccurate but provides a reasonable
    //   visual effect for rendering buildings
    double calcEstBuildingHeight(double baseArea);

    // buildPolylineAsTriStrip
    // * converts a set of points and a lineWidth
    //   to a vertex array defining a triangle strip
    // * outline is generated as a single vertex array
    //   using the lineWidth and an outlineType [center,left,right]
    void buildPolylineAsTriStrip(std::vector<Vec3> const &polyLine,
                                 double lineWidth,
                                 OutlineType outlineType,
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

    // readFileAsString
    std::string readFileAsString(std::string const &fileName);

    // getFontList
    // * get list of unique fonts from style configs
    void getFontList(std::vector<std::string> &listFonts);

    // getMaxWayLayer
    // *
    size_t getMaxWayLayer();
    size_t getMaxAreaLayer();

    // getTypeName
    std::string getTypeName(TypeId typeId);

    // debug - remove later
    void printVector(Vec3 const &myVector);

    // MEMBERS
    std::vector<std::string> m_listMessages;
};

}

#endif
