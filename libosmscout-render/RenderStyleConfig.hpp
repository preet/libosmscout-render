#ifndef OSMSCOUT_RENDERSTYLECONFIG_HPP
#define OSMSCOUT_RENDERSTYLECONFIG_HPP

#include <set>
#include <string>
#include <limits>
#include <vector>
#include <algorithm>

#include <osmscout/Types.h>
#include <osmscout/TypeSet.h>
#include <osmscout/TypeConfig.h>

namespace osmsrender
{

    // ========================================================================== //
    // ========================================================================== //

    class ColorRGBA
    {
    public:
        ColorRGBA() : R(1), G(1), B(1), A(1)
        {}

        ColorRGBA& operator=(ColorRGBA const &otherColor)
        {
            this->R = otherColor.R;
            this->G = otherColor.G;
            this->B = otherColor.B;
            this->A = otherColor.A;
            return *this;
        }

        double R;
        double G;
        double B;
        double A;
    };

    // ========================================================================== //
    // ========================================================================== //

    enum SymbolStyleType
    {
        SYMBOL_TRIANGLE_UP,
        SYMBOL_TRIANGLE_DOWN,
        SYMBOL_SQUARE,
        SYMBOL_CIRCLE
    };

    enum SymbolLabelPos
    {
        SYMBOL_TOP,
        SYMBOL_TOPRIGHT,
        SYMBOL_RIGHT,
        SYMBOL_BTMRIGHT,
        SYMBOL_BTM,
        SYMBOL_BTMLEFT,
        SYMBOL_LEFT,
        SYMBOL_TOPLEFT
    };

    class SymbolStyle
    {
    public:
        SymbolStyle() :
            m_id(0),m_offsetHeight(0),m_symbolSize(0),
            m_symbolType(SYMBOL_SQUARE),
            m_labelPos(SYMBOL_TOP)
        {}

        SymbolStyle(SymbolStyle const &symbolStyle)
        {
            m_id            = symbolStyle.GetId();
            m_offsetHeight  = symbolStyle.GetOffsetHeight();
            m_symbolSize    = symbolStyle.GetSymbolSize();
            m_symbolType    = symbolStyle.GetSymbolType();
            m_labelPos      = symbolStyle.GetLabelPos();
        }

        void SetId(size_t symbolId)
        {   m_id = symbolId;   }

        void SetSymbolSize(double symbolSize)
        {   m_symbolSize = symbolSize;   }

        void SetOffsetHeight(double offsetHeight)
        {   m_offsetHeight = offsetHeight;   }

        void SetSymbolType(SymbolStyleType symbolType)
        {   m_symbolType = symbolType;   }

        void SetLabelPos(SymbolLabelPos labelPos)
        {   m_labelPos = labelPos;   }

        unsigned int GetId() const
        {   return m_id;   }

        double GetSymbolSize() const
        {   return m_symbolSize;   }

        double GetOffsetHeight() const
        {   return m_offsetHeight;   }

        SymbolStyleType GetSymbolType() const
        {   return m_symbolType;   }

        SymbolLabelPos GetLabelPos() const
        {   return m_labelPos;   }

    private:
        size_t m_id;
        double m_offsetHeight;
        double m_symbolSize;
        SymbolStyleType m_symbolType;
        SymbolLabelPos m_labelPos;
    };

    // ========================================================================== //
    // ========================================================================== //

    class FillStyle
    {
    public:
        FillStyle() :
            m_id(0),
            m_outlineWidth(0)
        {}

        FillStyle(FillStyle const &fillStyle)
        {
            m_id            = fillStyle.GetId();
            m_fillColor     = fillStyle.GetFillColor();
            m_outlineColor  = fillStyle.GetOutlineColor();
            m_outlineWidth  = fillStyle.GetOutlineWidth();
        }

        void SetId(size_t fillId)
        {   m_id = fillId;   }

        void SetFillColor(ColorRGBA const &fillColor)
        {   m_fillColor = fillColor;   }

        void SetOutlineColor(ColorRGBA const &outlineColor)
        {   m_outlineColor = outlineColor;   }

        void SetOutlineWidth(double outlineWidth)
        {   m_outlineWidth = outlineWidth;   }

        inline size_t GetId() const
        {   return m_id;   }

