#include "stat_reader.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

void StatReader::ParseAndPrintStat(std::istream& in, std::ostream& out, transport::TransportCatalogue& tc) {
    input_reader_.FillCatalogue(in, tc);

    size_t m;
    in >> m;
    in.ignore();

    std::vector<std::string> query_lines;
    query_lines.reserve(m);
    std::string line;
    for (size_t i = 0; i < m; ++i) {
        std::getline(in, line);
        query_lines.push_back(line);
    }

    for (const auto& query : query_lines) {
        if (query.substr(0, 3) == "Bus") {
            std::string bus_name = query.substr(4);
            const transport::Bus* bus = tc.GetBus(bus_name);
            if (!bus) {
                out << "Bus " << bus_name << ": not found\n";
                continue;
            }
            const transport::TransportCatalogue::BusStats bus_info = tc.GetBusInfo(bus);

            out << std::fixed << std::setprecision(6);
            out << "Bus " << bus_name << ": " << bus_info.stops_on_route
                << " stops on route, " << bus_info.unique_stops
                << " unique stops, " << bus_info.route_length << " route length, "
                << bus_info.curvature << " curvature\n";
        }
        else if (query.substr(0, 4) == "Stop") {
            std::string stop_name = query.substr(5);
            stop_name = input_reader_.Trim(stop_name);

            const auto& buses_opt = tc.GetStopInformation(stop_name);

            if (buses_opt == nullptr) {
                out << "Stop " << stop_name << ": not found\n";
                continue;
            }

            if (buses_opt->empty()) {
                out << "Stop " << stop_name << ": no buses\n";
                continue;
            }

            out << "Stop " << stop_name << ": buses";
            for (const std::string_view num : *buses_opt) {
                out << " " << num;
            }
            out << "\n";
        }
    }
}