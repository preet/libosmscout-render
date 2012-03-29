#ifndef OSMSCOUT_RENDERSTYLECONFIG_HPP
#define OSMSCOUT_RENDERSTYLECONFIG_HPP

#include <set>
#include <string>
#include <limits>
#include <vector>

#include <osmscout/Types.h>
#include <osmscout/TypeConfig.h>

namespace osmscout
{

    // ========================================================================== //
    // ========================================================================== //

    class OSMSCOUT_API ColorRGBA
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

    class OSMSCOUT_API FillRenderStyle
    {
    public:
        FillRenderStyle() :
            m_outlineWidth(0)
        {}

        FillRenderStyle(FillRenderStyle const &fillRenderStyle)
        {
            m_fillColor = fillRenderStyle.GetFillColor();
            m_outlineColor = fillRenderStyle.GetOutlineColor();
            m_outlineWidth = fillRenderStyle.GetOutlineWidth();
        }

        void SetFillColor(ColorRGBA const &fillColor)
        {   m_fillColor = fillColor;   }

        void SetOutlineColor(ColorRGBA const &outlineColor)
        {   m_outlineColor = outlineColor;   }

        void SetOutlineWidth(double outlineWidth)
        {   m_outlineWidth = outlineWidth;   }

        inline ColorRGBA GetFillColor() const
        {   return m_fillColor;   }

        inline ColorRGBA GetOutlineColor() const
        {   return m_outlineColor;   }

        inline double GetOutlineWidth() const
        {   return m_outlineWidth;   }

    private:
        ColorRGBA m_fillColor;
        ColorRGBA m_outlineColor;
        double m_outlineWidth;
    };

    // ========================================================================== //
    // ========================================================================== //

    class OSMSCOUT_API LineRenderStyle
    {
    public:
        LineRenderStyle() :
            m_lineWidth(0),
            m_outlineWidth(0)
        {}

        LineRenderStyle(LineRenderStyle const &lineRenderStyle)
        {
            m_lineWidth = lineRenderStyle.GetLineWidth();
            m_lineColor = lineRenderStyle.GetLineColor();
            m_outlineWidth = lineRenderStyle.GetOutlineWidth();
            m_outlineColor = lineRenderStyle.GetOutlineColor();
        }

        void SetLineWidth(double lineWidth)
        {   m_lineWidth = (lineWidth > 1) ? lineWidth : 1;   }

        void SetOutlineWidth(double outlineWidth)
        {   m_outlineWidth = outlineWidth;   }

        void SetLineColor(ColorRGBA const & lineColor)
        {   m_lineColor = lineColor;   }

        void SetOutlineColor(ColorRGBA const & outlineColor)
        {   m_outlineColor = outlineColor;   }

        inline double GetLineWidth() const
        {   return m_lineWidth;   }

        inline double GetOutlineWidth() const
        {   return m_outlineWidth;   }

        inline ColorRGBA GetLineColor() const
        {   return m_lineColor;   }

        inline ColorRGBA GetOutlineColor() const
        {   return m_outlineColor;   }

    private:
        double      m_lineWidth;
        double      m_outlineWidth;
        ColorRGBA   m_lineColor;
        ColorRGBA   m_outlineColor;
    };

    // ========================================================================== //
    // ========================================================================== //

    enum LabelRenderStyleType
    {
        LABEL_DEFAULT,
        LABEL_CONTOUR
    };

    class OSMSCOUT_API LabelRenderStyle
    {
    public:
        LabelRenderStyle() :
            m_fontSize(0),
            m_fontOutlineSize(0)
        {}

        LabelRenderStyle(LabelRenderStyle const &labelRenderStyle)
        {
            m_fontSize = labelRenderStyle.GetFontSize();
            m_fontColor = labelRenderStyle.GetFontColor();
            m_fontFamily = labelRenderStyle.GetFontFamily();
            m_fontOutlineSize = labelRenderStyle.GetFontOutlineSize();
            m_fontOutlineColor = labelRenderStyle.GetFontOutlineColor();
            m_labelType = labelRenderStyle.GetLabelType();
        }

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

        void SetLabelType(LabelRenderStyleType labelType)
        {   m_labelType = labelType;   }

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

        inline LabelRenderStyleType GetLabelType() const
        {   return m_labelType;   }

    private:
        double      m_fontSize;
        ColorRGBA   m_fontColor;
        std::string m_fontFamily;
        double      m_fontOutlineSize;
        ColorRGBA   m_fontOutlineColor;
        LabelRenderStyleType m_labelType;
    };

    // ========================================================================== //
    // ========================================================================== //

    class OSMSCOUT_API RenderStyleConfig
    {
    public:
        RenderStyleConfig(TypeConfig *typeConfig) :
            m_typeConfig(typeConfig),
            m_minDistance(0),
            m_maxDistance(250)
        {
            //TypeInfo list map
            m_numTypes = typeConfig->GetTypes().size();

            m_wayLayers.resize(m_numTypes,0);
            m_wayLineRenderStyles.resize(m_numTypes,NULL);
            m_wayNameLabelRenderStyles.resize(m_numTypes,NULL);

            m_areaFillRenderStyles.resize(m_numTypes,NULL);
            m_areaNameLabelRenderStyles.resize(m_numTypes,NULL);
        }

