#ifndef RENDERSTYLEREADER_H
#define RENDERSTYLEREADER_H

// stl includes
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

// jansson
#include "jansson/jansson.h"

// osmscout includes
#include <osmscout/TypeConfig.h>

// osmscout-render includes
#include "SimpleLogger.hpp"
#include "RenderStyleConfig.hpp"

namespace osmsrender
{

class RenderStyleReader
{
public:
    RenderStyleReader(std::string const &filePath,
                      osmscout::TypeConfig const * typeConfig,
                      std::vector<RenderStyleConfig*> & listStyleConfigs,
                      bool &opOk);

    void GetDebugLog(std::vector<std::string> &listDebugMessages);

private:
    bool getMagRange(json_t* jsonMinMag, json_t* jsonMaxMag,
                     double &minMag, double &maxMag);

    bool getSymbolStyle(json_t *jsonSymbolStyle,
                        SymbolStyle &symbolStyle);

    bool getFillStyle(json_t *jsonFillStyle,
                      FillStyle &fillStyle);

    bool getLineStyle(json_t *jsonLineStyle,
                      LineStyle &lineStyle);

    bool getLabelStyle(json_t *jsonLabelStyle,
                       LabelStyle &labelStyle);

    bool parseColorRGBA(std::string const &strColor,
                        ColorRGBA &myColor);

    void logJsonError();

    std::string convIntToString(int myInt);

    bool m_hasErrors;
    json_error_t m_jsonError;
    std::vector<std::string> m_listMessages;

    unsigned int m_cLineStyleId;
    unsigned int m_cFillStyleId;
    unsigned int m_cLabelStyleId;
    unsigned int m_cSymbolStyleId;
};

}

#endif
