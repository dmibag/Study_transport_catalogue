#include "request_handler.h"
#include "transport_catalogue.h"

namespace request {

using namespace std::literals;

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view &bus_name) const {
	if (db_ == nullptr) {
		throw TransportError("TransportCatalogue is not initialized");
	}
	auto *bus = db_->GetBus(bus_name);
	if (!bus) {
		return std::nullopt;
	}
	BusStat busstat;
	busstat.stop_count = bus->GetAllStopsCount();
	busstat.curvature = (double) db_->GetRealDistance(bus) / db_->GetGeoDistance(bus);
	busstat.route_length = db_->GetRealDistance(bus);
	busstat.unique_stop_count = bus->GetUniqueStopsCount();
	return busstat;
}

const std::set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view &stop_name) const {
	if (db_ == nullptr) {
		throw TransportError("TransportCatalogue is not initialized");
	}
	auto *stop = db_->GetStop(stop_name);
	if (!stop) {
		return nullptr;
	}
	return db_->GetBusesNamesByStopName(stop_name);
}

svg::Document RequestHandler::RenderMap() {
	if (renderer_ == nullptr) {
		throw TransportError("Renderer is not initialized");
	}
	return renderer_->RenderMap();
}

std::optional<typename graph::Router<double>::RouteInfo> RequestHandler::BuildRoute(const request::Request &request) {
	if (router_ == nullptr) {
		throw TransportError("Router is not initialized");
	}
	return router_->BuildRoute(db_->GetStop(request.from)->id, db_->GetStop(request.to)->id);
}

} // request
