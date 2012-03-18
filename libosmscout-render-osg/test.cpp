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
    osmscout::PointLLA camLLA(43.6731,-79.4078, 300);
    mapRenderer.InitializeScene(camLLA,osmscout::CAM_ISO_NE);
    EndTiming();

    // viewer
    osgViewer::Viewer viewer;
    viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    viewer.setUpViewInWindow(100,100,800,480);
    viewer.setSceneData(mapRenderer.m_osg_root.get());
    viewer.run();

//    // start viewers
//    osgViewer::Viewer viewer;
//    viewer.setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
//    viewer.setUpViewInWindow(100,100,800,480);
//    viewer.setSceneData(mapRenderer.m_osg_root.get());
//    //viewer.setCameraManipulator(new osgGA::TrackballManipulator);
//    viewer.getCamera()->setViewMatrixAsLookAt(osg::Vec3(camEye.x,camEye.y,camEye.z),
//                                              osg::Vec3(camViewpoint.x,
//                                                        camViewpoint.y,
//                                                        camViewpoint.z),
//                                              osg::Vec3(camUp.x,camUp.y,camUp.z));

    return 0;
}
