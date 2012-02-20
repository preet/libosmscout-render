#include "RendererStyleConfig.h"

namespace osmscout
{

// ========================================================================== //
// ========================================================================== //

LineRenderStyle::LineRenderStyle() :
    m_lineWidth(0),
    m_outlineWidth(0)
{}

LineRenderStyle& LineRenderStyle::SetLineWidth(double lineWidth)
{
    m_lineWidth = (lineWidth > 1) ? lineWidth : 1;
    return *this;
}

LineRenderStyle& LineRenderStyle::SetOutlineWidth(double outlineWidth)
{
    m_outlineWidth = outlineWidth;
    return *this;
}

LineRenderStyle& LineRenderStyle::SetLineColor(ColorRGBA const &lineColor)
{
    m_lineColor.R = lineColor.R;
    m_lineColor.G = lineColor.G;
    m_lineColor.B = lineColor.B;
    m_lineColor.A = lineColor.A;
    return *this;
}

LineRenderStyle& LineRenderStyle::SetOutlineColor(ColorRGBA const &outlineColor)
{
    m_outlineColor.R = outlineColor.R;
    m_outlineColor.G = outlineColor.G;
    m_outlineColor.B = outlineColor.B;
    m_outlineColor.A = outlineColor.A;
    return *this;
}

// ========================================================================== //
// ========================================================================== //

LabelRenderStyle::LabelRenderStyle() :
    m_fontSize(0),
    m_fontOutlineSize(0)
{}

LabelRenderStyle& LabelRenderStyle::SetFontSize(double fontSize)
{   m_fontSize = fontSize;   }

LabelRenderStyle& LabelRenderStyle::SetFontColor(const ColorRGBA &fontColor)
{
    m_fontColor.R = fontColor.R;
    m_fontColor.G = fontColor.G;
    m_fontColor.B = fontColor.B;
    m_fontColor.A = fontColor.A;
}

LabelRenderStyle& LabelRenderStyle::SetFontFamily(const std::string &fontFamily)
{   m_fontFamily = fontFamily;   }

LabelRenderStyle& LabelRenderStyle::SetFontOutlineSize(double fontOutlineSize)
{   m_fontOutlineSize = fontOutlineSize;   }

LabelRenderStyle& LabelRenderStyle::SetFontOutlineColor(const ColorRGBA &fontOutlineColor)
{
    m_fontOutlineColor.R = fontOutlineColor.R;
    m_fontOutlineColor.G = fontOutlineColor.G;
    m_fontOutlineColor.B = fontOutlineColor.B;
    m_fontOutlineColor.A = fontOutlineColor.A;
}

// ========================================================================== //
// ========================================================================== //

}
