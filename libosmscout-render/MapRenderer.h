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

//#ifdef USE_BOOST
//    #include <boost/unordered_map.hpp>
//    #define TYPE_UNORDERED_MAP boost::unordered::unordered_map
//    #define TYPE_UNORDERED_SET boost::unordered::unordered_set
//    #define TYPE_UNORDERED_MULTIMAP boost::unordered::unordered_multimap
//#else
//    #include <unordered_map>
//    #define TYPE_UNORDERED_MAP std::unordered_map
//    #define TYPE_UNORDERED_SET std::unordered_set
//    #define TYPE_UNORDERED_MULTIMAP std::unordered_multimap
//#endif

// osmscout includes
#include <osmscout/Database.h>
#include <osmscout/ObjectRef.h>
#include <osmscout/Way.h>

// clipper
#include "clipper/clipper.hpp"

// OpenCTM
#include "openctm/openctm.h"

// osmscout-render includes
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "SimpleLogger.hpp"
#include "RenderStyleReader.h"
#include "RenderStyleConfig.hpp"
#include "DataSet.hpp"

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

// circumference
#define CIR_EQ 40075017   // around equator  (meters)
#define CIR_MD 40007860   // around meridian (meters)
#define CIR_AV 40041438   // average (meters)

// option: keep track of shared nodes/intersections
#define OPT_TRACK_SHARED_NODES 0

namespace osmsrender
{

// ========================================================================== //
// ========================================================================== //

class MapRenderer
{
public:
    MapRenderer();
    virtual ~MapRenderer();

    // Add/RemoveDataSet TODO why wont the base ptr arg work?
    void AddDataSet(DataSetOSM * dataSet);
    void RemoveDataSet(DataSetOSM * dataSet);

    void AddDataSet(DataSetOSMCoast * dataSet);
    void RemoveDataSet(DataSetOSMCoast * dataSet);

    void AddDataSet(DataSetTemp * dataSet);
    void RemoveDataSet(DataSetTemp * dataSet);

    void SetRenderStyle(std::string const &stylePath);

    // GetDebugLog
    void GetDebugLog(std::vector<std::string> &listDebugMessages);

    // InitializeScene
    // * initializes scene using camera
    // * if no camera is specified, the default camera looks
    //   down at the center of the dataset from an alt of 500m
    void InitializeScene();
    void InitializeScene(PointLLA const &camLLA,
                         double fovy, double aspectRatio);

