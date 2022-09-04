#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

// Проецирует широту и долготу в координаты внутри SVG-изображения
svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
    };
}

void MapRenderer::RenderRoutes(std::deque<const Bus*> buses) {
    size_t color_index = 0;
    size_t colors_count = render_settings_.color_palette.size();
    
    // -- RenderLines
    for(const Bus* bus : buses) {
        auto color = render_settings_.color_palette[color_index % colors_count];
        auto route = RenderRoute(bus, color);
        if(route.has_value()) {
            document_.Add(route.value()
                .SetFillColor("none")
                .SetStrokeWidth(render_settings_.line_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            ++color_index;
        }
    }
    // -- RenderRouteNames
    color_index = 0;
    for(const Bus* bus : buses) {
        auto color = render_settings_.color_palette[color_index % colors_count];
        if(bus->stops_.empty()) {
            continue;
        }
        RenderRouteNames(bus, color);
        ++color_index;
    }
}

std::optional<svg::Polyline> MapRenderer::RenderRoute(const Bus* bus, svg::Color color) {
    if(bus->stops_.empty()) {
        return std::nullopt;
    }

    svg::Polyline route;
    route.SetStrokeColor(color);
    
    if(bus->bus_type_ == BusType::CYCLED) {
        for(const Stop* stop : bus->stops_) {
            route.AddPoint(sphere_proector_->operator()(stop->cordinates_));
        }
    } else if(bus->bus_type_ == BusType::ORDINARY) {
        for(const Stop* stop : bus->stops_) {
            route.AddPoint(sphere_proector_->operator()(stop->cordinates_));
        }
        for(auto stop = next(bus->stops_.rbegin()); 
                stop !=  bus->stops_.rend();
                ++stop) 
        {
            route.AddPoint(sphere_proector_->operator()((*stop)->cordinates_));
        }

    } else {
        throw std::logic_error("wrong bus");
    }    
    
    return route;
}

void MapRenderer::Render(const svg::RenderContext& context) const {
    document_.Render(context.out);
}


void MapRenderer::RenderRouteNames(const Bus* bus, svg::Color color) {
    if(bus->bus_type_ == BusType::CYCLED) {
        svg::Point first_stop_point = sphere_proector_->operator()(bus->stops_.front()->cordinates_);
        RenderBusLabel(bus->name_, color, first_stop_point);
    } else if(bus->bus_type_ == BusType::ORDINARY) {
        svg::Point first_stop_point = sphere_proector_->operator()(bus->stops_.front()->cordinates_);
        RenderBusLabel(bus->name_, color, first_stop_point);
        // если начальная и конечная остановки маршрута не совпадают
        if(bus->stops_.front() != bus->stops_.back()) {
            svg::Point last_stop_point = sphere_proector_->operator()(bus->stops_.back()->cordinates_);
            RenderBusLabel(bus->name_, color, last_stop_point);
        }

    } else {
        throw std::logic_error("wrong bus");
    }
}

void MapRenderer::RenderBusLabel(std::string_view text, svg::Color color, svg::Point point) {
    RenderBusLabelUnderlayer(text, color, point);
    RenderBusLabelToplayer(text, color, point);
}

void MapRenderer::RenderBusLabelUnderlayer(std::string_view text, [[maybe_unused]] svg::Color color, svg::Point point) {
    document_.Add(svg::Text()
        .SetData(std::string(text))
        .SetPosition(point)
        .SetOffset(render_settings_.bus_label_offset)
        .SetFontSize(render_settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(render_settings_.underlayer_color)
        .SetStrokeColor(render_settings_.underlayer_color)
        .SetStrokeWidth(render_settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
}
void MapRenderer::RenderBusLabelToplayer(std::string_view text, svg::Color color, svg::Point point) {
    document_.Add(svg::Text()
        .SetData(std::string(text))
        .SetPosition(point)
        .SetOffset(render_settings_.bus_label_offset)
        .SetFontSize(render_settings_.bus_label_font_size)
        .SetFontFamily("Verdana")
        .SetFontWeight("bold")
        .SetFillColor(color));
}

void MapRenderer::RenderStops(const std::map<std::string_view, const Stop*>& stopname_to_stops,
                    const std::unordered_map<const Stop*, std::set<const Bus*>>& stops_to_buses) {
    // -- render circles
    for(const auto& [stop_name, stop_ptr] : stopname_to_stops) {
        if(stops_to_buses.count(stop_ptr) != 0
            && !stops_to_buses.at(stop_ptr).empty()) {
                RenderStopCircle(stop_ptr);
            }
    }

    // -- render stop labels
    for(const auto& [stop_name, stop_ptr] : stopname_to_stops) {
        if(stops_to_buses.count(stop_ptr) != 0
            && !stops_to_buses.at(stop_ptr).empty()) {
                RenderStopLabel(stop_ptr);
            }
    }
}
void MapRenderer::RenderStopCircle(const Stop* stop) {
    svg::Point center = sphere_proector_->operator()(stop->cordinates_);
    document_.Add(svg::Circle()
        .SetCenter(center)
        .SetFillColor("white")
        .SetRadius(render_settings_.stop_radius));
}

void MapRenderer::RenderStopLabel(const Stop* stop) {
    svg::Point point = sphere_proector_->operator()(stop->cordinates_);

    RenderStopLabelUnderlayer(stop->name_, point);
    RenderStopLabelToplayer(stop->name_, point);
}

    void MapRenderer::RenderStopLabelUnderlayer(std::string_view stop_name, svg::Point point) {
        document_.Add(svg::Text()
        .SetData(std::string(stop_name))
        .SetPosition(point)
        .SetOffset(render_settings_.stop_label_offset)
        .SetFontSize(render_settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor(render_settings_.underlayer_color)
        .SetStrokeColor(render_settings_.underlayer_color)
        .SetStrokeWidth(render_settings_.underlayer_width)
        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
    }
    void MapRenderer::RenderStopLabelToplayer(std::string_view stop_name, svg::Point point) {
        document_.Add(svg::Text()
        .SetData(std::string(stop_name))
        .SetPosition(point)
        .SetOffset(render_settings_.stop_label_offset)
        .SetFontSize(render_settings_.stop_label_font_size)
        .SetFontFamily("Verdana")
        .SetFillColor("black")
        );
    }

const RenderSettings& MapRenderer::GetRenderSettings() const {
    return render_settings_;
}

}
