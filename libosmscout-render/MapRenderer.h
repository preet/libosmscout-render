#ifndef OSMSCOUT_MAP_RENDERER_H
#define OSMSCOUT_MAP_RENDERER_H

#include <math.h>

#include <osmscout/ObjectRef.h>
#include <osmscout/Way.h>

#include "RenderStyleConfig.hpp"

// setup some constants

// PI!
#define K_PI 3.141592653589

// WGS84 ellipsoid parameters (http://en.wikipedia.org/wiki/WGS_84)
#define ELL_SEMI_MAJOR 6378137.0            // meters
#define ELL_SEMI_MINOR 6356752.3142         // meters
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
    PointLLA(double myLat, double myLon, double myAlt) :
        lon(myLon),lat(myLat),alt(myAlt) {}

    PointLLA(double myLat, double myLon) :
        lat(myLat),lon(myLon) {}

    double lon;
    double lat;
    double alt;
};

// cartesian coordinate system point class
class Point3D
{
public:
    Point3D() :
        x(0),y(0),z(0) {}

    Point3D(double myX, double myY, double myZ) :
        x(myX),y(myY),z(myZ) {}

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
    void convLLAToECEF(PointLLA const &pointLLA, Point3D &pointECEF);

    // convECEFToLLA
    // * converts point data in ECEF X/Y/Z to its corresponding
    //   Longitude/Latitude/Altitude coordinates
    void convECEFToLLA(Point3D const &pointECEF, PointLLA &pointLLA);

    // convWayPathToOffsets
    // * offsets ordered way point data to give the line 'thickness'
    //   as an actual 2D polygon representing a road/street
    void convWayPathToOffsets(std::vector<Point3D> const &listWayPoints,
                              std::vector<Point3D> &listOffsetPointsA,
                              std::vector<Point3D> &listOffsetPointsB,
                              Point3D &pointEarthCenter,
                              double lineWidth);

    // ShiftRef

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
