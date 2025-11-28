#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include "geo.h"

namespace transport {
    struct Stop {
        std::string name;
        double latitude = 0.0;
        double longitude = 0.0;
    };

    struct Bus {
        std::string number;
        std::vector<std::string> route;
        bool is_ring = false;
    };



    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, double latitude, double longitude);
        void AddBus(const std::string& number, const std::vector<std::string>& stop_names, bool is_ring);
        std::optional<const Stop*> GetStop(const std::string& name) const;
        std::optional<const Bus*> GetBus(const std::string& number) const;
        double CalculateRouteLength(const Bus* bus) const;
        size_t CountUniqueStops(const Bus* bus) const;
        size_t CountStopsOnRoute(const Bus* bus) const;
        std::optional<std::unordered_set<std::string>> GetStopInformation(const std::string& stop_name);

    private:
        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, Bus> buses_;
        std::unordered_map<std::string, std::unordered_set<std::string>> stop_to_buses_;
    };
}