        inline ColorRGBA GetFillColor() const
        {   return m_fillColor;   }

        inline ColorRGBA GetOutlineColor() const
        {   return m_outlineColor;   }

        inline double GetOutlineWidth() const
        {   return m_outlineWidth;   }

    private:
        size_t m_id;
        ColorRGBA m_fillColor;
        ColorRGBA m_outlineColor;
        double m_outlineWidth;
    };

    // ========================================================================== //
    // ========================================================================== //

    class LineStyle
    {
    public:
        LineStyle() :
            m_id(0),
            m_lineWidth(5),
            m_outlineWidth(0),
            m_symbolWidth(0),
            m_symbolSpacing(10),
            m_dashSpacing(0)
        {}

        LineStyle(LineStyle const &lineStyle)
        {
            m_id            = lineStyle.GetId();

            m_lineWidth     = lineStyle.GetLineWidth();
            m_lineColor     = lineStyle.GetLineColor();

            m_outlineWidth  = lineStyle.GetOutlineWidth();
            m_outlineColor  = lineStyle.GetOutlineColor();

            m_symbolWidth   = lineStyle.GetSymbolWidth();
            m_symbolColor   = lineStyle.GetSymbolColor();
            m_symbolSpacing = lineStyle.GetSymbolSpacing();

            m_dashSpacing   = lineStyle.GetDashSpacing();
            m_dashColor     = lineStyle.GetDashColor();
        }

        void SetId(size_t lineId)
        {   m_id = lineId;   }

        void SetLineWidth(double lineWidth)
        {   m_lineWidth = (lineWidth > 1) ? lineWidth : 1;   }

        void SetLineColor(ColorRGBA const & lineColor)
        {   m_lineColor = lineColor;   }

        void SetOutlineWidth(double outlineWidth)
        {   m_outlineWidth = outlineWidth;   }

        void SetOutlineColor(ColorRGBA const & outlineColor)
        {   m_outlineColor = outlineColor;   }

        void SetSymbolWidth(double symbolWidth)
        {   m_symbolWidth = symbolWidth;   }

        void SetSymbolSpacing(double symbolSpacing)
        {   m_symbolSpacing = symbolSpacing;   }

        void SetSymbolColor(ColorRGBA symbolColor)
        {   m_symbolColor = symbolColor;   }

        void SetDashSpacing(double dashSpacing)
        {   m_dashSpacing = dashSpacing;   }

        void SetDashColor(ColorRGBA const &dashColor)
        {   m_dashColor = dashColor;   }

        inline size_t GetId() const
        {   return m_id;   }

        inline double GetLineWidth() const
        {   return m_lineWidth;   }

        inline ColorRGBA GetLineColor() const
        {   return m_lineColor;   }

        inline double GetOutlineWidth() const
        {   return m_outlineWidth;   }

        inline ColorRGBA GetOutlineColor() const
        {   return m_outlineColor;   }

        inline double GetSymbolWidth() const
        {   return m_symbolWidth;   }

        inline double GetSymbolSpacing() const
        {   return m_symbolSpacing;   }

        inline ColorRGBA GetSymbolColor() const
        {   return m_symbolColor;   }

        inline double GetDashSpacing() const
        {   return m_dashSpacing;   }

        inline ColorRGBA GetDashColor() const
        {   return m_dashColor;   }

    private:
        size_t      m_id;
        double      m_lineWidth;
        double      m_outlineWidth;
        double      m_symbolWidth;
        double      m_symbolSpacing;
        double      m_dashSpacing;
        ColorRGBA   m_lineColor;
        ColorRGBA   m_outlineColor;
        ColorRGBA   m_symbolColor;
        ColorRGBA   m_dashColor;
    };

    // ========================================================================== //
    // ========================================================================== //

    enum LabelStyleType
    {
        LABEL_DEFAULT,
        LABEL_PLATE,
        LABEL_CONTOUR
    };

    class LabelStyle
    {
    public:
        LabelStyle() :
            m_id(0),
            m_fontSize(10.0),
            m_fontOutlineSize(1.0),
            m_contourPadding(0.5),
            m_offsetDist(5.0),
            m_maxWidth(0.0),
            m_wayPointDist(0.0),
            m_platePadding(1.0),
            m_plateOutlineWidth(0),
            m_labelType(LABEL_DEFAULT)
        {}

