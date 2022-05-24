#include "serialization.h"

namespace serialization {

struct SolutionColor {
	svg_db::Color operator()(std::monostate) const {
		svg_db::Color color;
		color.set_type(0);
		return color;
	}
	svg_db::Color operator()(std::string str) const {
		svg_db::Color color;
		color.set_type(1);
		*color.mutable_valstr() = str;
		return color;
	}
	svg_db::Color operator()(svg::Rgb rgb) const {
		svg_db::Color color;
		color.set_type(2);
		color.mutable_valrgba()->set_red(rgb.red);
		color.mutable_valrgba()->set_green(rgb.green);
		color.mutable_valrgba()->set_blue(rgb.blue);
		color.mutable_valrgba()->set_opacity(0.);
		return color;
	}
	svg_db::Color operator()(svg::Rgba rgba) const {
		svg_db::Color color;
		color.set_type(3);
		color.mutable_valrgba()->set_red(rgba.red);
		color.mutable_valrgba()->set_green(rgba.green);
		color.mutable_valrgba()->set_blue(rgba.blue);
		color.mutable_valrgba()->set_opacity(rgba.opacity);
		return color;
	}
};

svg::Color ToColor(const svg_db::Color &color) {
	svg::Color ret;
	switch (color.type()) {
	case 1:
		ret = color.valstr();
		break;
	case 2:
		ret = svg::Rgb(color.valrgba().red(), color.valrgba().green(), color.valrgba().blue());
		break;
	case 3:
		ret = svg::Rgba(color.valrgba().red(), color.valrgba().green(), color.valrgba().blue(),
				color.valrgba().opacity());
		break;
	}
	return ret;
}

transport_db::TransportCatalogue Serial::MakeDB(const TransportCatalogue &db) {
	transport_db::TransportCatalogue pack;
	*pack.mutable_transport() = std::move(CreateTransportDB(db));
	*pack.mutable_map() = std::move(CreateMapSetDB());
	*pack.mutable_router() = std::move(CreateRouterDB());
	return pack;
}

void Serial::SaveDB(const TransportCatalogue &db) {
	std::ofstream out(reader_.GetSerialSettings().filename, std::ios::binary);
	MakeDB(db).SerializePartialToOstream(&out);
}

transport_db::BaseTransportCatalogue Serial::CreateTransportDB(const TransportCatalogue &db) {
	transport_db::BaseTransportCatalogue transport;
	*transport.mutable_stops() = std::move(CreateStopsDB(db));
	*transport.mutable_dists() = std::move(CreateDistancesDB(db));
	*transport.mutable_buses() = std::move(CreateBusesDB(db));
	return transport;
}

transport_db::Stops Serial::CreateStopsDB(const TransportCatalogue &db) {
	transport_db::Stops stops_db;
	for (const auto &stop : *db.GetStops()) {
		transport_db::Stop stop_db;
		*stop_db.mutable_name() = stop.name;
		stop_db.mutable_coord()->set_lat(stop.coord.lat);
		stop_db.mutable_coord()->set_lng(stop.coord.lng);
		stop_db.set_id(stop.id);
		*stops_db.add_stops() = std::move(stop_db);
	}
	return stops_db;
}

transport_db::Distances Serial::CreateDistancesDB(const TransportCatalogue &db) {
	transport_db::Distances dists_db;
	for (const auto &dist : *db.GetStopsToDist()) {
		transport_db::Distance dist_db;
		dist_db.set_stop_from(db.GetStop(dist.first.first->name)->id);
		dist_db.set_stop_to(db.GetStop(dist.first.second->name)->id);
		dist_db.set_distance(dist.second);
		*dists_db.add_distances() = std::move(dist_db);
	}
	return dists_db;
}

transport_db::Buses Serial::CreateBusesDB(const TransportCatalogue &db) {
	transport_db::Buses buses_db;
	for (const auto &bus : *db.GetBuses()) {
		transport_db::Bus bus_db;
		bus_db.set_id(bus.id);
		*bus_db.mutable_name() = bus.name;
		for (const auto &stop : bus.stops) {
			bus_db.add_stops(db.GetStop(stop->name)->id);
		}
		for (const auto &stop_unique : bus.stops_unique) {
			bus_db.add_stops_unique(db.GetStop(stop_unique->name)->id);
		}
		bus_db.set_cicle(bus.cicle);
		*buses_db.add_buses() = std::move(bus_db);
	}
	return buses_db;
}

renderer_db::MapSettings Serial::CreateMapSetDB() {
	renderer_db::MapSettings map_set_db;
	const auto &map_set = reader_.GetMapSettings();
	map_set_db.set_width(map_set.width);
	map_set_db.set_height(map_set.height);
	map_set_db.set_padding(map_set.padding);
	map_set_db.set_line_width(map_set.line_width);
	map_set_db.set_stop_radius(map_set.stop_radius);
	map_set_db.set_bus_label_font_size(map_set.bus_label_font_size);
	map_set_db.mutable_bus_label_offset()->set_x(map_set.bus_label_offset.x);
	map_set_db.mutable_bus_label_offset()->set_y(map_set.bus_label_offset.y);
	map_set_db.set_stop_label_font_size(map_set.stop_label_font_size);
	map_set_db.mutable_stop_label_offset()->set_x(map_set.stop_label_offset.x);
	map_set_db.mutable_stop_label_offset()->set_y(map_set.stop_label_offset.y);
	*map_set_db.mutable_underlayer_color() = std::move(std::visit(SolutionColor { }, map_set.underlayer_color));
	map_set_db.set_underlayer_width(map_set.underlayer_width);
	for (const auto &color : map_set.color_palette) {
		*map_set_db.add_color_palette() = std::move(std::move(std::visit(SolutionColor { }, color)));
	}
	return map_set_db;
}

router_db::RoutingSettings Serial::CreateRouteSetDB() {
	router_db::RoutingSettings route_set_db;
	route_set_db.set_bus_velocity(reader_.GetRouterSettings().bus_velocity);
	route_set_db.set_bus_wait_time(reader_.GetRouterSettings().bus_wait_time);
	return route_set_db;
}

graph_db::GraphClass Serial::CreateGraphDB() {
	graph_db::GraphClass graph_db;
	const auto &edges = (*retgraph_).GetEdges();

	for (const graph::Edge<double> &edge : edges) {
		graph_db::Edge edge_db;
		edge_db.set_from(edge.from);
		edge_db.set_to(edge.to);
		edge_db.set_weight(edge.weight);
		*graph_db.add_edges() = std::move(edge_db);
	}
	const auto &inclists = (*retgraph_).GetIncidenceLists();
	for (const std::vector<graph::EdgeId> &list : inclists) {
		graph_db::IncidenceList list_db;
		for (size_t id : list) {
			list_db.add_edgeid(id);
		}
		*graph_db.add_incidence_lists() = std::move(list_db);
	}
	const auto &params = (*retgraph_).GetParams();
	for (const auto &pair : params) {
		graph_db::EdgeIdToParams params_db;
		params_db.mutable_params()->set_bus_id(pair.second.bus_id);
		params_db.mutable_params()->set_stop_id(pair.second.stop_id);
		params_db.mutable_params()->set_span_count(pair.second.span_count);
		params_db.mutable_params()->set_weight_bus(pair.second.weight_bus);
		params_db.mutable_params()->set_bus_wait_time(pair.second.bus_wait_time);
		params_db.set_edgeid(pair.first);
		*graph_db.add_edgeid_to_params() = std::move(params_db);
	}
	return graph_db;
}

router_db::RouterClass Serial::CreateRouterClassDB() {
	router_db::RouterClass router_db;
	const auto &rid = (*retrouter_).GetRoutesInternalData();
	router_db::RoutesInternalData rid_db;
	for (const auto &vrid : rid) {
		router_db::VectorRID vec_db;
		for (const auto &part : vrid) {
			router_db::RouteInternalDataOpt opt_db;
			if (part) {
				router_db::RouteInternalData r;
				r.set_weight(part->weight);
				if (part->prev_edge)
					r.mutable_prev()->set_prev_edge(*part->prev_edge);
				*opt_db.mutable_route_internal_data() = std::move(r);
			}
			*vec_db.add_route_internal_data_opt() = std::move(opt_db);
		}
		*rid_db.add_vector_rid() = std::move(vec_db);
	}
	*router_db.mutable_routes_internal_data() = std::move(rid_db);
	return router_db;
}

router_db::Router Serial::CreateRouterDB() {
	router_db::Router router;
	*router.mutable_route() = std::move(CreateRouteSetDB());
	*router.mutable_graph() = std::move(CreateGraphDB());
	*router.mutable_router() = std::move(CreateRouterClassDB());
	return router;
}

TransportCatalogue Serial::LoadDB() {
	transport_db::TransportCatalogue pack;
	{
		std::ifstream in(reader_.GetSerialSettings().filename, std::ios::binary);
		pack.ParseFromIstream(&in);
	}
	return CreateTransportCatalogue(pack);
}

TransportCatalogue Serial::CreateTransportCatalogue(const transport_db::TransportCatalogue &pack) {
	ImportMapSetDB(pack.map());
	ImportRouterDB(pack.router());
	return UpdateTransportCatalogue(pack.transport());
}

TransportCatalogue Serial::UpdateTransportCatalogue(const transport_db::BaseTransportCatalogue &pack) {
	TransportCatalogue tc;
	UpdateStopsDB(tc, pack.stops());
	UpdateDistancesDB(tc, pack.dists());
	UpdateBusesDB(tc, pack.buses());
	return tc;
}

void Serial::ImportMapSetDB(const renderer_db::MapSettings &map_set_db) {
	auto &map_settings = reader_.GetMapSettings();
	map_settings.width = map_set_db.width();
	map_settings.height = map_set_db.height();
	map_settings.padding = map_set_db.padding();
	map_settings.line_width = map_set_db.line_width();
	map_settings.stop_radius = map_set_db.stop_radius();
	map_settings.bus_label_font_size = map_set_db.bus_label_font_size();
	map_settings.bus_label_offset.x = map_set_db.bus_label_offset().x();
	map_settings.bus_label_offset.y = map_set_db.bus_label_offset().y();
	map_settings.stop_label_font_size = map_set_db.stop_label_font_size();
	map_settings.stop_label_offset.x = map_set_db.stop_label_offset().x();
	map_settings.stop_label_offset.y = map_set_db.stop_label_offset().y();
	map_settings.underlayer_color = ToColor(map_set_db.underlayer_color());
	map_settings.underlayer_width = map_set_db.underlayer_width();
	for (int i = 0; i < map_set_db.color_palette_size(); ++i) {
		map_settings.color_palette.push_back(ToColor(map_set_db.color_palette(i)));
	}
}

void Serial::ImportRouterDB(const router_db::Router &pack) {
	ImportRouteSetDB(pack.route());
	ImportGraphClassDB(pack.graph());
	ImportRouterClassDB(pack.router());
}

void Serial::UpdateStopsDB(TransportCatalogue &tc_for_update, const transport_db::Stops &stops) {
	for (int i = 0; i < stops.stops_size(); ++i) {
		Stop stop;
		stop.id = stops.stops(i).id();
		stop.name = stops.stops(i).name();
		stop.coord.lat = stops.stops(i).coord().lat();
		stop.coord.lng = stops.stops(i).coord().lng();
		tc_for_update.AddStop(stop);
	}
}

void Serial::UpdateDistancesDB(TransportCatalogue &tc_for_update, const transport_db::Distances &dists) {
	for (int i = dists.distances_size() - 1; i >= 0; --i) {
		std::string_view stop_from = (*tc_for_update.GetStops()).at(dists.distances(i).stop_from()).name;
		std::string_view stop_to = (*tc_for_update.GetStops()).at(dists.distances(i).stop_to()).name;
		uint64_t loaded_dist = dists.distances(i).distance();
		tc_for_update.AddDistanceBetween(tc_for_update.GetStop(stop_from), tc_for_update.GetStop(stop_to), loaded_dist);
	}
}

void Serial::UpdateBusesDB(TransportCatalogue &tc_for_update, const transport_db::Buses &buses) {
	for (int i = 0; i < buses.buses_size(); ++i) {
		Bus bus;
		bus.id = buses.buses(i).id();
		bus.name = buses.buses(i).name();
		bus.cicle = buses.buses(i).cicle();
		for (int j = 0; j < buses.buses(i).stops_size(); ++j) {
			bus.stops.push_back(&(*tc_for_update.GetStops()).at(buses.buses(i).stops(j)));
			bus.stops_unique.insert(bus.stops.back());
		}
		tc_for_update.AddBus(bus);
	}
}

void Serial::ImportRouteSetDB(const router_db::RoutingSettings &route_set_db) {
	reader_.GetRouterSettings().bus_velocity = route_set_db.bus_velocity();
	reader_.GetRouterSettings().bus_wait_time = route_set_db.bus_wait_time();
}

void Serial::ImportGraphClassDB(const graph_db::GraphClass &graph) {
	std::vector<graph::Edge<double>> edges;
	std::vector<std::vector<graph::EdgeId>> incidence_lists;
	std::map<graph::EdgeId, graph::EdgeData> edgeid_to_params;

	for (int i = 0; i < graph.edges_size(); ++i) {
		edges.push_back( { graph.edges(i).from(), graph.edges(i).to(), graph.edges(i).weight() });
	}
	for (int i = 0; i < graph.incidence_lists_size(); ++i) {
		std::vector<graph::EdgeId> vec_edge;
		for (int j = 0; j < graph.incidence_lists(i).edgeid_size(); j++) {
			vec_edge.push_back(graph.incidence_lists(i).edgeid(j));
		}
		incidence_lists.push_back(vec_edge);
	}
	for (int i = 0; i < graph.edgeid_to_params_size(); ++i) {
		graph::EdgeData data;
		data.bus_id = graph.edgeid_to_params(i).params().bus_id();
		data.stop_id = graph.edgeid_to_params(i).params().stop_id();
		data.span_count = graph.edgeid_to_params(i).params().span_count();
		data.weight_bus = graph.edgeid_to_params(i).params().weight_bus();
		data.bus_wait_time = graph.edgeid_to_params(i).params().bus_wait_time();
		edgeid_to_params[graph.edgeid_to_params(i).edgeid()] = std::move(data);
	}
	retgraph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(std::move(edges), std::move(incidence_lists),
			std::move(edgeid_to_params));
}

void Serial::ImportRouterClassDB(const router_db::RouterClass &router) {
	using opt_rid = std::optional<graph::Router<double>::RouteInternalData>;

	std::vector<std::vector<opt_rid>> routes_internal_data;
	for (int i = 0; i < router.routes_internal_data().vector_rid_size(); ++i) {
		std::vector<opt_rid> vector_rid;
		for (int j = 0; j < router.routes_internal_data().vector_rid(i).route_internal_data_opt_size(); ++j) {
			opt_rid r(std::nullopt);
			if (router.routes_internal_data().vector_rid(i).route_internal_data_opt(j).has_route_internal_data()) {
				double w =
						router.routes_internal_data().vector_rid(i).route_internal_data_opt(j).route_internal_data().weight();
				std::optional<size_t> prev_edge;
				if (router.routes_internal_data().vector_rid(i).route_internal_data_opt(j).route_internal_data().has_prev()) {
					prev_edge =
							router.routes_internal_data().vector_rid(i).route_internal_data_opt(j).route_internal_data().prev().prev_edge();
				}
				r = { w, prev_edge };
			}
			vector_rid.push_back(r);
		}
		routes_internal_data.push_back(vector_rid);
	}
	retrouter_ = std::make_unique<graph::Router<double>>(*retgraph_, std::move(routes_internal_data));
}

} // namespace
