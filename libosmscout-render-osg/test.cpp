#include <iostream>
#include <sys/time.h>
#include <osg/Node>
#include <osg/PolygonMode>
#include <osgViewer/Viewer>
#include <osmscout/Database.h>
#include "RenderStyleConfigReader.h"
#include "MapRendererOSG.h"


int main(int argc, char *argv[])
{
    // timing vars
    timeval t1,t2;

    // load database
    std::string dataPath("/home/preet/Documents/Maps/toronto");
    osmscout::DatabaseParameter databaseParam;
    osmscout::Database database(databaseParam);
    if(database.Open(dataPath))
    {   std::cerr << "INFO: Opened Database Successfully" << std::endl;   }
    else
    {   std::cerr << "ERROR: Could not open database";   }


    // load style data
    std::string stylePath("/home/preet/Dev/libosmscout-render/libosmscout-render/standard1.json");
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
    osmscout::MapRendererOSG mapRenderer(&database);
    mapRenderer.SetRenderStyleConfigs(listStyleConfigs);

    osmscout::PointLLA camLLA(43.6731,-79.4078, 300);
    osmscout::Vec3 camNorth,camEast,camDown;
    mapRenderer.calcECEFNorthEastDown(camLLA,camNorth,camEast,camDown);

    osmscout::Vec3 camEye = mapRenderer.convLLAToECEF(camLLA);
    osmscout::Vec3 camViewpoint(0,0,0);
    osmscout::Vec3 camViewDirn = camViewpoint-camEye;
    osmscout::Vec3 camUp = camNorth;
    double camFovY = 40.0;
    double camAspectRatio = 1.33;
    double camNearDist,camFarDist;

    // rotate
    camViewDirn = camViewDirn.RotatedBy(osmscout::Vec3(0,0,1),75);
    camViewpoint = camEye + camViewDirn;
    camUp = camUp.RotatedBy(osmscout::Vec3(0,0,1),75);

    std::cout.precision(8);

    std::cout << "camEye (" << camEye.x
              << "," << camEye.y
              << "," << camEye.z
              << ")" << std::endl;

    std::cout << "camViewpoint (" << camViewpoint.x
              << "," << camViewpoint.y
              << "," << camViewpoint.z << ")" << std::endl;

    std::cout << "camUp (" << camUp.x
              << "," << camUp.y
              << "," << camUp.z << ")" << std::endl;

    std::cout << "camFovY " << camFovY << std::endl;
    std::cout << "camAspectRatio " << camAspectRatio << std::endl;

    // initial call to UpdateSceneContents
    gettimeofday(&t1,NULL);
    mapRenderer.UpdateSceneContents(camEye,camViewpoint,camUp,
                                    camFovY,camAspectRatio,
                                    camNearDist,camFarDist);
    gettimeofday(&t2,NULL);

    double timeTaken = 0;
    timeTaken += (t2.tv_sec - t1.tv_sec) * 1000.0 * 1000.0;
    timeTaken += (t2.tv_usec - t1.tv_usec);
    std::cout << "INFO: Time taken for initial run of UpdateSceneContents():" << std::endl;
    std::cout << "INFO: > " << timeTaken << " microseconds" << std::endl;

    // render mode
    osg::PolygonMode *polygonMode = new osg::PolygonMode();
    polygonMode->setMode(osg::PolygonMode::FRONT_AND_BACK,
                         osg::PolygonMode::LINE);
    mapRenderer.m_osg_root->getOrCreateStateSet()->setAttributeAndModes(polygonMode,
        osg::StateAttribute::ON);

    mapRenderer.m_osg_root->getOrCreateStateSet()->setMode(GL_LIGHTING,
                                                           osg::StateAttribute::OFF);

    // start viewers
    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    viewer.setUpViewInWindow(100,100,800,480);
    viewer.setSceneData(mapRenderer.m_osg_root.get());
    return viewer.run();

//    // change camera and call UpdateSceneContents again
//    camViewDirn = camViewDirn.RotatedBy(osmscout::Vec3(0,0,1),20);
//    camViewpoint = camEye + camViewDirn;
//    camUp = camUp.RotatedBy(osmscout::Vec3(0,0,1),20);

//    gettimeofday(&t1,NULL);
//    mapRenderer.UpdateSceneContents(camEye,camViewpoint,camUp,
//                                    camFovY,camAspectRatio,
//                                    camNearDist,camFarDist);
//    gettimeofday(&t2,NULL);

//    timeTaken = 0;
//    timeTaken += (t2.tv_sec - t1.tv_sec) * 1000.0 * 1000.0;
//    timeTaken += (t2.tv_usec - t1.tv_usec);
//    std::cout << "INFO: Time taken for second run of UpdateSceneContents():" << std::endl;
//    std::cout << "INFO: > " << timeTaken << " microseconds" << std::endl;

    return 0;
}