    // SetCamera
    // * set the camera directly
    // * updates scene contents if required
    void SetCamera(PointLLA const &camLLA,
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

    // UpdateSceneContents
    // * updates scene contents belonging to specified
    //   DataSet only -- should be called when data is
    //   manually added/removed from a specific DataSet
    void UpdateSceneContents(DataSet const *dataSet);

    // UpdateSceneContentsAll
    // * updates scene contents for all DataSets
    void UpdateSceneContentsAll();

    // GetCamera
    Camera const * GetCamera();

    //
    virtual void ShowPlanetSurface() = 0;
    virtual void HidePlanetSurface() = 0;

    virtual void ShowPlanetCoastlines() = 0;
    virtual void HidePlanetCoastlines() = 0;

    virtual void ShowPlanetAdmin0() = 0;
    virtual void HidePlanetAdmin0() = 0;

private:
    // METHODS

    // rebuildAllData
    void rebuildAllData();

    // if the render engine wants to do anything with the
    // new style data (cache certain stuff, etc), it
    // should be done here -- it isn't mandatory to do
    // anything within this function as we send style
    // data over with add[]ToScene regardless
    virtual void rebuildStyleData(std::vector<DataSet const *> const &listDataSets) = 0;

    virtual void addNodeToScene(NodeRenderData &nodeData) = 0;
    virtual void addWayToScene(WayRenderData &wayData) = 0;
    virtual void addAreaToScene(AreaRenderData &areaData) = 0;
    virtual void addRelAreaToScene(RelAreaRenderData &relAreaData) = 0;

    virtual void doneUpdatingWays() = 0;
    virtual void doneUpdatingAreas() = 0;
    virtual void doneUpdatingRelAreas() = 0;

    virtual void removeNodeFromScene(NodeRenderData const &nodeData) = 0;
    virtual void removeWayFromScene(WayRenderData const &wayData) = 0;
    virtual void removeAreaFromScene(AreaRenderData const &areaData) = 0;
    virtual void removeRelAreaFromScene(RelAreaRenderData const &relAreaData) = 0;

    virtual void toggleSceneVisibility(bool isVisibile) = 0;
    virtual void removeAllFromScene() = 0;
    virtual void showCameraViewArea(Camera &sceneCam) = 0;

    // updateSceneContents
    // * this method uses the active camera's position and
    //   orientation to update the map data that should be
    //   displayed, and calls the renderer driver's functions
    //   to update the scene
    void updateSceneContents(std::vector<DataSet*> &listDataSets);

    // updateSceneBasedOnCamera
    // * compares the last known view extents with the current
    //   camera view extents and calls updateSceneContents()
    //   if there is enough of a difference between the two
    //   (if the overlap of their view extent areas is < 75%)
    void updateSceneBasedOnCamera();

    // update[]RenderData
    // * removes drawable objects no longer in the scene
    //   and adds drawable objects newly present in the scene
    void updateNodeRenderData(DataSet *dataSet,ListNodeRefsByLod &listNodeRefs);
    void updateWayRenderData(DataSet *dataSet,ListWayRefsByLod &listWayRefs);
    void updateAreaRenderData(DataSet *dataSet,ListAreaRefsByLod &listAreaRefs);
    void updateRelWayRenderData(DataSet *dataSet,ListRelWayRefsByLod &listRelWayRefs);
    void updateRelAreaRenderData(DataSet *dataSet,ListRelAreaRefsByLod &listRelAreaRefs);

    // gen[]RenderData
    // * generates render data given a []Ref
    //   and its associated RenderStyleConfig
    bool genNodeRenderData(DataSet *dataSet,
                           osmscout::NodeRef const &nodeRef,
                           RenderStyleConfig const *renderStyle,
                           NodeRenderData &nodeRenderData);

    bool genWayRenderData(DataSet *dataSet,
                          osmscout::WayRef const &wayRef,
                          RenderStyleConfig const *renderStyle,
                          ListSharedNodes &listSharedNodes,
                          WayRenderData &wayRenderData);

    bool genAreaRenderData(DataSet *dataSet,
                           osmscout::WayRef const &areaRef,
                           RenderStyleConfig const *renderStyle,
                           AreaRenderData &areaRenderData);

    bool genRelWayRenderData(DataSet *dataSet,
                             osmscout::RelationRef const &relRef,
                             RenderStyleConfig const *renderStyle,
                             RelWayRenderData &relRenderData);

    bool genRelAreaRenderData(DataSet *dataSet,
                              osmscout::RelationRef const &relRef,
                              RenderStyleConfig const *renderStyle,
                              RelAreaRenderData &relRenderData);

    // clear[]RenderData
    // * clears render data for map geometry, but keeps
    //   osmscout and driver implementation references
    // * this releases a bunch of memory for data that
    //   never gets used again before its deleted
    void clearNodeRenderData(NodeRenderData &nodeRenderData);
    void clearWayRenderData(WayRenderData &wayRenderData);
    void clearAreaRenderData(AreaRenderData &areaRenderData);
    void clearRelWayRenderData(RelWayRenderData &relRenderData);
    void clearRelAreaRenderData(RelAreaRenderData &relRenderData);

    // getListIntersections
    void getListSharedWayNodes(ListSharedNodes &listSharedNodes,
                               osmscout::WayRef const &wayRef,
                               std::vector<std::vector<WayXSec*> > &listWayXSec);

    // removeWayFromSharedNodes
    // * remove all nodes belonging to way from shared nodes list
    void removeWayFromSharedNodes(ListSharedNodes &listSharedNodes,
                                  osmscout::WayRef const &wayRef);

    std::string                                m_stylePath;
    std::vector<DataSet*>                      m_listDataSets;

    // render style config list (todo shouldnt this be <RenderStyleConfig const *>)?
    std::vector<RenderStyleConfig*>            m_listRenderStyleConfigs;

    // camera vars
    Camera m_camera;
    Vec3 m_data_exTL;
    Vec3 m_data_exTR;
    Vec3 m_data_exBR;
    Vec3 m_data_exBL;

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

    // convIntToStr
    std::string convIntToStr(int number);
    std::string convIntToStr(size_t number);

    // [2d]

    // calcTriangleSurfArea
    // * computes the surface area of a triangle
    double calcTriangleArea(Vec3 const &vxA,
                            Vec3 const &vxB,
                            Vec3 const &vxC);

    // calcRectOverlapArea [unused]
    // * checks whether or not two rectangles overlap and
    //   returns the area of the overlapping rectangle
    double calcRectOverlapArea(double r1_bl_x, double r1_bl_y,
                               double r1_tr_x, double r1_tr_y,
                               double r2_bl_x, double r2_bl_y,
                               double r2_tr_x, double r2_tr_y);

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

    // calcPointInPoly
    bool calcPointInPoly(std::vector<Vec2> const &listVx,
                         Vec2 const &vxTest);

    // calcQuadraticEquationReal
    // * computes the solutions to a quadratic equation with
    //   parameters a, b and c
    //   note: doesn't work with complex roots (will save empty vector)
    void calcQuadraticEquationReal(double a, double b, double c,
                                   std::vector<double> &listRoots);

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

    // calcSimplePolyCentroid
    void calcSimplePolyCentroid(std::vector<Vec2> const &listVx,
                                Vec2 &vxCentroid);


    // [3d]

    // calcPolylineLength
    // * computes the length of a series of ordered points
    double calcPolylineLength(std::vector<Vec3> const &listVx);

    // calcPolylineSegmentDist
    // * computes the distance from the staring vertex
    //   to the end of each segment in a polyline
    void calcPolylineSegmentDist(std::vector<Vec3> const &listVx,
                                 std::vector<double> &listSegLengths);

    // calcPolylineVxAtDist
    // * finds the vertex at a given distance along a
    //   polyline by interpolating along segments
    void calcPolylineVxAtDist(std::vector<Vec3> const &listVx,
                              double const polylineDist,
                              Vec3 &vxAtDist);

    // calcPolylineTrimmed
    // * trims the start and end of a polyine according
    //   to the provided start and end distances
    void calcPolylineTrimmed(std::vector<Vec3> const &listVx,
                             double distStart,double distEnd,
                             std::vector<Vec3> &listVxTrim);

    // calcPolylineResample
    // * resamples a polyline by adding vertices according
    //   to the specified spacing distance
    // * the resampling preserves existing vertices so the
    //   distance between vertices won't always be equal
    void calcPolylineResample(std::vector<Vec3> const &listVx,
                              double const distResample,
                              std::vector<Vec3> &listVxRes);

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

    // calcMinPointPlaneDistance
    // * computes the minimum distance between a given
    //   point and plane
    double calcMinPointPlaneDistance(Vec3 const &distalPoint,
                                     Vec3 const &planePoint,
                                     Vec3 const &planeNormal);

    // calcMinLineLineDistance
    // * computes the minimum distance between two line segments
    double calcMinLineLineDistance(Vec3 const &seg1_p1,
                                   Vec3 const &seg1_p2,
                                   Vec3 const &seg2_p1,
                                   Vec3 const &seg2_p2);

    // calcPointLiesAlongRay
    // * check if a given point lies on/in the specified ray
    // * the ray's direction vector is taken into account
    bool calcPointLiesAlongRay(Vec3 const &distalPoint,
                               Vec3 const &rayPoint,
                               Vec3 const &rayDirn);

    // calcEstSkewLineProj
    // * given two skew lines, calculates the projection of the
    //   second line onto the first (the intersection point if the
    //   lines were touching) -- used to estimate the int. pt of
    //   two lines in 3d that do int. or are close to intersecting
    bool calcEstSkewLineProj(const Vec3 &a_p1, const Vec3 &a_p2,
                             const Vec3 &b_p1, const Vec3 &b_p2,
                             Vec3 &i_p);

    // calcPointPlaneProjection
    bool calcPointPlaneProjection(const Vec3 &planeNormal,
                                  const Vec3 &planeVx,
                                  const std::vector<Vec3> &listVx,
                                  std::vector<Vec3> &listProjVx);

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

    // calcBoundsIntersection
    bool calcBoundsIntersection(std::vector<Vec3> const &listVxB1,
                                std::vector<Vec3> const &listVxB2,
                                std::vector<Vec3> &listVxROI,
                                Vec3 &vxROICentroid);

    // [geo]

    // calcECEFNorthEastDown
    // * calculate direction vectors in ECEF along North,
    //   East and Down given Latitude,Longitude
    void calcECEFNorthEastDown(PointLLA const &pointLLA,
                               Vec3 &vecNorth,
                               Vec3 &vecEast,
                               Vec3 &vecDown);

    // calcGeographicDistance
    // * finds the coordinate that is 'distanceMeters' out from
    //   the starting point at a bearing of 'bearingDegrees'
    // * bearing is degrees CW from North
    // * assumes that Earth is a spheroid, should be good
    //   enough for an approximation
    void calcGeographicDistance(PointLLA const &pointStart,
                                double bearingDegrees,
                                double distanceMeters,
                                PointLLA &pointDest);

    //
    void calcDistBoundingBox(PointLLA const &ptCenter,
                             double distMeters,
                             Vec3 &exTL,Vec3 &exTR,
                             Vec3 &exBR,Vec3 &exBL);

    // calcEnclosingGeoBounds
    void calcEnclosingGeoBounds(std::vector<Vec3> const &listPolyVx,
                                std::vector<GeoBounds> &listBounds);

    /*
    void calcEnclosingGeoBounds(std::vector<Vec3> const &listPolyVx,
                                std::vector<GeoBounds> &listBounds,
                                Vec3 const &vxCentroid);
                                */

    // [camera]

    // calcCameraViewExtents
    // * uses the camera's view frustum to find the view
    //   extents of the camera, in terms of a lat/lon box
    // * return true if the Earth is visible with the
    //   camera's parameters, else return false
    bool calcCamViewExtents(Camera &cam);

    // getCameraMinViewDist
    void calcCamViewDistances(double &minViewDist,
                              double &maxViewDist);


    // [misc]

    // calcEstBuildingHeight
    // * uses the building's area to estimate a height;
    //   grossly inaccurate but provides a reasonable
    //   visual effect for rendering buildings
    double calcEstBuildingHeight(double baseArea);

    // calcValidAngle
    void calcValidAngle(double &angle);

    // calcRainbowGradient
    // * calculates rgb along a rainbow gradient:
    //   red->yellow->green->cyan->blue->violet
    // * cVal should be in between 0 and 1
    ColorRGBA calcRainbowGradient(double cVal);

    // [builders]

    // buildPolylineAsTriStrip
    // * converts a set of points and a given width to
    //   '3d' as a triangle strip
    // * calculates vertices, texcoords and total length
    void buildPolylineAsTriStrip(std::vector<Vec3> const &listPolylineVx,
                                 double const polylineWidth,
                                 std::vector<Vec3> &listVx,
                                 std::vector<Vec2> &listTx,
                                 double &polylineLength,
                                 bool cleanOverlaps=false);

    // buildContourSideWalls
    // * extrude a contour along the offsetHeight
    //   vector and build its side walls as tris
    void buildContourSideWalls(std::vector<Vec3> const &listContourVx,
                               Vec3 const &offsetHeight,
                               std::vector<Vec3> &listSideTriVx,
                               std::vector<Vec3> &listSideTriNx);

    // buildContourWireframe
    // * extrude a contour along the offsetHeight
    //   vector and build a wireframe along its edges
    void buildContourWireframe(std::vector<Vec3> const &listContourVx,
                               Vec3 const &offsetHeight,
                               std::vector<Vec3> &listVx,
                               std::vector<size_t> &listIx);

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

    // buildEarthSurfaceGeometry
    // * build the ellipsoid geometry of the earth
    //   in ECEF coordinate space,bounded by minLat,
    //   minLon,matLat and maxLon
    // * mesh resolution is based on lat/lonSegments
    bool buildEarthSurfaceGeometry(double minLon,double minLat,
                                   double maxLon,double maxLat,
                                   size_t lonSegments,
                                   size_t latSegments,
                                   std::vector<Vec3> &vertexArray,
                                   std::vector<Vec2> &texCoords,
                                   std::vector<size_t> &triIdx);

    // buildCoastlinePointCloud
    // * build low-res global coastline point cloud
    bool buildCoastlinePointCloud(std::string const &filePath,
                                  std::vector<Vec3> &listVx);

    // buildCoastlineLines
    // * build low-res global coastline geometry as
    //   a bunch of GL_LINES
    bool buildCoastlineLines(std::string const &filePath,
                             std::vector<Vec3> &listVx,
                             std::vector<unsigned int> &listIx);

    // buildAdmin0Lines
    // * build low-res admin0 (country border) geometry
    bool buildAdmin0Lines(std::string const &filePath,
                          std::vector<Vec3> &listVx,
                          std::vector<size_t> &listIx);

    // buildLabelText
    // * builds proper label text string from style data
    std::string buildLabelText(std::string const &parseText,
                               std::string const &sName,
                               std::string const &sRef);

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
    std::string getTypeName(DataSet * dataSet,
                            osmscout::TypeId typeId);

    // debug - remove later
    void printVector(Vec3 const &myVector);
    void printLLA(PointLLA const &myPointLLA);
    void printCamera(Camera const &cam);

    // MEMBERS
    std::vector<std::string> m_listMessages;
};

}

#endif
