#include "transport_catalogue.h"
#include <unordered_set>
#include <algorithm>
#include "geo.h"

namespace transport {

    void TransportCatalogue::SetRoadDistance(const std::string_view from_stop, const std::string_view to_stop, double distance) {
        road_distances_[{from_stop.data(), to_stop.data()}] = distance;
    }

    int TransportCatalogue::GetRoadDistance(const std::string_view from_stop, const std::string_view to_stop) const {
        auto it = road_distances_.find({ from_stop.data(), to_stop.data() });
        if (it != road_distances_.end()) {
            return it->second;
        }
        auto reverse_it = road_distances_.find({ to_stop.data(), from_stop.data() });
        if (reverse_it != road_distances_.end()) {
            return reverse_it->second;
        }
        return 0;
    }

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
        auto it = stops_.find(name.data());
        if (it == stops_.end()) return nullptr;
        return &it->second;
    }

    const std::unordered_map<std::string, Stop>* TransportCatalogue::GetStops() const {
        return &stops_;
    }

    const std::unordered_map<std::string, Bus>* TransportCatalogue::GetBuses() const {
        return &buses_;
    }

    const Bus* TransportCatalogue::GetBus(const std::string_view number) const {
        auto it = buses_.find(number.data());
        if (it == buses_.end()) return nullptr;
        return &it->second;
    }

    double TransportCatalogue::CalculateRoadLength(const Bus* bus) const {
        double distance = 0;
        for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
            distance += GetRoadDistance(bus->route[i], bus->route[i + 1]);
        }
        if (!bus->is_ring) {
            for (size_t i = bus->route.size() - 1; i > 0; --i) {
                distance += GetRoadDistance(bus->route[i], bus->route[i - 1]);
            }
        }
        return distance;
    }

    double TransportCatalogue::CalculateGeoLength(const Bus* bus) const {
        if (!bus || bus->route.empty()) return 0.0;
        double distance = 0.0;

        for (size_t i = 0; i + 1 < bus->route.size(); ++i) {
            auto stop1 = GetStop(bus->route[i]);
            auto stop2 = GetStop(bus->route[i + 1]);
            if (stop1 && stop2) {
                distance += geo::ComputeDistance(
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
                    distance += geo::ComputeDistance(
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

    const std::set<std::string_view>* TransportCatalogue::GetStopInformation(const std::string_view stop_name) const {
        static const std::set<std::string_view> empty_set;
        if (GetStop(stop_name) == nullptr) {
            return nullptr;
        }
        auto it = stop_to_buses_.find(stop_name.data());
        if (it == stop_to_buses_.end()) {
            return &empty_set;
        }
        return &it->second;
    }

    const transport::TransportCatalogue::BusStats TransportCatalogue::GetBusInfo(const Bus* bus) const {
        double road_length = CalculateRoadLength(bus);
        double geo_length = CalculateGeoLength(bus);

        size_t count_unique_stops = CountUniqueStops(bus);
        size_t count_stops_on_route = CountStopsOnRoute(bus);

        double curvature = 0.0;
        if (geo_length > 1e-6) {
            curvature = road_length / geo_length;
        }
        return { count_stops_on_route, count_unique_stops, road_length, curvature };
    }

    void TransportCatalogue::AddRoutingSettings(const double bus_wait_time, const double bus_velocity) {
        bus_wait_time_ = bus_wait_time;
        bus_velocity_ = bus_velocity;
    }

    size_t TransportCatalogue::WaitVertex(const std::string& stop_name) const {
        return stop_to_index_.at(stop_name) * 2;
    }

    size_t TransportCatalogue::BoardVertex(const std::string& stop_name) const {
        return stop_to_index_.at(stop_name) * 2 + 1;
    }

    void TransportCatalogue::BuildGraph() {
        // каждой остановке присваиваю индекс
        size_t idx = 0;
        for (const auto& [name, stop] : stops_) {
            stop_to_index_[name] = idx;
            index_to_stop_.push_back(name);
            ++idx;
        }

        // по 2 вершины для каждой остановки для ожидания и для езды
        vertex_count_ = stops_.size() * 2;
        adjacency_.resize(vertex_count_);
        edges_.clear();

        // добавляю ребра ожидания с весом время ожидания
        for (const auto& [name, stop] : stops_) {
            size_t wait_v = WaitVertex(name);
            size_t board_v = BoardVertex(name);

            GraphEdge edge;
            edge.from = wait_v;
            edge.to = board_v;
            edge.weight = bus_wait_time_;
            edge.is_wait = true;
            edge.stop_name = name;
            edge.span_count = 0;

            size_t edge_id = edges_.size();
            edges_.push_back(std::move(edge));
            adjacency_[wait_v].push_back(edge_id);
        }

        // добавляю ребра езды
        double speed_m_per_min = bus_velocity_ * (1000.0 / 60.0);

        for (const auto& [bus_name, bus] : buses_) {
            const auto& route = bus.route;

            // не кольцевой маршрут
            for (size_t i = 0; i < route.size(); ++i) {
                double accumulated_distance = 0.0;
                for (size_t j = i + 1; j < route.size(); ++j) {
                    accumulated_distance += GetRoadDistance(route[j - 1], route[j]);
                    double travel_time = accumulated_distance / speed_m_per_min;

                    GraphEdge edge;
                    edge.from = BoardVertex(route[i]);
                    edge.to = WaitVertex(route[j]);
                    edge.weight = travel_time;
                    edge.is_wait = false;
                    edge.bus_name = bus_name;
                    edge.span_count = static_cast<int>(j - i);

                    size_t edge_id = edges_.size();
                    edges_.push_back(std::move(edge));
                    adjacency_[BoardVertex(route[i])].push_back(edge_id);
                }
            }

            // братное направление для не-кольцевых маршрутов
            if (!bus.is_ring) {
                for (size_t i = route.size(); i-- > 0;) {
                    double accumulated_distance = 0.0;
                    for (size_t j = i; j-- > 0;) {
                        accumulated_distance += GetRoadDistance(route[j + 1], route[j]);
                        double travel_time = accumulated_distance / speed_m_per_min;

                        GraphEdge edge;
                        edge.from = BoardVertex(route[i]);
                        edge.to = WaitVertex(route[j]);
                        edge.weight = travel_time;
                        edge.is_wait = false;
                        edge.bus_name = bus_name;
                        edge.span_count = static_cast<int>(i - j);

                        size_t edge_id = edges_.size();
                        edges_.push_back(std::move(edge));
                        adjacency_[BoardVertex(route[i])].push_back(edge_id);
                    }
                }
            }
        }

        // предвычился все кратчайшие пути один раз
        PrecomputeAllRoutes();
    }

    void TransportCatalogue::PrecomputeAllRoutes() {
        const double INF = std::numeric_limits<double>::infinity();
        size_t n_stops = stops_.size();
        all_dist_.assign(n_stops, std::vector<double>(vertex_count_, INF));
        all_prev_.assign(n_stops, std::vector<int>(vertex_count_, -1));

        using PQItem = std::pair<double, size_t>;

        for (size_t si = 0; si < n_stops; ++si) {
            size_t start = si * 2; 
            auto& dist = all_dist_[si];
            auto& prev_e = all_prev_[si];
            dist[start] = 0.0;

            std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;
            pq.push({ 0.0, start });

            while (!pq.empty()) {
                auto [d, v] = pq.top();
                pq.pop();
                if (d > dist[v]) continue;
                for (size_t edge_id : adjacency_[v]) {
                    const GraphEdge& e = edges_[edge_id];
                    double nd = dist[v] + e.weight;
                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        prev_e[e.to] = static_cast<int>(edge_id);
                        pq.push({ nd, e.to });
                    }
                }
            }
        }
    }

    RouteResult TransportCatalogue::FindRoute(const std::string& from, const std::string& to) const {
        RouteResult result;

        auto it_from = stop_to_index_.find(from);
        auto it_to = stop_to_index_.find(to);
        if (it_from == stop_to_index_.end() || it_to == stop_to_index_.end()) {
            return result;
        }

        if (from == to) {
            result.found = true;
            result.total_time = 0.0;
            return result;
        }

        size_t from_idx = it_from->second;
        size_t to_idx = it_to->second;

        const auto& dist = all_dist_[from_idx];
        const auto& prev_e = all_prev_[from_idx];

        size_t finish_wait = to_idx * 2;
        size_t finish_board = to_idx * 2 + 1;
        size_t finish = (dist[finish_wait] <= dist[finish_board]) ? finish_wait : finish_board;

        const double INF = std::numeric_limits<double>::infinity();
        if (dist[finish] == INF) {
            return result;
        }

        result.found = true;
        result.total_time = dist[finish];

        // восстановление пути
        std::vector<const GraphEdge*> path_edges;
        size_t cur = finish;
        while (prev_e[cur] != -1) {
            const GraphEdge* e = &edges_[prev_e[cur]];
            path_edges.push_back(e);
            cur = e->from;
        }
        std::reverse(path_edges.begin(), path_edges.end());

        for (const GraphEdge* e : path_edges) {
            RouteItem item;
            item.is_wait = e->is_wait;
            item.time = e->weight;
            if (e->is_wait) {
                item.stop_name = e->stop_name;
            }
            else {
                item.bus_name = e->bus_name;
                item.span_count = e->span_count;
            }
            result.items.push_back(std::move(item));
        }

        return result;
    }

    std::optional<TransportCatalogue::BusStats> TransportCatalogue::GetBusStatistics(const std::string_view number) const {
        const Bus* bus = GetBus(number);
        if (!bus) {
            return std::nullopt;
        }
        return GetBusInfo(bus);
    }

} // namespace transport
