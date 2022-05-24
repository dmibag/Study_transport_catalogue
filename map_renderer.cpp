#include "map_renderer.h"

#include <memory>

namespace renderer {

using namespace std;

inline const double EPSILON = 1e-6;

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

SphereProjector::SphereProjector(std::deque<const transport::Stop*>::iterator points_begin,
        std::deque<const transport::Stop*>::iterator points_end, double max_width, double max_height, double padding) :
        padding_(padding) {
    if (points_begin == points_end) {
        return;
    }

    const auto [left_it, right_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
        return lhs->coord.lng < rhs->coord.lng;
    });
    min_lon_ = (*left_it)->coord.lng;
    const double max_lon = (*right_it)->coord.lng;

    const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
        return lhs->coord.lat < rhs->coord.lat;
    });

    const double min_lat = (*bottom_it)->coord.lat;
    max_lat_ = (*top_it)->coord.lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom) {
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom) {
        zoom_coeff_ = *height_zoom;
    }
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
}

namespace shapes {

void BusLine::Draw(svg::ObjectContainer &container) const {
    using namespace std::literals;
    auto busline =
            svg::Polyline().SetStrokeColor(color_).SetStrokeWidth(width_).SetFillColor(NoneColor).SetStrokeLineCap(
                    StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);

    for (const Point point : points_) {
        busline.AddPoint(point);
    }
    container.Add(std::move(busline));
}

void StopSymbol::Draw(svg::ObjectContainer &container) const {
    using namespace std::literals;
    auto circle = svg::Circle().SetFillColor("white"s).SetCenter(point_).SetRadius(stop_radius_);

    container.Add(std::move(circle));
}

void MapText::Draw(svg::ObjectContainer &container) const {
    using namespace std::literals;
    auto basetext = svg::Text().SetPosition(position_).SetOffset(offset_).SetFontSize(label_font_size_).SetFontFamily(
            "Verdana"s).SetData(std::string(data_));
    if (font_weight_) basetext.SetFontWeight("bold"s);

    auto textunder = Text { basetext }.SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_).SetStrokeWidth(
            underlayer_width_).SetStrokeLineCap(StrokeLineCap::ROUND).SetStrokeLineJoin(StrokeLineJoin::ROUND);
    auto text = Text { basetext }.SetFillColor(fill_);

    container.Add(std::move(textunder));
    container.Add(std::move(text));
}

} // shapes

svg::Document MapRenderer::RenderMap() {
    SphereProjector sp(std::begin(stops_sorted_), std::end(stops_sorted_), map_settings_.width, map_settings_.height,
            map_settings_.padding);
    svg::Document doc;
// bus lines
    for (const transport::Bus *bus : buses_sorted_) {
        std::vector<svg::Point> stops;

        for (const auto *stop : bus->stops) {
            stops.push_back(sp(stop->coord));
        }
        if (!bus->cicle) {
            for (auto it = bus->stops.rbegin() + 1; it != bus->stops.rend(); ++it) {
                stops.push_back(sp((*it)->coord));
            }
        }
        shapes::BusLine(std::move(stops), map_settings_.line_width, GetNextColor()).Draw(doc);
    }
// bus names
    CleanColor();
    for (const transport::Bus *bus : buses_sorted_) {
        auto color = GetNextColor();
        shapes::MapText(bus->name, sp(bus->stops.front()->coord), map_settings_.bus_label_offset,
                map_settings_.bus_label_font_size, map_settings_.underlayer_color, map_settings_.underlayer_width,
                color).Draw(doc);
        if (!bus->cicle && bus->stops.front()->name != bus->stops.back()->name) shapes::MapText(bus->name,
                sp(bus->stops.back()->coord), map_settings_.bus_label_offset, map_settings_.bus_label_font_size,
                map_settings_.underlayer_color, map_settings_.underlayer_width, color).Draw(doc);
    }
// stop symbols
    for (const auto *stop : stops_sorted_) {
        shapes::StopSymbol(sp(stop->coord), map_settings_.stop_radius).Draw(doc);
    }
// stop names
    for (const auto *stop : stops_sorted_) {
        shapes::MapText(stop->name, sp(stop->coord), map_settings_.stop_label_offset,
                map_settings_.stop_label_font_size, map_settings_.underlayer_color, map_settings_.underlayer_width,
                "black"s, false).Draw(doc);
    }

    return doc;
}

const svg::Color MapRenderer::GetNextColor() {
    svg::Color color = map_settings_.color_palette[color_num_];
    color_num_ = (color_num_ + 1 >= (int) (map_settings_.color_palette.size())) ? 0 : color_num_ + 1;
    return color;
}

} // render
