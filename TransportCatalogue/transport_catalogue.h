#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <set>
#include "geo.h"

namespace transport {

    struct StringPairHasher {
        size_t operator()(const std::pair<std::string, std::string>& p) const {
            return std::hash<std::string>{}(p.first) * 37 + std::hash<std::string>{}(p.second);
        }
    };

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
            double curvature; 
        };

        void AddStop(const std::string& name, const Coordinate& coordinate);
        void AddBus(const std::string& number, const std::vector<std::string>& stop_names, bool is_ring);

        void SetRoadDistance(const std::string& from_stop, const std::string& to_stop, int distance);
        int GetRoadDistance(const Stop* from_stop, const Stop* to_stop) const;

        const Stop* GetStop(std::string_view name) const;
        const Bus* GetBus(std::string_view number) const;
        std::optional<BusStats> GetBusStatistics(const std::string_view number) const;

        const std::set<std::string_view>* GetStopInformation(const std::string_view stop_name) const;
        const BusStats GetBusInfo(const Bus* bus) const;


    private:
        double CalculateRoadLength(const Bus* bus) const;
        double CalculateGeoLength(const Bus* bus) const;

        size_t CountUniqueStops(const Bus* bus) const;
        size_t CountStopsOnRoute(const Bus* bus) const;

        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, Bus> buses_;
        std::unordered_map<std::string, std::set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<std::string, std::string>, int, StringPairHasher> road_distances_;
    };

}