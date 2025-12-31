#include "request_handler.h"
#include <algorithm>
#include <string>

using namespace std;

// ===== ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДОСТУПА К Dict =====
static const json::Node* FindValue(const json::Dict& dict, const string& key) {
    for (const auto& [k, v] : dict) {
        if (k == key) {
            return &v;
        }
    }
    return nullptr;
}

// =================== SERIALIZATION ===================
void RequestHandler::Serialization(transport::TransportCatalogue& tc,
    const json::Node& root) {
    const auto& root_map = root.AsMap();
    const json::Node* base = FindValue(root_map, "base_requests");
    if (!base) return;

    const auto& requests = base->AsArray();

    // 1️⃣ Добавляем остановки
    for (const auto& req : requests) {
        const auto& m = req.AsMap();
        if (FindValue(m, "type")->AsString() == "Stop") {
            tc.AddStop(
                FindValue(m, "name")->AsString(),
                {
                    FindValue(m, "latitude")->AsDouble(),
                    FindValue(m, "longitude")->AsDouble()
                }
            );
        }
    }

    // 2️⃣ Добавляем расстояния
    for (const auto& req : requests) {
        const auto& m = req.AsMap();
        if (FindValue(m, "type")->AsString() == "Stop") {
            const auto* dist_node = FindValue(m, "road_distances");
            if (!dist_node) continue;

            const string& from = FindValue(m, "name")->AsString();
            for (const auto& [to, dist] : dist_node->AsMap()) {
                tc.SetRoadDistance(from, to, dist.AsInt());
            }
        }
    }

    // 3️⃣ Добавляем автобусы
    for (const auto& req : requests) {
        const auto& m = req.AsMap();
        if (FindValue(m, "type")->AsString() == "Bus") {
            vector<string> stops;
            for (const auto& s : FindValue(m, "stops")->AsArray()) {
                stops.push_back(s.AsString());
            }

            tc.AddBus(
                FindValue(m, "name")->AsString(),
                stops,
                FindValue(m, "is_roundtrip")->AsBool()
            );
        }
    }
}

// ================= DESERIALIZATION ===================
json::Node RequestHandler::DeSerialization(const transport::TransportCatalogue& tc, const json::Node& root) {
    json::Array response;
    const auto& root_map = root.AsMap();
    const json::Node* stat_requests = FindValue(root_map, "stat_requests");

    if (!stat_requests) return json::Node(response);

    for (const auto& req : stat_requests->AsArray()) {
        const auto& m = req.AsMap();
        int id = FindValue(m, "id")->AsInt();
        string type = FindValue(m, "type")->AsString();

        json::Dict res_dict;

        if (type == "Bus") {
            const string& name = FindValue(m, "name")->AsString();
            const auto* bus = tc.GetBus(name);

            res_dict.push_back({ "request_id"s, json::Node(id) }); // Явное создание пары
            if (!bus) {
                res_dict.push_back({ "error_message"s, json::Node("not found"s) });
            }
            else {
                auto stat = tc.GetBusInfo(bus);
                res_dict.push_back({ "curvature"s, json::Node(stat.curvature) });
                res_dict.push_back({ "route_length"s, json::Node(static_cast<double>(stat.route_length)) });
                res_dict.push_back({ "stop_count"s, json::Node(static_cast<int>(stat.stops_on_route)) });
                res_dict.push_back({ "unique_stop_count"s, json::Node(static_cast<int>(stat.unique_stops)) });
            }
        }
        else if (type == "Stop") {
            const string& name = FindValue(m, "name")->AsString();
            const auto* info = tc.GetStopInformation(name);

            res_dict.push_back({ "request_id"s, json::Node(id) });
            if (!info) {
                res_dict.push_back({ "error_message"s, json::Node("not found"s) });
            }
            else {
                json::Array buses_node;
                // info — это set<string_view>, он уже отсортирован
                for (const auto& b : *info) {
                    buses_node.push_back(json::Node(string(b)));
                }
                res_dict.push_back({ "buses"s, json::Node(move(buses_node)) });
            }
        }
        response.push_back(json::Node(move(res_dict)));
    }
    return json::Node(move(response));
}
