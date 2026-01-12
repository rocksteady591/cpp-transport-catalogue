#include "map_renderer.h"
#include <utility>
#include <sstream>
#include <map>

Map::MapRenderer::MapRenderer(const RenderSettings& render_settings, const transport::TransportCatalogue& tc)
	: render_settings_(render_settings), tc_(tc) {
};

void Map::MapRenderer::AddPolyline(svg::Document& doc, const std::map<std::string_view, 
    const transport::Bus*>& sorted_buses, const SphereProjector& projector){

    size_t color_idx = 0;
    for (const auto& [name, bus_ptr] : sorted_buses) {
        svg::Polyline line;
        line.SetFillColor("none")
            .SetStrokeWidth(render_settings_.line_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeColor(GetColorFromPalette(color_idx));

        for (const auto& stop_name : bus_ptr->route) {
            line.AddPoint(projector({ tc_.GetStop(stop_name)->coordinate.latitude,
                                     tc_.GetStop(stop_name)->coordinate.longitude }));
        }
        if (!bus_ptr->is_ring) {
            for (auto it = bus_ptr->route.rbegin() + 1; it != bus_ptr->route.rend(); ++it) {
                line.AddPoint(projector({ tc_.GetStop(*it)->coordinate.latitude,
                                         tc_.GetStop(*it)->coordinate.longitude }));
            }
        }
        doc.Add(std::move(line));
        color_idx++;
    }
}
void Map::MapRenderer::AddRouteText(svg::Document& doc, const std::map<std::string_view, 
    const transport::Bus*>& sorted_buses, const SphereProjector& projector){
    size_t color_idx = 0;
    for (const auto& [name, bus_ptr] : sorted_buses) {
        auto add_bus_label = [&](const transport::Stop* stop, std::string_view label) {
            svg::Point pos = projector({ stop->coordinate.latitude, stop->coordinate.longitude });

            // подложка
            doc.Add(svg::Text()
                .SetPosition(pos).SetOffset({ render_settings_.bus_label_offset_.x_, render_settings_.bus_label_offset_.y_ })
                .SetFontSize(render_settings_.bus_label_font_size_).SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(std::string(label)).SetFillColor(render_settings_.underlayer_color_).SetStrokeColor(render_settings_.underlayer_color_)
                .SetStrokeWidth(render_settings_.underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            // надпись
            doc.Add(svg::Text()
                .SetPosition(pos).SetOffset({ render_settings_.bus_label_offset_.x_, render_settings_.bus_label_offset_.y_ })
                .SetFontSize(render_settings_.bus_label_font_size_).SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(std::string(label)).SetFillColor(GetColorFromPalette(color_idx)));
            };

        const auto* start_stop = tc_.GetStop(bus_ptr->route.front());
        add_bus_label(start_stop, name);

        if (!bus_ptr->is_ring && bus_ptr->route.front() != bus_ptr->route.back()) {
            const auto* end_stop = tc_.GetStop(bus_ptr->route.back());
            add_bus_label(end_stop, name);
        }
        color_idx++;
    }
}

void Map::MapRenderer::AddCirrcle(svg::Document& doc, const std::map<std::string_view, 
    const transport::Stop*>& active_stops, const SphereProjector& projector){
    for (const auto& [name, stop_ptr] : active_stops) {
        doc.Add(svg::Circle()
            .SetCenter(projector({ stop_ptr->coordinate.latitude, stop_ptr->coordinate.longitude }))
            .SetRadius(render_settings_.stop_radius_)
            .SetFillColor("white"));
    }
}

void Map::MapRenderer::AddStopText(svg::Document& doc, const std::map<std::string_view, 
    const transport::Stop*>& active_stops, const SphereProjector& projector){
        for (const auto& [name, stop_ptr] : active_stops) {
        svg::Point pos = projector({ stop_ptr->coordinate.latitude, stop_ptr->coordinate.longitude });

        // подложка
        doc.Add(svg::Text()
            .SetPosition(pos).SetOffset({ render_settings_.stop_label_offset_.x_, render_settings_.stop_label_offset_.y_ })
            .SetFontSize(render_settings_.stop_label_font_size_).SetFontFamily("Verdana")
            .SetData(std::string(name)).SetFillColor(render_settings_.underlayer_color_).SetStrokeColor(render_settings_.underlayer_color_)
            .SetStrokeWidth(render_settings_.underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        // надпись
        doc.Add(svg::Text()
            .SetPosition(pos).SetOffset({ render_settings_.stop_label_offset_.x_, render_settings_.stop_label_offset_.y_ })
            .SetFontSize(render_settings_.stop_label_font_size_).SetFontFamily("Verdana")
            .SetData(std::string(name)).SetFillColor("black"));
    }
}

std::ostringstream Map::MapRenderer::Render() {
    // собираю все остановки и маршруты
    std::map<std::string_view, const transport::Bus*> sorted_buses;
    std::map<std::string_view, const transport::Stop*> active_stops;

    for (const auto& [name, bus] : *tc_.GetBuses()) {
        if (bus.route.empty()) continue;
        sorted_buses[name] = &bus;
        for (const auto& stop_name : bus.route) {
            auto stop_ptr = tc_.GetStop(stop_name);
            active_stops[stop_ptr->name] = stop_ptr;
        }
    }

	//передаю в проектор координаты
    std::vector<geo::Coordinates> coords;
    for (const auto& [name, stop] : active_stops) {
        coords.push_back({ stop->coordinate.latitude, stop->coordinate.longitude });
    }
    SphereProjector projector(coords.begin(), coords.end(), render_settings_.width_, render_settings_.height_, render_settings_.padding_);

    svg::Document doc;

    //полилайны строю и добавляю в документ
    AddPolyline(doc, sorted_buses, projector);

    // маршруты добавляю в док
    AddRouteText(doc, sorted_buses, projector);

    // точки остановок
    AddCirrcle(doc, active_stops, projector);

    //названия остановок
    AddStopText(doc, active_stops, projector);

	std::ostringstream oss;
    doc.Render(oss);
	std::string data = oss.str();
	std::ostringstream picture_out(data);
	return picture_out;
}

// получаю цвет
svg::Color Map::MapRenderer::GetColorFromPalette(size_t index) const {
    const auto& node = render_settings_.color_palette_[index % render_settings_.color_palette_.size()];
    if (node.IsString()) return node.AsString();
    const auto& arr = node.AsArray();
    if (arr.size() == 3) {
        return svg::Rgb{ static_cast<uint8_t>(arr[0].AsInt()),
                        static_cast<uint8_t>(arr[1].AsInt()),
                        static_cast<uint8_t>(arr[2].AsInt()) };
    }
    return svg::Rgba{ static_cast<uint8_t>(arr[0].AsInt()),
                     static_cast<uint8_t>(arr[1].AsInt()),
                     static_cast<uint8_t>(arr[2].AsInt()),
                     arr[3].AsDouble() };
}