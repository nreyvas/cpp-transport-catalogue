#pragma once
#include "svg.h"
#include "domain.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <variant>
#include <deque>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <map>
#include <set>

namespace renderer {

struct RenderSettings {
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;

    double line_width = 0.0;
    double stop_radius = 0.0;

    int bus_label_font_size = 0;
    svg::Point bus_label_offset {};

    int stop_label_font_size = 0;
    svg::Point stop_label_offset {};

    svg::Color underlayer_color;
    double underlayer_width = 0.0;

    std::vector<svg::Color> color_palette;
};

inline const double EPSILON = 1e-6;

bool IsZero(double value);


class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding);
    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    explicit MapRenderer(RenderSettings&& render_settings, std::deque<const Bus*> routes)
        : render_settings_(render_settings) {
        
        std::unordered_set<geo::Coordinates, CoordinatesHasher> points;
        for(const Bus* route : routes) {
            for(const Stop* stop : route->stops_) {
                points.insert(stop->cordinates_);
            }
        }

        sphere_proector_ = std::make_unique<SphereProjector>(
            points.begin(),
            points.end(),
            render_settings_.width,
            render_settings_.height,
            render_settings_.padding
        );
  
    }

    std::optional<svg::Polyline> RenderRoute(const Bus* bus, svg::Color color);

    void RenderRoutes(std::deque<const Bus*> buses);
    void RenderStops(const std::map<std::string_view, const Stop*>& stopname_to_stops,
                    const std::unordered_map<const Stop*, std::set<const Bus*>>& stops_to_buses);
    void Render(const svg::RenderContext& context) const;

    
    void RenderRouteNames(const Bus* bus, svg::Color color);
    void RenderBusLabel(std::string_view text, svg::Color color, svg::Point point);
    void RenderBusLabelToplayer(std::string_view text, svg::Color color, svg::Point point);
    void RenderBusLabelUnderlayer(std::string_view text, [[maybe_unused]] svg::Color color, svg::Point point);
    
    void RenderStopCircle(const Stop* stop);
    void RenderStopLabel(const Stop* stop);
    void RenderStopLabelUnderlayer(std::string_view stop_name_, svg::Point point);
    void RenderStopLabelToplayer(std::string_view stop_name_, svg::Point point);

    // ---- for serialization purposes
    const RenderSettings& GetRenderSettings() const;
private:
    const RenderSettings render_settings_;
    std::unique_ptr<SphereProjector> sphere_proector_ = nullptr;
    svg::Document document_;
};


// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
    : padding_(padding) //
{
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}
}