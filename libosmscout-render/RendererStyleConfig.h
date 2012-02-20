#ifndef OSMSCOUT_RENDERSTYLECONFIG_H
#define OSMSCOUT_RENDERSTYLECONFIG_H

#include <string>

#include <osmscout/Types.h>
#include <osmscout/TypeConfig.h>

namespace osmscout
{

class OSMSCOUT_API ColorRGBA
{
public:
    ColorRGBA() : R(1), G(1), B(1), A(1)
    {   /* empty */   }

    double R;
    double G;
    double B;
    double A;
};

class OSMSCOUT_API LineRenderStyle
{
public:
    LineRenderStyle();
    LineRenderStyle& SetLineWidth(double lineWidth);
    LineRenderStyle& SetOutlineWidth(double outlineWidth);
    LineRenderStyle& SetLineColor(ColorRGBA const &lineColor);
    LineRenderStyle& SetOutlineColor(ColorRGBA const &outlineColor);

    inline double GetLineWidth() const                  {   return m_lineWidth;             }
    inline double GetOutlineWidth() const               {   return m_outlineWidth;          }
    inline ColorRGBA GetLineColor() const               {   return m_lineColor;             }
    inline ColorRGBA GetOutlineColor() const            {   return m_outlineColor;          }

private:
    double m_lineWidth;
    double m_outlineWidth;
    ColorRGBA m_lineColor;
    ColorRGBA m_outlineColor;
};

class OSMSCOUT_API LabelRenderStyle
{
public:
    LabelRenderStyle();
    LabelRenderStyle& SetFontSize(double fontSize);
    LabelRenderStyle& SetFontColor(ColorRGBA const &fontColor);
    LabelRenderStyle& SetFontFamily(std::string const &fontFamily);
    LabelRenderStyle& SetFontOutlineSize(double fontOutlineSize);
    LabelRenderStyle& SetFontOutlineColor(ColorRGBA const &fontOutlineColor);

    inline double GetFontSize() const                   {   return m_fontSize;              }
    inline ColorRGBA GetFontColor() const               {   return m_fontColor;             }
    inline std::string GetFontFamily() const            {   return m_fontFamily;            }
    inline double GetFontOutlineSize() const            {   return m_fontOutlineSize;       }
    inline ColorRGBA GetFontOutlineColor() const        {   return m_fontOutlineColor;      }

private:
    double m_fontSize;
    ColorRGBA m_fontColor;
    std::string m_fontFamily;
    double m_fontOutlineSize;
    ColorRGBA m_fontOutlineColor;
};

//class OSMSCOUT_API RendererStyleConfig
//{

//};





}



#endif
