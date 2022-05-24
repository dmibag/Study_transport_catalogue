#pragma once

#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <deque>
#include <set>
#include <string>
#include <unordered_map>

namespace renderer {

using namespace svg;
using namespace std::literals;

struct MapSettings {
    double width = 0.;
    double height = 0.;
    double padding = 0.;
    double line_width = 0.;
    double stop_radius = 0.;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset;
    int stop_label_font_size = 0;
    svg::Point stop_label_offset;
    Color underlayer_color;
    double underlayer_width = 0.;
    std::vector<Color> color_palette;
};

class SphereProjector {
public:
    SphereProjector(std::deque<const transport::Stop*>::iterator points_begin,
            std::deque<const transport::Stop*>::iterator points_end, double max_width, double max_height,
            double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_ = 0.;
    double min_lon_ = 0.;
    double max_lat_ = 0.;
    double zoom_coeff_ = 0.;
};

namespace shapes {

class BusLine final : public svg::Drawable {
public:
    BusLine(std::vector<svg::Point> points, double width, svg::Color color) :
            points_(std::move(points)), width_(width), color_(color) {
    }

    void Draw(svg::ObjectContainer &container) const override;

private:
    std::vector<svg::Point> points_;
    double width_;
    svg::Color color_;
};

class StopSymbol final : public svg::Drawable {
public:
    StopSymbol(svg::Point point, double stop_radius) :
            point_(point), stop_radius_(stop_radius) {
    }

    void Draw(svg::ObjectContainer &container) const override;

private:
    svg::Point point_;
    double stop_radius_;
};

class MapText final : public svg::Drawable {
public:
    MapText(std::string_view data, svg::Point position, svg::Point offset, int label_font_size,
            svg::Color underlayer_color, double underlayer_width, svg::Color fill, bool font_weight = true) :
            data_(data), position_(position), offset_(offset), label_font_size_(label_font_size), underlayer_color_(
                    underlayer_color), underlayer_width_(underlayer_width), fill_(fill), font_weight_(font_weight) {
    }

    void Draw(svg::ObjectContainer &container) const override;

private:
    std::string_view data_;
    svg::Point position_;
    svg::Point offset_;
    int label_font_size_;
    svg::Color underlayer_color_;
    double underlayer_width_;
    svg::Color fill_;
    bool font_weight_;
};

} // shapes

class MapRenderer {
public:
	MapRenderer(transport::TransportCatalogue &db, MapSettings map_settings) : map_settings_(std::move(map_settings)) {
        for (auto it = db.GetBuses()->begin(); it != db.GetBuses()->end(); ++it) {
            buses_sorted_.push_back(&(*it));
        }
        std::sort(buses_sorted_.begin(), buses_sorted_.end(),
                [](const transport::Bus *bus_left, const transport::Bus *bus_right) {
                    return bus_left->name < bus_right->name;
                });

        for (auto it = db.GetStopsWithBuses()->begin(); it != db.GetStopsWithBuses()->end(); ++it) {
            stops_sorted_.push_back(it->first);
        }
        [[maybe_unused]] size_t s = stops_sorted_.size();
        std::sort(stops_sorted_.begin(), stops_sorted_.end(),
                [](const transport::Stop *stop_left, const transport::Stop *stop_right) {
                    return stop_left->name < stop_right->name;
                });
    }

    svg::Document RenderMap();

private:
    MapSettings map_settings_;
    int color_num_ = 0;
    std::deque<const transport::Bus*> buses_sorted_;
    std::deque<const transport::Stop*> stops_sorted_;

    void CleanColor() {
        color_num_ = 0;
    }
    const svg::Color GetNextColor();
};

} // renderer