        LabelStyle(LabelStyle const &labelRenderStyle)
        {
            m_id                = labelRenderStyle.GetId();
            m_fontSize          = labelRenderStyle.GetFontSize();
            m_fontColor         = labelRenderStyle.GetFontColor();
            m_fontFamily        = labelRenderStyle.GetFontFamily();
            m_fontOutlineSize   = labelRenderStyle.GetFontOutlineSize();
            m_fontOutlineColor  = labelRenderStyle.GetFontOutlineColor();
            m_labelType         = labelRenderStyle.GetLabelType();
            m_labelText         = labelRenderStyle.GetLabelText();
            m_contourPadding    = labelRenderStyle.GetContourPadding();
            m_offsetDist        = labelRenderStyle.GetOffsetDist();
            m_maxWidth          = labelRenderStyle.GetMaxWidth();
            m_wayPointDist      = labelRenderStyle.GetWayPointDist();
            m_platePadding      = labelRenderStyle.GetPlatePadding();
            m_plateColor        = labelRenderStyle.GetPlateColor();
            m_plateOutlineWidth = labelRenderStyle.GetPlateOutlineWidth();
            m_plateOutlineColor = labelRenderStyle.GetPlateOutlineColor();
        }

        // SET methods for all label types
        void SetId(size_t labelId)
        {   m_id = labelId;   }

        void SetFontSize(double fontSize)
        {   m_fontSize = fontSize;   }

        void SetFontColor(ColorRGBA const &fontColor)
        {   m_fontColor = fontColor;   }

        void SetFontFamily(const std::string &fontFamily)
        {   m_fontFamily = fontFamily;   }

        void SetFontOutlineSize(double fontOutlineSize)
        {   m_fontOutlineSize = fontOutlineSize;   }

        void SetFontOutlineColor(const ColorRGBA &fontOutlineColor)
        {   m_fontOutlineColor = fontOutlineColor;   }

        void SetLabelType(LabelStyleType labelType)
        {   m_labelType = labelType;   }

        void SetLabelText(std::string const &labelText)
        {   m_labelText = labelText;   }

        // SET methods for contour only
        void SetContourPadding(double contourPadding)
        {   m_contourPadding = contourPadding;  }

        // SET methods for default and plate
        void SetOffsetDist(double offsetDist)
        {   m_offsetDist = offsetDist;   }

        void SetMaxWidth(double maxWidth)
        {   m_maxWidth = maxWidth;   }

        void SetWayPointDist(double wayPointDist)
        {   m_wayPointDist = wayPointDist;   }

        // SET methods for plate only
        void SetPlatePadding(double platePadding)
        {   m_platePadding = platePadding;   }

        void SetPlateColor(ColorRGBA const &plateColor)
        {   m_plateColor = plateColor;   }

        void SetPlateOutlineWidth(double plateOutlineWidth)
        {   m_plateOutlineWidth = plateOutlineWidth;   }

        void SetPlateOutlineColor(ColorRGBA const &plateOutlineColor)
        {   m_plateOutlineColor = plateOutlineColor;   }

        // GET methods for all label types
        inline size_t GetId() const
        {   return m_id;   }

        inline double GetFontSize() const
        {   return m_fontSize;   }

        inline ColorRGBA GetFontColor() const
        {   return m_fontColor;   }

        inline std::string GetFontFamily() const
        {   return m_fontFamily;   }

        inline double GetFontOutlineSize() const
        {   return m_fontOutlineSize;   }

        inline ColorRGBA GetFontOutlineColor() const
        {   return m_fontOutlineColor;   }

        inline LabelStyleType GetLabelType() const
        {   return m_labelType;   }

        inline std::string GetLabelText() const
        {   return m_labelText;   }

        // GET methods for contour only
        inline double GetContourPadding() const
        {   return m_contourPadding;   }

        // GET methods for default and plate
        inline double GetOffsetDist() const
        {   return m_offsetDist;   }

        inline double GetMaxWidth()  const
        {   return m_maxWidth;   }

        inline double GetWayPointDist() const
        {   return m_wayPointDist;   }

        // GET methods for plate only
        inline double GetPlatePadding() const
        {   return m_platePadding;   }

