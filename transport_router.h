#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace graph {

using namespace transport;

struct RoutingSettings {
	double bus_wait_time = 0.;
	double bus_velocity = 1.;
};

class GraphBuilder {
public:
	GraphBuilder(const TransportCatalogue &db, RoutingSettings rs) :
			db_(db), settings_(std::move(rs)) {
	}

	std::unique_ptr<DirectedWeightedGraph<double>> Build();

private:
	const TransportCatalogue &db_;
	RoutingSettings settings_;

	template<typename It>
	void AddEdges(DirectedWeightedGraph<double> &graph_, const Bus &bus, double &weight_bus, int &span_count, It it,
			It it2) {
		weight_bus = weight_bus + db_.GetDistanceBeetwen(*(it2 - 1), *it2) * GetOneMeterTime();
		auto edgeid = graph_.AddEdge(
				{ db_.GetStop((*it)->name)->id, db_.GetStop((*it2)->name)->id, settings_.bus_wait_time + weight_bus });
		graph_.SetParams()[edgeid] = { bus.id, (*it)->id, ++span_count, weight_bus, settings_.bus_wait_time };
	}

	inline double GetOneMeterTime() const {
		return 60.0 / (settings_.bus_velocity * 1000.0);
	}
};

class RouterBuilder {
public:
	RouterBuilder(DirectedWeightedGraph<double> &graph) :
			graph_(graph) {
	}
	std::unique_ptr<Router<double>> Build() {
		return std::make_unique<Router<double>>(graph_);
	}
private:
	DirectedWeightedGraph<double> &graph_;
};

} // graph
