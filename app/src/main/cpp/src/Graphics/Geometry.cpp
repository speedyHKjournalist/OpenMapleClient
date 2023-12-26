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
#include "Geometry.h"

namespace ms {
void Geometry::draw(int16_t x,
                    int16_t y,
                    int16_t w,
                    int16_t h,
                    Color::Name cid,
                    float opacity) const {
    if (w == 0 || h == 0 || opacity <= 0.0f) {
        return;
    }

    const float *color = Color::colors[cid];

    GraphicsGL::get()
        .draw_rectangle(x, y, w, h, color[0], color[1], color[2], opacity);
}

ColorBox::ColorBox(int16_t w, int16_t h, Color::Name c, float o) :
    width_(w),
    height_(h),
    color_(c),
    opacity_(o) {}

ColorBox::ColorBox() : ColorBox(0, 0, Color::Name::BLACK, 0.0f) {}

void ColorBox::set_width(int16_t w) {
    width_ = w;
}

void ColorBox::set_height(int16_t h) {
    height_ = h;
}

void ColorBox::set_color(Color::Name c) {
    color_ = c;
}

void ColorBox::set_opacity(float o) {
    opacity_ = o;
}

void ColorBox::draw(const DrawArgument &args) const {
    Point<int16_t> absp = args.get_pos();
    int16_t absw = args.get_stretch().x();

    if (absw == 0) {
        absw = width_;
    }

    int16_t absh = args.get_stretch().y();

    if (absh == 0) {
        absh = height_;
    }

    absw = static_cast<int16_t>(absw * args.get_xscale());
    absh = static_cast<int16_t>(absh * args.get_yscale());

    float absopc = opacity_ * args.get_color().a();

    Geometry::draw(absp.x(), absp.y(), absw, absh, color_, absopc);
}

ColorLine::ColorLine(int16_t w, Color::Name c, float o) :
    width_(w),
    color_(c),
    opacity_(o) {}

ColorLine::ColorLine() : ColorLine(0, Color::Name::BLACK, 0.0f) {}

void ColorLine::set_width(int16_t w) {
    width_ = w;
}

void ColorLine::set_color(Color::Name c) {
    color_ = c;
}

void ColorLine::set_opacity(float o) {
    opacity_ = o;
}

void ColorLine::draw(const DrawArgument &args) const {
    Point<int16_t> absp = args.get_pos();
    int16_t absw = args.get_stretch().x();

    if (absw == 0) {
        absw = width_;
    }

    int16_t absh = args.get_stretch().y();

    if (absh == 0) {
        absh = 1;
    }

    absw = static_cast<int16_t>(absw * args.get_xscale());
    absh = static_cast<int16_t>(absh * args.get_yscale());

    float absopc = opacity_ * args.get_color().a();

    Geometry::draw(absp.x(), absp.y(), absw, absh, color_, absopc);
}

void MobHpBar::draw(Point<int16_t> position, int16_t hppercent) const {
    int16_t fillw =
        static_cast<int16_t>((WIDTH - 6) * static_cast<float>(hppercent) / 100);
    int16_t x = position.x() - WIDTH / 2;
    int16_t y = position.y() - HEIGHT * 3;

    Geometry::draw(x, y, WIDTH, HEIGHT, Color::Name::BLACK, 1.0f);
    Geometry::draw(x + 1, y + 1, WIDTH - 2, 1, Color::Name::WHITE, 1.0f);
    Geometry::draw(x + 1,
                   y + HEIGHT - 2,
                   WIDTH - 2,
                   1,
                   Color::Name::WHITE,
                   1.0f);
    Geometry::draw(x + 1, y + 2, 1, HEIGHT - 4, Color::Name::WHITE, 1.0f);
    Geometry::draw(x + WIDTH - 2,
                   y + 2,
                   1,
                   HEIGHT - 4,
                   Color::Name::WHITE,
                   1.0f);
    Geometry::draw(x + 3, y + 3, fillw, 3, Color::Name::LIGHTGREEN, 1.0f);
    Geometry::draw(x + 3, y + 6, fillw, 1, Color::Name::JAPANESELAUREL, 1.0f);
}
}  // namespace ms