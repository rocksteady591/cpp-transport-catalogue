#include "transport_catalogue.h"
#include <unordered_set>
#include <algorithm>

namespace transport {

    void TransportCatalogue::AddStop(const std::string& name, const Coordinate& coordinate) {
        stops_.emplace(name, Stop{ name, coordinate });
    }

    void TransportCatalogue::AddBus(const std::string& number, const std::vector<std::string>& stop_names, bool is_ring) {
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

    const Stop* TransportCatalogue::GetStop(const std::string_view name) const {
        auto it = stops_.find(std::string(name));
        if (it == stops_.end()) return nullptr;
        return &it->second;
    }

    const Bus* TransportCatalogue::GetBus(const std::string_view number) const {
        auto it = buses_.find(std::string(number));
        if (it == buses_.end()) return nullptr;
        return &it->second;
    }

    double TransportCatalogue::CalculateRouteLength(const Bus* bus) const {
        if (!bus || bus->route.empty()) return 0.0;
        double distance = 0.0;

        for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
            auto stop1 = GetStop(bus->route[i]);
            auto stop2 = GetStop(bus->route[i + 1]);
            if (stop1 && stop2) {
                distance += ComputeDistance(
                    { stop1->coordinate.latitude, stop1->coordinate.longitude },
                    { stop2->coordinate.latitude, stop2->coordinate.longitude }
                );
            }
        }

        if (!bus->is_ring && bus->route.size() > 1) {
            for (size_t i = bus->route.size() - 1; i > 0; --i) {
                auto stop1 = GetStop(bus->route[i]);
                auto stop2 = GetStop(bus->route[i - 1]);
                if (stop1 && stop2) {
                    distance += ComputeDistance(
                        { stop1->coordinate.latitude, stop1->coordinate.longitude },
                        { stop2->coordinate.latitude, stop2->coordinate.longitude }
                    );
                }
            }
        }

        return distance;
    }

    size_t TransportCatalogue::CountUniqueStops(const Bus* bus) const {
        if (!bus) return 0;
        std::unordered_set<std::string> unique;
        for (const auto& stop_name : bus->route) {
            if (stops_.count(stop_name)) {
                unique.insert(stop_name);
            }
        }
        return unique.size();
    }

    size_t TransportCatalogue::CountStopsOnRoute(const Bus* bus) const {
        if (!bus) return 0;
        if (bus->is_ring) return bus->route.size();
        if (bus->route.size() <= 1) return bus->route.size();
        return bus->route.size() * 2 - 1;
    }

    std::optional<std::set<std::string_view>> TransportCatalogue::GetStopInformation(const std::string_view stop_name) const {
        if (GetStop(stop_name) == nullptr) {
            return std::nullopt;
        }

        auto it = stop_to_buses_.find(std::string(stop_name));
        if (it == stop_to_buses_.end()) {
            return std::set<std::string_view>{};
        }
        return it->second;
    }

} 
