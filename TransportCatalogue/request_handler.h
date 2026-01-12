#pragma once
#include "transport_catalogue.h"

class RequestHandler {
public:
    std::ostringstream ReadJson(transport::TransportCatalogue& tc, std::istream& in);
};
