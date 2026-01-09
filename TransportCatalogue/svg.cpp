#include "svg.h"
#define _USE_MATH_DEFINES
#include <math.h>
namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }


    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        if (fill_color_.has_value()) {
            out << " fill=\""sv;
            std::visit(ColorSender{ out }, fill_color_.value());
            out << "\""sv; // убрал лишний пробел в конце, он добавится перед следующим атрибутом
        }
        if (stroke_color_ != std::nullopt) {
            out << " stroke=\""sv;
            std::visit(ColorSender{ out }, stroke_color_.value());
            out << "\""sv;
        }
        if (stroke_width_ != std::nullopt) {
            out << "stroke-width=\""sv << stroke_width_.value() << "\" "sv;
        }
        if (line_cap_ != std::nullopt) {
            out << "stroke-linecap=\""sv << line_cap_.value() << "\" "sv;
        }
        if (line_join_ != std::nullopt) {
            out << "stroke-linejoin=\""sv << line_join_.value() << "\" "sv;
        }
        out << "/>"sv;
    }
    //----------- Polyline -----------------
    Polyline& Polyline::AddPoint(Point point) {
        line_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        for (size_t i = 0; i < line_.size(); ++i) {
            out << line_[i].x << "," << line_[i].y;
            if (i + 1 < line_.size()) {
                out << " ";
            }
        }
        out << "\"";
        if (fill_color_) {
            out << " fill=\"";
            std::visit(ColorSender{ out }, *fill_color_);
            out << "\"";
        }
        if (stroke_color_) {
            out << " stroke=\"";
            std::visit(ColorSender{ out }, *stroke_color_);
            out << "\"";
        }
        if (stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if (line_cap_) {
            out << " stroke-linecap=\"" << *line_cap_ << "\"";
        }
        if (line_join_) {
            out << " stroke-linejoin=\"" << *line_join_ << "\"";
        }
        out << "/>";
    }

    //----------- Text ---------------------
    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        if (fill_color_) {
            out << " fill=\"";
            std::visit(ColorSender{out}, *fill_color_);
            out << "\"";
        }
        if (stroke_color_) {
            out << " stroke=\"";
            std::visit(ColorSender{out}, *stroke_color_);
            out << "\"";
        }
        if (stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if (line_cap_) {
            out << " stroke-linecap=\"" << *line_cap_ << "\"";
        }
        if (line_join_) {
            out << " stroke-linejoin=\"" << *line_join_ << "\"";
        }
        out << " x=\"" << position_.x << "\""
            << " y=\"" << position_.y << "\""
            << " dx=\"" << offset_.x << "\""
            << " dy=\"" << offset_.y << "\""
            << " font-size=\"" << font_size_ << "\"";
        if (!font_family_.empty()) {
            out << " font-family=\"" << font_family_ << "\"";
        }
        if (!font_weight_.empty()) {
            out << " font-weight=\"" << font_weight_ << "\"";
        }
        out << ">" << data_ << "</text>";
    }


    //----------- Document -----------------
    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        document_.push_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext rc(out);
        for (const auto& obj : document_) {
            obj->Render(rc);
        }
        out << "</svg>"sv;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        default:
            out << "square";
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case StrokeLineJoin::MITER:
            out << "miter";
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        default:
            out << "round";
            break;
        }
        return out;
    }


}  // namespace svg

namespace shapes {
    Triangle::Triangle(const svg::Point p1, const svg::Point p2, const svg::Point p3)
        : p1_(p1), p2_(p2), p3_(p3) {
    }

    void Triangle::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    Star::Star(svg::Point center, double outer_rad, double inner_rad, int num_rays)
        : center_(center),
        outer_rad_(outer_rad),
        inner_rad_(inner_rad),
        num_rays_(num_rays) {
    }

    void Star::Draw(svg::ObjectContainer& container) const {
        const std::string back = "red";
        const std::string stroke = "black";
        svg::Polyline polyline;
        polyline.SetFillColor(back).SetStrokeColor(stroke);
        for (int i = 0; i <= num_rays_; ++i) {
            double angle = 2 * M_PI * (i % static_cast<int>(num_rays_)) / num_rays_;
            polyline.AddPoint({ center_.x + outer_rad_ * sin(angle), center_.y - outer_rad_ * cos(angle) });
            if (i == num_rays_) {
                break;
            }
            angle += M_PI / num_rays_;
            polyline.AddPoint({ center_.x + inner_rad_ * sin(angle), center_.y - inner_rad_ * cos(angle) });
        }
        container.Add(std::move(polyline));
    }

    Snowman::Snowman(svg::Point center, double radius)
        : center_(center), radius_(radius) {
    }

    void Snowman::Draw(svg::ObjectContainer& container) const {
        const std::string back = "rgb(240,240,240)";
        const std::string stroke = "black";
        // 1. Нижний круг (рисуем первым, чтобы он был на заднем плане)
        container.Add(svg::Circle()
            .SetCenter({ center_.x, center_.y + 5 * radius_ }) // смещение 2r + 3r
            .SetRadius(radius_ * 2.0)
            .SetFillColor(back)
            .SetStrokeColor(stroke));

        // 2. Средний круг
        container.Add(svg::Circle()
            .SetCenter({ center_.x, center_.y + 2 * radius_ }) // смещение 2r
            .SetRadius(radius_ * 1.5)
            .SetFillColor(back)
            .SetStrokeColor(stroke));

        // 3. Верхний круг (голова)
        container.Add(svg::Circle()
            .SetCenter(center_)
            .SetRadius(radius_)
            .SetFillColor(back)
            .SetStrokeColor(stroke));
    }
}