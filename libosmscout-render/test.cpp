#include <iostream>

#include <osmscout/Database.h>
#include "RenderStyleConfig.hpp"
#include "RenderStyleConfigReader.h"
#include "MapRenderer.h"


int main(int argc, char *argv[])
{
    osmscout::DatabaseParameter osmDbParam;
    osmscout::Database osmDb(osmDbParam);
    osmDb.Open("/home/preet/Documents/Maps/toronto");

    std::cout << "=====================================" << std::endl;
    std::cout << "=====================================" << std::endl;

    // print out list of types
//    std::vector<osmscout::TypeInfo> listTypeId = osmDb.GetTypeConfig()->GetTypes();
//    for(int i=0; i < listTypeId.size(); i++)
//    {
//        osmscout::TypeInfo myTypeInfo = listTypeId.at(i);
//        std::cerr << "Type Id: " << myTypeInfo.GetId() << " | "
//                  << "Type Name: " << myTypeInfo.GetName() << std::endl;
//    }

    // get list of StyleConfigs
    std::string filePath("/home/preet/Dev/libosmscout-render/libosmscout-render/standard.json");
    std::vector<osmscout::RenderStyleConfig*> listStyleConfigs;
    osmscout::RenderStyleConfigReader myStyleConfigReader(filePath.c_str(),
                                                          osmDb.GetTypeConfig(),
                                                          listStyleConfigs);
    if(myStyleConfigReader.HasErrors())
    {
        std::vector<std::string> listErrors;
        myStyleConfigReader.GetDebugLog(listErrors);
        for(int i=0; i < listErrors.size(); i++)
        {   std::cout << listErrors.at(i) << std::endl;   }
    }

    // remember to clean up
    for(int i=0; i < listStyleConfigs.size(); i++)
    {   delete listStyleConfigs[i];   }

    std::cout << "=====================================" << std::endl;
    std::cout << "=====================================" << std::endl;


//    std::cout.precision(8);

//    osmscout::MapRenderer mapRenderer("/home/preet/Documents/Maps/toronto");

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
//    std::cout << "Debug Output" << std::endl;
//    std::vector<std::string> debugLog;
//    mapRenderer.GetDebugLog(debugLog);

//    for(int i=0; i < debugLog.size(); i++)
//    {   std::cout << debugLog.at(i) << std::endl;   }


    return 0;
}
