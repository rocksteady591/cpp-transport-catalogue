#pragma once
#include "transport_catalogue.h"
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct GraphEdge{
    size_t from;
    size_t to;
    double weight;
    bool is_wait;
    std::string stop_name;// для wait
    std::string bus_name;// для bus
    int span_count; //для bus
};

class Graph{
public:
    const std::unordered_map<std::string, size_t>& GetStopToIndex() const{
        return stop_to_index_;
    }
    const std::vector<std::vector<double>>& GetAllDist() const{
        return all_dist_;
    }
    const std::vector<std::vector<int>>& GetAllPrev()const{
        return all_prev_;
    }
    const std::vector<GraphEdge>& GetEdges() const{
        return edges_;
    }
    size_t WaitVertex(const std::string& name) const{
        return stop_to_index_.at(name) * 2;
    }
    size_t BoardVertex(const std::string& name) const{
        return stop_to_index_.at(name) * 2 + 1;
    }
    void BuildGraph(const transport::TransportCatalogue& tc){
        const std::unordered_map<std::string, transport::Stop>* all_stops = tc.GetStops();
        const std::unordered_map<std::string, transport::Bus>* all_buses = tc.GetBuses();
        double bus_wait_time = tc.GetWaitTime();;
        double bus_velocity = tc.GetVelocity();
        size_t idx = 0;
        stop_to_index_.reserve(all_stops->size());
        index_to_stop_.reserve(all_stops->size());
        for(const auto& [name, stop] : *all_stops){
            stop_to_index_[name] = idx;
            index_to_stop_.push_back(name);
            ++idx;
        }

        vertex_count_ = all_stops->size() * 2;
        adjacency_.resize(vertex_count_);
        edges_.clear();

        for(const auto& [name, stop] : *all_stops){
            size_t wait_vertex = WaitVertex(name);
            size_t board_vertex = BoardVertex(name);

            GraphEdge edge;
            edge.from = wait_vertex;
            edge.to = board_vertex;
            edge.weight = bus_wait_time;
            edge.is_wait = true;
            edge.stop_name = name;
            edge.span_count = 0;

            size_t edge_id = edges_.size();
            edges_.push_back(std::move(edge));
            adjacency_[wait_vertex].push_back(edge_id);
        }

        double speed_m_per_min = bus_velocity * (1000.0 / 60.0);

        for(const auto& [bus_name, bus] : *all_buses){
            const auto route = bus.route;
            for(int i = 0; i < route.size(); ++i){
                double accumulate_distance = 0.0;
                for(int j = i + 1; j < route.size(); ++j){
                    accumulate_distance += tc.GetRoadDistance(route[j - 1], route[j]);
                    double travel_time = accumulate_distance / speed_m_per_min;

                    GraphEdge edge;
                    edge.from = BoardVertex(route[i]);
                    edge.to = WaitVertex(route[j]);
                    edge.weight = travel_time;
                    edge.is_wait = false;
                    edge.bus_name = bus_name;
                    edge.span_count = static_cast<int>(j - i);

                    size_t edge_id = edges_.size();
                    edges_.push_back(edge);
                    adjacency_[edge.from].push_back(edge_id);
                }
            }
            //обратное направление для некольцевого маршрута
            if(!bus.is_ring){
                for(size_t i = route.size(); i-- > 0;){
                    double accumulate_distance = 0.0;
                    for(size_t j = i; j-- > 0;){
                        accumulate_distance += tc.GetRoadDistance(route[j + 1], route[j]);
                        double travel_time = accumulate_distance / speed_m_per_min;

                        GraphEdge edge;
                        edge.from = BoardVertex(route[i]);
                        edge.to = WaitVertex(route[j]);
                        edge.weight = travel_time;
                        edge.is_wait = false;
                        edge.bus_name = bus_name;
                        edge.span_count = static_cast<int>(j - i);

                        size_t edge_id = edges_.size();
                        edges_.push_back(edge);
                        adjacency_[edge.from].push_back(edge_id);
                    }
                }
            }
        }
        PrecomputeAllRoutes(tc);
    }
private:
    // Предвычисленные результаты
    void PrecomputeAllRoutes(const transport::TransportCatalogue& tc){
        const double INF = std::numeric_limits<double>::infinity();
        size_t n_stops = tc.GetStops()->size();
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
    std::unordered_map<std::string, size_t> stop_to_index_;//имя остановки -> базовый индекс
    std::vector<std::string> index_to_stop_;// базовый индекс -> имя остановки
    std::vector<std::vector<size_t>> adjacency_;
    std::vector<GraphEdge> edges_;
    size_t vertex_count_ = 0;
    // all_dist_[stop_idx][vertex] = мин время из wait-вершины stop_idx
    std::vector<std::vector<double>> all_dist_;
    // all_prev_[stop_idx][vertex] = индекс ребра, по которому пришли
    std::vector<std::vector<int>> all_prev_;
};