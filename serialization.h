#pragma once

#include "json_reader.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <graph.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>

#include <fstream>
#include <map>
#include <memory>
#include <string>

namespace serialization {

using namespace transport;

class Serial {
public:
	explicit Serial(reader::Reader &reader) :
			reader_(reader) {
	}
	void AddRouting(std::unique_ptr<graph::DirectedWeightedGraph<double>> &graph,
			std::unique_ptr<graph::Router<double>> &router) {
		retrouter_ = std::move(router);
		retgraph_ = std::move(graph);
	}

	void SaveDB(const TransportCatalogue &db);
	TransportCatalogue LoadDB();
	std::unique_ptr<graph::DirectedWeightedGraph<double>> ExtractGraph() {
		return std::move(retgraph_);
	}

	std::unique_ptr<graph::Router<double>> ExtractRouter() {
		return std::move(retrouter_);
	}

private:
	reader::Reader &reader_;
	std::unique_ptr<graph::DirectedWeightedGraph<double>> retgraph_;
	std::unique_ptr<graph::Router<double>> retrouter_;
// export
	transport_db::TransportCatalogue MakeDB(const TransportCatalogue &db);

	transport_db::BaseTransportCatalogue CreateTransportDB(const TransportCatalogue &db);
	renderer_db::MapSettings CreateMapSetDB();
	router_db::Router CreateRouterDB();

	transport_db::Stops CreateStopsDB(const TransportCatalogue &db);
	transport_db::Distances CreateDistancesDB(const TransportCatalogue &db);
	transport_db::Buses CreateBusesDB(const TransportCatalogue &db);

	router_db::RoutingSettings CreateRouteSetDB();
	graph_db::GraphClass CreateGraphDB();
	router_db::RouterClass CreateRouterClassDB();
// import
	TransportCatalogue CreateTransportCatalogue(const transport_db::TransportCatalogue &pack);

	TransportCatalogue UpdateTransportCatalogue(const transport_db::BaseTransportCatalogue &pack);
	void ImportMapSetDB(const renderer_db::MapSettings &map_set);
	void ImportRouterDB(const router_db::Router &pack);

	void UpdateStopsDB(TransportCatalogue &tc_for_update, const transport_db::Stops &stops);
	void UpdateDistancesDB(TransportCatalogue &tc_for_update, const transport_db::Distances &dists);
	void UpdateBusesDB(TransportCatalogue &tc_for_update, const transport_db::Buses &buses);

	void ImportRouteSetDB(const router_db::RoutingSettings &route_set);
	void ImportGraphClassDB(const graph_db::GraphClass &graph);
	void ImportRouterClassDB(const router_db::RouterClass &router);

};

} //namespace
