#include "json_reader.h"
#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "transport_router.h"


static const json::Node* FindValue(const json::Dict& dict, const std::string_view key) {
    for (const auto& [k, v] : dict) {
        if (k == key) {
            return &v;
        }
    }
    return nullptr;
}

void JsonReader::AddStops(const json::Array& requests, transport::TransportCatalogue& tc) {
    for (const auto& req : requests) {
        const auto& map = req.AsMap();
        if (FindValue(map, "type")->AsString() == "Stop") {
            tc.AddStop(
                FindValue(map, "name")->AsString(),
                {
                    FindValue(map, "latitude")->AsDouble(),
                    FindValue(map, "longitude")->AsDouble()
                }
            );
        }
    }
}

void JsonReader::AddRoutes(const json::Array& requests, transport::TransportCatalogue& tc) {
    for (const auto& req : requests) {
        const auto& map = req.AsMap();
        if (FindValue(map, "type")->AsString() == "Stop") {
            const auto* dist_node = FindValue(map, "road_distances");
            if (!dist_node) continue;

            const std::string& from = FindValue(map, "name")->AsString();
            for (const auto& [to, dist] : dist_node->AsMap()) {
                tc.SetRoadDistance(from, to, dist.AsInt());
            }
        }
    }
}

void JsonReader::AddBuses(const json::Array& requests, transport::TransportCatalogue& tc) {
    for (const auto& req : requests) {
        const auto& map = req.AsMap();
        if (FindValue(map, "type")->AsString() == "Bus") {
            std::vector<std::string> stops;
            for (const auto& s : FindValue(map, "stops")->AsArray()) {
                stops.push_back(s.AsString());
            }

            tc.AddBus(
                FindValue(map, "name")->AsString(),
                stops,
                FindValue(map, "is_roundtrip")->AsBool()
            );
        }
    }
}

