#include "map_renderer.h"

using namespace std::literals;

namespace renderer
{
    bool IsZero(double value)
    {
        return std::abs(value) < EPSILON;
    }


    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    void MapRenderer::SetWidth(double width)
    {
        if (width <= 0 || width >= 100000)
        {
            throw std::invalid_argument("Width should be a positive number"s);
        }
        width_ = width;
    }

    void MapRenderer::SetHeight(double height)
    {
        if (height <= 0 || height >= 100000)
        {
            throw std::invalid_argument("Width should be a positive number"s);
        }
        height_ = height;
    }

    void MapRenderer::SetPadding(double padding)
    {
        if (padding < 0 || padding >= std::min(width_, height_))
        {
            throw std::invalid_argument("Bad padding value"s);
        }
        padding_ = padding;
    }

    void MapRenderer::SetLineWidth(double line_width)
    {
        if (line_width <= 0 || line_width >= 100000)
        {
            throw std::invalid_argument("LineWidth should be a positive number"s);
        }
        line_width_ = line_width;
    }

    void MapRenderer::SetStopRadius(double stop_radius)
    {
        if (stop_radius <= 0 || stop_radius >= 100000)
        {
            throw std::invalid_argument("StopRadius should be a positive number"s);
        }
        stop_radius_ = stop_radius;
    }

    void MapRenderer::SetBusLabelFontSize(int bus_label_font_size)
    {
        if (bus_label_font_size <= 0 || bus_label_font_size >= 100000)
        {
            throw std::invalid_argument("BusLabelFontSize should be a positive number"s);
        }
        bus_label_font_size_ = bus_label_font_size;
    }

    void MapRenderer::SetBusLabelOffset(svg::Point bus_label_offset)
    {
        if (bus_label_offset.x <= -100000 || bus_label_offset.x >= 100000 ||
            bus_label_offset.y <= -100000 || bus_label_offset.y >= 100000)
        {
            throw std::invalid_argument("Bad argument"s);
        }
        bus_label_offset_ = bus_label_offset;
    }

    void MapRenderer::SetStopLabelFontSize(int stop_label_font_size)
    {
        if (stop_label_font_size <= 0 || stop_label_font_size >= 100000)
        {
            throw std::invalid_argument("StopLabelFontSize should be a positive number"s);
        }
        stop_label_font_size_ = stop_label_font_size;
    }

    void MapRenderer::SetStopLabelOffset(svg::Point stop_label_offset)
    {
        if (stop_label_offset.x <= -100000 || stop_label_offset.x >= 100000 ||
            stop_label_offset.y <= -100000 || stop_label_offset.y >= 100000)
        {
            throw std::invalid_argument("Bad argument"s);
        }
        stop_label_offset_ = stop_label_offset;
    }

    void MapRenderer::SetUnderlayerColor(svg::Color underlayer_color)
    {
        underlayer_color_ = underlayer_color;
    }

    void MapRenderer::SetUnderlayerWidth(double underlayer_width)
    {
        if (underlayer_width <= 0 || underlayer_width >= 100000)
        {
            throw std::invalid_argument("UnderlayerWidth should be a positive number"s);
        }
        underlayer_width_ = underlayer_width;
    }

    void MapRenderer::SetColorPalette(std::vector<svg::Color> color_palette)
    {
        color_palette_ = std::move(color_palette);
    }

    double MapRenderer::GetWidth() const
    {
        return width_;
    }

    double MapRenderer::GetHeight() const
    {
        return height_;
    }

    double MapRenderer::GetPadding() const
    {
        return padding_;
    }

    double MapRenderer::GetLineWidth() const
    {
        return line_width_;
    }

    double MapRenderer::GetStopRadius() const
    {
        return stop_radius_;
    }

    int MapRenderer::GetBusLabelFontSize() const
    {
        return bus_label_font_size_;
    }

    svg::Point MapRenderer::GetBusLabelOffset() const
    {
        return bus_label_offset_;
    }

    int MapRenderer::GetStopLabelFontSize() const
    {
        return stop_label_font_size_;
    }

    svg::Point MapRenderer::GetStopLabelOffset() const
    {
        return stop_label_offset_;
    }

    svg::Color MapRenderer::GetUnderlayerColor() const
    {
        return underlayer_color_;
    }

    double MapRenderer::GetUnderlayerWidth() const
    {
        return underlayer_width_;
    }

    const std::vector<svg::Color>& MapRenderer::GetColorPalette() const
    {
        return color_palette_;
    }
}
