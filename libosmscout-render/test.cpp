#include <iostream>

#include <osmscout/Database.h>
#include "RenderStyleConfig.hpp"
#include "RenderStyleConfigReader.h"
#include "MapRenderer.h"


int main(int argc, char *argv[])
{
    // load database
    std::string dataPath("/home/preet/Documents/Maps/toronto");
    osmscout::DatabaseParameter databaseParam;
    osmscout::Database database(databaseParam);
    if(database.Open(dataPath))
    {   std::cerr << "INFO: Opened Database Successfully" << std::endl;   }
    else
    {   std::cerr << "ERROR: Could not open database";   }


    // load style data
    std::string stylePath("/home/preet/Dev/libosmscout-render/libosmscout-render/standard.json");
    std::vector<osmscout::RenderStyleConfig*> listStyleConfigs;
    osmscout::RenderStyleConfigReader styleConfigReader(stylePath,
                                                        database.GetTypeConfig(),
                                                        listStyleConfigs);
    if(styleConfigReader.HasErrors())
    {
        std::vector<std::string> listErrors;
        styleConfigReader.GetDebugLog(listErrors);
        for(int i=0; i < listErrors.size(); i++)
        {   std::cout << "ERROR: " << listErrors.at(i) << std::endl;   }
    }
    else
    {   std::cerr << "INFO: Read Style Configs Successfully" << std::endl;   }


    // load map renderer
    osmscout::MapRenderer mapRenderer(&database);
    mapRenderer.SetRenderStyleConfigs(listStyleConfigs);

    double minLat = 43.6342;
    double maxLat = 43.6999;
    double minLon = -79.4455;
    double maxLon = -79.3376;

    // define our camera eye above Toronto (43.685414,-79.464835)
    osmscout::Vec3 camEye;
    osmscout::PointLLA camCC((minLat+maxLat)/2,(minLon+maxLon)/2,150);
    mapRenderer.convLLAToECEF(camCC,camEye);

    std::cout.precision(8);
    std::cout << "INFO: Camera Position: ("
             << camEye.x << ","
             << camEye.y << ","
             << camEye.z << ")" << std::endl;

    // test
    osmscout::PointLLA pointStart(53,-34);
    osmscout::PointLLA pointDest;
    mapRenderer.calcGeographicDestination(pointStart,
                                          180+45,
                                          100000,
                                          pointDest);
    std::cout << "INFO: Destination Point: ("
              << pointDest.lat << "," << pointDest.lon << ")" << std::endl;


//    osmscout::Vec3 camEye(ELL_SEMI_MAJOR*2,0,0);
//    osmscout::Vec3 camViewpoint(0,ELL_SEMI_MAJOR*0.5,ELL_SEMI_MAJOR*0.5);
//    osmscout::Vec3 camUp(-0.5,0,1);
//    double camFovY = 20.0;
//    double camAspectRatio = 1.33;

//    double camNear,camFar,minLat,maxLat,minLon,maxLon;
//    bool gotViewExtents = mapRenderer.calcCameraViewExtents(camEye,
//                                                            camViewpoint,
//                                                            camUp,
//                                                            camFovY,
//                                                            camAspectRatio,
//                                                            camNear,
//                                                            camFar,
//                                                            minLat,maxLat,
//                                                            minLon,maxLon);

//    std::cout << "camEye (" << camEye.x << "  " << camEye.y
//              << "  " << camEye.z << ")" << std::endl;

//    std::cout << "camViewpoint (" << camViewpoint.x << "  " << camViewpoint.y
//              << "  " << camViewpoint.z << ")" << std::endl;

//    std::cout << "camUp (" << camUp.x << "  " << camUp.y
//              << "  " << camUp.z << ")" << std::endl;

//    std::cout << "camFovY " << camFovY << std::endl;
//    std::cout << "camAspectRatio " << camAspectRatio << std::endl;

//    if(gotViewExtents)
//    {
//        std::cout << "camNear " << camNear << std::endl;
//        std::cout << "camFar " << camFar << std::endl;
//        std::cout << "minLat: " << minLat << ", maxLat: " << maxLat << std::endl;
//        std::cout << "minLon: " << minLon << ", maxLon: " << maxLon << std::endl;

//    }
//    else
//    {   std::cout << "Failed to obtain Geographic View Extents" << std::endl;   }


//    // DEBUG OUTPUT
//    std::vector<std::string> debugLog;
//    mapRenderer.GetDebugLog(debugLog);

//    for(int i=0; i < debugLog.size(); i++)
//    {   std::cout << debugLog.at(i) << std::endl;   }


    return 0;
}
