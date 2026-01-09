#include "map_renderer.h"
#include <fstream>
#include <utility>

MapRenderer::MapRenderer(const double width, const double height, const double padding, const double stop_radius,
	const double line_width, const size_t bus_label_font_size, const LabelOffset bus_label_offset, const size_t stop_label_font_size,
	const LabelOffset stop_label_offset, const svg::Color underlayer_color,
	const double underlayer_width, const json::Array color_palette)
	: width_(width), height_(height), padding_(padding),
	stop_radius_(stop_radius), line_width_(line_width),
	bus_label_font_size_(bus_label_font_size), bus_label_offset_(bus_label_offset),
	stop_label_font_size_(stop_label_font_size), stop_label_offset_(stop_label_offset),
	underlayer_color_(underlayer_color), underlayer_width_(underlayer_width), color_palette_(color_palette) {
};

void MapRenderer::Render(const transport::TransportCatalogue& tc){
	std::vector<geo::Coordinates> all_coordinates;
	all_coordinates.reserve(tc.GetStops()->size());
	for (const auto& [bus_name, bus] : *tc.GetBuses()) {
		if (bus.route.empty()) continue;
		for (const auto& stop_name : bus.route) {
			auto stop_ptr = tc.GetStop(stop_name);
			all_coordinates.push_back({ stop_ptr->coordinate.latitude, stop_ptr->coordinate.longitude });
		}
	}
	SphereProjector projector(all_coordinates.begin(), all_coordinates.end(), width_, height_, padding_);
	svg::Document doc;
	std::map<std::string_view, transport::Bus> not_void_routes;
	std::vector<svg::Circle> all_circles;
	std::vector<std::pair<svg::Text, svg::Text>> stops_texts;
	all_circles.reserve(all_coordinates.size());
	std::vector<std::pair<svg::Text, svg::Text>> route_texts;
	for (const auto& [key, value] : *tc.GetBuses()) {
		if (!value.route.empty()) {
			not_void_routes[key] = value;
		}
	}
	size_t color_index = 0;
	
	for (const auto& [key, value] : not_void_routes) {
		if (color_index == color_palette_.size()) {
			color_index = 0;
		}
		svg::Polyline polyline;
		svg::Circle circle;
		svg::Text route_text_stroke;
		circle.SetRadius(stop_radius_)
			.SetFillColor("white");
		polyline.SetFillColor("none")
			.SetStrokeWidth(line_width_)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		route_text_stroke
			.SetFontSize(bus_label_font_size_)
			.SetOffset({ bus_label_offset_.x_, bus_label_offset_.y_ })
			.SetFontFamily("Verdana")
			.SetStrokeWidth(underlayer_width_)
			.SetFontWeight("bold")
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
			.SetPosition(projector({
				tc.GetStop(value.route[0])->coordinate.latitude,
				tc.GetStop(value.route[0])->coordinate.longitude
			}))
			.SetData(key.data())
			.SetFillColor(underlayer_color_)
			.SetStrokeColor(underlayer_color_);
		svg::Text route_text;
		route_text
			.SetData(key.data())
			.SetFontSize(bus_label_font_size_)
			.SetOffset({ bus_label_offset_.x_, bus_label_offset_.y_ })
			.SetFontFamily("Verdana")
			.SetFontWeight("bold")
			.SetPosition(projector({
				tc.GetStop(value.route[0])->coordinate.latitude,
				tc.GetStop(value.route[0])->coordinate.longitude
				}));
		route_texts.emplace_back(route_text_stroke, route_text);
		int counter = 0;
		for (const auto& stop : value.route) {
			if(counter != value.route.size())
			circle.SetCenter(projector({
				tc.GetStop(stop)->coordinate.latitude,
				tc.GetStop(stop)->coordinate.longitude
				}));
			all_circles.push_back(circle);
			polyline.AddPoint(projector({
				tc.GetStop(stop)->coordinate.latitude, 
				tc.GetStop(stop)->coordinate.longitude
				}));
			svg::Text stop_stroke_text;
			stop_stroke_text
				.SetStrokeColor(underlayer_color_)
				.SetData(stop.data())
				.SetOffset({ stop_label_offset_.x_, stop_label_offset_.y_ })
				.SetFontSize(stop_label_font_size_)
				.SetFontFamily("Verdana")
				.SetStrokeWidth(underlayer_width_)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
				.SetPosition(projector({
						tc.GetStop(stop)->coordinate.latitude,
						tc.GetStop(stop)->coordinate.longitude
					}))
				.SetFillColor(underlayer_color_);
			svg::Text stop_text;
			stop_text
				.SetData(stop.data())
				.SetOffset({ stop_label_offset_.x_, stop_label_offset_.y_ })
				.SetFontSize(stop_label_font_size_)
				.SetFontFamily("Verdana")
				.SetPosition(projector({
						tc.GetStop(stop)->coordinate.latitude,
						tc.GetStop(stop)->coordinate.longitude
					}))
				.SetFillColor("black");
			stops_texts.emplace_back(stop_stroke_text, stop_text);
		}
		if (!value.is_ring) {
			for (auto it = value.route.rbegin() + 1; it != value.route.rend(); ++it) {
				polyline.AddPoint(projector({
					tc.GetStop(*it)->coordinate.latitude,
					tc.GetStop(*it)->coordinate.longitude
					}));
			}
			auto it_last_elem = value.route.end() - 1;
			svg::Text second_route_text_stroke;
			second_route_text_stroke.SetFontSize(bus_label_font_size_)
				.SetOffset({ bus_label_offset_.x_, bus_label_offset_.y_ })
				.SetFontFamily("Verdana")
				.SetFontWeight("bold")
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
				.SetPosition(projector({
					tc.GetStop(*it_last_elem)->coordinate.latitude,
					tc.GetStop(*it_last_elem)->coordinate.longitude
					}))
				.SetData(key.data())
				.SetFillColor(underlayer_color_)
				.SetStrokeColor(underlayer_color_)
				.SetStrokeWidth(underlayer_width_);
			svg::Text second_route_text = route_text;
			second_route_text.SetPosition(projector({
					tc.GetStop(*it_last_elem)->coordinate.latitude,
					tc.GetStop(*it_last_elem)->coordinate.longitude
				}));
			second_route_text.SetPosition(projector({
					tc.GetStop(*it_last_elem)->coordinate.latitude,
					tc.GetStop(*it_last_elem)->coordinate.longitude
				}));
			route_texts.emplace_back(second_route_text_stroke, second_route_text);
		}
		else {
			all_circles.pop_back();
			stops_texts.pop_back();
		}
		if (color_palette_[color_index].IsString()) {
			polyline.SetStrokeColor(color_palette_[color_index].AsString());
			for (size_t i = 0; i < route_texts.size(); ++i) {
				route_texts[i].second
					.SetFillColor(color_palette_[color_index].AsString());
			}
		}
		else if (color_palette_[color_index].IsArray()) {
			if (color_palette_[color_index].AsArray().size() == 3) {
				svg::Rgb rgb(
					color_palette_[color_index].AsArray()[0].AsInt(),
					color_palette_[color_index].AsArray()[1].AsInt(),
					color_palette_[color_index].AsArray()[2].AsInt()
				);
				polyline.SetStrokeColor(rgb);
				for (size_t i = 0; i < route_texts.size(); ++i) {
					route_texts[i].second
						.SetFillColor(rgb);
				}
			}
			else if (color_palette_[color_index].AsArray().size() == 4) {
				svg::Rgba rgba(
					color_palette_[color_index].AsArray()[0].AsInt(),
					color_palette_[color_index].AsArray()[1].AsInt(),
					color_palette_[color_index].AsArray()[2].AsInt(),
					color_palette_[color_index].AsArray()[3].AsDouble()
				);
				polyline.SetStrokeColor(rgba);
				for (size_t i = 0; i < route_texts.size(); ++i) {
					route_texts[i].second
						.SetFillColor(rgba);
				}
			}
		}
		
		
		++color_index;
		doc.Add(polyline);
		
		//route_texts.clear();
		
		//all_circles.clear();
		
		//stops_texts.clear();
	}
	for (const auto& text : route_texts) {
		doc.Add(text.first);
		doc.Add(text.second);
	}
	for (const svg::Circle& circle : all_circles) {
		doc.Add(circle);
	}
	for (const auto& texts_pair : stops_texts) {
		doc.Add(texts_pair.first);
		doc.Add(texts_pair.second);
	}
	doc.Render(std::cout);
	std::fstream out_file("out.svg");
	doc.Render(out_file);
};
