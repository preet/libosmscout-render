#include <iostream>
#include <sys/time.h>
#include <osg/Node>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osmscout/Database.h>
#include "RenderStyleConfigReader.h"
#include "MapRendererOSG.h"

// timing var
timeval t1,t2;
std::string timingDesc;

void StartTiming(std::string const &desc)
{
    timingDesc = desc;
    gettimeofday(&t1,NULL);
}

void EndTiming()
{
    gettimeofday(&t2,NULL);
    double timeTaken = 0;
    timeTaken += (t2.tv_sec - t1.tv_sec) * 1000.0 * 1000.0;
    timeTaken += (t2.tv_usec - t1.tv_usec);
    std::cout << "INFO: " << timingDesc << ": \t\t"
              << timeTaken << " microseconds" << std::endl;
}

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
    std::string stylePath("/home/preet/Dev/libosmscout-render/libosmscout-render/styles/way_node_layering.json");
    std::vector<osmscout::RenderStyleConfig*> listStyleConfigs;
    osmscout::RenderStyleConfigReader styleConfigReader(stylePath,
                                                        database.GetTypeConfig(),
                                                        listStyleConfigs);
    if(styleConfigReader.HasErrors())
    {
        std::cerr << "ERROR: Could not read Style Config" << std::endl;
        return 0;
//        std::vector<std::string> listErrors;
//        styleConfigReader.GetDebugLog(listErrors);
//        for(int i=0; i < listErrors.size(); i++)
//        {   std::cout << listErrors.at(i) << std::endl;   }
    }
    else
    {   std::cerr << "INFO: Read Style Configs Successfully" << std::endl;   }

    // load map renderer
    osmscout::MapRendererOSG mapRenderer(&database);
    mapRenderer.SetRenderStyleConfigs(listStyleConfigs);

    double minLon,minLat,maxLat,maxLon;
    minLon = -79.4076;
    maxLon = -79.3931;
    minLat = 43.6463;
    maxLat = 43.67980;

    // create camera trajectory
    // used: http://www.math.uri.edu/~bkaskosz/flashmo/parcur/
    std::vector<osmscout::PointLLA> camTrajectory;
    for(double t=0; t <= 4*K_PI; t+=(K_PI/500.0))
    {
        osmscout::PointLLA camPoint;
//        camPoint.lon = (cos(t)+1)/2.0 * (maxLon-minLon) + minLon;
//        camPoint.lat = (sin(t)+1)/2.0 * (maxLat-minLat) + minLat;

        camPoint.lon = (minLon+maxLon)/2;
        camPoint.lat = (minLat+maxLat)/2;

        camPoint.alt = (t/(4*K_PI)) * 2000 + 700;
        camTrajectory.push_back(camPoint);
    }

    // init scene
//    osmscout::PointLLA scene1(43.67,-79.4076,500);
//    StartTiming("[Scene Initialization]");
//    mapRenderer.InitializeScene(scene1,osmscout::CAM_2D);
//    EndTiming();

    osmscout::PointLLA scene2(43.66065,-79.39,1200);
    StartTiming("[Scene Initialization]");
    mapRenderer.InitializeScene(scene2,osmscout::CAM_2D);
    EndTiming();

//    osmscout::PointLLA scene3(43.67976,-79.42438,1400);
//    StartTiming("[Scene Initialization]");
//    mapRenderer.InitializeScene(scene3,osmscout::CAM_2D);
//    EndTiming();

//    osmscout::PointLLA scene4(43.72916,-79.34234,1200);
//    StartTiming("[Scene Initialization]");
//    mapRenderer.InitializeScene(scene4,osmscout::CAM_2D);
//    EndTiming();


    // setup viewersasd
    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    viewer.setUpViewInWindow(100,100,800,480);
    viewer.setSceneData(mapRenderer.m_nodeRoot.get());
    viewer.getCamera()->setClearColor(osg::Vec4(0.1,0.1,0.1,1));
    return viewer.run();

//    viewer.getCamera()->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);
//    osmscout::Camera const * myCamera = mapRenderer.GetCamera();

//    mapRenderer.InitializeScene(camTrajectory[0],osmscout::CAM_2D);

//    for(int t=1; t < camTrajectory.size(); t+=5)
//    {
//        if(!viewer.done())
//        {
//            mapRenderer.SetCamera(camTrajectory[t],osmscout::CAM_2D);

//            // camera data
////            std::cout << "INFO: Camera Lon: " << camTrajectory[t].lon << std::endl;
////            std::cout << "INFO: Camera Lat: " << camTrajectory[t].lat << std::endl;
////            std::cout << "INFO: Camera Alt: " << camTrajectory[t].alt << std::endl;

//            viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(myCamera->eye.x,
//                                                                myCamera->eye.y,
//                                                                myCamera->eye.z),

//                                                      osg::Vec3(myCamera->viewPt.x,
//                                                                myCamera->viewPt.y,
//                                                                myCamera->viewPt.z),

//                                                      osg::Vec3(myCamera->up.x,
//                                                                myCamera->up.y,
//                                                                myCamera->up.z));

//            viewer.getCamera()->setProjectionMatrixAsPerspective(30,1.33,
//                                                                 myCamera->nearDist,
//                                                                 myCamera->farDist);
//            viewer.frame();

////            std::cout << "INFO: " << mapRenderer.m_osg_osmWays->getNumChildren()
////                      << " objects in scene: " << std::endl;
//        }
//        else
//        {   break;   }
//    }

    return 0;
}
