#pragma once
#include "graph.h"
#include <string>
#include <vector>

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

class TransportRouter{
public:
    TransportRouter(const Graph& graph);
    // Ищет оптимальный маршрут между двумя остановками
    RouteResult FindRoute(const std::string& from, const std::string& to) const;
private:
    const Graph& graph_;
};