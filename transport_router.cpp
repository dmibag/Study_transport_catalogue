#include "transport_router.h"

namespace graph {

using namespace std;

std::unique_ptr<DirectedWeightedGraph<double>> GraphBuilder::Build() {
	auto graph_ = std::make_unique<DirectedWeightedGraph<double>>((*db_.GetStops()).size());
	for (const auto &bus : *db_.GetBuses()) {
		if (bus.cicle) {
			for (auto it = bus.stops.begin(); it != bus.stops.end() - 1; ++it) {
				double weight_bus { 0 };
				int span_count { 0 };
				for (auto it2 = it + 1; it2 != bus.stops.end(); ++it2) {
					AddEdges(*graph_, bus, weight_bus, span_count, it, it2);
				}
			}
		} else { // не круговой маршрут
			std::vector<const Stop*> new_stops;
			for (auto it = bus.stops.begin(); it != bus.stops.end(); ++it) {
				new_stops.emplace_back(*it);
			}
			for (auto it = bus.stops.rbegin() + 1; it != bus.stops.rend(); ++it) {
				new_stops.emplace_back(*it);
			}
			for (auto it = new_stops.begin(); it != new_stops.end() - 1; ++it) {
				double weight_bus { 0 };
				int span_count { 0 };
				for (auto it2 = it + 1; it2 != new_stops.end(); ++it2) {
					AddEdges(*graph_, bus, weight_bus, span_count, it, it2);
				}
			}
		}
	}
	return graph_;
}

} // graph
