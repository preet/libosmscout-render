#include "MapRenderer.h"

namespace osmscout
{


MapRenderer::MapRenderer()
{}

MapRenderer::~MapRenderer()
{}

void MapRenderer::convLLAToECEF(const PointLLA &pointLLA, Point3D &pointECEF)
{
    // remember to convert deg->rad
    double sinLat = sin(pointLLA.lat * K_PI/180.0f);
    double sinLon = sin(pointLLA.lon * K_PI/180.0f);
    double cosLat = cos(pointLLA.lat * K_PI/180.0f);
    double cosLon = cos(pointLLA.lon * K_PI/180.0f);

    // v = radius of curvature (meters)
    double v = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinLat*sinLat)));
    pointECEF.x = (v + pointLLA.alt) * cosLat * cosLon;
    pointECEF.y = (v + pointLLA.alt) * cosLat * sinLon;
    pointECEF.z = ((1-ELL_ECC_EXP2)*v + pointLLA.alt)*sinLat;
}

void MapRenderer::convECEFToLLA(const Point3D &pointECEF, PointLLA &pointLLA)
{
    double p = (sqrt(pow(pointECEF.x,2) + pow(pointECEF.y,2)));
    double th = atan2(pointECEF.z*ELL_SEMI_MAJOR, p*ELL_SEMI_MINOR);
    double sinTh = sin(th);
    double cosTh = cos(th);

    // calc longitude
    pointLLA.lon = atan2(pointECEF.y, pointECEF.x);

    // calc latitude
    pointLLA.lat = atan2(pointECEF.z + ELL_ECC2_EXP2*ELL_SEMI_MINOR*sinTh*sinTh*sinTh,
                         p - ELL_ECC_EXP2*ELL_SEMI_MAJOR*cosTh*cosTh*cosTh);
    // calc altitude
    double sinLat = sin(pointLLA.lat);
    double N = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinLat*sinLat)));
    pointLLA.alt = (p/cos(pointLLA.lat)) - N;

    // convert from rad to deg
    pointLLA.lon = pointLLA.lon * 180.0/K_PI;
    pointLLA.lat = pointLLA.lat * 180.0/K_PI;
}

//// ITERATIVE METHOD (need to test accuracy/speed later)
//void MapRenderer::convECEFToLLA(const Point3D &pointECEF, PointLLA &pointLLA)
//{
//    double p = (sqrt(pow(pointECEF.x,2) + pow(pointECEF.y,2)));
//    double th = atan2(pointECEF.z*ELL_SEMI_MAJOR, p*ELL_SEMI_MINOR);

//    pointLLA.lon = atan2(pointECEF.y, pointECEF.x);
//    pointLLA.lat = atan2(pointECEF.z, p*(1-ELL_ECC_EXP2));
//    pointLLA.alt = 0;

//    // iterate to converge (no reference for good error value)
//    double delta;   double maxError=1.0e-11;   int i=0;   int maxIter=8;
//    while (((delta > maxError) || (delta < -maxError)) && (i < maxIter))
//    {
//        double sinPrevLat = sin(pointLLA.lat * K_PI/180.0f);
//        double cosPrevLat = cos(pointLLA.lat * K_PI/180.0f);
//        double N = ELL_SEMI_MAJOR / (sqrt(1-(ELL_ECC_EXP2*sinPrevLat*sinPrevLat)));

//        double prevAlt = pointLLA.alt;
//        pointLLA.alt = (p / cosPrevLat) - N;
//        pointLLA.lat = atan2(pointECEF.z, p*(1-ELL_ECC_EXP2*(N/(N+prevAlt+1))));
//    }
//}



void MapRenderer::convWayPathToOffsets(const std::vector<Point3D> &listWayPoints,
                                       std::vector<Point3D> &listOffsetPointsA,
                                       std::vector<Point3D> &listOffsetPointsB,
                                       Point3D &pointEarthCenter,
                                       double lineWidth)
{}

void MapRenderer::UpdateSceneContents(const double &camAlt,
                                      const PointLLA &camViewNWCorner,
                                      const PointLLA &camViewSECorner)
{}


}
