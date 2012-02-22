#ifndef OSMSCOUT_RENDERSTYLECONFIG_HPP
#define OSMSCOUT_RENDERSTYLECONFIG_HPP

#include <set>
#include <string>
#include <limits>

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

    class OSMSCOUT_API LabelRenderStyle
    {
    public:
        LabelRenderStyle() :
            m_fontSize(0),
            m_fontOutlineSize(0)
        {}

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

    private:
        double      m_fontSize;
        ColorRGBA   m_fontColor;
        std::string m_fontFamily;
        double      m_fontOutlineSize;
        ColorRGBA   m_fontOutlineColor;
    };

    // ========================================================================== //
    // ========================================================================== //

    class OSMSCOUT_API RenderStyleConfig
    {
    public:
        RenderStyleConfig(TypeConfig *typeConfig) :
            m_minMag(0),
            m_maxMag(250),
            m_maxPriority(9999)
        {
            //TypeInfo list map
            size_t numTags = typeConfig->GetTypes().size();
            m_wayPrios.resize(numTags,m_maxPriority);
            m_wayLineRenderStyles.resize(numTags,NULL);
            m_wayNameLabelRenderStyles.resize(numTags,NULL);
        }

        ~RenderStyleConfig()
        {
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
        }

        void PostProcess()
        {
            // sort way types by priority
            std::map<size_t,TypeId> wayPriosNoNulls;
            for(int i=0; i < m_wayPrios.size(); i++)
            {
                if(!(m_wayPrios.at(i) == m_maxPriority))
                {   wayPriosNoNulls[m_wayPrios.at(i)] = i;   }
            }

            // save way TypeIds by priority
            m_wayTypesByPrio.clear();
            std::map<size_t,TypeId>::iterator mapIt;
            for(mapIt = wayPriosNoNulls.begin(); mapIt != wayPriosNoNulls.end(); mapIt++)
            {   m_wayTypesByPrio.push_back(mapIt->second);   }
        }

        // Set RendererStyleConfig parameters
        void SetMinMag(double minMag)
        {   m_minMag = minMag;   }

        void SetMaxMag(double maxMag)
        {   m_maxMag = maxMag;   }

        // Set WAY parameters
        void SetWayPrio(TypeId wayType, size_t wayPrio)
        {
            m_wayPrios[wayType] = wayPrio;
        }

        void SetWayLineRenderStyle(TypeId wayType, LineRenderStyle const &lineRenderStyle)
        {
            LineRenderStyle * myLineStyle = new LineRenderStyle();
            myLineStyle->SetLineColor(lineRenderStyle.GetLineColor());
            myLineStyle->SetLineWidth(lineRenderStyle.GetLineWidth());
            myLineStyle->SetOutlineColor(lineRenderStyle.GetOutlineColor());
            myLineStyle->SetOutlineWidth(lineRenderStyle.GetOutlineWidth());

            m_wayLineRenderStyles[wayType] = myLineStyle;
        }

        void SetWayNameLabelRenderStyle(TypeId wayType, LabelRenderStyle const &labelRenderStyle)
        {
            m_wayNameLabelRenderStyles[wayType] = new LabelRenderStyle(labelRenderStyle);
        }

        // Get RendererStyleConfig parameters
        TypeConfig* GetTypeConfig() const
        {   return m_typeConfig;   }

        double GetMinMag() const
        {   return m_minMag;   }

        double GetMaxMag() const
        {   return m_maxMag;   }

        // Get WAY parameters
        size_t GetWayPrio(TypeId wayType) const
        {   return m_wayPrios[wayType];   }

        LineRenderStyle* GetWayLineRenderStyle(TypeId wayType) const
        {
            if(wayType < m_wayLineRenderStyles.size())
            {   return m_wayLineRenderStyles[wayType];   }
            else
            {   return NULL;   }
        }

        LabelRenderStyle* GetWayNameLabelRenderStyle(TypeId wayType) const
        {
            if(wayType < m_wayNameLabelRenderStyles.size())
            {   return m_wayNameLabelRenderStyles[wayType];   }
            else
            {   return NULL;   }
        }

        void GetWayTypesByPrio(std::vector<TypeId> & wayTypes) const
        {
            wayTypes.clear();
            wayTypes.reserve(m_wayTypesByPrio.size());

            for(size_t i=0; i < m_wayTypesByPrio.size(); i++)
            {   wayTypes.push_back(m_wayTypesByPrio[i]);   }
        }

    private:
        TypeConfig                      *m_typeConfig;
        double                             m_minMag;
        double                             m_maxMag;

        // WAY
        std::vector<LineRenderStyle*>   m_wayLineRenderStyles;
        std::vector<LabelRenderStyle*>  m_wayNameLabelRenderStyles;
        std::vector<size_t>             m_wayPrios;
        std::vector<TypeId>             m_wayTypesByPrio;
        size_t                          m_maxPriority;
    };

    // ========================================================================== //
    // ========================================================================== //
}



#endif
