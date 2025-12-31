#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {
    struct Rgb;
    struct Rgba;
    using namespace std::literals;


    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы


    struct Rgb {
    public:
        Rgb() = default;
        Rgb(const int r, const int g, const int b) : red(r), green(g), blue(b) {}
        uint8_t red, green, blue;
    };

    struct Rgba {
    public:
        Rgba() = default;
        Rgba(const int r, const int g, const int b, const double a) : red(r), green(g), blue(b), opacity(a) {}
        uint8_t red{}, green{}, blue{};
        double opacity = 1.0;
    };

    struct ColorSender {
        std::ostream& out; // Храним ссылку на поток

        void operator()(std::monostate) const {
            out << "none"sv;
        }
        void operator()(const std::string& color) const {
            out << color;
        }
        void operator()(const Rgb rgb) const {
            out << "rgb("sv << +rgb.red << ","sv << +rgb.green << ","sv << +rgb.blue << ")"sv;
        }
        void operator()(const Rgba rgba) const {
            out << "rgba("sv << +rgba.red << ","sv << +rgba.green << ","sv << +rgba.blue << ","sv << rgba.opacity << ")"sv;
        }
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    // 3. И только теперь создаем константу
    inline const Color NoneColor = "none";

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };



    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap);

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join);


    template <typename T>
    class PathProps {
    public:
        T& SetFillColor(const Color& color) {
            fill_color_ = color;
            return AsOwner();
        }
        T& SetStrokeColor(const Color& color) {
            stroke_color_ = color;
            return AsOwner();
        }
        T& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        T& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = line_cap;
            return AsOwner();
        }
        T& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = line_join;
            return AsOwner();
        }

    protected:
        std::optional<Color> fill_color_ = std::nullopt;
        std::optional<Color> stroke_color_ = std::nullopt;
        std::optional<double> stroke_width_ = std::nullopt;
        std::optional<StrokeLineCap> line_cap_ = std::nullopt;
        std::optional<StrokeLineJoin> line_join_ = std::nullopt;
    private:
        T& AsOwner() {
            return static_cast<T&>(*this);
        }
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };



    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle() = default;
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point center_{ 0.0, 0.0 };
        double radius_ = 1.0;
    };

    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline> {
    public:
        Polyline() = default;
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
    private:
        std::vector<Point> line_;
        void RenderObject(const RenderContext& context) const override;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text : public Object, public PathProps<Text> {
    public:
        Text() = default;
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        Point position_{ 0.0, 0.0 };
        Point offset_{ 0.0, 0.0 };
        uint32_t font_size_ = 1;
        std::string font_family_ = "";
        std::string font_weight_ = "";
        std::string data_ = "";
        void RenderObject(const RenderContext& context) const override;
    };



    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;

        template <typename Obj>
        void Add(Obj object) {
            AddPtr(std::make_unique<Obj>(std::forward<Obj>(object)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    class Drawable {//подумать над конструктором и деструктором
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        Document() = default;
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
        */
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;

        // Прочие методы и данные, необходимые для реализации класса Document
    private:
        std::vector<std::unique_ptr<Object>> document_;
    };

}  // namespace svg

namespace shapes {
    class Triangle final : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star final : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_rad, double inner_rad, int num_rays);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point center_;
        double outer_rad_;
        double inner_rad_;
        double num_rays_;
    };

    class Snowman final : public svg::Drawable {
    public:
        Snowman(svg::Point center, double radius);
        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point center_;
        double radius_;
    };
}