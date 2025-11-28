#include "transport_catalogue.h"
#include <unordered_set>
#include <algorithm>

void transport::TransportCatalogue::AddStop(const std::string& name, double latitude, double longitude) {
    stops_.emplace(name, Stop{ name, latitude, longitude });
}

void transport::TransportCatalogue::AddBus(const std::string& number, const std::vector<std::string>& stop_names, bool is_ring) {
    Bus bus;
    bus.number = number;
    bus.route = stop_names;
    bus.is_ring = is_ring;

    auto [it, inserted] = buses_.emplace(number, std::move(bus));
    const std::string& bus_number = it->first;

    for (const auto& stop_name : stop_names) {
        stop_to_buses_[stop_name].insert(bus_number);
    }
}

std::optional<const transport::Stop*> transport::TransportCatalogue::GetStop(const std::string& name) const {
    auto it = stops_.find(name);
    if (it != stops_.end()) return &it->second;
    return std::nullopt;
}

std::optional<const transport::Bus*> transport::TransportCatalogue::GetBus(const std::string& number) const {
    auto it = buses_.find(number);
    if (it != buses_.end()) return &it->second;
    return std::nullopt;
}

double transport::TransportCatalogue::CalculateRouteLength(const Bus* bus) const {
    if (!bus || bus->route.empty()) return 0.0;
    double distance = 0.0;

    for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
        auto stop1 = GetStop(bus->route[i]);
        auto stop2 = GetStop(bus->route[i + 1]);
        if (stop1 && stop2) {
            distance += ComputeDistance(
                { (*stop1)->latitude, (*stop1)->longitude },
                { (*stop2)->latitude, (*stop2)->longitude }
            );
        }
    }

    if (!bus->is_ring && bus->route.size() > 1) {
        for (size_t i = bus->route.size() - 1; i >= 1; --i) {
            auto stop1 = GetStop(bus->route[i]);
            auto stop2 = GetStop(bus->route[i - 1]);
            if (stop1 && stop2) {
                distance += ComputeDistance(
                    { (*stop1)->latitude, (*stop1)->longitude },
                    { (*stop2)->latitude, (*stop2)->longitude }
                );
            }
        }
    }

    return distance;
}

size_t transport::TransportCatalogue::CountUniqueStops(const Bus* bus) const {
    std::unordered_set<std::string> unique;
    for (const auto& stop_name : bus->route) {
        if (stops_.count(stop_name)) {
            unique.insert(stop_name);
        }
    }
    return unique.size();
}

size_t transport::TransportCatalogue::CountStopsOnRoute(const Bus* bus) const {
    if (!bus) return 0;
    if (bus->is_ring) return bus->route.size();
    if (bus->route.size() <= 1) return bus->route.size();
    return bus->route.size() * 2 - 1;
}

std::optional<std::unordered_set<std::string>> transport::TransportCatalogue::GetStopInformation(const std::string& stop_name) {
    if (GetStop(stop_name) == std::nullopt) {
        return std::nullopt;
    }

    auto it = stop_to_buses_.find(stop_name);

    if (it == stop_to_buses_.end()) {
        return std::unordered_set<std::string>{};
    }
    return it->second;
}
