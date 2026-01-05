#include "map_renderer.h"
#include <fstream>

MapRenderer::MapRenderer(const size_t width, const size_t height, const size_t padding, const size_t stop_radius,
	const size_t line_width, const size_t bus_label_font_size, const LabelOffset bus_label_offset, const size_t stop_label_font_size,
	const LabelOffset stop_label_offset, const svg::Rgba underlayer_color,
	const size_t underlayer_width, const json::Array color_palette)
	: width_(width), height_(height), padding_(padding),
	stop_radius_(stop_radius), line_width_(line_width),
	bus_label_font_size_(bus_label_font_size), bus_label_offset_(bus_label_offset),
	stop_label_font_size_(stop_label_font_size), stop_label_offset_(stop_label_offset),
	underlayer_color_(underlayer_color), underlayer_width_(underlayer_width), color_palette_(color_palette) {
};

void MapRenderer::Render(const transport::TransportCatalogue& tc){
	std::vector<geo::Coordinates> all_coordinates;
	for (const auto& [key, value] : *tc.GetStops()) {
		all_coordinates.emplace_back(value.coordinate.latitude, value.coordinate.longitude);
	}
	SphereProjector projector(all_coordinates.begin(), all_coordinates.end(), width_, height_, padding_);
	svg::Document doc;
	std::map<std::string_view, transport::Bus> not_void_routes;
	for (const auto& [key, value] : *tc.GetBuses()) {
		if (!value.route.empty()) {
			not_void_routes[key] = value;
		}
	}
	size_t color_index = 0;
	for (const auto& [key, value] : not_void_routes) {
		if (color_index == not_void_routes.size()) {
			color_index = 0;
		}
		svg::Polyline polyline;
		polyline.SetFillColor("none")
			.SetStrokeWidth(line_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		for (const auto& stop : value.route) {
			polyline.AddPoint(projector({
				tc.GetStop(stop)->coordinate.latitude, 
				tc.GetStop(stop)->coordinate.longitude
				}));
			
		}
		if (!value.is_ring) {
			for (auto it = value.route.rbegin() + 1; it != value.route.rend(); ++it) {
				polyline.AddPoint(projector({
					tc.GetStop(*it)->coordinate.latitude,
					tc.GetStop(*it)->coordinate.longitude
					}));
			}
		}
		if (color_palette_[color_index].IsString()) {
			polyline.SetStrokeColor(color_palette_[color_index].AsString());
		}
		else if (color_palette_[color_index].IsArray()) {
			if (color_palette_[color_index].AsArray().size() == 3) {
				svg::Rgb rgb(
					color_palette_[color_index].AsArray()[0].AsInt(),
					color_palette_[color_index].AsArray()[1].AsInt(),
					color_palette_[color_index].AsArray()[2].AsInt()
				);
				polyline.SetStrokeColor(rgb);
			}
			else if (color_palette_[color_index].AsArray().size() == 4) {
				svg::Rgba rgba(
					color_palette_[color_index].AsArray()[0].AsInt(),
					color_palette_[color_index].AsArray()[1].AsInt(),
					color_palette_[color_index].AsArray()[2].AsInt(),
					color_palette_[color_index].AsArray()[3].AsDouble()
				);
				polyline.SetStrokeColor(rgba);
			}
		}
		++color_index;
		doc.Add(polyline);
	}
	doc.Render(std::cout);
	std::fstream out_file("out.svg");
	doc.Render(out_file);
};
