#pragma once

#include <unordered_set>
#include <string>
#include <vector>

#include "geo.h"

namespace transport {

struct Stop {
	size_t id;
    std::string name;
    geo::Coordinates coord;
};

struct Bus {
	size_t id;
    std::string name;
    std::vector<const Stop*> stops;
    std::unordered_set<const Stop*> stops_unique;
    bool cicle;

    int GetAllStopsCount() const {
        return (cicle) ? stops.size() : stops.size() * 2 - 1;
    }

    int GetUniqueStopsCount() const {
        return stops_unique.size();
    }
};

} // transport

namespace serialization {

struct SerialSettings {
    std::string filename;
};

} // serialization
