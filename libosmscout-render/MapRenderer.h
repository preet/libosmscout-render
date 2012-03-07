#ifndef OSMSCOUT_MAP_RENDERER_H
#define OSMSCOUT_MAP_RENDERER_H

#include <math.h>
#include <vector>
#include <iostream>

#include <osmscout/ObjectRef.h>
#include <osmscout/Way.h>

#include "RenderStyleConfig.hpp"

// setup some constants

// PI!
#define K_PI 3.141592653589

// WGS84 ellipsoid parameters (http://en.wikipedia.org/wiki/WGS_84)
#define ELL_SEMI_MAJOR 6378137.0            // meters
#define ELL_SEMI_MAJOR_EXP2 40680631590769

#define ELL_SEMI_MINOR 6356752.3142         // meters
#define ELL_SEMI_MINOR_EXP2 40408299984087.1

#define ELL_F 1/298.257223563
#define ELL_ECC_EXP2 6.69437999014e-3
#define ELL_ECC2_EXP2 6.73949674228e-3


namespace osmscout
{

class MapRenderData
{
public:
    std::vector<WayRef>        m_listWays;
};

// longitude, latitude, altitude point class
class PointLLA
{
public:

    PointLLA() :
        lon(0),lat(0),alt(0) {}

    PointLLA(double myLat, double myLon) :
        lat(myLat),lon(myLon) {}

    PointLLA(double myLat, double myLon, double myAlt) :
        lon(myLon),lat(myLat),alt(myAlt) {}

    double lon;
    double lat;
    double alt;
};

// cartesian coordinate system vector class
// TODO if this class is too inefficient, try Eigen
class Vec3
{
public:
    Vec3() :
        x(0),y(0),z(0) {}

    Vec3(double myX, double myY, double myZ) :
        x(myX),y(myY),z(myZ) {}

    inline double Dot(Vec3 const & otherVec) const
    {
        return (x*otherVec.x)+
               (y*otherVec.y)+
               (z*otherVec.z);
    }

    inline Vec3 Cross(Vec3 const & otherVec) const
    {
        return Vec3((y*otherVec.z - z*otherVec.y),
                    (z*otherVec.x - x*otherVec.z),
                    (x*otherVec.y - y*otherVec.x));
    }

    inline double DistanceTo(Vec3 const &otherVec) const
    {
        return sqrt((x-otherVec.x)*(x-otherVec.x) +
                    (y-otherVec.y)*(y-otherVec.y) +
                    (z-otherVec.z)*(z-otherVec.z));
    }

    inline double Distance2To(Vec3 const &otherVec) const
    {
        return ((x-otherVec.x)*(x-otherVec.x) +
                (y-otherVec.y)*(y-otherVec.y) +
                (z-otherVec.z)*(z-otherVec.z));
    }

    inline Vec3 Normalized() const
    {      
        double vecMagnitude = sqrt(x*x + y*y + z*z);

        return Vec3(x/vecMagnitude,
                    y/vecMagnitude,
                    z/vecMagnitude);
    }

    inline Vec3 ScaledBy(double scaleFactor) const
    {
        return Vec3(x*scaleFactor,
                    y*scaleFactor,
                    z*scaleFactor);
    }

    inline Vec3 operator+ (const Vec3 &otherVec) const
    {
        return Vec3(x+otherVec.x,
                    y+otherVec.y,
                    z+otherVec.z);
    }

    inline Vec3 operator- (const Vec3 &otherVec) const
    {
        return Vec3(x-otherVec.x,
                    y-otherVec.y,
                    z-otherVec.z);
    }

    double x;
    double y;
    double z;
};


class MapRenderer
{
public:
    MapRenderer();
    virtual ~MapRenderer();

    //protected:
    // convLLAToECEF
    // * converts point data in Latitude/Longitude/Altitude to
    //   its corresponding X/Y/Z in ECEF coordinates
    void convLLAToECEF(PointLLA const &pointLLA, Vec3 &pointECEF);

    // convECEFToLLA
    // * converts point data in ECEF X/Y/Z to its corresponding
    //   Longitude/Latitude/Altitude coordinates
    void convECEFToLLA(Vec3 const &pointECEF, PointLLA &pointLLA);

    // convWayPathToOffsets
    // * offsets ordered way point data to give the line 'thickness'
    //   as an actual 2D polygon representing a road/street
    void convWayPathToOffsets(std::vector<Vec3> const &listWayPoints,
                              std::vector<Vec3> &listOffsetPointsA,
                              std::vector<Vec3> &listOffsetPointsB,
                              Vec3 &pointEarthCenter,
                              double lineWidth);

    // calcQuadraticEquationReal
    // * computes the solutions to a quadratic equation with
    //   parameters a, b and c, and accounts for numerical error
    //   note: doesn't work with complex roots (will save empty vector)
    void calcQuadraticEquationReal(double a, double b, double c,
                                   std::vector<double> &listRoots);

    // calcLinePlaneMinDistance
    // * computes the minimum distance between a given
    //   point and plane
    double calcMinPointPlaneDistance(Vec3 const &distalPoint,
                                     Vec3 const &planePoint,
                                     Vec3 const &planeNormal);

    // calcLinePlaneIntersection
    // * computes the intersection point between a given
    //   line and plane
    // * returns false if no intersection point exists
    bool calcLinePlaneIntersection(Vec3 const &linePoint,
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

    // ShiftRefs

    // UpdateSceneContents
    // * this method takes the active camera's altitude and view
    //   projection (onto Earth's surface) to determine the map
    //   data that should be displayed, and calls the renderer
    //   driver's functions to update the scene
    // * call this function whenever the scene's view
    //   changes through panning/zooming/rotation (NOT
    //   every frame, but at the END of a view change)
    void UpdateSceneContents(double const &camAlt,
                             PointLLA const &camViewNWCorner,
                             PointLLA const &camViewSECorner);

};


}


#endif