void JsonReader::AddMap(const json::Dict& root_map, transport::TransportCatalogue& tc) {
    using namespace std::literals;
    const json::Dict render_settings = FindValue(root_map, "render_settings"sv)->AsMap();
    const double width = FindValue(render_settings, "width"sv)->AsDouble();
    const double height = FindValue(render_settings, "height"sv)->AsDouble();
    const double padding = FindValue(render_settings, "padding"sv)->AsDouble();
    const double stop_radius = FindValue(render_settings, "stop_radius"sv)->AsDouble();
    const double line_width = FindValue(render_settings, "line_width"sv)->AsDouble();
    const size_t bus_label_font_size = FindValue(render_settings, "bus_label_font_size"sv)->AsInt();
    const Map::LabelOffset bus_label_offset{
        FindValue(render_settings, "bus_label_offset"sv)->AsArray()[0].AsDouble(),
        FindValue(render_settings, "bus_label_offset"sv)->AsArray()[1].AsDouble()
    };
    const size_t stop_label_font_size = FindValue(render_settings, "stop_label_font_size"sv)->AsInt();
    const Map::LabelOffset stop_label_offset{
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

    Map::RenderSettings r_settings{ width, height, padding, stop_radius, line_width, bus_label_font_size, bus_label_offset,
        stop_label_font_size, stop_label_offset, underlayer_color, underlayer_width, color_palette };

    Map::MapRenderer mr(r_settings, tc);
    map_out_ = mr.Render();
}

void JsonReader::ReadAndExecuteBaseRequests(transport::TransportCatalogue& tc, const json::Node& root) {
    using namespace std::literals;

    const auto& root_map = root.AsMap();
    const json::Node* base = FindValue(root_map, "base_requests");
    if (!base) return;

    const auto& requests = base->AsArray();

    AddStops(requests, tc);
    AddRoutes(requests, tc);
    AddBuses(requests, tc);
    AddMap(root_map, tc);
    AddRoutingSettings(tc, root);
}

const std::ostringstream& JsonReader::GetMap() {
    return map_out_;
}

void JsonReader::AddStopBuilder(json::Builder& builder, const transport::TransportCatalogue& tc, const json::Dict& this_map, const int id) {
    using namespace std::literals;
    const std::string& name = FindValue(this_map, "name")->AsString();
    const auto* info = tc.GetStopInformation(name);
    builder.Key("request_id"s).Value(json::Node(id));
    if (!info) {
        builder.Key("error_message"s).Value(json::Node("not found"s));
    }
    else {
        json::Array buses_node;
        for (const auto& b : *info) {
            buses_node.push_back(json::Node(std::string(b)));
        }
        builder.Key("buses"s).Value(json::Node(std::move(buses_node)));
    }
}

void JsonReader::AddBusBuilder(json::Builder& builder, const transport::TransportCatalogue& tc, const json::Dict& this_map, const int id) {
    using namespace std::literals;
    const std::string& name = FindValue(this_map, "name")->AsString();
    const auto* bus = tc.GetBus(name);
    builder.Key("request_id"s).Value(json::Node(id));
    if (!bus) {
        builder.Key("error_message"s).Value(json::Node("not found"s));
    }
    else {
        auto stat = tc.GetBusInfo(bus);
        builder.Key("curvature"s).Value(json::Node(stat.curvature));
        builder.Key("route_length"s).Value(json::Node(static_cast<double>(stat.route_length)));
        builder.Key("stop_count"s).Value(json::Node(static_cast<int>(stat.stops_on_route)));
        builder.Key("unique_stop_count"s).Value(json::Node(static_cast<int>(stat.unique_stops)));
    }
}

void JsonReader::AddRouteBuilder(json::Builder& builder, const json::Dict& this_map, const int id, const TransportRouter& router) {
    using namespace std::literals;
    const std::string& from = FindValue(this_map, "from")->AsString();
    const std::string& to = FindValue(this_map, "to")->AsString();

    builder.Key("request_id"s).Value(json::Node(id));

    RouteResult route = router.FindRoute(from, to);

    if (!route.found) {
        builder.Key("error_message"s).Value(json::Node("not found"s));
        return;
    }

    builder.Key("total_time"s).Value(json::Node(route.total_time));
    builder.Key("items"s).StartArray();

    for (const auto& item : route.items) {
        builder.StartDict();
        if (item.is_wait) {
            builder.Key("type"s).Value(json::Node("Wait"s));
            builder.Key("stop_name"s).Value(json::Node(item.stop_name));
            builder.Key("time"s).Value(json::Node(item.time));
        }
        else {
            builder.Key("type"s).Value(json::Node("Bus"s));
            builder.Key("bus"s).Value(json::Node(item.bus_name));
            builder.Key("span_count"s).Value(json::Node(item.span_count));
            builder.Key("time"s).Value(json::Node(item.time));
        }
        builder.EndDict();
    }

    builder.EndArray();
}

json::Node JsonReader::ExecuteStatRequests(const transport::TransportCatalogue& tc, const json::Node& root, const TransportRouter& router) {
    using namespace std::literals;
    json::Builder builder;
    builder.StartArray();
    const auto& root_map = root.AsMap();
    const json::Node* stat_requests = FindValue(root_map, "stat_requests");

    if (!stat_requests) return json::Node(json::Array{});

    for (const auto& req : stat_requests->AsArray()) {
        const auto& this_map = req.AsMap();
        int id = FindValue(this_map, "id")->AsInt();
        std::string type = FindValue(this_map, "type")->AsString();
        builder.StartDict();

        if (type == "Bus") {
            AddBusBuilder(builder, tc, this_map, id);
        }
        else if (type == "Stop") {
            AddStopBuilder(builder, tc, this_map, id);
        }
        else if (type == "Map") {
            const std::ostringstream& picture = GetMap();
            builder.Key("map"s).Value(picture.str());
            builder.Key("request_id"s).Value(json::Node(id));
        }
        else if (type == "Route") {
            AddRouteBuilder(builder,  this_map, id, router);
        }

        builder.EndDict();
    }
    builder.EndArray();
    return builder.Build();
}

void JsonReader::AddRoutingSettings(transport::TransportCatalogue& tc, const json::Node& root) {
    const auto& root_map = root.AsMap();
    const json::Node* routing = FindValue(root_map, "routing_settings");
    if (!routing) return;

    double bus_wait_time = FindValue(routing->AsMap(), "bus_wait_time")->AsDouble();
    double bus_velocity = FindValue(routing->AsMap(), "bus_velocity")->AsDouble();
    tc.AddRoutingSettings(bus_wait_time, bus_velocity);
}
