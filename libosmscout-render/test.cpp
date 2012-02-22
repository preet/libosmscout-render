#include <iostream>

#include <osmscout/Database.h>
#include "RenderStyleConfig.hpp"
#include "RenderStyleConfigReader.h"

int main(int argc, char *argv[])
{
    osmscout::DatabaseParameter osmDbParam;
    osmscout::Database osmDb(osmDbParam);
    osmDb.Open("/home/preet/Documents/Maps/toronto");

    std::cout << "=====================================" << std::endl;
    std::cout << "=====================================" << std::endl;

    // print out list of types
    std::vector<osmscout::TypeInfo> listTypeId = osmDb.GetTypeConfig()->GetTypes();
    for(int i=0; i < listTypeId.size(); i++)
    {
        osmscout::TypeInfo myTypeInfo = listTypeId.at(i);
        std::cerr << "Type Id: " << myTypeInfo.GetId() << " | "
                  << "Type Name: " << myTypeInfo.GetName() << std::endl;
    }

    // get list of StyleConfigs
    std::string filePath("/home/preet/Dev/libosmscout-render/libosmscout-render/standard.json");
    std::vector<osmscout::RenderStyleConfig*> listStyleConfigs;
    osmscout::RenderStyleConfigReader myStyleConfigReader(filePath.c_str(),
                                                          osmDb.GetTypeConfig(),
                                                          listStyleConfigs);
    if(myStyleConfigReader.HasErrors())
    {
        std::vector<std::string> listErrors;
        myStyleConfigReader.GetErrors(listErrors);
        for(int i=0; i < listErrors.size(); i++)
        {   std::cout << listErrors.at(i) << std::endl;   }
    }

    // remember to clean up
    for(int i=0; i < listStyleConfigs.size(); i++)
    {   delete listStyleConfigs[i];   }

    std::cout << "=====================================" << std::endl;
    std::cout << "=====================================" << std::endl;
    std::cerr << "END PROGRAM!" << std::endl;
    return 0;
}
