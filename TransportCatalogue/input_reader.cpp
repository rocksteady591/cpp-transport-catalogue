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

            std::string coords_and_distances = line.substr(colon + 1);
            std::istringstream iss(coords_and_distances);
            std::string token;

            std::getline(iss, token, ',');
            double lat = std::stod(Trim(token));
            std::getline(iss, token, ',');
            double lng = std::stod(Trim(token));

            const transport::Coordinate cd{ lat, lng };
            tc.AddStop(stop_name, cd);

            while (std::getline(iss, token, ',')) {
                std::string segment = Trim(token);
                if (segment.empty()) continue;
                size_t m_pos = segment.find('m');
                if (m_pos == std::string::npos) continue;
                int distance = std::stoi(segment.substr(0, m_pos));
                size_t to_pos = segment.find(" to ");
                if (to_pos == std::string::npos) continue;
                std::string target_stop_name = Trim(segment.substr(to_pos + 4));
                tc.SetRoadDistance(stop_name, target_stop_name, distance);
            }
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