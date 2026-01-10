#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include <sstream>

class RequestHandler {
public:
    void Serialization(transport::TransportCatalogue& tc, const json::Node& root);
    std::ostringstream GetPicture(const transport::TransportCatalogue& tc, const json::Node& root);

    json::Node DeSerialization(const transport::TransportCatalogue& tc,
        const json::Node& root);
};
