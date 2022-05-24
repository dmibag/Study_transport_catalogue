#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <unordered_map>

namespace transport {

class TransportError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TransportCatalogue {
private:
    struct DistHash {
        size_t operator()(const std::pair<const Stop*, const Stop*> &bell) const {
            return std::hash<const void*>{}(bell.first) + std::hash<const void*>{}(bell.second) * 37;
        }
    };

public:
	using DistCont = std::unordered_map<std::pair<const Stop*, const Stop*>, uint64_t, DistHash>;

    void AddStop(Stop &stop);
    void AddBus(Bus &bus);
    const std::set<std::string_view>* GetBusesNamesByStopName(const std::string_view name) const;
    const Bus* GetBus(const std::string_view name) const;
    const Stop* GetStop(const std::string_view name) const;

    void AddDistanceBetween(const Stop *stop1, const Stop *stop2, uint64_t dist);
    uint64_t GetDistanceBeetwen(const Stop *stop1, const Stop *stop2) const;
    uint64_t GetRealDistance(const Bus *bus) const;
    double GetGeoDistance(const Bus *bus) const;

    const std::deque<Bus>* GetBuses() const {
        return &buses_;
    }
    const std::deque<Stop>* GetStops() const {
        return &stops_;
    }
    const DistCont* GetStopsToDist() const {
        return &stops_to_distance_;
    }
    const std::unordered_map<const Stop*, std::set<std::string_view>>* GetStopsWithBuses() const {
        return &stop_to_buses_;
    }

private:

    std::unordered_map<std::string_view, const Bus*> name_to_bus_;
    std::unordered_map<std::string_view, const Stop*> name_to_stop_;
    std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_buses_;
    DistCont stops_to_distance_;
    std::deque<Bus> buses_;
    std::deque<Stop> stops_;

};

} // namespace transport
