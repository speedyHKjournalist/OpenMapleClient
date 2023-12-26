//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2019  Daniel Allendorf, Ryan Payton
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Affero General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Affero General Public License for more details.
//
//	You should have received a copy of the GNU Affero General Public License
//	along with this program.  If not, see <https://www.gnu.org/licenses/>.
#include "Text.h"

#include <utility>

#include "../Util/StringHandling.h"
#include "GraphicsGL.h"

namespace ms {
Text::Text(Font f,
           Alignment a,
           Color::Name c,
           Background b,
           const std::string &t,
           uint16_t mw,
           bool fm,
           int16_t la) :
    font_(f),
    alignment_(a),
    color_(c),
    background_(b),
    max_width_(mw),
    formatted_(fm),
    line_adj_(la) {
    change_text(t);
}

Text::Text(Font f,
           Alignment a,
           Color::Name c,
           const std::string &t,
           uint16_t mw,
           bool fm,
           int16_t la) :
    Text(f, a, c, Background::NONE, t, mw, fm, la) {}

Text::Text() : Text(Font::A11M, Alignment::LEFT, Color::BLACK) {}

void Text::reset_layout() {
    if (text_as_values_.empty()) {
        return;
    }

    layout_ = GraphicsGL::get().create_layout(text_as_values_,
                                              font_,
                                              alignment_,
                                              max_width_,
                                              formatted_,
                                              line_adj_);
}

void Text::change_text(const std::string &t) {
    change_text(string_conversion::to_utf8_vector(t));
    text_ = t;
}

void Text::change_text(const std::vector<uint32_t> &t) {
    if (text_as_values_ == t) {
        return;
    }

    if (text_.empty()) {
        text_ = "testing";
    }

    text_as_values_ = t;

    reset_layout();
}

void Text::change_color(Color::Name c) {
    if (color_ == c) {
        return;
    }

    color_ = c;

    reset_layout();
}

void Text::set_background(Background b) {
    background_ = b;
}

void Text::draw(const DrawArgument &args) const {
    draw(args, Range<int16_t>(0, 0));
}

void Text::draw(const DrawArgument &args,
                const Range<int16_t> &vertical) const {
    GraphicsGL::get().draw_text(args,
                                vertical,
                                text_as_values_,
                                layout_,
                                font_,
                                color_,
                                background_);
}

uint16_t Text::advance(size_t pos) const {
    return layout_.advance(pos);
}

bool Text::empty() const {
    return text_.empty();
}

size_t Text::length() const {
    return text_.size();
}

int16_t Text::width() const {
    return layout_.width();
}

int16_t Text::height() const {
    return layout_.height();
}

Point<int16_t> Text::dimensions() const {
    return layout_.get_dimensions();
}

Point<int16_t> Text::end_offset() const {
    return layout_.get_endoffset();
}

const std::string &Text::get_text() const {
    return text_;
}

Text::Layout::Layout(std::vector<Layout::Line> l,
                     std::vector<int16_t> a,
                     int16_t w,
                     int16_t h,
                     int16_t ex,
                     int16_t ey) :
    lines_(std::move(l)),
    advances_(std::move(a)),
    dimensions_(w, h),
    end_offset_(ex, ey) {}

Text::Layout::Layout() :
    Layout(std::vector<Layout::Line>(), std::vector<int16_t>(), 0, 0, 0, 0) {}

int16_t Text::Layout::width() const {
    return dimensions_.x();
}

int16_t Text::Layout::height() const {
    return dimensions_.y();
}

int16_t Text::Layout::advance(size_t index) const {
    return index < advances_.size() ? advances_[index] : 0;
}

Point<int16_t> Text::Layout::get_dimensions() const {
    return dimensions_;
}

Point<int16_t> Text::Layout::get_endoffset() const {
    return end_offset_;
}

Text::Layout::iterator Text::Layout::begin() const {
    return lines_.begin();
}

Text::Layout::iterator Text::Layout::end() const {
    return lines_.end();
}
}  // namespace ms