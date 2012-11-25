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

#include "RenderStyleReader.h"

namespace osmsrender
{

RenderStyleReader::RenderStyleReader(std::string const &filePath,
                                     osmscout::TypeConfig const * typeConfig,
                                     std::vector<RenderStyleConfig*> &listStyleConfigs,
                                     bool &opOk) :
    m_cLineStyleId(0),
    m_cFillStyleId(0),
    m_cLabelStyleId(0),
    m_cSymbolStyleId(0)
{
    opOk = false;

    for(size_t i=0; i < listStyleConfigs.size(); i++)
    {   delete listStyleConfigs[i];   }
    listStyleConfigs.clear();

    // [ROOT]
    json_t * jRoot = json_load_file(filePath.c_str(),0,&m_jsonError);
    if(!jRoot)   {   logJsonError();  return;   }

    // [STYLECONFIGS]
    json_t * jStyleConfigs = json_object_get(jRoot,"STYLECONFIGS");
    if(json_array_size(jStyleConfigs) < 1)
    {   OSRDEBUG << "ERROR: No STYLECONFIG objects found";   return;   }

    for(size_t i=0; i < json_array_size(jStyleConfigs); i++)
    {
        RenderStyleConfig * myStyleConfig = new RenderStyleConfig(typeConfig);
        listStyleConfigs.push_back(myStyleConfig);

        json_t * jStyleConfig = json_array_get(jStyleConfigs,i);

        // [minDistance and maxDistance]
        double minDist,maxDist;
        json_t * jsonMinMag = json_object_get(jStyleConfig,"minDistance");
        json_t * jsonMaxMag = json_object_get(jStyleConfig,"maxDistance");
        if(!getMagRange(jsonMinMag,jsonMaxMag,minDist,maxDist))
        {   return;   }
        myStyleConfig->SetMinDistance(minDist);
        myStyleConfig->SetMaxDistance(maxDist);

        // [NODES]
        json_t * jListNodes = json_object_get(jStyleConfig,"NODES");
        if(json_array_size(jListNodes) < 1)   {
            OSRDEBUG << "WARN: No Node styles found in range: "
                     << minDist << "-" << maxDist;
        }
        for(size_t j=0; j < json_array_size(jListNodes); j++)
        {
            json_t * jNode = json_array_get(jListNodes,j);

            // [type]
            json_t * jNodeType = json_object_get(jNode,"type");
            if(json_string_value(jNodeType) == NULL)   {
                OSRDEBUG << "ERROR: Missing Node Type " << j;
                return;
            }
            std::string strTypeId(json_string_value(jNodeType));
            osmscout::TypeId nodeType = typeConfig->GetNodeTypeId(strTypeId);
            if(nodeType == osmscout::typeIgnore)   {
                OSRDEBUG << "WARN: Unknown Node Type "
                         << strTypeId << " (Ignoring)";
                continue;
            }

            // [SymbolStyle]
            SymbolStyle symbolStyle;
            json_t * jSymbolStyle = json_object_get(jNode,"SymbolStyle");
            if(!getSymbolStyle(jSymbolStyle,symbolStyle))   {   return;   }
            myStyleConfig->SetNodeSymbolStyle(nodeType,symbolStyle);

            // [FillStyle]
            FillStyle fillStyle;
            json_t * jFillStyle = json_object_get(jNode,"FillStyle");
            if(!getFillStyle(jFillStyle,fillStyle))   {   return;   }
            myStyleConfig->SetNodeFillStyle(nodeType,fillStyle);

            // [NameLabelStyle] (optional)
            LabelStyle nameLabelStyle;
            json_t * jNameLabelStyle = json_object_get(jNode,"NameLabelStyle");
            if(!(jNameLabelStyle == NULL))   {
                if(!getLabelStyle(jNameLabelStyle,nameLabelStyle))   {   return;   }
                myStyleConfig->SetNodeNameLabelStyle(nodeType,nameLabelStyle);
            }

            // save as active node type
            myStyleConfig->SetNodeTypeActive(nodeType);
        }

        // [WAYS]
        json_t * jListWays = json_object_get(jStyleConfig,"WAYS");
        if(json_array_size(jListWays) < 1)   {
            OSRDEBUG << "WARN: No Way styles found in range: "
                     << minDist << "-" << maxDist;
        }
        for(size_t j=0; j < json_array_size(jListWays); j++)
        {
            json_t * jWay = json_array_get(jListWays,j);

            // [type]
            json_t * jWayType = json_object_get(jWay,"type");
            if(json_string_value(jWayType) == NULL)   {
                OSRDEBUG << "ERROR: Missing Way type " << j;
                return;
            }
            std::string strTypeId(json_string_value(jWayType));
            osmscout::TypeId wayType = typeConfig->GetWayTypeId(strTypeId);
            if(wayType == osmscout::typeIgnore)   {
                OSRDEBUG << "WARN: Unknown Way Type "
                         << strTypeId << " (Ignoring)";
                continue;
            }

            // [layer] (optional)
            size_t wayLayer =0;
            json_t * jWayLayer = json_object_get(jWay,"layer");
            if(jWayLayer == NULL)
            {   OSRDEBUG << "WARN: No layer specified (" << j << ")";   }
            else if(json_number_value(jWayLayer) < 0)
            {   OSRDEBUG << "WARN: Invalid layer specified (" << j << ")";   }
            else
            {   wayLayer = json_number_value(jWayLayer);   }
            myStyleConfig->SetWayLayer(wayType,wayLayer);

            // [LineStyle]
            LineStyle wayLineStyle;
            json_t * jLineStyle = json_object_get(jWay,"LineStyle");
            if(!getLineStyle(jLineStyle,wayLineStyle))   {   return;   }
            myStyleConfig->SetWayLineStyle(wayType,wayLineStyle);

            // [NameLabelStyle] (optional)
            LabelStyle wayNameLabelStyle;
            json_t * jNameLabelStyle = json_object_get(jWay,"NameLabelStyle");
            if(!(jNameLabelStyle == NULL))   {
                if(!getLabelStyle(jNameLabelStyle,wayNameLabelStyle))   {   return;   }
                myStyleConfig->SetWayNameLabelStyle(wayType,wayNameLabelStyle);
            }

            // save as active node type
            myStyleConfig->SetWayTypeActive(wayType);
        }

        // [AREAS]
        json_t * jListAreas = json_object_get(jStyleConfig,"AREAS");
        if(json_array_size(jListAreas) < 1)   {
            OSRDEBUG << "WARN: No Area Styles found in range: "
                     << minDist << "-" << maxDist;
        }
        for(size_t j=0; j < json_array_size(jListAreas); j++)
        {
            json_t * jArea = json_array_get(jListAreas,j);

            // [type]
            json_t * jAreaType = json_object_get(jArea,"type");
            if(json_string_value(jAreaType) == NULL)   {
                OSRDEBUG << "ERROR: Missing Area type " << j;
                return;
            }
            std::string strTypeId(json_string_value(jAreaType));
            osmscout::TypeId areaType = typeConfig->GetAreaTypeId(strTypeId);
            if(areaType == osmscout::typeIgnore)   {
                OSRDEBUG << "WARN: Unknown Area Type "
                         << strTypeId << " (Ignoring)";
                continue;
            }

            // [layer] (optional)
            size_t areaLayer = 0;
            json_t * jAreaLayer = json_object_get(jArea,"layer");
            if(jAreaLayer == NULL)
            {   OSRDEBUG << "WARN: No area layer specified (" << j << ")";   }
            else if(json_number_value(jAreaLayer) < 0)
            {   OSRDEBUG << "WARN: Invalid area layer specified(" << j << ")";   }
            else
            {   areaLayer = json_number_value(jAreaLayer);   }
            myStyleConfig->SetAreaLayer(areaType,areaLayer);

            // [FillStyle]
            FillStyle areaFillStyle;
            json_t * jFillStyle = json_object_get(jArea,"FillStyle");
            if(!getFillStyle(jFillStyle,areaFillStyle))   {   return;   }
            myStyleConfig->SetAreaFillStyle(areaType,areaFillStyle);

            // [NameLabelStyle] (optional)
            LabelStyle areaNameLabelStyle;
            json_t * jNameLabelStyle = json_object_get(jArea,"NameLabelStyle");
            if(!(jNameLabelStyle == NULL))   {
                if(!getLabelStyle(jNameLabelStyle,areaNameLabelStyle))   {   return;   }
                myStyleConfig->SetAreaNameLabelStyle(areaType,areaNameLabelStyle);
            }

            // save as active area type
            myStyleConfig->SetAreaTypeActive(areaType);
        }
        myStyleConfig->PostProcess();
    }

    // [PLANET]
    // * we attach planet style info to every styleconfig
    // * this is wasteful, but required if we want to
    //   maintain the current style config structure
    bool showPlanetSurf = false;
    bool showPlanetCoast = false;
    bool showPlanetAdmin0 = false;
    ColorRGBA planetSurfColor,planetCoastColor,planetAdmin0Color;
    json_t * jPlanetStyle = json_object_get(jRoot,"PLANET");
    if(jPlanetStyle == NULL)   {
        OSRDEBUG << "WARN: No Planet Style specified";
    }
    else   {
        // [surfaceColor]
        json_t * jSurfColor = json_object_get(jPlanetStyle,"surfaceColor");
        if(!(json_string_value(jSurfColor) == NULL))   {
            std::string strSurfColor(json_string_value(jSurfColor));
            showPlanetSurf = parseColorRGBA(strSurfColor,planetSurfColor);
        }

        // [coastlineColor]
        json_t * jCoastColor = json_object_get(jPlanetStyle,"coastlineColor");
        if(!(json_string_value(jCoastColor)) == NULL)   {
            std::string strCoastColor(json_string_value(jCoastColor));
            showPlanetCoast = parseColorRGBA(strCoastColor,planetCoastColor);
        }

        // [admin0Color]
        json_t * jAdmin0Color = json_object_get(jPlanetStyle,"admin0Color");
        if(!(json_string_value(jAdmin0Color)) == NULL)   {
            std::string strAdmin0Color(json_string_value(jAdmin0Color));
            showPlanetAdmin0 = parseColorRGBA(strAdmin0Color,planetAdmin0Color);
        }

        if(!showPlanetSurf)
        {   OSRDEBUG << "WARN: -> (Planet Surface won't be rendered)";   }

        if(!showPlanetCoast)
        {   OSRDEBUG << "WARN: -> (Planet Coastline won't be rendered)";   }

        if(!showPlanetAdmin0)
        {   OSRDEBUG << "WARN: -> (Planet Admin0 won't be rendered)";   }
    }
    // save planet style
    for(size_t j=0; j < listStyleConfigs.size(); j++)   {
        RenderStyleConfig * styleConfig = listStyleConfigs[j];
        styleConfig->SetPlanetShowSurface(showPlanetSurf);
        styleConfig->SetPlanetShowCoastline(showPlanetCoast);
        styleConfig->SetPlanetShowAdmin0(showPlanetAdmin0);
        styleConfig->SetPlanetSurfaceColor(planetSurfColor);
        styleConfig->SetPlanetCoastlineColor(planetCoastColor);
        styleConfig->SetPlanetAdmin0Color(planetAdmin0Color);
    }

    opOk = true;
}

void RenderStyleReader::GetDebugLog(std::vector<std::string> &listDebugMessages)
{
    for(int i=0; i < m_listMessages.size(); i++)
    {   listDebugMessages.push_back(m_listMessages.at(i));   }
}

bool RenderStyleReader::getMagRange(json_t *jsonMinMag,
                                    json_t *jsonMaxMag,
                                    double &minMag,
                                    double &maxMag)
{
    double minMagValue = json_number_value(jsonMinMag);
    double maxMagValue = json_number_value(jsonMaxMag);

    if(maxMagValue <= minMagValue)
    {   OSRDEBUG << "ERROR: Max Distance <= Min Distance";   return false;   }
    else if((maxMagValue-minMagValue) < 50)
    {   OSRDEBUG << "ERROR: Distance range < 50m";   return false;   }
    else   {
        minMag = minMagValue;
        maxMag = maxMagValue;
        return true;
    }
}

bool RenderStyleReader::getSymbolStyle(json_t *jsonSymbolStyle,
                                       SymbolStyle &symbolStyle)
{
    if(jsonSymbolStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (SymbolStyle doesn't exist)";   return false;   }

    // SymbolStyle.type
    SymbolStyleType symbolType;
    json_t * jsonSymbolType = json_object_get(jsonSymbolStyle,"type");
    if(json_string_value(jsonSymbolType) == NULL)
    {   OSRDEBUG << "WARN: -> (Missing SymbolType type)";   }
    else
    {
        std::string symbolTypeStr(json_string_value(jsonSymbolType));

        if(symbolTypeStr.compare("triangle") == 0)
        {   symbolType = SYMBOL_TRIANGLE_UP;   }
        else if(symbolTypeStr.compare("triangle_up") == 0)
        {   symbolType = SYMBOL_TRIANGLE_UP;   }
        else if(symbolTypeStr.compare("triangle_down") == 0)
        {   symbolType = SYMBOL_TRIANGLE_DOWN;   }
        else if(symbolTypeStr.compare("square") == 0)
        {   symbolType = SYMBOL_SQUARE;   }
        else if(symbolTypeStr.compare("circle") == 0)
        {   symbolType = SYMBOL_CIRCLE;   }
        else
        {   OSRDEBUG << "WARN: -> (Invalid SymbolStyle type)";   }

        symbolStyle.SetSymbolType(symbolType);
    }

    // SymbolStyle.size (optional)
    json_t * jsonSymbolSize = json_object_get(jsonSymbolStyle,"size");
    double symbolSize = json_number_value(jsonSymbolSize);
    if(symbolSize <= 0)
    {   OSRDEBUG << "WARN: -> (Invalid SymbolStyle size)";   }
    else
    {   symbolStyle.SetSymbolSize(symbolSize);   }

    // SymbolStyle.offsetHeight (optional)
    json_t * jsonSymbolHeight = json_object_get(jsonSymbolStyle,"offsetHeight");
    double offsetHeight = json_number_value(jsonSymbolHeight);
    if(offsetHeight < 0)
    {   OSRDEBUG << "WARN: -> (Invalid SymbolStyle offsetHeight)";   }
    else
    {   symbolStyle.SetOffsetHeight(offsetHeight);   }

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
    symbolStyle.SetLabelPos(labelPos);
    symbolStyle.SetId(m_cSymbolStyleId);
    m_cSymbolStyleId++;

    return true;
}

bool RenderStyleReader::getFillStyle(json_t *jsonFillStyle,
                                     FillStyle &fillStyle)
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
        {   fillStyle.SetFillColor(fillColor);   }
    }

    // FillStyle.outlineWidth
    json_t * jsonOutlineWidth = json_object_get(jsonFillStyle,"outlineWidth");
    double outlineWidth = json_number_value(jsonOutlineWidth);
    if(outlineWidth < 0)
    {   OSRDEBUG << "WARN: -> (Invalid outlineWidth value)";   }
    else
    {
        fillStyle.SetOutlineWidth(outlineWidth);

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
                {   fillStyle.SetOutlineColor(outlineColor);   }
            }
        }
    }

    fillStyle.SetId(m_cFillStyleId);
    m_cFillStyleId++;

    return true;
}

