#include "request_handler.h"
#include "map_renderer.h"
#include "svg.h"
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;


static const json::Node* FindValue(const json::Dict& dict, const std::string_view key) {
    for (const auto& [k, v] : dict) {
        if (k == key) {
            return &v;
        }
    }
    return nullptr;
}

// сериализация
void RequestHandler::Serialization(transport::TransportCatalogue& tc,
    const json::Node& root) {
    const auto& root_map = root.AsMap();
    const json::Node* base = FindValue(root_map, "base_requests");
    if (!base) return;

    const auto& requests = base->AsArray();

    // добавляю остановки
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

    // добавляю расстояния
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

    // добавляю автобусы
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

std::ostringstream RequestHandler::GetPicture(const transport::TransportCatalogue& tc, const json::Node& root)
{
    const auto& root_map = root.AsMap();
    const json::Dict render_settings = FindValue(root_map, "render_settings"sv)->AsMap();
    const double width = FindValue(render_settings, "width"sv)->AsDouble();
    const double height = FindValue(render_settings, "height"sv)->AsDouble();
    const double padding = FindValue(render_settings, "padding"sv)->AsDouble();
    const double stop_radius = FindValue(render_settings, "stop_radius"sv)->AsDouble();
    const double line_width = FindValue(render_settings, "line_width"sv)->AsDouble();
    const size_t bus_label_font_size = FindValue(render_settings, "bus_label_font_size"sv)->AsInt();
    const LabelOffset bus_label_offset{
        FindValue(render_settings, "bus_label_offset"sv)->AsArray()[0].AsDouble(),
        FindValue(render_settings, "bus_label_offset"sv)->AsArray()[1].AsDouble()
    };
    const size_t stop_label_font_size = FindValue(render_settings, "stop_label_font_size"sv)->AsInt();
    const LabelOffset stop_label_offset{
        FindValue(render_settings, "stop_label_offset"sv)->AsArray()[0].AsDouble(),
        FindValue(render_settings, "stop_label_offset"sv)->AsArray()[1].AsDouble()
    };
    svg::Color underlayer_color;
    if (FindValue(render_settings, "underlayer_color"sv)->IsArray()) {
        if (FindValue(render_settings, "underlayer_color"sv)->AsArray().size() == 3) {
            underlayer_color = svg::Rgb{
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[0].AsInt(),
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[1].AsInt(),
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[2].AsInt()
            };
        }
        else if (FindValue(render_settings, "underlayer_color"sv)->AsArray().size() == 4) {
            underlayer_color = svg::Rgba{
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[0].AsInt(),
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[1].AsInt(),
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[2].AsInt(),
                FindValue(render_settings, "underlayer_color"sv)->AsArray()[3].AsDouble()
            };
        }
    }
    else if (FindValue(render_settings, "underlayer_color"sv)->IsString()) {
        underlayer_color = FindValue(render_settings, "underlayer_color"sv)->AsString();
    }
    
    const double underlayer_width = FindValue(render_settings, "underlayer_width"sv)->AsDouble();
    const json::Array color_palette = FindValue(render_settings, "color_palette"sv)->AsArray();
    
    MapRenderer mr(width, height, padding, stop_radius, line_width, bus_label_font_size, bus_label_offset,
        stop_label_font_size, stop_label_offset, underlayer_color, underlayer_width, color_palette, tc);

    std::ostringstream res = mr.Render();
    return res;
}

// десериализация
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

            res_dict.push_back({ "request_id"s, json::Node(id) });
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
                for (const auto& b : *info) {
                    buses_node.push_back(json::Node(string(b)));
                }
                res_dict.push_back({ "buses"s, json::Node(move(buses_node)) });
            }
        }
        else if(type == "Map"){
            std::ostringstream picture = GetPicture(tc, root);
            std::string picture_in_string = picture.str();
            res_dict.push_back({"map", picture_in_string});
            res_dict.push_back({"request_id", json::Node(id)});
        }
        response.push_back(json::Node(move(res_dict)));
    }
    return json::Node(move(response));
}
