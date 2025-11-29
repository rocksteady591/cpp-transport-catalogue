#include "input_reader.h"
#include <sstream>
#include <algorithm>
#include <cctype>

void parce::InputReader::ParseLines(const std::vector<std::string>& lines, transport::TransportCatalogue& tc) {
    for (const auto& line : lines) {
        if (line.empty()) continue;

        if (line.substr(0, 4) == "Stop") {
            size_t colon = line.find(':');
            std::string stop_name = Trim(line.substr(5, colon - 5));

            size_t comma = line.find(',', colon);
            double lat = std::stod(line.substr(colon + 1, comma - colon - 1));
            double lng = std::stod(line.substr(comma + 1));

            const transport::Coordinate cd{ lat, lng };
            tc.AddStop(stop_name, cd);
        }
        else if (line.substr(0, 3) == "Bus") {
            size_t colon = line.find(':');
            std::string bus_name = Trim(line.substr(4, colon - 4));
            std::string stops_str = line.substr(colon + 1);

            bool is_ring = stops_str.find('>') != std::string::npos;
            char sep = is_ring ? '>' : '-';

            std::vector<std::string> stops;
            std::istringstream iss(stops_str);
            std::string token;
            while (std::getline(iss, token, sep)) {
                stops.push_back(Trim(token));
            }

            tc.AddBus(bus_name, stops, is_ring);
        }
    }
}

std::string parce::InputReader::Trim(const std::string& str) const {
    size_t start = str.find_first_not_of(' ');
    size_t end = str.find_last_not_of(' ');
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}

void parce::InputReader::FillCatalogue(std::istream& in, transport::TransportCatalogue& tc) {
    size_t n;
    in >> n;
    in.ignore();

    std::vector<std::string> lines;
    lines.reserve(n);
    std::string line;
    for (size_t i = 0; i < n; ++i) {
        std::getline(in, line);
        lines.push_back(line);
    }

    ParseLines(lines, tc);
}
