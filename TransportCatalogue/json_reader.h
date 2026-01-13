#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include <sstream>

class JsonReader {
public:
    void ReadAndExecuteBaseRequests(transport::TransportCatalogue& tc, const json::Node& root);
    const std::ostringstream& GetMap();

    json::Node ExecuteStatRequests(const transport::TransportCatalogue& tc,
        const json::Node& root);
private:
    void AddStops(const json::Array& requests, transport::TransportCatalogue& tc);
    void AddRoutes(const json::Array& requests, transport::TransportCatalogue& tc);
    void AddBuses(const json::Array& requests, transport::TransportCatalogue& tc);
    void AddMap(const json::Dict& root_map, transport::TransportCatalogue& tc);
    void AddStop(json::Dict& res_dict, const transport::TransportCatalogue& tc, const json::Dict& this_map, const int id);
    void AddBus(json::Dict& res_dict, const transport::TransportCatalogue& tc, const json::Dict& this_map, const int id);
    std::ostringstream map_out_;
};
