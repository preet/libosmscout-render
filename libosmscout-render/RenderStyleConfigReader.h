#ifndef RENDERSTYLECONFIGREADER_H
#define RENDERSTYLECONFIGREADER_H

// stl includes
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

// jansson (json parser)
#include <jansson.h>

//osmscout includes
#include <osmscout/TypeConfig.h>
#include "RenderStyleConfig.hpp"

namespace osmscout
{

class RenderStyleConfigReader
{
public:
    RenderStyleConfigReader(const char* filePath,
                            TypeConfig * typeConfig,
                            std::vector<RenderStyleConfig*> & renderStyle);
    //~RenderStyleConfigReader();

    bool HasErrors();
    void GetErrors(std::vector<std::string> &listErrors);

private:
    bool getMagRange(json_t* jsonMinMag, json_t* jsonMaxMag,
                     double &minMag, double &maxMag);

    bool getLineRenderStyle(json_t *jsonLineStyle,
                            LineRenderStyle &lineRenderStyle);

    bool getLabelRenderStyle(json_t *jsonLabelStyle,
                             LabelRenderStyle &labelRenderStyle);

    bool parseColorRGBA(std::string const &strColor,
                        ColorRGBA &myColor);

    void logJsonError();
    void logError(std::string const &);
    std::string convIntToString(int myInt);

    bool m_hasErrors;
    json_error_t m_jsonError;
    std::vector<std::string> m_errorLog;
};

}

#endif
