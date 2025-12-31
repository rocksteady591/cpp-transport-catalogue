#pragma once

#include "json.h"
#include "transport_catalogue.h"

class RequestHandler {
public:
    // FIX: используем const Node& вместо Node*
    // WHY: исключаем линковочные ошибки и nullptr
    void Serialization(transport::TransportCatalogue& tc, const json::Node& root);

    json::Node DeSerialization(const transport::TransportCatalogue& tc,
        const json::Node& root);
};
