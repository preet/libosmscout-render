#include "RenderStyleConfigReader.h"

namespace osmscout
{

RenderStyleConfigReader::RenderStyleConfigReader(const char * filePath,
                                                 TypeConfig * typeConfig,
                                                 std::vector<RenderStyleConfig*> &listStyleConfigs)
{
    // load style desc json file
    json_t * jsonRoot = json_load_file(filePath, 0, &m_jsonError);

    if(!jsonRoot)
    {   logJsonError();  return;   }

    // STYLECONFIGS
    json_t * jsonStyleConfigs = json_object_get(jsonRoot,"STYLECONFIGS");
    if(json_array_size(jsonStyleConfigs) < 1)
    {   logError("No STYLECONFIG objects found");   return;   }

    for(int i=0; i < json_array_size(jsonStyleConfigs); i++)
    {
        // save new RenderStyleConfig
        RenderStyleConfig * myStyleConfig;
        myStyleConfig = new RenderStyleConfig(typeConfig);
        listStyleConfigs.push_back(myStyleConfig);

        json_t * jsonStyleConfig = json_array_get(jsonStyleConfigs,i);

        // minMag and maxMag
        double minMag, maxMag;
        json_t * jsonMinMag = json_object_get(jsonStyleConfig,"minMag");
        json_t * jsonMaxMag = json_object_get(jsonStyleConfig,"maxMag");
        if(!getMagRange(jsonMinMag,jsonMaxMag,minMag,maxMag))
        {   return;   }

        myStyleConfig->SetMinMag(minMag);
        myStyleConfig->SetMaxMag(maxMag);

        // WAYS
        json_t * jsonListWays = json_object_get(jsonStyleConfig,"WAYS");
        if(json_array_size(jsonListWays) < 1)
        {   logError("No WAY objects found");   return;   }

        for(int j=0; j < json_array_size(jsonListWays); j++)
        {
            json_t * jsonWay = json_array_get(jsonListWays,j);

            // TYPE
            json_t * jsonWayType = json_object_get(jsonWay,"type");
            if(json_string_value(jsonWayType) == NULL)
            {   logError("Invalid Way type");   return;   }

            TypeId wayType;
            std::string strTypeId(json_string_value(jsonWayType));
            wayType = typeConfig->GetWayTypeId(strTypeId);
            if(wayType == typeIgnore)
            {   logError("Unknown Way type");   return;   }


            // PRIORITY
            json_t * jsonWayPrio = json_object_get(jsonWay,"priority");
            if(jsonWayPrio == NULL)
            {   logError("No priority found");   return;   }
            int wayPrio = json_number_value(jsonWayPrio);

            myStyleConfig->SetWayPrio(wayType,wayPrio);


            // LINESTYLE
            json_t * jsonLineRenderStyle = json_object_get(jsonWay,"LineStyle");
            LineRenderStyle wayLineRenderStyle;
            if(!getLineRenderStyle(jsonLineRenderStyle,wayLineRenderStyle))
            {   return;   }

            myStyleConfig->SetWayLineRenderStyle(wayType,wayLineRenderStyle);


            // NAMELABELSTYLE (optional)
            json_t * jsonLabelRenderStyle = json_object_get(jsonWay,"NameLabelStyle");
            LabelRenderStyle wayNameLabelRenderStyle;
            if(!(jsonLabelRenderStyle == NULL))
            {
                if(!getLabelRenderStyle(jsonLabelRenderStyle,wayNameLabelRenderStyle))
                {   return;   }

                myStyleConfig->SetWayNameLabelRenderStyle(wayType,wayNameLabelRenderStyle);
            }


            // REFLABELSTYLE (optional)
            // TODO
        }

        myStyleConfig->PostProcess();
    }
}

bool RenderStyleConfigReader::HasErrors()
{   return m_hasErrors;   }

void RenderStyleConfigReader::GetErrors(std::vector<std::string> &listErrors)
{   listErrors = m_errorLog;   }

bool RenderStyleConfigReader::getMagRange(json_t *jsonMinMag, json_t *jsonMaxMag,
                                          double &minMag, double &maxMag)
{
    double minMagValue = json_number_value(jsonMinMag);
    double maxMagValue = json_number_value(jsonMaxMag);

    if(maxMagValue <= minMagValue)
    {   logError("Invalid magnification range");   return false;   }
    else
    {
        minMag = minMagValue;
        maxMag = maxMagValue;
        return true;
    }
}

bool RenderStyleConfigReader::getLineRenderStyle(json_t *jsonLineStyle,
                                                 LineRenderStyle &lineRenderStyle)
{
    if(jsonLineStyle == NULL)
    {   logError("LineStyle doesn't exist");   return false;   }

    // LineStyle.lineWidth
    json_t * jsonLineWidth = json_object_get(jsonLineStyle,"lineWidth");
    double lineWidth = json_number_value(jsonLineWidth);

    // LineStyle.outlineWidth
    json_t * jsonOutlineWidth = json_object_get(jsonLineStyle,"outlineWidth");
    double outlineWidth = json_number_value(jsonOutlineWidth);

    // LineStyle.lineColor
    json_t * jsonLineColor = json_object_get(jsonLineStyle,"lineColor");
    if(json_string_value(jsonLineColor) == NULL)
    {   logError("Invalid lineColor value");   return false;   }

    ColorRGBA lineColor;
    std::string strLineColor(json_string_value(jsonLineColor));
    if(!parseColorRGBA(strLineColor,lineColor))
    {   logError("Could not parse lineColor string");   return false;   }

    // LineStyle.outlineColor
    json_t * jsonOutlineColor = json_object_get(jsonLineStyle,"outlineColor");
    if(json_string_value(jsonOutlineColor) == NULL)
    {   logError("Invalid outlineColor value");   return false;   }

    ColorRGBA outlineColor;
    std::string strOutlineColor(json_string_value(jsonOutlineColor));
    if(!parseColorRGBA(strOutlineColor,outlineColor))
    {   logError("Could not parse outlineColor string");   return false;   }

    // save
    lineRenderStyle.SetLineWidth(lineWidth);
    lineRenderStyle.SetOutlineWidth(outlineWidth);
    lineRenderStyle.SetLineColor(lineColor);
    lineRenderStyle.SetOutlineColor(outlineColor);

    return true;
}

bool RenderStyleConfigReader::getLabelRenderStyle(json_t *jsonLabelStyle,
                                                  LabelRenderStyle &labelRenderStyle)
{
    // LabelStyle.fontFamily
    json_t * jsonFontFamily = json_object_get(jsonLabelStyle,"fontFamily");
    if(json_string_value(jsonFontFamily) == NULL)
    {   logError("Invalid fontFamily");   return false;   }

    std::string fontFamily(json_string_value(jsonFontFamily));

    // LabelStyle.fontColor
    json_t * jsonFontColor = json_object_get(jsonLabelStyle,"fontColor");
    if(json_string_value(jsonFontColor) == NULL)
    {   logError("Invalid fontColor");   return false;   }

    ColorRGBA fontColor;
    std::string strFontColor(json_string_value(jsonFontColor));
    if(!parseColorRGBA(strFontColor,fontColor))
    {   logError("Invalid fontColor");   return false;   }

    // LabelStyle.fontSize
    json_t * jsonFontSize = json_object_get(jsonLabelStyle,"fontSize");
    double fontSize = json_number_value(jsonFontSize);

    // save
    labelRenderStyle.SetFontFamily(fontFamily);
    labelRenderStyle.SetFontColor(fontColor);
    labelRenderStyle.SetFontSize(fontSize);

    return true;
}

bool RenderStyleConfigReader::parseColorRGBA(std::string const &strColor,
                                             ColorRGBA &myColor)
{
    // expect "#RRGGBBAA"
    if(strColor.length() != 9)
    {   return false;   }

    std::string myStrColor = strColor;
    myStrColor.erase(0,1);

    for(int i=0; i < 4; i++)
    {   // 0,1,2,3
        std::string strColorByte;
        strColorByte.append(1,myStrColor.at(i*2));
        strColorByte.append(1,myStrColor.at((i*2)+1));

        std::stringstream strStream;
        int fByte;

        strStream << std::hex << strColorByte;
        strStream >> fByte;

        switch(i)
        {
        case 0:
            myColor.R = (fByte/255.0); break;
        case 1:
            myColor.G = (fByte/255.0); break;
        case 2:
            myColor.B = (fByte/255.0); break;
        case 3:
            myColor.A = (fByte/255.0); break;
        default:
            break;
        }
    }

    return true;
}

void RenderStyleConfigReader::logJsonError()
{
    std::string errorText(m_jsonError.text);
    std::string errorLine(convIntToString(m_jsonError.line));
    std::string errorPos(convIntToString(m_jsonError.position));
    m_errorLog.push_back(errorText);
    m_errorLog.push_back(errorLine);
    m_errorLog.push_back(errorPos);
    m_hasErrors = true;
}

void RenderStyleConfigReader::logError(const std::string &myError)
{
    m_errorLog.push_back(myError);
    m_hasErrors = true;
}

std::string RenderStyleConfigReader::convIntToString(int myInt)
{
    std::stringstream ss;
    std::string str;
    ss << myInt;
    ss >> str;
    return str;
}

}
