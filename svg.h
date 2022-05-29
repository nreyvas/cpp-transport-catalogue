#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg
{
    struct Rgb
    {
        Rgb(uint8_t r = 0u, uint8_t g = 0u, uint8_t b = 0u);
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct Rgba
    {
        Rgba(uint8_t r = 0u, uint8_t g = 0u, uint8_t b = 0u, double a = 1.0);
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        double opacity;
    };

    //using NonceColor = std::monostate;
    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ std::monostate{} };

    struct ColorPrinter
    {
        std::ostream& out;

        void operator()(std::monostate) const;

        void operator()(std::string) const;

        void operator()(Rgb) const;

        void operator()(Rgba) const;
    };

    std::ostream& operator<<(std::ostream& out, Color color);

    struct Point
    {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {}
        double x = 0;
        double y = 0;
    };

    Point operator+(const Point& lhs, const Point& rhs);

    /*
     * Subsidiary structure, holding context for output of a SVG-document with indentations
     * Keeps a reference to an out stream, current value and an indentation for an element output
     */
    struct RenderContext
    {
        RenderContext(std::ostream& out)
            : out(out) {}

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {}

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const
        {
            for (int i = 0; i < indent; ++i)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
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

    template <typename Owner>
    class PathProps     // abstract class
    {
    public:

        Owner& SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width)
        {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap)
        {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            stroke_line_join = line_join;
            return AsOwner();
        }

    protected:

        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color_)
            {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            else
            {
                out << " fill=\"none\""sv;
            }
            if (stroke_color_)
            {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_)
            {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_)
            {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join)
            {
                out << " stroke-linejoin=\""sv << *stroke_line_join << "\""sv;
            }
        }

    private:

        Owner& AsOwner()
        {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join;
    };

    class Object
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:

        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Circle class simulates <circle> element for depicting a circle
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle>
    {
    public:

        Circle& SetCenter(Point center);

        Circle& SetRadius(double radius);

    private:

        void RenderObject(const RenderContext& context) const override;

        Point center_ = { 0,0 };
        double radius_ = 1.0;
    };

    /*
     * Class Polyline simulates <polyline> element for rendering polylines
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline : public Object, public PathProps<Polyline>
    {
    public:

        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    /*
     * Class Text simulates <text> element for rendering text
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public PathProps<Text>
    {
    public:

        Text& SetPosition(Point pos);

        Text& SetOffset(Point offset);

        Text& SetFontSize(uint32_t size);

        Text& SetFontFamily(std::string font_family);

        Text& SetFontWeight(std::string font_weight);

        Text& SetData(std::string data);

    private:

        void RenderObject(const RenderContext& context) const override;

        Point pos_{ 0.0 ,0.0 };
        Point offset_{ 0.0 ,0.0 };
        uint32_t font_size_{ 1 };
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    //-------------------------------------------------------------------------------------------------------

    class ObjectContainer   // abstract class
    {
    public:

        ObjectContainer() = default;

        template <typename Obj>
        void Add(Obj obj)
        {
            AddPtr(std::move(std::make_unique<Obj>(std::move(obj))));
        }

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        virtual ~ObjectContainer() {};

    protected:

        std::vector<std::unique_ptr<Object>> objects_;
    };

    class Document : public ObjectContainer
    {
    public:

        Document(Document&& other_document) noexcept;

        Document() = default;

        void AddPtr(std::unique_ptr<Object>&& obj) override;

        void Render(std::ostream& out) const;
    };

    class Drawable  // abstract class
    {
    public:

        virtual void Draw(svg::ObjectContainer& container) const = 0;

        virtual ~Drawable() {};
    };

}  // namespace svg