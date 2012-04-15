#ifndef RENDERSTYLECONFIGREADER_H
#define RENDERSTYLECONFIGREADER_H

// stl includes
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

// jansson (json parser)
#include <jansson.h>

// osmscout includes
#include <osmscout/TypeConfig.h>

// osmscout-render includes
#include "SimpleLogger.hpp"
#include "RenderStyleConfig.hpp"

namespace osmscout
{

class RenderStyleConfigReader
{
public:
    RenderStyleConfigReader(std::string const &filePath,TypeConfig * typeConfig,
                            std::vector<RenderStyleConfig*> & listStyleConfigs);
    bool HasErrors();
    void GetDebugLog(std::vector<std::string> &listDebugMessages);

private:
    bool getMagRange(json_t* jsonMinMag, json_t* jsonMaxMag,
                     double &minMag, double &maxMag);

    bool getSymbolRenderStyle(json_t *jsonSymbolStyle,
                              SymbolRenderStyle &symbolRenderStyle);

    bool getFillRenderStyle(json_t *jsonFillStyle,
                            FillRenderStyle &fillRenderStyle);

    bool getLineRenderStyle(json_t *jsonLineStyle,
                            LineRenderStyle &lineRenderStyle);

    bool getLabelRenderStyle(json_t *jsonLabelStyle,
                             LabelRenderStyle &labelRenderStyle);

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
