/*
    libosmscout-render

    Copyright (C) 2012, Preet Desai

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "RenderStyleConfigReader.h"

namespace osmscout
{

RenderStyleConfigReader::RenderStyleConfigReader(std::string const &filePath,
                                                 TypeConfig * typeConfig,
                                                 std::vector<RenderStyleConfig*> &listStyleConfigs)
{
    m_hasErrors = true;

    // load style desc json file
    json_t * jsonRoot = json_load_file(filePath.c_str(), 0, &m_jsonError);

    if(!jsonRoot)
    {   logJsonError();  return;   }

    // STYLECONFIGS
    json_t * jsonStyleConfigs = json_object_get(jsonRoot,"STYLECONFIGS");
    if(json_array_size(jsonStyleConfigs) < 1)
    {   OSRDEBUG << "No STYLECONFIG objects found";   return;   }

    // keep track of style number count to set ids
    m_cLineStyleId = 0;
    m_cFillStyleId = 0;
    m_cLabelStyleId = 0;
    m_cSymbolStyleId = 0;

    for(int i=0; i < json_array_size(jsonStyleConfigs); i++)
    {
        // save new RenderStyleConfig
        RenderStyleConfig * myStyleConfig;
        myStyleConfig = new RenderStyleConfig(typeConfig);
        listStyleConfigs.push_back(myStyleConfig);

        json_t * jsonStyleConfig = json_array_get(jsonStyleConfigs,i);

        // minMag and maxMag
        double minDistance, maxDistance;
        json_t * jsonMinMag = json_object_get(jsonStyleConfig,"minDistance");
        json_t * jsonMaxMag = json_object_get(jsonStyleConfig,"maxDistance");
        if(!getMagRange(jsonMinMag,jsonMaxMag,minDistance,maxDistance))
        {   return;   }

        myStyleConfig->SetMinDistance(minDistance);
        myStyleConfig->SetMaxDistance(maxDistance);

        // TODO we shouldnt fail to read the style file if
        //      AREAS,WAYS, etc arent found (maybe throw
        //      up a warning though)

        // TODO mandate that styles must be specified in the
        //      order (nodes,ways,areas), or FAIL the reader!
        //      ... the styleIds depend on this order (actually
        //      this might not be relevant anymore, should dbl check)

        // NODES
        json_t * jsonListNodes = json_object_get(jsonStyleConfig,"NODES");
        if(json_array_size(jsonListNodes) < 1)   {
            OSRDEBUG << "WARN: No Node styles found in range "
                     << minDistance << "-" << maxDistance;
        }

        for(int j=0; j < json_array_size(jsonListNodes); j++)
        {
            json_t * jsonNode = json_array_get(jsonListNodes,j);

            // TYPE
            json_t * jsonNodeType = json_object_get(jsonNode,"type");
            if(json_string_value(jsonNodeType) == NULL)
            {   OSRDEBUG << "ERROR: Missing Node type (" << j << ")";   return;   }

            TypeId nodeType;
            std::string strTypeId(json_string_value(jsonNodeType));
            nodeType = typeConfig->GetNodeTypeId(strTypeId);
            if(nodeType == typeIgnore)
            {   OSRDEBUG << "ERROR: Unknown Node type (" << strTypeId << ")";   return;   }

            OSRDEBUG << "INFO: Style for Node Type: " << strTypeId;

            // SYMBOLSTYLE
            OSRDEBUG << "INFO: -> SymbolStyle";
            json_t *jsonSymbolRenderStyle = json_object_get(jsonNode,"SymbolStyle");
            SymbolRenderStyle symbolRenderStyle;
            if(!getSymbolRenderStyle(jsonSymbolRenderStyle,symbolRenderStyle))
            {   return;   }
            myStyleConfig->SetNodeSymbolRenderStyle(nodeType,symbolRenderStyle);

            // FILLSTYLE
            OSRDEBUG << "INFO: -> FillStyle";
            json_t * jsonFillRenderStyle = json_object_get(jsonNode,"FillStyle");
            FillRenderStyle fillRenderStyle;
            if(!getFillRenderStyle(jsonFillRenderStyle,fillRenderStyle))
            {   return;   }
            myStyleConfig->SetNodeFillRenderStyle(nodeType,fillRenderStyle);

            // NAMELABELSTYLE (optional)
            OSRDEBUG << "INFO: -> NameLabelStyle";
            json_t * jsonNameLabelStyle = json_object_get(jsonNode,"NameLabelStyle");
            LabelRenderStyle nameLabelRenderStyle;
            if(!(jsonNameLabelStyle == NULL))  {
                if(!getLabelRenderStyle(jsonNameLabelStyle,nameLabelRenderStyle))
                {   return;   }
                myStyleConfig->SetNodeNameLabelRenderStyle(nodeType,nameLabelRenderStyle);
            }

            // save as active node type
            myStyleConfig->SetNodeTypeActive(nodeType);
        }

        // WAYS
        json_t * jsonListWays = json_object_get(jsonStyleConfig,"WAYS");
        if(json_array_size(jsonListWays) < 1)   {
            OSRDEBUG << "WARN: No node styles found in range "
                     << minDistance << "-" << maxDistance;
        }

        for(int j=0; j < json_array_size(jsonListWays); j++)
        {
            json_t * jsonWay = json_array_get(jsonListWays,j);

            // TYPE
            json_t * jsonWayType = json_object_get(jsonWay,"type");
            if(json_string_value(jsonWayType) == NULL)
            {   OSRDEBUG << "ERROR: Missing Way type (" << j << ")";   return;   }

            TypeId wayType;
            std::string strTypeId(json_string_value(jsonWayType));
            wayType = typeConfig->GetWayTypeId(strTypeId);
            if(wayType == typeIgnore)
            {   OSRDEBUG << "ERROR: Unknown Way type (" << strTypeId << ")";   return;   }

            OSRDEBUG << "INFO: Style for Way Type: " << strTypeId;

            // LAYER (optional)
            OSRDEBUG << "INFO: -> Layer";
            unsigned int wayLayer=0;
            json_t * jsonWayLayer = json_object_get(jsonWay,"layer");
            if(jsonWayLayer == NULL)
            {   OSRDEBUG << "WARN: -> No layer specified";   }
            else if(json_number_value(jsonWayLayer) < 0)
            {   OSRDEBUG << "WARN: -> Invalid layer specified";   }
            else
            {   wayLayer = json_number_value(jsonWayLayer);   }
            myStyleConfig->SetWayLayer(wayType,wayLayer);

            // LINESTYLE
            OSRDEBUG << "INFO: -> LineStyle";
            json_t * jsonLineRenderStyle = json_object_get(jsonWay,"LineStyle");
            LineRenderStyle wayLineRenderStyle;
            if(!getLineRenderStyle(jsonLineRenderStyle,wayLineRenderStyle))
            {   return;   }
            myStyleConfig->SetWayLineRenderStyle(wayType,wayLineRenderStyle);

            // NAMELABELSTYLE (optional)
            OSRDEBUG << "INFO: -> NameLabelStyle";
            json_t * jsonLabelRenderStyle = json_object_get(jsonWay,"NameLabelStyle");
            LabelRenderStyle wayNameLabelRenderStyle;
            if(!(jsonLabelRenderStyle == NULL))   {
                if(!getLabelRenderStyle(jsonLabelRenderStyle,wayNameLabelRenderStyle))
                {   return;   }
                myStyleConfig->SetWayNameLabelRenderStyle(wayType,wayNameLabelRenderStyle);
            }

            // save as active way type
            myStyleConfig->SetWayTypeActive(wayType);
        }

        // AREAS
        json_t * jsonListAreas = json_object_get(jsonStyleConfig,"AREAS");
        if(json_array_size(jsonListAreas) < 1)   {
            OSRDEBUG << "INFO: No Area objects found in range "
                     << minDistance << "-" << maxDistance;
        }

        for(int j=0; j < json_array_size(jsonListAreas); j++)
        {
            json_t * jsonArea = json_array_get(jsonListAreas,j);

            // TYPE
            json_t * jsonAreaType = json_object_get(jsonArea,"type");
            if(json_string_value(jsonAreaType) == NULL)
            {   OSRDEBUG << "ERROR: Missing Area type (" << j << ")";   return;   }

            TypeId areaType;
            std::string strTypeId(json_string_value(jsonAreaType));
            areaType = typeConfig->GetAreaTypeId(strTypeId);
            if(areaType == typeIgnore)
            {   OSRDEBUG << "ERROR: Unknown Area type (" << strTypeId << ")";   return;   }

            OSRDEBUG << "INFO: Style for Way Type: " << strTypeId;

            // LAYER (optional)
            OSRDEBUG << "INFO: -> Layer";
            unsigned int areaLayer=0;
            json_t * jsonAreaLayer = json_object_get(jsonArea,"layer");
            if(jsonAreaLayer == NULL)
            {   OSRDEBUG << "WARN: -> No layer specified";   }
            else if(json_number_value(jsonAreaLayer) < 0)
            {   OSRDEBUG << "WARN: -> Invalid layer specified";   }
            else
            {   areaLayer = json_number_value(jsonAreaLayer);   }
            myStyleConfig->SetAreaLayer(areaType,areaLayer);

            // FILLSTYLE
            OSRDEBUG << "INFO: -> FillStyle";
            json_t * jsonFillRenderStyle = json_object_get(jsonArea,"FillStyle");
            FillRenderStyle areaFillRenderStyle;
            if(!getFillRenderStyle(jsonFillRenderStyle,areaFillRenderStyle))
            {   return;   }
            myStyleConfig->SetAreaFillRenderStyle(areaType,areaFillRenderStyle);

            // NAMELABELSTYLE (optional)
            OSRDEBUG << "INFO: -> NameLabelStyle";
            json_t * jsonLabelRenderStyle = json_object_get(jsonArea,"NameLabelStyle");
            LabelRenderStyle areaNameLabelRenderStyle;
            if(!(jsonLabelRenderStyle == NULL))   {
                if(!getLabelRenderStyle(jsonLabelRenderStyle,areaNameLabelRenderStyle))
                {   return;   }
                myStyleConfig->SetAreaNameLabelRenderStyle(areaType,areaNameLabelRenderStyle);
            }

            // save as active area type
            myStyleConfig->SetAreaTypeActive(areaType);
        }

        myStyleConfig->PostProcess();
    }

    m_hasErrors = false;
}

bool RenderStyleConfigReader::HasErrors()
{   return m_hasErrors;   }

void RenderStyleConfigReader::GetDebugLog(std::vector<std::string> &listDebugMessages)
{
    for(int i=0; i < m_listMessages.size(); i++)
    {   listDebugMessages.push_back(m_listMessages.at(i));   }
}

bool RenderStyleConfigReader::getMagRange(json_t *jsonMinMag, json_t *jsonMaxMag,
                                          double &minMag, double &maxMag)
{
    double minMagValue = json_number_value(jsonMinMag);
    double maxMagValue = json_number_value(jsonMaxMag);

    if(maxMagValue <= minMagValue)
    {   OSRDEBUG << "ERROR: Max Distance <= Min Distance";   return false;   }
    else if((maxMagValue-minMagValue) < 50)
    {   OSRDEBUG << "ERROR: Distance range < 50m";   return false;   }
    else
    {
        minMag = minMagValue;
        maxMag = maxMagValue;
        return true;
    }
}

bool RenderStyleConfigReader::getSymbolRenderStyle(json_t *jsonSymbolStyle,
                                                   SymbolRenderStyle &symbolRenderStyle)
{
    if(jsonSymbolStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (SymbolStyle doesn't exist)";   return false;   }

    // SymbolStyle.type
    SymbolRenderStyleType symbolType;
    json_t * jsonSymbolType = json_object_get(jsonSymbolStyle,"type");
    if(json_string_value(jsonSymbolType) == NULL)
    {   OSRDEBUG << "WARN: -> (Missing SymbolType type)";   }
    else
    {
        std::string symbolTypeStr(json_string_value(jsonSymbolType));

        if(symbolTypeStr.compare("triangle") == 0)
        {   symbolType = SYMBOL_TRIANGLE;   }
        else if(symbolTypeStr.compare("square") == 0)
        {   symbolType = SYMBOL_SQUARE;   }
        else if(symbolTypeStr.compare("circle") == 0)
        {   symbolType = SYMBOL_CIRCLE;   }
        else
        {   OSRDEBUG << "WARN: -> (Invalid SymbolStyle type)";   }

        symbolRenderStyle.SetSymbolType(symbolType);
    }

    // SymbolStyle.size (optional)
    json_t * jsonSymbolSize = json_object_get(jsonSymbolStyle,"size");
    double symbolSize = json_number_value(jsonSymbolSize);
    if(symbolSize <= 0)
    {   OSRDEBUG << "WARN: -> (Invalid SymbolStyle size)";   }
    else
    {   symbolRenderStyle.SetSymbolSize(symbolSize);   }

    // SymbolStyle.offsetHeight (optional)
    json_t * jsonSymbolHeight = json_object_get(jsonSymbolStyle,"offsetHeight");
    double offsetHeight = json_number_value(jsonSymbolHeight);
    if(offsetHeight < 0)
    {   OSRDEBUG << "WARN: -> (Invalid SymbolStyle offsetHeight)";   }
    else
    {   symbolRenderStyle.SetOffsetHeight(offsetHeight);   }

    // SymbolStyle.labelPos (optional)
    SymbolLabelPos labelPos;
    json_t * jsonLabelPos = json_object_get(jsonSymbolStyle,"labelPos");
    if(json_string_value(jsonLabelPos) == NULL)
    {   labelPos = SYMBOL_TOP;   }
    else
    {
        std::string labelPosStr(json_string_value(jsonLabelPos));

        if(labelPosStr.compare("top") == 0)
        {   labelPos = SYMBOL_TOP;   }
        else if(labelPosStr.compare("top_right") == 0)
        {   labelPos = SYMBOL_TOPRIGHT;   }
        else if(labelPosStr.compare("right") == 0)
        {   labelPos = SYMBOL_RIGHT;   }
        else if(labelPosStr.compare("btm_right") == 0)
        {   labelPos = SYMBOL_BTMRIGHT;   }
        else if(labelPosStr.compare("btm") == 0)
        {   labelPos = SYMBOL_BTM;   }
        else if(labelPosStr.compare("btm_left") == 0)
        {   labelPos = SYMBOL_BTMLEFT;   }
        else if(labelPosStr.compare("left") == 0)
        {   labelPos = SYMBOL_LEFT;   }
        else if(labelPosStr.compare("top_left") == 0)
        {   labelPos = SYMBOL_TOPLEFT;   }
        else
        {
            OSRDEBUG << "WARN: -> (Invalid SymbolStyle labelPos)";
            OSRDEBUG << "WARN: -> (labelPos must be one of top,top_right,"
                        "right,btm_right,btm,btm_left,left,top_left)";
            labelPos = SYMBOL_TOP;
        }
    }
    symbolRenderStyle.SetLabelPos(labelPos);    // defaults to 'top'

    symbolRenderStyle.SetId(m_cSymbolStyleId);
    m_cSymbolStyleId++;

    return true;
}

bool RenderStyleConfigReader::getFillRenderStyle(json_t *jsonFillStyle,
                                                 FillRenderStyle &fillRenderStyle)
{
    if(jsonFillStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (FillStyle doesn't exist)";   return false;   }

    // FillStyle.fillColors
    json_t * jsonFillColor = json_object_get(jsonFillStyle,"fillColor");
    if(json_string_value(jsonFillColor) == NULL)
    {   OSRDEBUG << "WARN: -> (Missing fillColor value)";   }
    else
    {
        ColorRGBA fillColor;
        std::string strFillColor(json_string_value(jsonFillColor));
        if(!parseColorRGBA(strFillColor,fillColor))
        {   OSRDEBUG << "WARN: -> (Could not parse fillColor string)";   }
        else
        {   fillRenderStyle.SetFillColor(fillColor);   }
    }

    // FillStyle.outlineWidth
    json_t * jsonOutlineWidth = json_object_get(jsonFillStyle,"outlineWidth");
    double outlineWidth = json_number_value(jsonOutlineWidth);
    if(outlineWidth < 0)
    {   OSRDEBUG << "WARN: -> (Invalid outlineWidth value)";   }
    else
    {
        fillRenderStyle.SetOutlineWidth(outlineWidth);

        // FillStyle.outlineColor
        if(outlineWidth > 0)
        {
            json_t * jsonOutlineColor = json_object_get(jsonFillStyle,"outlineColor");
            if(json_string_value(jsonOutlineColor) == NULL)
            {   OSRDEBUG << "WARN: -> (Missing outlineColor value)";   }
            else
            {
                ColorRGBA outlineColor;
                std::string strOutlineColor(json_string_value(jsonOutlineColor));
                if(!parseColorRGBA(strOutlineColor,outlineColor))
                {   OSRDEBUG << "WARN: -> (Could not parse outlineColor string(";   }
                else
                {   fillRenderStyle.SetOutlineColor(outlineColor);   }
            }
        }
    }

    fillRenderStyle.SetId(m_cFillStyleId);
    m_cFillStyleId++;

    return true;
}

bool RenderStyleConfigReader::getLineRenderStyle(json_t *jsonLineStyle,
                                                 LineRenderStyle &lineRenderStyle)
{
    if(jsonLineStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (LineStyle doesn't exist)";   return false;   }

    // LineStyle.lineWidth
    json_t * jsonLineWidth = json_object_get(jsonLineStyle,"lineWidth");
    double lineWidth = json_number_value(jsonLineWidth);
    if(lineWidth <= 0)
    {   OSRDEBUG << "WARN: -> (Invalid lineWidth value)";   }
    else
    {   lineRenderStyle.SetLineWidth(lineWidth);   }

    // LineStyle.lineColor
    json_t * jsonLineColor = json_object_get(jsonLineStyle,"lineColor");
    if(json_string_value(jsonLineColor) == NULL)
    {   OSRDEBUG << "WARN: -> (Missing lineColor value)";   }
    else
    {
        ColorRGBA lineColor;
        std::string strLineColor(json_string_value(jsonLineColor));
        if(!parseColorRGBA(strLineColor,lineColor))
        {   OSRDEBUG << "WARN: -> (Could not parse lineColor string)";   }
        else
        {   lineRenderStyle.SetLineColor(lineColor);   }
    }

    // LineStyle.outlineWidth
    json_t * jsonOutlineWidth = json_object_get(jsonLineStyle,"outlineWidth");
    double outlineWidth = json_number_value(jsonOutlineWidth);
    if(outlineWidth < 0)
    {   OSRDEBUG << "WARN: -> (Invalid outlineWidth value)";   }
    else
    {
        lineRenderStyle.SetOutlineWidth(outlineWidth);

        // LineStyle.outlineColor
        if(outlineWidth > 0)
        {
            json_t * jsonOutlineColor = json_object_get(jsonLineStyle,"outlineColor");
            if(json_string_value(jsonOutlineColor) == NULL)
            {   OSRDEBUG << "WARN: -> (Missing outlineColor value)";   }
            else
            {
                ColorRGBA outlineColor;
                std::string strOutlineColor(json_string_value(jsonOutlineColor));
                if(!parseColorRGBA(strOutlineColor,outlineColor))
                {   OSRDEBUG << "WARN: -> (Could not parse outlineColor)";   }
                else
                {   lineRenderStyle.SetOutlineColor(outlineColor);   }
            }
        }
    }

    // LineStyle.onewayWidth
    json_t * jsonOnewayWidth = json_object_get(jsonLineStyle,"onewayWidth");
    double onewayWidth = json_number_value(jsonOnewayWidth);
    if(onewayWidth < 0)
    {   OSRDEBUG << "WARN: -> (Invalid onewayWidth value)";   }
    else
    {
        lineRenderStyle.SetOnewayWidth(onewayWidth);

        // LineStyle.onewayColor
        if(onewayWidth > 0)
        {
            json_t * jsonOnewayColor = json_object_get(jsonLineStyle,"onewayColor");
            if(json_string_value(jsonOnewayColor) == NULL)
            {   OSRDEBUG << "WARN: -> (Missing onewayColor value)";   }
            else
            {
                ColorRGBA onewayColor;
                std::string strOnewayColor(json_string_value(jsonOnewayColor));
                if(!parseColorRGBA(strOnewayColor,onewayColor))
                {   OSRDEBUG << "WARN: -> (Could not parse onewayColor)";   }
                else
                {   lineRenderStyle.SetOnewayColor(onewayColor);   }
            }
        }
    }

    // LineStyle.onewayPadding
    json_t * jsonOnewayPadding = json_object_get(jsonLineStyle,"onewayPadding");
    double onewayPadding = json_number_value(jsonOnewayPadding);
    if(onewayPadding < 0)
    {   OSRDEBUG << "WARN: -> (Invalid onewayPadding value)";   }
    else
    {   lineRenderStyle.SetOnewayPadding(onewayPadding);   }

    // save
    lineRenderStyle.SetId(m_cLineStyleId);

    // LineStyle.Id
    m_cLineStyleId++;

    return true;
}

bool RenderStyleConfigReader::getLabelRenderStyle(json_t *jsonLabelStyle,
                                                  LabelRenderStyle &labelRenderStyle)
{
    if(jsonLabelStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (LabelStyle doesn't exist)";   return false;   }

    // LabelStyle.fontFamily (must be explicitly specified)
    json_t * jsonFontFamily = json_object_get(jsonLabelStyle,"fontFamily");
    if(json_string_value(jsonFontFamily) == NULL)
    {   OSRDEBUG << "ERROR: -> (Missing fontFamily)";   return false;   }
    std::string fontFamily(json_string_value(jsonFontFamily));
    labelRenderStyle.SetFontFamily(fontFamily);

    // LabelStyle.fontColor
    json_t * jsonFontColor = json_object_get(jsonLabelStyle,"fontColor");
    if(json_string_value(jsonFontColor) == NULL)
    {   OSRDEBUG << "WARN: -> (Missing fontColor)";   }
    else
    {
        ColorRGBA fontColor;
        std::string strFontColor(json_string_value(jsonFontColor));
        if(!parseColorRGBA(strFontColor,fontColor))
        {   OSRDEBUG << "WARN: -> (Could not parse fontColor)";   }
        else
        {   labelRenderStyle.SetFontColor(fontColor);   }
    }

    // LabelStyle.fontSize
    json_t * jsonFontSize = json_object_get(jsonLabelStyle,"fontSize");
    double fontSize = json_number_value(jsonFontSize);
    if(fontSize <= 0)
    {   OSRDEBUG << "WARN: -> (Invalid fontSize)";   }
    else
    {   labelRenderStyle.SetFontSize(fontSize);   }

    // LabelStyle.type
    LabelRenderStyleType labelType;
    json_t * jsonLabelType = json_object_get(jsonLabelStyle,"type");
    if(json_string_value(jsonLabelType) == NULL)
    {   labelType = LABEL_DEFAULT;   }
    else
    {
        std::string labelTypeStr(json_string_value(jsonLabelType));

        if(labelTypeStr.compare("default") == 0)
        {   labelType = LABEL_DEFAULT;   }

        else if(labelTypeStr.compare("plate") == 0)
        {   labelType = LABEL_PLATE;   }

        else if(labelTypeStr.compare("contour") == 0)
        {   labelType = LABEL_CONTOUR;   }

        labelRenderStyle.SetLabelType(labelType);
    }

    // LabelStyle.contourPadding
    if(labelType == LABEL_CONTOUR)
    {
        json_t * jsonContourPadding = json_object_get(jsonLabelStyle,"contourPadding");
        double contourPadding = json_number_value(jsonContourPadding);
        if(contourPadding <= 0)
        {   OSRDEBUG << "WARN: -> (Invalid contourPadding)";   }
        else
        {   labelRenderStyle.SetContourPadding(contourPadding);   }
    }

    // LabelStyle.offsetDist
    if(labelType == LABEL_DEFAULT || labelType == LABEL_PLATE)
    {
        json_t * jsonOffsetDist= json_object_get(jsonLabelStyle,"offsetDist");
        double offsetDist = json_number_value(jsonOffsetDist);
        if(offsetDist < 0)
        {   OSRDEBUG << "WARN: -> (Invalid offsetDist)";   }
        else
        {   labelRenderStyle.SetOffsetDist(offsetDist);   }
    }

    if(labelType == LABEL_PLATE)
    {
        // LabelStyle.platePadding
        json_t *jsonPlatePadding = json_object_get(jsonLabelStyle,"platePadding");
        double platePadding = json_number_value(jsonPlatePadding);
        if(platePadding < 0)
        {   OSRDEBUG << "WARN: -> (Invalid platePadding)";   }
        else
        {   labelRenderStyle.SetPlatePadding(platePadding);   }

        // LabelStyle.plateColor
        json_t * jsonPlateColor = json_object_get(jsonLabelStyle,"plateColor");
        if(json_string_value(jsonPlateColor) == NULL)
        {   OSRDEBUG << "WARN: -> (Invalid plateColor)";   }
        else
        {   ColorRGBA plateColor;
            std::string strPlateColor(json_string_value(jsonPlateColor));
            if(!parseColorRGBA(strPlateColor,plateColor))
            {   OSRDEBUG << "WARN: -> (Could not parse plateColor)";   }
            else
            {   labelRenderStyle.SetPlateColor(plateColor);   }
        }

        // LabelStyle.plateOutlineWidth
        json_t *jsonPlateOutlineWidth = json_object_get(jsonLabelStyle,"plateOutlineWidth");
        double plateOutlineWidth = json_number_value(jsonPlateOutlineWidth);
        if(plateOutlineWidth < 0)
        {   OSRDEBUG << "WARN: -> (Invalid plateOutlineWidth)";   }
        else
        {
            labelRenderStyle.SetPlateOutlineWidth(plateOutlineWidth);

            // LabelStyle.plateOutlineColor
            if(plateOutlineWidth > 0)
            {
                json_t * jsonPlateOutlineColor = json_object_get(jsonLabelStyle,"plateOutlineColor");
                if(json_string_value(jsonPlateOutlineColor) == NULL)
                {   OSRDEBUG << "WARN: -> (Invalid plateOutlineColor)";   }
                else
                {
                    ColorRGBA plateOutlineColor;
                    std::string strPlateOutlineColor(json_string_value(jsonPlateOutlineColor));
                    if(!parseColorRGBA(strPlateOutlineColor,plateOutlineColor))
                    {   OSRDEBUG << "WARN: -> (Could not parse plateOutlineColor)";   }
                    else
                    {   labelRenderStyle.SetPlateOutlineColor(plateOutlineColor);   }
                }
            }
        }
    }

    labelRenderStyle.SetId(m_cLabelStyleId);
    m_cLabelStyleId++;

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
    OSRDEBUG << errorText;
    OSRDEBUG << errorLine;
    OSRDEBUG << errorPos;
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
