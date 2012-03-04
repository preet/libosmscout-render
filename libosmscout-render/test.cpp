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



    // MapRenderer stuff
    osmscout::MapRenderer mapRenderer;

    // test ray-ellipsoid intersection

    osmscout::PointLLA pointLLA(90.0, 0.0, 0.0);
    osmscout::Point3D expectedPOI;
    mapRenderer.convLLAToECEF(pointLLA,expectedPOI);

    std::cout << "Expected POI: (" << expectedPOI.x
              << "," << expectedPOI.y
              << "," << expectedPOI.z << ")" << std::endl;

    osmscout::Point3D rayPoint(0.0, 0.0, 6500000);
    osmscout::Point3D rayDirn(0.0, 0.0, 1.0f);
    mapRenderer.normalizeVector(rayDirn);
    osmscout::Point3D rayIntersection;

    mapRenderer.solveRayEarthIntersection(rayPoint,
                                          rayDirn,
                                          rayIntersection);

    std::cout << "Actual POI: (" << rayIntersection.x
              << "," << rayIntersection.y
              << "," << rayIntersection.z << ")" << std::endl;



//    // sample LLA
//    osmscout::PointLLA pointLLA(43.76818075776,-79.2489659786,5);
//    osmscout::Point3D pointECEF(860580,-4532357,4389531);
//    //mapRenderer.convLLAToECEF(pointLLA,pointECEF);
//    mapRenderer.convECEFToLLA(pointECEF,pointLLA);

//    std::cout << "ECEF:" << std::endl;
//    std::cout << "X: " << pointECEF.x << std::endl;
//    std::cout << "Y: " << pointECEF.y << std::endl;
//    std::cout << "Z: " << pointECEF.z << std::endl << std::endl;

//    std::cout << "LLA:" << std::endl;
//    std::cout << "Lat: " << pointLLA.lat << std::endl;
//    std::cout << "Lon: " << pointLLA.lon << std::endl;
//    std::cout << "Alt: " << pointLLA.alt << std::endl;

    return 0;
}
