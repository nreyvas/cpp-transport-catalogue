#include "svg.h"

namespace svg {

    using namespace std::literals;

    Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r),
        green(g),
        blue(b) {}

    Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
        : red(r),
        green(g),
        blue(b),
        opacity(a) {}

    void ColorPrinter::operator()(std::monostate) const
    {
        out << "none"s;
    }

    void ColorPrinter::operator()(std::string col_string) const
    {
        out << col_string;
    }

    void ColorPrinter::operator()(Rgb col_rgb) const
    {
        out << "rgb(" << (int)col_rgb.red << ',' << (int)col_rgb.green <<
            ',' << (int)col_rgb.blue << ')';
    }

    void ColorPrinter::operator()(Rgba col_rgba) const
    {
        out << "rgba(" << (int)col_rgba.red << ',' << (int)col_rgba.green <<
            ',' << (int)col_rgba.blue << ',' << col_rgba.opacity << ')';
    }

    std::ostream& operator<<(std::ostream& out, Color color)
    {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    Point operator+(const Point& lhs, const Point& rhs)
    {
        return Point{ (lhs.x + rhs.x), (lhs.y + rhs.y) };
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap)
    {
        switch (line_cap)
        {
        case svg::StrokeLineCap::BUTT:
            out << "butt"s;
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round"s;
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square"s;
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join)
    {
        switch (line_join)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs"s;
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel"s;
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter"s;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"s;
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round"s;
            break;
        default:
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const
    {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<circle";
        out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first_point = true;
        for (Point p : points_)
        {
            if (!first_point)
            {
                out << ' ';
            }
            out << p.x << ',' << p.y;
            first_point = false;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        std::string processed_text;
        for (char ch : data_)
        {
            if (ch == '\"')
            {
                processed_text.append("&quot;");
            }
            else if (ch == '<')
            {
                processed_text.append("&lt;");
            }
            else if (ch == '>')
            {
                processed_text.append("&gt;");
            }
            else if (ch == '\'')
            {
                processed_text.append("&apos;");
            }
            else if (ch == '&')
            {
                processed_text.append("&amp;");
            }
            else
            {
                processed_text.push_back(ch);
            }
        }
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\"";

        if (!font_family_.empty())
        {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (!font_weight_.empty())
        {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << processed_text << "</text>"sv;
    }

    Document::Document(Document&& other_document) noexcept
    {
        objects_ = std::move(other_document.objects_);
    }

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext context(out, 2, 2);
        for (auto& obj : objects_)
        {
            obj->Render(context);
        }

        out << "</svg>"sv;
    }
}  // namespace svg