        ~RenderStyleConfig()
        {
            // clear WAY style data
            for(size_t i=0; i < m_wayLineRenderStyles.size(); i++)
            {
                if(!(m_wayLineRenderStyles.at(i) == NULL))
                {   delete m_wayLineRenderStyles[i];    }
            }

            for(size_t i=0; i < m_wayNameLabelRenderStyles.size(); i++)
            {
                if(!(m_wayNameLabelRenderStyles.at(i) == NULL))
                {   delete m_wayNameLabelRenderStyles[i];   }
            }


            // clear AREA style data
            for(size_t i=0; i < m_areaFillRenderStyles.size(); i++)
            {
                if(!(m_areaFillRenderStyles.at(i) == NULL))
                {   delete m_areaFillRenderStyles[i];   }
            }

            for(size_t i=0; i < m_areaNameLabelRenderStyles.size(); i++)
            {
                if(!(m_areaNameLabelRenderStyles.at(i) == NULL))
                {   delete m_areaNameLabelRenderStyles[i];   }
            }
        }

        void PostProcess()
        {
            // use sparsely populated property lists
            // to generate a list of unique types

            for(TypeId i=0; i < m_numTypes; i++)
            {
                // ways MUST have a layer specified
                if(m_wayLayers[i] != 0)
                {   m_wayTypes.push_back(i);   }

                // areas MUST have a fill type specified
                if(!(m_areaFillRenderStyles[i] == NULL))
                {   m_areaTypes.push_back(i);  }
            }
        }


        // Set RendererStyleConfig parameters
        void SetMinDistance(double minDistance)
        {   m_minDistance = minDistance;   }

        void SetMaxDistance(double maxDistance)
        {   m_maxDistance = maxDistance;   }


        // Set WAY info
        void SetWayLayer(TypeId wayType, size_t wayLayer)
        {   m_wayLayers[wayType] = wayLayer;   }

        void SetWayLineRenderStyle(TypeId wayType,LineRenderStyle const &lineRenderStyle)
        {   m_wayLineRenderStyles[wayType] = new LineRenderStyle(lineRenderStyle);   }

        void SetWayNameLabelRenderStyle(TypeId wayType,LabelRenderStyle const &labelRenderStyle)
        {   m_wayNameLabelRenderStyles[wayType] = new LabelRenderStyle(labelRenderStyle);   }


        // Set AREA info
        void SetAreaFillRenderStyle(TypeId areaType, FillRenderStyle const &fillRenderStyle)
        {   m_areaFillRenderStyles[areaType] = new FillRenderStyle(fillRenderStyle);   }

        void SetAreaNameLabelRenderStyle(TypeId areaType, LabelRenderStyle const &labelRenderStyle)
        {   m_areaNameLabelRenderStyles[areaType] = new LabelRenderStyle(labelRenderStyle);   }


        // Get RendererStyleConfig parameters
        TypeConfig* GetTypeConfig() const
        {   return m_typeConfig;   }

        double GetMinDistance() const
        {   return m_minDistance;   }

        double GetMaxDistance() const
        {   return m_maxDistance;   }

        void GetActiveTypes(std::vector<TypeId> &activeTypes) const
        {   // get types that have style data

            activeTypes.clear();
            activeTypes.resize(m_wayTypes.size() +
                               m_areaTypes.size());

            int i=0;

            for(int j=0; j < m_wayTypes.size(); j++)
            {   activeTypes[i] = m_wayTypes[j];  i++;   }

            for(int j=0; j < m_areaTypes.size(); j++)
            {   activeTypes[i] = m_areaTypes[j]; i++;   }
        }

        // Get WAY info
        void GetWayTypes(std::vector<TypeId> & wayTypes) const
        {
            wayTypes.clear();
            wayTypes.resize(m_wayTypes.size());

            for(int i=0; i < m_wayTypes.size(); i++)
            {   wayTypes[i] = m_wayTypes[i];   }
        }

        size_t GetWayLayer(TypeId wayType) const
        {   return m_wayLayers[wayType];   }

        LineRenderStyle* GetWayLineRenderStyle(TypeId wayType) const
        {   return (wayType < m_numTypes) ? m_wayLineRenderStyles[wayType] : NULL;   }

        LabelRenderStyle* GetWayNameLabelRenderStyle(TypeId wayType) const
        {   return (wayType < m_numTypes) ? m_wayNameLabelRenderStyles[wayType] : NULL;   }


        // Get AREA info
        void GetAreaTypes(std::vector<TypeId> & areaTypes) const
        {
            areaTypes.clear();
            areaTypes.resize(m_areaTypes.size());

            for(int i=0; i < m_areaTypes.size(); i++)
            {   areaTypes[i] = m_areaTypes[i];   }
        }

        FillRenderStyle* GetAreaFillRenderStyle(TypeId areaType) const
        {   return (areaType < m_numTypes) ? m_areaFillRenderStyles[areaType] : NULL;   }

        LabelRenderStyle* GetAreaNameLabelRenderStyle(TypeId areaType) const
        {   return (areaType < m_numTypes) ? m_areaNameLabelRenderStyles[areaType] : NULL;   }


    private:
        TypeConfig*                     m_typeConfig;
        unsigned int                    m_numTypes;
        double                          m_minDistance;
        double                          m_maxDistance;

        // WAYS
        std::vector<TypeId>             m_wayTypes;

        // sparsely populated lists
        std::vector<size_t>             m_wayLayers;
        std::vector<LineRenderStyle*>   m_wayLineRenderStyles;
        std::vector<LabelRenderStyle*>  m_wayNameLabelRenderStyles;


        // AREAS
        std::vector<TypeId>             m_areaTypes;

        // sparsely populated lists
        std::vector<FillRenderStyle*>   m_areaFillRenderStyles;
        std::vector<LabelRenderStyle*>  m_areaNameLabelRenderStyles;
    };

    // ========================================================================== //
    // ========================================================================== //
}



#endif
