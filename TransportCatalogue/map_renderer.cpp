#include "map_renderer.h"
#include <fstream>
#include <utility>
#include <sstream>
#include <iostream>

MapRenderer::MapRenderer(const double width, const double height, const double padding, const double stop_radius,
	const double line_width, const size_t bus_label_font_size, const LabelOffset bus_label_offset, const size_t stop_label_font_size,
	const LabelOffset stop_label_offset, const svg::Color underlayer_color,
	const double underlayer_width, const json::Array color_palette, const transport::TransportCatalogue& tc)
	: width_(width), height_(height), padding_(padding),
	stop_radius_(stop_radius), line_width_(line_width),
	bus_label_font_size_(bus_label_font_size), bus_label_offset_(bus_label_offset),
	stop_label_font_size_(stop_label_font_size), stop_label_offset_(stop_label_offset),
	underlayer_color_(underlayer_color), underlayer_width_(underlayer_width), color_palette_(color_palette), tc_(tc) {
};

std::ostringstream MapRenderer::Render() {
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
    SphereProjector projector(coords.begin(), coords.end(), width_, height_, padding_);

    svg::Document doc;

    //полилайны строю и добавляю в документ
    size_t color_idx = 0;
    for (const auto& [name, bus_ptr] : sorted_buses) {
        svg::Polyline line;
        line.SetFillColor("none")
            .SetStrokeWidth(line_width_)
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

    // маршруты добавляю в док
    color_idx = 0;
    for (const auto& [name, bus_ptr] : sorted_buses) {
        auto add_bus_label = [&](const transport::Stop* stop, std::string_view label) {
            svg::Point pos = projector({ stop->coordinate.latitude, stop->coordinate.longitude });

            // подложка
            doc.Add(svg::Text()
                .SetPosition(pos).SetOffset({ bus_label_offset_.x_, bus_label_offset_.y_ })
                .SetFontSize(bus_label_font_size_).SetFontFamily("Verdana").SetFontWeight("bold")
                .SetData(std::string(label)).SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_)
                .SetStrokeWidth(underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            // надпись
            doc.Add(svg::Text()
                .SetPosition(pos).SetOffset({ bus_label_offset_.x_, bus_label_offset_.y_ })
                .SetFontSize(bus_label_font_size_).SetFontFamily("Verdana").SetFontWeight("bold")
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

    // точки остановок
    for (const auto& [name, stop_ptr] : active_stops) {
        doc.Add(svg::Circle()
            .SetCenter(projector({ stop_ptr->coordinate.latitude, stop_ptr->coordinate.longitude }))
            .SetRadius(stop_radius_)
            .SetFillColor("white"));
    }

    //названия остановок
    for (const auto& [name, stop_ptr] : active_stops) {
        svg::Point pos = projector({ stop_ptr->coordinate.latitude, stop_ptr->coordinate.longitude });

        // подложка
        doc.Add(svg::Text()
            .SetPosition(pos).SetOffset({ stop_label_offset_.x_, stop_label_offset_.y_ })
            .SetFontSize(stop_label_font_size_).SetFontFamily("Verdana")
            .SetData(std::string(name)).SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_)
            .SetStrokeWidth(underlayer_width_).SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
        // надпись
        doc.Add(svg::Text()
            .SetPosition(pos).SetOffset({ stop_label_offset_.x_, stop_label_offset_.y_ })
            .SetFontSize(stop_label_font_size_).SetFontFamily("Verdana")
            .SetData(std::string(name)).SetFillColor("black"));
    }
	std::ostringstream oss;
    doc.Render(oss);
	std::string data = oss.str();
	std::ostringstream picture_out(data);
	return picture_out;
}

// получаю цвет
svg::Color MapRenderer::GetColorFromPalette(size_t index) const {
    const auto& node = color_palette_[index % color_palette_.size()];
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