#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream &stream = std::cerr) {
	stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		PrintUsage();
		return 1;
	}

	const std::string_view mode(argv[1]);

	if (mode == "make_base"sv)

	{

		transport::TransportCatalogue db;
		request::RequestHandler rh(&db);
		reader::Reader rd(rh);
		rd.LoadData(std::cin);
		auto graph = graph::GraphBuilder(db, rd.GetRouterSettings()).Build();
		auto router = graph::RouterBuilder(*graph).Build();

		serialization::Serial proto(rd);

		proto.AddRouting(graph, router);
		proto.SaveDB(db);

	} else if (mode == "process_requests"sv) {

		request::RequestHandler rh;
		reader::Reader rd(rh);
		rd.LoadData(std::cin);

		serialization::Serial proto(rd);

		transport::TransportCatalogue db = proto.LoadDB();
		auto graph = proto.ExtractGraph();
		auto router = proto.ExtractRouter();

		renderer::MapRenderer mr(db, rd.GetMapSettings());

		rh.InitTransportCatalogue(&db);
		rh.InitRenderer(&mr);
		rh.InitRouter(&(*router));
		rd.ProcessRequests(std::cout);

	} else {
		PrintUsage();
		return 1;
	}

}
