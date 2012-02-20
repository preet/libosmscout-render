#ifndef OSMSCOUT_RENDERSTYLECONFIG_H
#define OSMSCOUT_RENDERSTYLECONFIG_H

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

    class OSMSCOUT_API RendererStyleConfig
    {
    public:
        RendererStyleConfig(TypeConfig *typeConfig) :
            m_minMag(magCity),
            m_maxMag(magClose)
        {}

        ~RendererStyleConfig()
        {
            // clean up way render styles
            for(size_t i=0; i < m_wayLineRenderStyles.size(); i++)
            {   delete m_wayLineRenderStyles[i];   }

            for(size_t i=0; i < m_wayNameLabelRenderStyles.size(); i++)
            {   delete m_wayNameLabelRenderStyles[i];   }
        }

        void PostProcess()
        {
            // sort way types by priority (render higher prio first)
            std::set<size_t> prios;
            std::vector<size_t> sortedPrios;

            for(size_t i=0; i < m_wayLineRenderStyles.size() && i < m_wayPrios.size(); i++)
            {
                if(m_wayLineRenderStyles[i] != NULL)
                {   prios.insert(m_wayPrios[i]);   }
            }

            sortedPrios.reserve(prios.size());
            for(std::set<size_t>::const_iterator prio = prios.begin(); prio != prios.end(); prio++)
            {   sortedPrios.push_back(*prio);   }

            m_wayTypesByPrio.clear();
            m_wayTypesByPrio.reserve(sortedPrios.size());
            for(size_t p = 0; p < sortedPrios.size(); p++)
            {
                for(size_t i=0; i < m_wayLineRenderStyles.size() && i < m_wayPrios.size(); i++)
                {
                    if(m_wayLineRenderStyles[i] != NULL && m_wayPrios[i] == sortedPrios[p])
                    {   m_wayTypesByPrio.push_back(i);   }
                }
            }
        }

        // Set RendererStyleConfig parameters
        void SetMinMag(Mag minMag)
        {   m_minMag = minMag;   }

        void SetMaxMag(Mag maxMag)
        {   m_maxMag = maxMag;   }

        // Set WAY parameters
        void SetWayPrio(TypeId wayType, size_t wayPrio)
        {
            if(wayType >= m_wayPrios.size())
            {
                m_wayPrios.resize(wayType+1);
                m_wayLineRenderStyles.resize(wayType+1);
                m_wayNameLabelRenderStyles.resize(wayType+1);
            }

            m_wayPrios[wayType] = wayPrio;
        }

        void SetWayLineRenderStyle(TypeId wayType, const LineRenderStyle &lineRenderStyle)
        {
            if(wayType >= m_wayPrios.size())
            {
                m_wayPrios.resize(wayType+1, std::numeric_limits<size_t>::max());
                m_wayLineRenderStyles.resize(wayType+1, NULL);
                m_wayNameLabelRenderStyles.resize(wayType+1,NULL);
            }

            delete m_wayLineRenderStyles[wayType];
            m_wayLineRenderStyles[wayType] = new LineRenderStyle(lineRenderStyle);
        }

        void SetWayNameLabelRenderStyle(TypeId wayType, LabelRenderStyle const &labelRenderStyle)
        {
            if(wayType >= m_wayPrios.size())
            {
                m_wayPrios.resize(wayType+1, std::numeric_limits<size_t>::max());
                m_wayLineRenderStyles.resize(wayType+1, NULL);
                m_wayNameLabelRenderStyles.resize(wayType+1,NULL);
            }

            delete m_wayNameLabelRenderStyles[wayType];
            m_wayNameLabelRenderStyles[wayType] = new LabelStyle(labelRenderStyle);
        }

        // Get RendererStyleConfig parameters
        TypeConfig* GetTypeConfig() const
        {   return m_typeConfig;   }

        Mag GetMinMag() const
        {   return m_minMag;   }

        Mag GetMaxMag() const
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

        LineRenderStyle* GetWayNameLabelRenderStyle(TypeId wayType) const
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
        Mag                             *m_minMag;
        Mag                             *m_maxMag;

        // WAY
        std::vector<LineRenderStyle*>   m_wayLineRenderStyles;
        std::vector<LabelRenderStyle*>  m_wayNameLabelRenderStyles;
        std::vector<size_t>             m_wayPrios;
        std::vector<TypeId>             m_wayTypesByPrio;
    };

    // ========================================================================== //
    // ========================================================================== //
}



#endif
