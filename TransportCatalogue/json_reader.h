#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include <sstream>

class JsonReader {
public:
    void DeSerialization(transport::TransportCatalogue& tc, const json::Node& root);
    const std::ostringstream& GetMap();

    json::Node Serialization(const transport::TransportCatalogue& tc,
        const json::Node& root);
private:
    std::ostringstream map_out_;
};
