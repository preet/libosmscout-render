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
#include <algorithm>

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

struct WayRenderData
{
    WayRef                  wayRef;
    size_t                  wayLayer;
    std::vector<Vec3>       listPointData;
    LineRenderStyle const*  lineRenderStyle;

    LabelRenderStyle const* nameLabelRenderStyle;
    std::string             nameLabel;

    // geomPtr points to the engine specific data
    // structure that is used to render this way
    // (such as a node in a scene graph)
    void *geomPtr;
};

struct AreaRenderData
{
    WayRef                              areaRef;
    std::vector<Vec3>                   listPointData;
    FillRenderStyle const *             fillRenderStyle;

    LabelRenderStyle const  *           labelRenderStyle;
    std::string                         nameLabel;

    // geomPtr points to the engine specific data
    // structure that is used to render this way
    // (such as a node in a scene graph)
    void *geomPtr;
};

//struct AreaRenderData
//{
//    WayRef                                 areaRef;
//    std::vector<Vec3>                   listOuterPoints;        // TODO could be shared between lods
//    std::vector<std::vector<Vec3> >     listListInnerPoints;    // TODO could be shared between lods
//    FillRenderStyle const *             fillRenderStyle;

//    LabelRenderStyle const  *           labelRenderStyle;
//    std::string                         nameLabel;

//    // geomPtr points to the engine specific data
//    // structure that is used to render this way
//    // (such as a node in a scene graph)
//    void *geomPtr;
//};

// compare[]Ref
// * comparison of osmscout database references by id
inline bool CompareWayRefs(WayRef const &ref1, WayRef const &ref2)
{   return (ref1->GetId() < ref2->GetId());   }

// compare[]RenderData
// * comparison of render data objects by id
struct CompareId
{
    bool operator() (WayRef const &ref1, WayRef const &ref2) const
    {   return (ref1->GetId() < ref2->GetId());   }

    bool operator() (WayRenderData const &way1, WayRenderData const &way2) const
    {   return (way1.wayRef->GetId() < way2.wayRef->GetId());   }
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
    // -
    void InitializeScene(PointLLA const &camLLA,CameraMode camMode);

    // SetCamera
    // - set the camera up using LLA and a camera mode
    // - updates scene contents if required
    void SetCamera(PointLLA const &camLLA,CameraMode camMode);

    // Camera Manipulators
    // - rotate,pan and zoom camera
    // - updates scene contents if required
    void RotateCamera(Vec3 const &axisVec, double angleDegCCW);
    void PanCamera(Vec3 const &dirnVec, double distMeters);
    void ZoomCamera(double zoomAmount);

    // GetCamera
    Camera const * GetCamera();

    // RenderFrame
    // -
    virtual void RenderFrame() = 0;

private:
    // METHODS
    virtual void initScene() = 0;
    virtual void addWayToScene(WayRenderData &wayData) = 0;
    virtual void removeWayFromScene(WayRenderData const &wayData) = 0;
    virtual void removeAllPrimitivesFromScene() = 0;

    // updateSceneContents
    // - this method uses the active camera's position and
    //   orientation to update the map data that should be
    //   displayed, and calls the renderer driver's functions
    //   to update the scene
    void updateSceneContents();

    // updateSceneBasedOnCamera
    // - compares the last known view extents with the current
    //   camera view extents and calls updateSceneContents()
    //   if there is enough of a difference between the two
    //   (if the overlap of their view extent areas is < 50%)
    void updateSceneBasedOnCamera();

    // - convenience call to calcCameraViewExtents(...)
    //   that implicitly uses m_camera
    bool calcCameraViewExtents();

    // update[]RenderData
    // - removes drawable objects no longer in the scene
    //   and adds drawable objects newly present in the scene
    void updateWayRenderData(std::vector<WayRef> const &listWayRefs,int lodIdx);
    void updateAreaRenderData(std::vector<WayRef> const &listAreaRefs,int lodIdx);

    // genWayRenderData
    // - generates way render data given a WayRef
    //   and its associated RenderStyleConfig
    void genWayRenderData(WayRef const &wayRef,
                          RenderStyleConfig const *renderStyle,
                          WayRenderData &wayRenderData);

    void genAreaRenderData(ObjectRef const &areaRef,
                           RenderStyleConfig const *renderStyle,
                           AreaRenderData &areaRenderData);

    // clearAllRenderData
    // - removes all drawable objects in the scene that
    //   are dynamically updated based on camera position
    void clearAllRenderData();

    // MEMBERS
    Database const *m_database;

    // render style config list
    std::vector<RenderStyleConfig*>            m_listRenderStyleConfigs;

    // lists of geometry data lists
    std::vector<std::set<WayRenderData,CompareId> >      m_listWayDataLists;