bool RenderStyleReader::getLineStyle(json_t *jsonLineStyle,
                                     LineStyle &lineStyle)
{
    if(jsonLineStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (LineStyle doesn't exist)";   return false;   }

    // LineStyle.lineWidth
    json_t * jsonLineWidth = json_object_get(jsonLineStyle,"lineWidth");
    double lineWidth = json_number_value(jsonLineWidth);
    if(lineWidth <= 0)
    {   OSRDEBUG << "WARN: -> (Invalid lineWidth value)";   }
    else
    {   lineStyle.SetLineWidth(lineWidth);   }

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
        {   lineStyle.SetLineColor(lineColor);   }
    }

    // LineStyle.outlineWidth
    json_t * jsonOutlineWidth = json_object_get(jsonLineStyle,"outlineWidth");
    double outlineWidth = json_number_value(jsonOutlineWidth);
    if(outlineWidth < 0)
    {   OSRDEBUG << "WARN: -> (Invalid outlineWidth value)";   }
    else
    {
        lineStyle.SetOutlineWidth(outlineWidth);

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
                {   lineStyle.SetOutlineColor(outlineColor);   }
            }
        }
    }

    // LineStyle.symbolWidth
    json_t * jsonSymbolWidth = json_object_get(jsonLineStyle,"symbolWidth");
    double symbolWidth = json_number_value(jsonSymbolWidth);
    if(symbolWidth < 0)
    {   OSRDEBUG << "WARN: -> (Invalid symbolWidth value)";   }
    else
    {
        lineStyle.SetSymbolWidth(symbolWidth);

        // LineStyle.symbolColor
        if(symbolWidth > 0)
        {
            json_t * jsonSymbolColor = json_object_get(jsonLineStyle,"symbolColor");
            if(json_string_value(jsonSymbolColor) == NULL)
            {   OSRDEBUG << "WARN: -> (Missing symbolColor value)";   }
            else
            {
                ColorRGBA symbolColor;
                std::string strSymbolColor(json_string_value(jsonSymbolColor));
                if(!parseColorRGBA(strSymbolColor,symbolColor))
                {   OSRDEBUG << "WARN: -> (Could not parse symbolColor";   }
                else
                {   lineStyle.SetSymbolColor(symbolColor);   }
            }
        }
    }

    // LineStyle.symbolSpacing
    json_t * jsonSymbolSpacing = json_object_get(jsonLineStyle,"symbolSpacing");
    double symbolSpacing = json_number_value(jsonSymbolSpacing);
    if(symbolSpacing < 0)
    {   OSRDEBUG << "WARN: -> (Invalid symbolSpacing value)";   }
    else
    {   lineStyle.SetSymbolSpacing(symbolSpacing);   }


    // LineStyle.dashSpacing
    json_t * jsonDashSpacing = json_object_get(jsonLineStyle,"dashSpacing");
    double dashSpacing = json_number_value(jsonDashSpacing);
    if(dashSpacing < 0)
    {   OSRDEBUG << "WARN: -> (Invalid dashSpacing value)";   }
    else
    {
        lineStyle.SetDashSpacing(dashSpacing);

        if(dashSpacing > 0)
        {
            json_t * jsonDashColor = json_object_get(jsonLineStyle,"dashColor");
            if(json_string_value(jsonDashColor) == NULL)
            {   OSRDEBUG << "WARN: -> (Missing dashColor value)";   }
            else
            {
                ColorRGBA dashColor;
                std::string strDashColor(json_string_value(jsonDashColor));
                if(!parseColorRGBA(strDashColor,dashColor))
                {   OSRDEBUG << "WARN: -> (Could not parse dashColor";   }
                else
                {   lineStyle.SetDashColor(dashColor);   }
            }
        }
    }

    // save
    lineStyle.SetId(m_cLineStyleId);

    // LineStyle.Id
    m_cLineStyleId++;

    return true;
}

