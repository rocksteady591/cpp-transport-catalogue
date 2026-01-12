#pragma once
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <map>

namespace Map {
    struct LabelOffset {
        public:
            LabelOffset(double x, double y) : x_(x), y_(y){}
        double x_;
        double y_;
    };
    struct RenderSettings{
    public:
        RenderSettings(double width, double height, double padding, double stop_radius, double line_width,
        size_t bus_label_font_size, LabelOffset bus_label_offset, size_t stop_label_font_size, 
        LabelOffset stop_label_offset, svg::Color underlayer_color, double underlayer_width, json::Array color_palette) 
        : width_(width),height_(height),padding_(padding), stop_radius_(stop_radius),line_width_(line_width),
        bus_label_font_size_(bus_label_font_size), bus_label_offset_(bus_label_offset),
        stop_label_font_size_(stop_label_font_size), stop_label_offset_(stop_label_offset),
        underlayer_color_(underlayer_color), underlayer_width_(underlayer_width), color_palette_(color_palette){}

        double width_;
        double height_;
        double padding_;
        double stop_radius_;
        double line_width_;
        size_t bus_label_font_size_;
        LabelOffset bus_label_offset_;
        size_t stop_label_font_size_;
        LabelOffset stop_label_offset_;
        svg::Color underlayer_color_;
        double underlayer_width_;
        json::Array color_palette_;
    };

    inline const double EPSILON = 1e-6;
    inline bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) //
        {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };
    class MapRenderer {
    public:
        MapRenderer(const RenderSettings& render_settings, const transport::TransportCatalogue& tc);
        std::ostringstream Render();
        svg::Color GetColorFromPalette(size_t index) const;
    private:
        void AddPolyline(svg::Document& doc, const std::map<std::string_view, const transport::Bus*>& sorted_buses, const SphereProjector& projector);
        void AddRouteText(svg::Document& doc, const std::map<std::string_view, const transport::Bus*>& sorted_buses, const SphereProjector& projector);
        void AddCirrcle(svg::Document& doc, const std::map<std::string_view, const transport::Stop*>& active_stops, const SphereProjector& projector);
        void AddStopText(svg::Document& doc, const std::map<std::string_view, const transport::Stop*>& active_stops, const SphereProjector& projector);
        RenderSettings render_settings_;
        const transport::TransportCatalogue& tc_;
    };

}
