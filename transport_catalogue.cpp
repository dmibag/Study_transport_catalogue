#include "transport_catalogue.h"

namespace transport {

void TransportCatalogue::AddDistanceBetween(const Stop *stop1, const Stop *stop2, uint64_t dist) {
    stops_to_distance_[ { stop1, stop2 }] = dist;
}

uint64_t TransportCatalogue::GetDistanceBeetwen(const Stop *stop1, const Stop *stop2) const {
    if (stops_to_distance_.count( { stop1, stop2 })) return stops_to_distance_.at( { stop1, stop2 });
    else if (stops_to_distance_.count( { stop2, stop1 })) return stops_to_distance_.at( { stop2, stop1 });
    return 0;
}

void TransportCatalogue::AddStop(Stop &stop) {
    stops_.push_back(std::move(stop));
    name_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(Bus &bus) {
    buses_.push_back(std::move(bus));
    auto *ptr = &buses_.back();
    name_to_bus_[buses_.back().name] = ptr;
    for (const Stop *stop : ptr->stops)
        stop_to_buses_[stop].insert(ptr->name);
}

const std::set<std::string_view>* TransportCatalogue::GetBusesNamesByStopName(const std::string_view name) const {
    static const std::set<std::string_view> empty;
    const Stop *stop = name_to_stop_.at(name);
    if (stop_to_buses_.count(stop)) return &(stop_to_buses_.at(stop));
    return &empty;
}

const Bus* TransportCatalogue::GetBus(const std::string_view name) const {
    if (!name_to_bus_.count(name)) return nullptr;
    return name_to_bus_.at(name);
}

const Stop* TransportCatalogue::GetStop(const std::string_view name) const {
    if (!name_to_stop_.count(name)) return nullptr;
    return name_to_stop_.at(name);
}

uint64_t TransportCatalogue::GetRealDistance(const Bus *bus) const {
    uint64_t dist { 0 };
    for (auto it = bus->stops.begin(); it != bus->stops.end() - 1; ++it) {
        dist += GetDistanceBeetwen(*it, *(it + 1));
        if (!bus->cicle) dist += GetDistanceBeetwen(*(it + 1), *it);
    }
    return dist;
}

double TransportCatalogue::GetGeoDistance(const Bus *bus) const {
    double dist { 0. };
    for (auto it = bus->stops.begin(); it != bus->stops.end() - 1; ++it) {
        dist += geo::ComputeDistance((*it)->coord, (*(it + 1))->coord);
    }
    if (!bus->cicle) {
        dist = dist * 2;
    }
    return dist;
}

} // namespace transport
