#pragma once

#include "json_builder.h"
#include "request_handler.h"

namespace reader {

using namespace transport;

class Reader {
public:
	explicit Reader(request::RequestHandler &rh) : handler_(rh) {}
	void LoadData(std::istream &input);
	renderer::MapSettings& GetMapSettings() {
		return ms_;
	}
	graph::RoutingSettings& GetRouterSettings() {
		return rs_;
	}
	std::vector<request::Request>& GetRequests () {
		return requests_;
	}
	serialization::SerialSettings& GetSerialSettings() {
		return ss_;
	}
    void ProcessRequests(std::ostream& output);

private:
    std::vector<request::Request> requests_;
    request::RequestHandler &handler_;

    renderer::MapSettings ms_;
    graph::RoutingSettings rs_;
    serialization::SerialSettings ss_;

    request::Request GetRequestFromJSONMap(const json::Dict &dict) const;

    void UpdateTransportCatalogue(const json::Array &data);
    void LoadMapSettings(const json::Dict &data);
    void LoadRouterSettings(const json::Dict &data);
    void LoadSerialSettings(const json::Dict &data);

    void ProcessStop(json::Array &arr, const request::Request &request);
    void ProcessBus(json::Array &arr, const request::Request &request);
    void ProcessMap(json::Array &arr, const request::Request &request);
    void ProcessRoute(json::Array &arr, const request::Request &request);

};

} // reader
