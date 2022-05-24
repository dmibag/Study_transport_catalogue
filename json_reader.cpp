#include "json_reader.h"

namespace reader {

using namespace json;
using namespace std::literals;

void Reader::UpdateTransportCatalogue(const Array &data) {
	int count{0};
    for (const Node &node : data) {
        const Dict &dict = node.AsDict();
        if (dict.at("type"s).AsString() == "Stop"s) {
            Stop stop;
            stop.id = count++;
            stop.name = dict.at("name"s).AsString();
            stop.coord.lat = dict.at("latitude"s).AsDouble();
            stop.coord.lng = dict.at("longitude"s).AsDouble();
            handler_.AddStop(stop);
        }
    }
    for (const Node &node : data) {
        const Dict &dict = node.AsDict();
        if (dict.at("type"s).AsString() == "Stop"s) {
            if (dict.count("road_distances"s)) {
                auto stopname1 = dict.at("name"s).AsString();
                for (const auto &stop2 : dict.at("road_distances"s).AsDict()) {
                	handler_.AddDistanceBetween(handler_.GetStop(stopname1), handler_.GetStop(stop2.first), stop2.second.AsInt());
                }
            }
        }
    }
    count = 0;
    for (const Node &node : data) {
        const Dict &dict = node.AsDict();
        if (dict.at("type"s).AsString() == "Bus"s) {
            Bus bus;
            bus.id = count++;
            bus.name = dict.at("name"s).AsString();
            bus.cicle = dict.at("is_roundtrip"s).AsBool();
            for (const auto &stop : dict.at("stops"s).AsArray()) {
                bus.stops.push_back(handler_.GetStop(stop.AsString()));
                bus.stops_unique.insert(bus.stops.back());
            }
            handler_.AddBus(bus);
        }
    }
}

request::Request Reader::GetRequestFromJSONMap(const Dict &dict) const {
	request::Request request;
    request.id = dict.at("id"s).AsInt();
    if (dict.at("type"s).AsString() == "Stop"s) request.type = request::RequestType::Stop;
    else if (dict.at("type"s).AsString() == "Bus"s) request.type = request::RequestType::Bus;
    else if (dict.at("type"s).AsString() == "Map"s) request.type = request::RequestType::Map;
    else if (dict.at("type"s).AsString() == "Route"s) request.type = request::RequestType::Route;
    else throw TransportError("Error request type");
    if (dict.count("name"s)) request.name = dict.at("name"s).AsString();
    if (dict.count("from"s)) request.from = dict.at("from"s).AsString();
    if (dict.count("to"s)) request.to = dict.at("to"s).AsString();
    return request;
}

void Reader::LoadMapSettings(const Dict &data) {
    const json::Array *arr;

    ms_.width = data.at("width"s).AsDouble();
    ms_.height = data.at("height"s).AsDouble();
    ms_.padding = data.at("padding"s).AsDouble();
    ms_.line_width = data.at("line_width"s).AsDouble();
    ms_.stop_radius = data.at("stop_radius"s).AsDouble();
    ms_.bus_label_font_size = data.at("bus_label_font_size"s).AsInt();

    arr = &data.at("bus_label_offset"s).AsArray();
    ms_.bus_label_offset = { (*arr)[0].AsDouble(), (*arr)[1].AsDouble() };
    ms_.stop_label_font_size = data.at("stop_label_font_size"s).AsInt();

    arr = &data.at("stop_label_offset"s).AsArray();
    ms_.stop_label_offset = { (*arr)[0].AsDouble(), (*arr)[1].AsDouble() };

    const auto *node = &data.at("underlayer_color");
    if (node->IsArray()) {
        arr = &data.at("underlayer_color"s).AsArray();
        (arr->size() == 3) ?
                ms_.underlayer_color = svg::Rgb((*arr)[0].AsInt(), (*arr)[1].AsInt(), (*arr)[2].AsInt()) :
                ms_.underlayer_color = svg::Rgba((*arr)[0].AsInt(), (*arr)[1].AsInt(), (*arr)[2].AsInt(),
                        (*arr)[3].AsDouble());
    }
    else ms_.underlayer_color = data.at("underlayer_color"s).AsString();

    ms_.underlayer_width = data.at("underlayer_width"s).AsDouble();

    arr = &data.at("color_palette"s).AsArray();
    for (auto it = arr->begin(); it != arr->end(); ++it) {
        if (it->IsArray()) {
            const auto *arr_color = &it->AsArray();
            if (arr_color->size() == 3) {
                ms_.color_palette.push_back(
                        svg::Rgb((*arr_color)[0].AsInt(), (*arr_color)[1].AsInt(), (*arr_color)[2].AsInt()));
            }
            else ms_.color_palette.push_back(
                    svg::Rgba((*arr_color)[0].AsInt(), (*arr_color)[1].AsInt(), (*arr_color)[2].AsInt(),
                            (*arr_color)[3].AsDouble()));
        }
        else ms_.color_palette.push_back(it->AsString());
    }
}
void Reader::LoadRouterSettings(const Dict &data) {

    double bus_wait_time = data.at("bus_wait_time"s).AsDouble();
    double bus_velocity = data.at("bus_velocity"s).AsDouble();
    rs_ = { bus_wait_time, bus_velocity };
}
void Reader::LoadSerialSettings(const Dict &data) {

    std::string filename = data.at("file"s).AsString();
    ss_ = { std::move(filename) };
}

void Reader::LoadData(std::istream &input) {
    const auto basenode = Load(input).GetRoot();
    if (!basenode.IsDict()) throw TransportError("Error loading JSON");
    const Dict basedict = std::move(basenode.AsDict());
    if (basedict.count("base_requests"s)) {
        UpdateTransportCatalogue(basedict.at("base_requests"s).AsArray());
    }
    if (basedict.count("stat_requests"s)) {
        for (const auto &request : basedict.at("stat_requests"s).AsArray()) {
            requests_.push_back(GetRequestFromJSONMap(request.AsDict()));
        }
    }
    if (basedict.count("render_settings"s)) {
        LoadMapSettings(basedict.at("render_settings"s).AsDict());
    }
    if (basedict.count("routing_settings"s)) {
        LoadRouterSettings(basedict.at("routing_settings"s).AsDict());
    }
    if (basedict.count("serialization_settings"s)) {
        LoadSerialSettings(basedict.at("serialization_settings"s).AsDict());
    }
}
//-----------------------------------------------------------------------------------------

void Reader::ProcessStop(json::Array &arr, const request::Request &request) {
    auto *buses = handler_.GetBusesByStop(request.name);
    if (!buses) {
        arr.emplace_back(
                json::Builder{}.StartDict()
                    .Key("request_id"s).Value(request.id)
                    .Key("error_message"s).Value("not found"s)
                .EndDict().Build());
    }
    else {
        json::Array buses_names;
        for (auto bus : *buses) {
            buses_names.push_back(std::string(bus));
        }
        arr.emplace_back(
                json::Builder{}.StartDict()
                    .Key("request_id"s).Value(request.id)
                    .Key("buses"s).Value(buses_names)
                .EndDict().Build());
    }
}

void Reader::ProcessBus(json::Array &arr, const request::Request &request) {
    if (auto stat = handler_.GetBusStat(request.name)) {
        arr.emplace_back(
                json::Builder{}.StartDict()
                    .Key("request_id"s).Value(request.id)
                    .Key("curvature"s).Value(stat->curvature)
                    .Key("route_length"s).Value(stat->route_length)
                    .Key("stop_count"s).Value(stat->stop_count)
                    .Key("unique_stop_count"s).Value(stat->unique_stop_count)
                .EndDict().Build());
    }
    else {
		arr.emplace_back(
			json::Builder{}.StartDict()
				.Key("request_id"s).Value(request.id)
				.Key("error_message"s).Value("not found"s)
			.EndDict().Build());
    }
}

void Reader::ProcessMap(json::Array &arr, const request::Request &request) {
    std::stringstream map_string;
    handler_.RenderMap().Render(map_string);
	arr.emplace_back(
			json::Builder{}.StartDict()
				.Key("request_id"s).Value(request.id)
				.Key("map"s).Value(map_string.str())
			.EndDict().Build());
}

void Reader::ProcessRoute(json::Array &arr, const request::Request &request) {
    auto info = handler_.BuildRoute(request);

    if (info) {
        json::Array add_array;
        for (auto &bus : info->edges) {
            add_array.push_back(
                    json::Builder{}.StartDict()
                        .Key("stop_name").Value((*handler_.GetStops())[bus.stop_id].name)
                        .Key("time").Value(bus.bus_wait_time)
                        .Key("type").Value("Wait")
                    .EndDict().Build().AsDict());
            add_array.push_back(
                    json::Builder{}.StartDict()
                        .Key("bus").Value((*handler_.GetStops())[bus.bus_id].name)
                        .Key("span_count").Value(bus.span_count)
                        .Key("time").Value(bus.weight_bus)
                        .Key("type").Value("Bus")
                    .EndDict().Build().AsDict());
        }
        arr.emplace_back(
                json::Builder{}.StartDict()
                    .Key("request_id"s).Value(request.id)
                    .Key("total_time"s).Value(info->weight)
                    .Key("items").Value(add_array)
                .EndDict().Build());
    }
    else {
        arr.emplace_back(
                json::Builder{}.StartDict()
                    .Key("request_id"s).Value(request.id)
                    .Key("error_message"s).Value("not found"s)
                .EndDict().Build());
    }
}

void Reader::ProcessRequests(std::ostream& output) {
    if (!requests_.empty()) {
        json::Array arr;
        arr.reserve(requests_.size());
        for (const request::Request &request : requests_) {
            if (request.type == request::RequestType::Stop) {
                ProcessStop(arr, request);
            }
            else if (request.type == request::RequestType::Bus) {
                ProcessBus(arr, request);
            }
            else if (request.type == request::RequestType::Map) {
                ProcessMap(arr, request);
            }
            else if (request.type == request::RequestType::Route) {
                ProcessRoute(arr, request);
            }
        }
        json::Print(json::Document { arr }, output);
    }
}

} // reader

