#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>
#include <queue>
#include <limits>

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

    // Ребро графа
    struct GraphEdge {
        size_t from;
        size_t to;
        double weight;
        bool is_wait;           // true = Wait, false = Bus
        std::string stop_name;  // для Wait
        std::string bus_name;   // для Bus
        int span_count;         // для Bus
    };

    // Элемент маршрута для ответа
    struct RouteItem {
        bool is_wait;
        std::string stop_name;  // для Wait
        std::string bus_name;   // для Bus
        int span_count = 0;
        double time = 0.0;
    };

    // Результат поиска маршрута
    struct RouteResult {
        bool found = false;
        double total_time = 0.0;
        std::vector<RouteItem> items;
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

        void SetRoadDistance(const std::string_view from_stop, const std::string_view to_stop, double distance);
        int GetRoadDistance(const std::string_view from_stop, const std::string_view to_stop) const;

        const Stop* GetStop(std::string_view name) const;
        const std::unordered_map<std::string, Stop>* GetStops() const;
        const std::unordered_map<std::string, Bus>* GetBuses() const;
        const Bus* GetBus(std::string_view number) const;
        std::optional<BusStats> GetBusStatistics(const std::string_view number) const;

        const std::set<std::string_view>* GetStopInformation(const std::string_view stop_name) const;
        const BusStats GetBusInfo(const Bus* bus) const;

        void AddRoutingSettings(const double bus_wait_time, const double bus_velocity);

        // Строит граф на основе маршрутов
        void BuildGraph();

        // Ищет оптимальный маршрут между двумя остановками
        RouteResult FindRoute(const std::string& from, const std::string& to) const;

    private:
        double CalculateRoadLength(const Bus* bus) const;
        double CalculateGeoLength(const Bus* bus) const;

        size_t CountUniqueStops(const Bus* bus) const;
        size_t CountStopsOnRoute(const Bus* bus) const;

        // Индекс вершины ожидания для остановки
        size_t WaitVertex(const std::string& stop_name) const;
        // Индекс вершины посадки для остановки
        size_t BoardVertex(const std::string& stop_name) const;

        double bus_wait_time_ = 0.0;
        double bus_velocity_ = 0.0;

        std::unordered_map<std::string, Stop> stops_;
        std::unordered_map<std::string, Bus> buses_;
        std::unordered_map<std::string, std::set<std::string_view>> stop_to_buses_;
        std::unordered_map<std::pair<std::string, std::string>, double, StringPairHasher> road_distances_;

        // Граф
        std::unordered_map<std::string, size_t> stop_to_index_; // имя остановки -> базовый индекс (wait = base, board = base+1)
        std::vector<std::string> index_to_stop_;                 // базовый индекс -> имя остановки
        std::vector<std::vector<size_t>> adjacency_;             // adjacency_[v] = список индексов рёбер из v
        std::vector<GraphEdge> edges_;
        size_t vertex_count_ = 0;

        // Предвычисленные результаты: Dijkstra от каждой wait-вершины
        void PrecomputeAllRoutes();
        // all_dist_[stop_idx][vertex] = мин время из wait-вершины stop_idx
        std::vector<std::vector<double>> all_dist_;
        // all_prev_[stop_idx][vertex] = индекс ребра, по которому пришли
        std::vector<std::vector<int>> all_prev_;
    };

} // namespace transport
