#pragma once
#include <vector>
#include <string>
#include "transport_catalogue.h"

namespace parce {
    class InputReader {
    public:
        InputReader() = default;

        void ParseLines(const std::vector<std::string>& lines, transport::TransportCatalogue& tc);
        std::string Trim(const std::string& str) const;

    };
}