        inline ColorRGBA GetPlateColor() const
        {   return m_plateColor;   }

        inline double GetPlateOutlineWidth() const
        {   return m_plateOutlineWidth;   }

        inline ColorRGBA GetPlateOutlineColor() const
        {   return m_plateOutlineColor;   }

    private:
        // for all label types
        size_t          m_id;
        double          m_fontSize;
        ColorRGBA       m_fontColor;
        std::string     m_fontFamily;
        double          m_fontOutlineSize;
        ColorRGBA       m_fontOutlineColor;
        LabelStyleType  m_labelType;
        std::string     m_labelText;

        // for contour types
        double          m_contourPadding;

        // for default and plate types
        double          m_offsetDist;
        double          m_maxWidth;
        double          m_wayPointDist;

        // for plate types
        double          m_platePadding;
        double          m_plateOutlineWidth;
        ColorRGBA       m_plateColor;
        ColorRGBA       m_plateOutlineColor;
    };

    // ========================================================================== //
    // ========================================================================== //

    class RenderStyleConfig
    {
    public:
        RenderStyleConfig(osmscout::TypeConfig const *typeConfig) :
            m_typeConfig(typeConfig),
            m_minDistance(0),
            m_maxDistance(250),
            m_planetShowSurface(false),
            m_planetShowCoastline(false),
            m_planetShowAdmin0(false),
            m_planetShowBuildingEdges(false)
        {
            m_numTypes = typeConfig->GetTypes().size();

            m_activeNodeTypes.resize(m_numTypes,false);
            m_nodeFillStyles.resize(m_numTypes,NULL);
            m_nodeSymbolStyles.resize(m_numTypes,NULL);
            m_nodeNameLabelStyles.resize(m_numTypes,NULL);

            m_activeWayTypes.resize(m_numTypes,false);
            m_wayLayers.resize(m_numTypes,0);
            m_wayLineStyles.resize(m_numTypes,NULL);
            m_wayNameLabelStyles.resize(m_numTypes,NULL);

            m_activeAreaTypes.resize(m_numTypes,false);
            m_areaLayers.resize(m_numTypes,0);
            m_areaFillStyles.resize(m_numTypes,NULL);
            m_areaNameLabelStyles.resize(m_numTypes,NULL);
        }

        ~RenderStyleConfig()
        {
            // clear NODE style data
            for(size_t i=0; i < m_nodeFillStyles.size(); i++)
            {
                if(!(m_nodeFillStyles[i] == NULL))
                {   delete m_nodeFillStyles[i];   }
            }

            for(size_t i=0; i < m_nodeSymbolStyles.size(); i++)
            {
                if(!(m_nodeSymbolStyles[i] == NULL))
                {   delete m_nodeSymbolStyles[i];   }
            }

            for(size_t i=0; i < m_nodeNameLabelStyles.size(); i++)
            {
                if(!(m_nodeNameLabelStyles[i] == NULL))
                {   delete m_nodeNameLabelStyles[i];   }
            }

            // clear WAY style data
            for(size_t i=0; i < m_wayLineStyles.size(); i++)
            {
                if(!(m_wayLineStyles.at(i) == NULL))
                {   delete m_wayLineStyles[i];    }
            }

            for(size_t i=0; i < m_wayNameLabelStyles.size(); i++)
            {
                if(!(m_wayNameLabelStyles.at(i) == NULL))
                {   delete m_wayNameLabelStyles[i];   }
            }


            // clear AREA style data
            for(size_t i=0; i < m_areaFillStyles.size(); i++)
            {
                if(!(m_areaFillStyles.at(i) == NULL))
                {   delete m_areaFillStyles[i];   }
            }

            for(size_t i=0; i < m_areaNameLabelStyles.size(); i++)
            {
                if(!(m_areaNameLabelStyles.at(i) == NULL))
                {   delete m_areaNameLabelStyles[i];   }
            }
        }

