#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <set>
#include "geo.h"

namespace transport {

    struct Coordinate {
        double latitude = 0.0;
        double longitude = 0.0;
    };

    struct Stop {
        std::string name;
        Coordinate coordinate;
    };

    struct Bus {
        std::string number;
        std::vector<std::string> route;
        bool is_ring = false;
    };

    class TransportCatalogue {
    public:
        struct BusStats {
            size_t stops_on_route;
            size_t unique_stops;
            double route_length;
        };

        void AddStop(const std::string& name, const Coordinate& coordinate);
        void AddBus(const std::string& number, const std::vector<std::string>& stop_names, bool is_ring);

        const Stop* GetStop(const std::string_view name) const;
        const Bus* GetBus(const std::string_view number) const;

        std::optional<BusStats> GetBusStatistics(const std::string_view number) const;
        double CalculateRouteLength(const Bus* bus) const;
        size_t CountUniqueStops(const Bus* bus) const;
        size_t CountStopsOnRoute(const Bus* bus) const;

        std::optional<std::set<std::string_view>> GetStopInformation(const std::string_view stop_name) const;

    private:
        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, Bus> buses_;
        std::unordered_map<std::string, std::set<std::string_view>> stop_to_buses_;
    };

}