bool RenderStyleReader::getLabelStyle(json_t *jsonLabelStyle,
                                      LabelStyle &labelStyle)
{
    if(jsonLabelStyle == NULL)
    {   OSRDEBUG << "ERROR: -> (LabelStyle doesn't exist)";   return false;   }

    // LabelStyle.fontFamily (must be explicitly specified)
    json_t * jsonFontFamily = json_object_get(jsonLabelStyle,"fontFamily");
    if(json_string_value(jsonFontFamily) == NULL)
    {   OSRDEBUG << "ERROR: -> (Missing fontFamily)";   return false;   }
    std::string fontFamily(json_string_value(jsonFontFamily));
    labelStyle.SetFontFamily(fontFamily);

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
        {   labelStyle.SetFontColor(fontColor);   }
    }

    // LabelStyle.fontSize
    json_t * jsonFontSize = json_object_get(jsonLabelStyle,"fontSize");
    double fontSize = json_number_value(jsonFontSize);
    if(fontSize <= 0)
    {   OSRDEBUG << "WARN: -> (Invalid fontSize)";   }
    else
    {   labelStyle.SetFontSize(fontSize);   }

    // LabelStyle.type
    LabelStyleType labelType;
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

        labelStyle.SetLabelType(labelType);
    }

    // LabelStyle.contourPadding
    if(labelType == LABEL_CONTOUR)
    {
        json_t * jsonContourPadding = json_object_get(jsonLabelStyle,"contourPadding");
        double contourPadding = json_number_value(jsonContourPadding);
        if(contourPadding <= 0)
        {   OSRDEBUG << "WARN: -> (Invalid contourPadding)";   }
        else
        {   labelStyle.SetContourPadding(contourPadding);   }
    }

    // LabelStyle.offsetDist
    if(labelType == LABEL_DEFAULT || labelType == LABEL_PLATE)
    {
        json_t * jsonOffsetDist= json_object_get(jsonLabelStyle,"offsetDist");
        double offsetDist = json_number_value(jsonOffsetDist);
        if(offsetDist < 0)
        {   OSRDEBUG << "WARN: -> (Invalid offsetDist)";   }
        else
        {   labelStyle.SetOffsetDist(offsetDist);   }
    }

    if(labelType == LABEL_PLATE)
    {
        // LabelStyle.platePadding
        json_t *jsonPlatePadding = json_object_get(jsonLabelStyle,"platePadding");
        double platePadding = json_number_value(jsonPlatePadding);
        if(platePadding < 0)
        {   OSRDEBUG << "WARN: -> (Invalid platePadding)";   }
        else
        {   labelStyle.SetPlatePadding(platePadding);   }

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
            {   labelStyle.SetPlateColor(plateColor);   }
        }

        // LabelStyle.plateOutlineWidth
        json_t *jsonPlateOutlineWidth = json_object_get(jsonLabelStyle,"plateOutlineWidth");
        double plateOutlineWidth = json_number_value(jsonPlateOutlineWidth);
        if(plateOutlineWidth < 0)
        {   OSRDEBUG << "WARN: -> (Invalid plateOutlineWidth)";   }
        else
        {
            labelStyle.SetPlateOutlineWidth(plateOutlineWidth);

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
                    {   labelStyle.SetPlateOutlineColor(plateOutlineColor);   }
                }
            }
        }
    }

    labelStyle.SetId(m_cLabelStyleId);
    m_cLabelStyleId++;

    return true;
}

bool RenderStyleReader::parseColorRGBA(std::string const &strColor,
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

void RenderStyleReader::logJsonError()
{
    std::string errorText(m_jsonError.text);
    std::string errorLine(convIntToString(m_jsonError.line));
    std::string errorPos(convIntToString(m_jsonError.position));
    OSRDEBUG << "ERROR: Parsing JSON: Text: " << errorText;
    OSRDEBUG << "ERROR: Parsing JSON: Line: " << errorLine;
    OSRDEBUG << "ERROR: Parsing JSON: Pos: "  <<errorPos;
}

std::string RenderStyleReader::convIntToString(int myInt)
{
    std::stringstream ss;
    std::string str;
    ss << myInt;
    ss >> str;
    return str;
}

}