        void PostProcess()
        {
            // use sparsely populated property lists
            // to generate a list of unique types
            for(osmscout::TypeId i=0; i < m_numTypes; i++)
            {
                // nodes
                if(m_activeNodeTypes[i])   {
                    m_nodeTypes.push_back(i);
                    m_typeSet.SetType(i);
                }

                // ways
                if(m_activeWayTypes[i])   {
                    m_wayTypes.push_back(i);
                    m_typeSet.SetType(i);
                }

                // areas
                if(m_activeAreaTypes[i])   {
                    m_areaTypes.push_back(i);
                    m_typeSet.SetType(i);
                }
            }

            // generate font list
            LabelStyle *labelStyle;

            // m_wayNameLabelRenderStyles
            for(size_t i=0; i < m_wayTypes.size(); i++)  {
                labelStyle = m_wayNameLabelStyles[m_wayTypes[i]];

                if(!(labelStyle == NULL))
                {   m_listFonts.push_back(labelStyle->GetFontFamily());   }
            }

            // m_areaNameLabelRenderStyles
            for(size_t i=0; i < m_areaTypes.size(); i++)  {
                labelStyle = m_areaNameLabelStyles[m_areaTypes[i]];

                if(!(labelStyle == NULL))
                {   m_listFonts.push_back(labelStyle->GetFontFamily());   }
            }

            std::vector<std::string>::iterator it;
            std::sort(m_listFonts.begin(),m_listFonts.end());
            it = std::unique(m_listFonts.begin(),m_listFonts.end());
            m_listFonts.resize(it-m_listFonts.begin());

            // flip layer orders
            int maxWayLayer = this->GetMaxWayLayer()+1;
            for(size_t i=0; i < m_activeWayTypes.size(); i++)   {
                if(m_activeWayTypes[i])   {
                    m_wayLayers[i] = abs(m_wayLayers[i]-maxWayLayer);
                }
            }
            int maxAreaLayer = this->GetMaxAreaLayer()+1;
            for(size_t i=0; i < m_activeAreaTypes.size(); i++)   {
                if(m_activeAreaTypes[i])   {
                    m_areaLayers[i] = abs(m_areaLayers[i]-maxAreaLayer);
                }
            }
        }


        // Set RendererStyleConfig parameters
        void SetMinDistance(double minDistance)
        {   m_minDistance = minDistance;   }

        void SetMaxDistance(double maxDistance)
        {   m_maxDistance = maxDistance;   }


        // Set PLANET info
        void SetPlanetShowSurface(bool showSurf)
        {   m_planetShowSurface = showSurf;   }

        void SetPlanetShowCoastline(bool showCoast)
        {   m_planetShowCoastline = showCoast;   }

        void SetPlanetShowAdmin0(bool showAdmin0)
        {   m_planetShowAdmin0 = showAdmin0;   }

        void SetPlanetShowBuildingEdges(bool showBuildingEdges)
        {   m_planetShowBuildingEdges = showBuildingEdges;   }

        void SetPlanetSurfaceColor(ColorRGBA const &surfColor)
        {   m_planetSurfaceColor = surfColor;   }

        void SetPlanetCoastlineColor(ColorRGBA const &coastColor)
        {   m_planetCoastlineColor = coastColor;   }

        void SetPlanetAdmin0Color(ColorRGBA const &admin0Color)
        {   m_planetAdmin0Color = admin0Color;   }

        void SetPlanetBuildingEdgeColor(ColorRGBA const &buildingEdgeColor)
        {   m_planetBuildingEdgeColor = buildingEdgeColor;   }


        // Set NODE info
        void SetNodeTypeActive(osmscout::TypeId nodeType)
        {   m_activeNodeTypes[nodeType] = true;   }

        void SetNodeFillStyle(osmscout::TypeId nodeType, FillStyle const &fillStyle)
        {   m_nodeFillStyles[nodeType] = new FillStyle(fillStyle);   }

        void SetNodeSymbolStyle(osmscout::TypeId nodeType,SymbolStyle const &symbolStyle)
        {   m_nodeSymbolStyles[nodeType] = new SymbolStyle(symbolStyle);   }

        void SetNodeNameLabelStyle(osmscout::TypeId nodeType,LabelStyle const &labelStyle)
        {   m_nodeNameLabelStyles[nodeType] = new LabelStyle(labelStyle);   }


        // Set WAY info
        void SetWayTypeActive(osmscout::TypeId wayType)
        {   m_activeWayTypes[wayType] = true;   }

        void SetWayLayer(osmscout::TypeId wayType, size_t wayLayer)
        {   m_wayLayers[wayType] = wayLayer;   }