    Camera m_camera;
    double m_dataMinLat;
    double m_dataMinLon;
    double m_dataMaxLat;
    double m_dataMaxLon;


protected:
    // METHODS

    // convLLAToECEF
    // - converts point data in Latitude/Longitude/Altitude to
    //   its corresponding X/Y/Z in ECEF coordinates
    void convLLAToECEF(PointLLA const &pointLLA, Vec3 &pointECEF);
    Vec3 convLLAToECEF(PointLLA const &pointLLA);

    // convECEFToLLA
    // - converts point data in ECEF X/Y/Z to its corresponding
    //   Longitude/Latitude/Altitude coordinates
    void convECEFToLLA(Vec3 const &pointECEF, PointLLA &pointLLA);
    PointLLA convECEFToLLA(Vec3 const &pointECEF);

    // calcLTPVectorsNED
    // - calculate direction vectors in ECEF along North,
    //   East and Down given Latitude,Longitude
    void calcECEFNorthEastDown(PointLLA const &pointLLA,
                               Vec3 &vecNorth,
                               Vec3 &vecEast,
                               Vec3 &vecDown);

    // calcQuadraticEquationReal
    // - computes the solutions to a quadratic equation with
    //   parameters a, b and c, and accounts for numerical error
    //   note: doesn't work with complex roots (will save empty vector)
    void calcQuadraticEquationReal(double a, double b, double c,
                                   std::vector<double> &listRoots);

    // calcRectOverlap
    // - checks whether or not two rectangles overlap and
    //   returns the area of the overlapping rectangle
    double calcAreaRectOverlap(double r1_bl_x, double r1_bl_y,
                               double r1_tr_x, double r1_tr_y,
                               double r2_bl_x, double r2_bl_y,
                               double r2_tr_x, double r2_tr_y);

    // calcMinPointLineDistance
    // - computes the minimum distance between a given
    //   point and line segment
    double calcMinPointLineDistance(Vec3 const &distalPoint,
                                    Vec3 const &endPointA,
                                    Vec3 const &endPointB);

    // calcMaxPointLineDistnace
    // - computes the maximum distance between a given
    //   point and line segment
    double calcMaxPointLineDistance(Vec3 const &distalPoint,
                                    Vec3 const &endPointA,
                                    Vec3 const &endPointB);

    // calcLinePlaneMinDistance
    // - computes the minimum distance between a given
    //   point and plane
    double calcMinPointPlaneDistance(Vec3 const &distalPoint,
                                     Vec3 const &planePoint,
                                     Vec3 const &planeNormal);

    // calcGeographicDestination
    // - finds the coordinate that is 'distanceMeters' out from
    //   the starting point at a bearing of 'bearingDegrees'
    // - bearing is degrees CW from North
    // - assumes that Earth is a spheroid, should be good
    //   enough for an approximation
    bool calcGeographicDestination(PointLLA const &pointStart,
                                   double bearingDegrees,
                                   double distanceMeters,
                                   PointLLA &pointDest);

    // calcPointLiesAlongRay
    // - check if a given point lies on/in the specified ray
    // - the ray's direction vector is taken into account
    bool calcPointLiesAlongRay(Vec3 const &distalPoint,
                               Vec3 const &rayPoint,
                               Vec3 const &rayDirn);

    // calcLinePlaneIntersection
    // - computes the intersection point between a given
    //   line and plane
    // - returns false if no intersection point exists
    bool calcRayPlaneIntersection(Vec3 const &linePoint,
                                  Vec3 const &lineDirn,
                                  Vec3 const &planePoint,
                                  Vec3 const &planeNormal,
                                  Vec3 &intersectionPoint);

    // calcLineEarthIntersection
    // - computes the nearest intersection point (to the ray's
    //   origin) with the surface of the Earth defined with
    //   ECEF coordinates
    // - return true if at least one intersection point found
    //   else return false
    bool calcRayEarthIntersection(Vec3 const &rayPoint,
                                  Vec3 const &rayDirn,
                                  Vec3 &nearXsecPoint);

    // calcCameraViewExtents
    // - uses the camera's view frustum to find the view
    //   extents of the camera, in terms of a lat/lon box
    // - return true if the Earth is visible with the
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

    // buildEarthSurfaceGeometry
    // - build the ellipsoid geometry of the earth
    //   in ECEF coordinate space, corresponding
    //   mesh resolution is based on lat/lon segments
    bool buildEarthSurfaceGeometry(unsigned int latSegments,
                                   unsigned int lonSegments,
                                   std::vector<Vec3> &myVertices,
                                   std::vector<Vec3> &myNormals,
                                   std::vector<Vec2> &myTexCoords,
                                   std::vector<unsigned int> &myIndices);

    // MEMBERS
    std::vector<std::string> m_listMessages;
};

}

#endif
