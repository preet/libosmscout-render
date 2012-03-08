#include <iostream>

#include <osmscout/Database.h>
#include "RenderStyleConfig.hpp"
#include "RenderStyleConfigReader.h"
#include "MapRenderer.h"


int main(int argc, char *argv[])
{
//    osmscout::DatabaseParameter osmDbParam;
//    osmscout::Database osmDb(osmDbParam);
//    osmDb.Open("/home/preet/Documents/Maps/toronto");

//    std::cout << "=====================================" << std::endl;
//    std::cout << "=====================================" << std::endl;

//    // print out list of types
//    std::vector<osmscout::TypeInfo> listTypeId = osmDb.GetTypeConfig()->GetTypes();
//    for(int i=0; i < listTypeId.size(); i++)
//    {
//        osmscout::TypeInfo myTypeInfo = listTypeId.at(i);
//        std::cerr << "Type Id: " << myTypeInfo.GetId() << " | "
//                  << "Type Name: " << myTypeInfo.GetName() << std::endl;
//    }

//    // get list of StyleConfigs
//    std::string filePath("/home/preet/Dev/libosmscout-render/libosmscout-render/standard.json");
//    std::vector<osmscout::RenderStyleConfig*> listStyleConfigs;
//    osmscout::RenderStyleConfigReader myStyleConfigReader(filePath.c_str(),
//                                                          osmDb.GetTypeConfig(),
//                                                          listStyleConfigs);
//    if(myStyleConfigReader.HasErrors())
//    {
//        std::vector<std::string> listErrors;
//        myStyleConfigReader.GetErrors(listErrors);
//        for(int i=0; i < listErrors.size(); i++)
//        {   std::cout << listErrors.at(i) << std::endl;   }
//    }

//    // remember to clean up
//    for(int i=0; i < listStyleConfigs.size(); i++)
//    {   delete listStyleConfigs[i];   }

//    std::cout << "=====================================" << std::endl;
//    std::cout << "=====================================" << std::endl;
//    std::cerr << "END PROGRAM!" << std::endl;



    // MapRenderer Tests

    osmscout::MapRenderer mapRenderer;

    // 4 points of intersection (all lines intersect)
    std::cout << "FULL INTERSECTION" << std::endl;
    osmscout::Vec3 camEye(ELL_SEMI_MAJOR*2,0,0);
    osmscout::Vec3 camViewpoint(0,0,0);
    osmscout::Vec3 camUp(0,0,1);
    double camFovY = 20.0;
    double camAspectRatio = 1.33;
    double camNear,camFar,minLat,maxLat,minLon,maxLon;
    bool gotViewExtents = mapRenderer.calcCameraViewExtents(camEye,
                                                            camViewpoint,
                                                            camUp,
                                                            camFovY,
                                                            camAspectRatio,
                                                            camNear,
                                                            camFar,
                                                            minLat,maxLat,
                                                            minLon,maxLon);
    if(gotViewExtents)
    {   std::cout << "1. Got View Extents" << std::endl;   }
    else
    {   std::cout << "1. Failed to get View Extents :((" << std::endl;   }


    // 2 points of intersection (partial)
    std::cout << "PARTIAL INTERSECTION(2)" << std::endl;
    camViewpoint.y = ELL_SEMI_MAJOR*1;
    gotViewExtents = mapRenderer.calcCameraViewExtents(camEye,
                                                       camViewpoint,
                                                       camUp,
                                                       camFovY,
                                                       camAspectRatio,
                                                       camNear,
                                                       camFar,
                                                       minLat,maxLat,
                                                       minLon,maxLon);
    if(gotViewExtents)
    {   std::cout << "2. Got View Extents" << std::endl;   }
    else
    {   std::cout << "2. Failed to get View Extents :((" << std::endl;   }

    // 1 points of intersection (partial)
    std::cout << "PARTIAL INTERSECTION(3)" << std::endl;
    camViewpoint.y = ELL_SEMI_MAJOR*1.5;
    camUp.y = 2;
    gotViewExtents = mapRenderer.calcCameraViewExtents(camEye,
                                                       camViewpoint,
                                                       camUp,
                                                       camFovY,
                                                       camAspectRatio,
                                                       camNear,
                                                       camFar,
                                                       minLat,maxLat,
                                                       minLon,maxLon);
    if(gotViewExtents)
    {   std::cout << "3. Got View Extents" << std::endl;   }
    else
    {   std::cout << "3. Failed to get View Extents :((" << std::endl;   }

    // No points of intersection
    std::cout << "NO INTERSECTION" << std::endl;
    camViewpoint.y = ELL_SEMI_MAJOR*2.5;
    camUp.y = 2;
    gotViewExtents = mapRenderer.calcCameraViewExtents(camEye,
                                                       camViewpoint,
                                                       camUp,
                                                       camFovY,
                                                       camAspectRatio,
                                                       camNear,
                                                       camFar,
                                                       minLat,maxLat,
                                                       minLon,maxLon);
    if(gotViewExtents)
    {   std::cout << "4. Got View Extents" << std::endl;   }
    else
    {   std::cout << "4. Failed to get View Extents :((" << std::endl;   }


    return 0;
}
