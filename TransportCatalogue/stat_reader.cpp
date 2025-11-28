#include "stat_reader.h"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include "input_reader.h"

void StatReader::ParseAndPrintStat(transport::TransportCatalogue& tc) {
    size_t n;
    std::cin >> n;
    std::cin.ignore();

    std::vector<std::string> db_lines;
    db_lines.reserve(n);
    std::string line;
    for (size_t i = 0; i < n; ++i) {
        std::getline(std::cin, line);
        db_lines.push_back(line);
    }

    input_reader_.ParseLines(db_lines, tc);

    size_t m;
    std::cin >> m;
    std::cin.ignore();

    std::vector<std::string> query_lines;
    query_lines.reserve(m);
    for (size_t i = 0; i < m; ++i) {
        std::getline(std::cin, line);
        query_lines.push_back(line);
    }

    for (const auto& query : query_lines) {
        if (query.substr(0, 3) == "Bus") {
            std::string bus_name = query.substr(4);
            auto bus_opt = tc.GetBus(bus_name);
            if (!bus_opt) {
                std::cout << "Bus " << bus_name << ": not found\n";
                continue;
            }
            const transport::Bus* bus = *bus_opt;
            size_t stops_on_route = tc.CountStopsOnRoute(bus);
            size_t unique_stops = tc.CountUniqueStops(bus);
            double length = tc.CalculateRouteLength(bus);

            std::cout << std::fixed << std::setprecision(6);
            std::cout << "Bus " << bus_name << ": " << stops_on_route
                << " stops on route, " << unique_stops
                << " unique stops, " << length << " route length\n";
        }
        else if (query.substr(0, 4) == "Stop") {
            parce::InputReader ip;
            std::string stop_name_raw = query.substr(5); 
            std::string stop_name = ip.Trim(stop_name_raw); 

            std::optional<std::unordered_set<std::string>> res_opt = tc.GetStopInformation(stop_name);

            if (!res_opt.has_value()) {
                std::cout << "Stop " << stop_name << ": not found" << "\n";
                continue;
            }

            const::std::unordered_set<std::string>& bus_set = res_opt.value();

            if (res_opt.value().empty()) {
                std::cout << query << ": no buses" << "\n";
                continue;
            }
            std::vector<std::string> sort_res(bus_set.begin(), bus_set.end());
            std::sort(sort_res.begin(), sort_res.end());
            std::cout << query << ": buses";
            for (const std::string& num : sort_res) {
                std::cout << " " << num;
            }
            std::cout << "\n";
        }
    }
}
