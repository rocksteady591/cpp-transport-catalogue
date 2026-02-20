#include "transport_router.h"
#include "graph.h"
#include <algorithm>
#include <limits>

TransportRouter::TransportRouter(const Graph& graph) : graph_(graph){}

RouteResult TransportRouter::FindRoute(const std::string& from, const std::string& to) const {
        RouteResult result;

        auto it_from = graph_.GetStopToIndex().find(from);
        auto it_to = graph_.GetStopToIndex().find(to);
        if (it_from == graph_.GetStopToIndex().end() || it_to == graph_.GetStopToIndex().end()) {
            return result;
        }

        if (from == to) {
            result.found = true;
            result.total_time = 0.0;
            return result;
        }

        size_t from_idx = it_from->second;
        size_t to_idx = it_to->second;

        const auto& dist = graph_.GetAllDist()[from_idx];
        const auto& prev_e = graph_.GetAllPrev()[from_idx];

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
            const GraphEdge* e = &graph_.GetEdges()[prev_e[cur]];
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
