#pragma once

#include "map_renderer.h"
#include "transport_router.h"

#include <optional>
#include <string>

namespace request {

using namespace transport;

struct BusStat {
	double curvature;
	int route_length;
	int stop_count;
	int unique_stop_count;
};

enum class RequestType {
	Stop, Bus, Map, Route
};

struct Request {
	int id;
	RequestType type;
	std::string name;
	std::string from;
	std::string to;
};

class RequestHandler {
public:
	RequestHandler() = default;
	explicit RequestHandler(TransportCatalogue *db) :
			db_(db) {
	}
// Begin Transport methods
	std::optional<BusStat> GetBusStat(const std::string_view &bus_name) const;
	const std::set<std::string_view>* GetBusesByStop(const std::string_view &stop_name) const;
	void AddStop(Stop &stop) {
		if (db_ == nullptr) {
			throw TransportError("TransportCatalogue is not initialized");
		}
		db_->AddStop(stop);
	}
	void AddDistanceBetween(const Stop *stop_from, const Stop *stop_to, uint64_t dist) {
		if (db_ == nullptr) {
			throw TransportError("TransportCatalogue is not initialized");
		}
		db_->AddDistanceBetween(stop_from, stop_to, dist);
	}
	const Stop* GetStop(const std::string_view name) {
		if (db_ == nullptr) {
			throw TransportError("TransportCatalogue is not initialized");
		}
		return db_->GetStop(name);
	}
	void AddBus(Bus &bus) {
		if (db_ == nullptr) {
			throw TransportError("TransportCatalogue is not initialized");
		}
		db_->AddBus(bus);
	}
	const std::deque<Stop>* GetStops() {
		if (db_ == nullptr) {
			throw TransportError("TransportCatalogue is not initialized");
		}
		return db_->GetStops();
	}
// End Transport methods
	svg::Document RenderMap();

	std::optional<typename graph::Router<double>::RouteInfo> BuildRoute(const request::Request &request);

	void InitRenderer(renderer::MapRenderer *renderer) {
		renderer_ = renderer;
	}
	void InitRouter(graph::Router<double> *router) {
		router_ = router;
	}
	void InitTransportCatalogue(TransportCatalogue *db) {
		db_ = db;
	}

private:
	TransportCatalogue *db_ = nullptr;
	graph::Router<double> *router_ = nullptr;
	renderer::MapRenderer *renderer_ = nullptr;
};

} // request
