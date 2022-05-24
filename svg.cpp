#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext &context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

std::ostream& operator<<(std::ostream &out, const StrokeLineCap &line_cap) {
    using namespace std::literals;
    switch (line_cap) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const StrokeLineJoin &line_join) {
    using namespace std::literals;
    switch (line_join) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
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

void Circle::RenderObject(const RenderContext &context) const {
    auto &out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    std::ostringstream out;
    if (str_points_.length()) out << ' ';
    out << point.x << ',' << point.y;
    str_points_ += out.str();
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
    auto &out = context.out;
    out << "<polyline points=\""sv << str_points_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    for (auto c : data) {
        std::string rep;
        switch ((int) c) {
        case 34:
            rep = "&quot;"s;
            break; // "
        case 39:
            rep = "&apos;"s;
            break; // '
        case 60:
            rep = "&lt;"s;
            break; // <
        case 62:
            rep = "&gt;"s;
            break; // >
        case 38:
            rep = "&amp;"s;
            break; // &
        default:
            rep = c;
        }
        data_ += std::move(rep);
    }

    return *this;
}

void Text::RenderObject(const RenderContext &context) const {
    auto &out = context.out;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\""sv << " y=\""sv << pos_.y << "\""sv << " dx=\""sv << offset_.x << "\""sv
            << " dy=\""sv << offset_.y << "\""sv << " font-size=\""sv << size_ << "\""sv;
    if (font_family_.length()) out << " font-family=\"" << font_family_ << "\""sv;
    if (font_weight_.length()) out << " font-weight=\"" << font_weight_ << "\""sv;
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
    return;
}

}  // namespace svg
