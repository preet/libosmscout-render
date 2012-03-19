#include <iostream>
#include <sys/time.h>
#include <osg/Node>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osmscout/Database.h>
#include "RenderStyleConfigReader.h"
#include "MapRendererOSG.h"

// timing vars
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

    // init scene
    StartTiming("[Scene Initialization]");
    osmscout::PointLLA camLLA(43.655,-79.4,700);
    mapRenderer.InitializeScene(camLLA,osmscout::CAM_2D);
    EndTiming();

    // start viewer
    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    viewer.setUpViewInWindow(100,100,800,480);
    viewer.setSceneData(mapRenderer.m_osg_root.get());

    osmscout::Camera const * myCamera = mapRenderer.GetCamera();

    int loopCount = 200;
    while(!viewer.done())
    {
        osmscout::PointLLA camPos(43.655,-79.42 + (double(loopCount)/80000.0),700);
        mapRenderer.SetCamera(camPos,osmscout::CAM_2D);

        viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(myCamera->eye.x,
                                                            myCamera->eye.y,
                                                            myCamera->eye.z),

                                                  osg::Vec3(myCamera->viewPt.x,
                                                            myCamera->viewPt.y,
                                                            myCamera->viewPt.z),

                                                  osg::Vec3(myCamera->up.x,
                                                            myCamera->up.y,
                                                            myCamera->up.z));
        viewer.frame();

        loopCount+=5;
        if(loopCount > 8000)
        {   break;   }
    }

    return 0;
}