        void SetWayLineStyle(osmscout::TypeId wayType,LineStyle const &lineStyle)
        {   m_wayLineStyles[wayType] = new LineStyle(lineStyle);   }

        void SetWayNameLabelStyle(osmscout::TypeId wayType,LabelStyle const &labelStyle)
        {   m_wayNameLabelStyles[wayType] = new LabelStyle(labelStyle);   }


        // Set AREA info
        void SetAreaTypeActive(osmscout::TypeId areaType)
        {   m_activeAreaTypes[areaType] = true;   }

        void SetAreaLayer(osmscout::TypeId areaType, size_t areaLayer)
        {   m_areaLayers[areaType] = areaLayer;   }

        void SetAreaFillStyle(osmscout::TypeId areaType, FillStyle const &fillStyle)
        {   m_areaFillStyles[areaType] = new FillStyle(fillStyle);   }

        void SetAreaNameLabelStyle(osmscout::TypeId areaType, LabelStyle const &labelStyle)
        {   m_areaNameLabelStyles[areaType] = new LabelStyle(labelStyle);   }


        // Get RendererStyleConfig parameters
        osmscout::TypeConfig const * GetTypeConfig() const
        {   return m_typeConfig;   }

        double GetMinDistance() const
        {   return m_minDistance;   }

        double GetMaxDistance() const
        {   return m_maxDistance;   }

        void GetActiveTypes(std::vector<osmscout::TypeId> &activeTypes) const
        {   // get types that have style data
            activeTypes.clear();
            activeTypes.resize(m_nodeTypes.size() +
                               m_wayTypes.size() +
                               m_areaTypes.size());
            size_t i=0;

            for(size_t j=0; j < m_nodeTypes.size(); j++)
            {   activeTypes[i] = m_nodeTypes[j]; i++;   }

            for(size_t j=0; j < m_wayTypes.size(); j++)
            {   activeTypes[i] = m_wayTypes[j];  i++;   }

            for(size_t j=0; j < m_areaTypes.size(); j++)
            {   activeTypes[i] = m_areaTypes[j]; i++;   }
        }

        void GetActiveTypes(osmscout::TypeSet &typeSet) const
        {   typeSet = m_typeSet;   }

        void GetFontList(std::vector<std::string> &listFonts) const
        {   listFonts = m_listFonts;   }


        // Get PLANET info
        bool GetPlanetShowSurface() const
        {   return m_planetShowSurface;    }

        bool GetPlanetShowCoastline() const
        {   return m_planetShowCoastline;    }

        bool GetPlanetShowAdmin0() const
        {   return m_planetShowAdmin0;   }

        bool GetPlanetShowBuildingEdges() const
        {   return m_planetShowBuildingEdges;   }

        ColorRGBA GetPlanetSurfaceColor() const
        {   return m_planetSurfaceColor;    }

        ColorRGBA GetPlanetCoastlineColor() const
        {   return m_planetCoastlineColor;   }

        ColorRGBA GetPlanetAdmin0Color() const
        {   return m_planetAdmin0Color;   }

        ColorRGBA GetPlanetBuildingEdgeColor() const
        {   return m_planetBuildingEdgeColor;   }


        // Get NODE info
        void GetNodeTypes(std::vector<osmscout::TypeId> & nodeTypes) const
        {   nodeTypes = m_nodeTypes;   }

        bool GetNodeTypeIsValid(osmscout::TypeId nodeType) const
        {   return (m_activeNodeTypes[nodeType] == true);   }

        unsigned int GetNodeTypesCount() const
        {   return m_nodeTypes.size();   }

        FillStyle* GetNodeFillStyle(osmscout::TypeId nodeType) const
        {   return (nodeType < m_numTypes) ? m_nodeFillStyles[nodeType] : NULL;   }

        SymbolStyle* GetNodeSymbolStyle(osmscout::TypeId nodeType) const
        {   return (nodeType < m_numTypes) ? m_nodeSymbolStyles[nodeType] : NULL;   }

        LabelStyle*  GetNodeNameLabelStyle(osmscout::TypeId nodeType) const
        {   return (nodeType < m_numTypes) ? m_nodeNameLabelStyles[nodeType] : NULL;   }


