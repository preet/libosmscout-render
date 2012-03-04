#include "MapRenderer.h"

namespace osmscout
{


MapRenderer::MapRenderer()
{}

MapRenderer::~MapRenderer()
{}

void MapRenderer::convLLAToECEF(const PointLLA &pointLLA, Point3D &pointECEF)
{
    // conversion formula from...
    // hxxp://www.microem.ru/pages/u_blox/tech/dataconvert/GPS.G1-X-00006.pdf

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
    // conversion formula from...
    // hxxp://www.microem.ru/pages/u_blox/tech/dataconvert/GPS.G1-X-00006.pdf

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

double MapRenderer::solveDistance(const Point3D &pointA, const Point3D &pointB)
{
    return (sqrt(pow(pointA.x-pointB.x,2) + pow(pointA.y-pointB.y,2) + pow(pointA.z-pointB.z,2)));
}

void MapRenderer::normalizeVector(Point3D &dirnVector)
{
    double vecMagnitude = sqrt(pow(dirnVector.x,2) +
                               pow(dirnVector.y,2) +
                               pow(dirnVector.z,2));

    dirnVector.x /= vecMagnitude;
    dirnVector.y /= vecMagnitude;
    dirnVector.z /= vecMagnitude;
}

void MapRenderer::solveQuadraticEquationReal(double a, double b, double c,
                                             std::vector<double> &listRoots)
{
    // check discriminant
    double myDiscriminant = b*b - 4*a*c;

    if(myDiscriminant > 0)
    {
        double qSeg1 = (-1*b)/(2*a);
        double qSeg2 = sqrt(myDiscriminant)/(2*a);
        listRoots.push_back(qSeg1+qSeg2);
        listRoots.push_back(qSeg1-qSeg2);
    }
}

void MapRenderer::solveRayEarthIntersection(const Point3D &rayPoint,
                                            const Point3D &rayDirn,
                                            Point3D &nearXsecPoint)
{
    // the solution for intersection points between a ray
    // and the Earth's surface is a quadratic equation

    // first calculate the quadratic equation params:
    // a(x^2) + b(x) + c

    // a, b and c are found by substituting the parametric
    // equation of a line into the equation for a ellipsoid
    // and solving in terms of the line's parameter

    // * http://en.wikipedia.org/wiki/Ellipsoid
    // * http://gis.stackexchange.com/questions/20780/point-of-intersection-for-a-ray-and-earths-surface

    std::vector<double> listRoots;

    double a = (pow((rayDirn.x/ELL_SEMI_MAJOR),2)) +
               (pow((rayDirn.y/ELL_SEMI_MAJOR),2)) +
               (pow((rayDirn.z/ELL_SEMI_MINOR),2));

    double b = (2*rayPoint.x*rayDirn.x/pow(ELL_SEMI_MAJOR,2)) +
               (2*rayPoint.y*rayDirn.y/pow(ELL_SEMI_MAJOR,2)) +
               (2*rayPoint.z*rayDirn.z/pow(ELL_SEMI_MINOR,2));

    double c = (pow(rayPoint.x,2) / pow(ELL_SEMI_MAJOR,2)) +
               (pow(rayPoint.y,2) / pow(ELL_SEMI_MAJOR,2)) +
               (pow(rayPoint.z,2) / pow(ELL_SEMI_MINOR,2)) - 1;

    std::cout << " A: " << a << " B: " << b << " C: " << c << std::endl;


    solveQuadraticEquationReal(a,b,c,listRoots);

    if(!listRoots.empty())
    {
        std::cout << "root1: " << listRoots.at(0) << std::endl;
        std::cout << "root2: " << listRoots.at(1) << std::endl;

        Point3D point1;
        point1.x = rayPoint.x + listRoots.at(0)*rayDirn.x;
        point1.y = rayPoint.y + listRoots.at(0)*rayDirn.y;
        point1.z = rayPoint.z + listRoots.at(0)*rayDirn.z;
        std::cout << "POI: (" << point1.x
                  << "," << point1.y
                  << "," << point1.z << ")" << std::endl;

        Point3D point2;
        point2.x = rayPoint.x + listRoots.at(1)*rayDirn.x;
        point2.y = rayPoint.y + listRoots.at(1)*rayDirn.y;
        point2.z = rayPoint.z + listRoots.at(1)*rayDirn.z;
        std::cout << "POI: (" << point2.x
                  << "," << point2.y
                  << "," << point2.z << ")" << std::endl;

        // find the point nearest to the ray's origin
        if(solveDistance(rayPoint,point1) > solveDistance(rayPoint,point2))
        {   nearXsecPoint = point2;   }
        else
        {   nearXsecPoint = point1;   }
    }
}

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