        // Get WAY info
        void GetWayTypes(std::vector<osmscout::TypeId> & wayTypes) const
        {   wayTypes = m_wayTypes;   }

        bool GetWayTypeIsValid(osmscout::TypeId wayType) const
        {   return (m_activeWayTypes[wayType] == true);   }

        unsigned int GetWayTypesCount() const
        {   return m_wayTypes.size();   }

        size_t GetWayLayer(osmscout::TypeId wayType) const
        {   return m_wayLayers[wayType];   }

        size_t GetMaxWayLayer() const
        {
            size_t maxWayLayer = 0;
            for(int i=0; i < m_wayLayers.size(); i++)
            {   maxWayLayer = std::max(maxWayLayer,m_wayLayers[i]);   }

            return maxWayLayer;
        }

        LineStyle* GetWayLineStyle(osmscout::TypeId wayType) const
        {   return (wayType < m_numTypes) ? m_wayLineStyles[wayType] : NULL;   }

        LabelStyle* GetWayNameLabelStyle(osmscout::TypeId wayType) const
        {   return (wayType < m_numTypes) ? m_wayNameLabelStyles[wayType] : NULL;   }


        // Get AREA info
        void GetAreaTypes(std::vector<osmscout::TypeId> & areaTypes) const
        {   areaTypes = m_areaTypes;   }

        bool GetAreaTypeIsValid(osmscout::TypeId areaType) const
        {   return (m_activeAreaTypes[areaType] == true);   }

        unsigned int GetAreaTypesCount() const
        {   return m_areaTypes.size();   }

        size_t GetAreaLayer(osmscout::TypeId areaType) const
        {   return m_areaLayers[areaType];   }

        size_t GetMaxAreaLayer() const
        {
            size_t maxAreaLayer = 0;
            for(int i=0; i < m_areaLayers.size(); i++)
            {   maxAreaLayer = std::max(maxAreaLayer,m_areaLayers[i]);   }

            return maxAreaLayer;
        }

        FillStyle* GetAreaFillStyle(osmscout::TypeId areaType) const
        {   return (areaType < m_numTypes) ? m_areaFillStyles[areaType] : NULL;   }

        LabelStyle* GetAreaNameLabelStyle(osmscout::TypeId areaType) const
        {   return (areaType < m_numTypes) ? m_areaNameLabelStyles[areaType] : NULL;   }

    private:
        // PLANET
        bool                            m_planetShowSurface;
        bool                            m_planetShowCoastline;
        bool                            m_planetShowAdmin0;
        bool                            m_planetShowBuildingEdges;
        ColorRGBA                       m_planetSurfaceColor;
        ColorRGBA                       m_planetCoastlineColor;
        ColorRGBA                       m_planetAdmin0Color;
        ColorRGBA                       m_planetBuildingEdgeColor;

        // STYLECONFIG
        osmscout::TypeConfig const *    m_typeConfig;
        unsigned int                    m_numTypes;
        double                          m_minDistance;
        double                          m_maxDistance;

        // ALL
        osmscout::TypeSet               m_typeSet;

        // NODES
        std::vector<osmscout::TypeId>   m_nodeTypes;

        // sparsely populated lists
        std::vector<bool>               m_activeNodeTypes;
        std::vector<FillStyle*>         m_nodeFillStyles;
        std::vector<SymbolStyle*>       m_nodeSymbolStyles;
        std::vector<LabelStyle*>        m_nodeNameLabelStyles;

        // WAYS
        std::vector<osmscout::TypeId>   m_wayTypes;

        // sparsely populated lists
        std::vector<bool>               m_activeWayTypes;
        std::vector<size_t>             m_wayLayers;
        std::vector<LineStyle*>         m_wayLineStyles;
        std::vector<LabelStyle*>        m_wayNameLabelStyles;

        // AREAS
        std::vector<osmscout::TypeId>   m_areaTypes;

        // sparsely populated lists
        std::vector<bool>               m_activeAreaTypes;
        std::vector<size_t>             m_areaLayers;
        std::vector<FillStyle*>         m_areaFillStyles;
        std::vector<LabelStyle*>        m_areaNameLabelStyles;

        // FONTS
        std::vector<std::string>        m_listFonts;
    };

    // ========================================================================== //
    // ========================================================================== //
}

#endif
