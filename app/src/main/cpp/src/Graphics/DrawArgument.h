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
#pragma once

#include "../Template/Rectangle.h"
#include "Color.h"

namespace ms {
class DrawArgument {
public:
    constexpr DrawArgument() : DrawArgument(0, 0) {}

    constexpr DrawArgument(int16_t x, int16_t y) :
        DrawArgument(Point<int16_t>(x, y)) {}

    constexpr DrawArgument(Point<int16_t> position) :
        DrawArgument(position, 1.0f) {}

    constexpr DrawArgument(Point<int16_t> position,
                           float xscale,
                           float yscale) :
        DrawArgument(position, position, xscale, yscale, 1.0f) {}

    constexpr DrawArgument(Point<int16_t> position, Point<int16_t> stretch) :
        DrawArgument(position, position, stretch, 1.0f, 1.0f, 1.0f, 0.0f) {}

    constexpr DrawArgument(Point<int16_t> position, bool flip) :
        DrawArgument(position, flip, 1.0f) {}

    constexpr DrawArgument(float angle,
                           Point<int16_t> position,
                           float opacity) :
        DrawArgument(angle, position, false, opacity) {}

    constexpr DrawArgument(Point<int16_t> position, float opacity) :
        DrawArgument(position, false, opacity) {}

    constexpr DrawArgument(Point<int16_t> position, Color color) :
        DrawArgument(position,
                     position,
                     Point<int16_t>(0, 0),
                     1.0f,
                     1.0f,
                     color,
                     0.0f) {}

    constexpr DrawArgument(Point<int16_t> position,
                           bool flip,
                           Point<int16_t> center) :
        DrawArgument(position, center, flip ? -1.0f : 1.0f, 1.0f, 1.0f) {}

    constexpr DrawArgument(Point<int16_t> position,
                           Point<int16_t> center,
                           float xscale,
                           float yscale,
                           float opacity) :
        DrawArgument(position,
                     center,
                     Point<int16_t>(0, 0),
                     xscale,
                     yscale,
                     opacity,
                     0.0f) {}

    constexpr DrawArgument(bool flip) :
        DrawArgument(flip ? -1.0f : 1.0f, 1.0f, 1.0f) {}

    constexpr DrawArgument(float xscale, float yscale, float opacity) :
        DrawArgument(Point<int16_t>(0, 0), xscale, yscale, opacity) {}

    constexpr DrawArgument(Point<int16_t> position,
                           float xscale,
                           float yscale,
                           float opacity) :
        DrawArgument(position, position, xscale, yscale, opacity) {}

    constexpr DrawArgument(Point<int16_t> position, bool flip, float opacity) :
        DrawArgument(position, position, flip ? -1.0f : 1.0f, 1.0f, opacity) {}

    constexpr DrawArgument(float angle,
                           Point<int16_t> position,
                           bool flip,
                           float opacity) :
        DrawArgument(position,
                     position,
                     Point<int16_t>(0, 0),
                     flip ? -1.0f : 1.0f,
                     1.0f,
                     opacity,
                     angle) {}

    constexpr DrawArgument(Point<int16_t> position,
                           Point<int16_t> center,
                           Point<int16_t> stretch,
                           float xscale,
                           float yscale,
                           float opacity,
                           float angle) :
        pos_(position),
        center_(center),
        stretch_(stretch),
        xscale_(xscale),
        yscale_(yscale),
        angle_(angle),
        color_(1.0f, 1.0f, 1.0f, opacity) {}

    constexpr DrawArgument(Point<int16_t> position,
                           Point<int16_t> center,
                           Point<int16_t> stretch,
                           float xscale,
                           float yscale,
                           Color color,
                           float angle) :
        pos_(position),
        center_(center),
        stretch_(stretch),
        xscale_(xscale),
        yscale_(yscale),
        angle_(angle),
        color_(color) {}

    constexpr Point<int16_t> get_pos() const { return pos_; }

    constexpr Point<int16_t> get_stretch() const { return stretch_; }

    constexpr float get_xscale() const { return xscale_; }

    constexpr float get_yscale() const { return yscale_; }

    constexpr const Color &get_color() const { return color_; }

    constexpr float get_angle() const { return angle_; }

    constexpr DrawArgument operator+(Point<int16_t> argpos) const {
        return { pos_ + argpos, center_ + argpos, stretch_, xscale_,
                 yscale_,       color_,           angle_ };
    }

    constexpr DrawArgument operator+(float argopc) const {
        return { pos_,    center_, stretch_,
                 xscale_, yscale_, color_.a() * argopc,
                 angle_ };
    }

    constexpr DrawArgument operator+(const DrawArgument &o) const {
        return { pos_ + o.pos_,         center_ + o.center_,
                 stretch_ + o.stretch_, xscale_ * o.xscale_,
                 yscale_ * o.yscale_,   color_ * o.color_,
                 angle_ + o.angle_ };
    }

    constexpr DrawArgument operator-(const DrawArgument &o) const {
        return { pos_ - o.pos_,         center_ - o.center_,
                 stretch_ - o.stretch_, xscale_ / o.xscale_,
                 yscale_ / o.yscale_,   color_ / o.color_,
                 angle_ - o.angle_ };
    }

    Rectangle<int16_t> get_rectangle(Point<int16_t> origin,
                                     Point<int16_t> dimensions) const {
        int16_t w = stretch_.x();

        if (w == 0) {
            w = dimensions.x();
        }

        int16_t h = stretch_.y();

        if (h == 0) {
            h = dimensions.y();
        }

        Point<int16_t> rlt = pos_ - center_ - origin;
        int16_t rl = rlt.x();
        int16_t rr = rlt.x() + w;
        int16_t rt = rlt.y();
        int16_t rb = rlt.y() + h;
        int16_t cx = center_.x();
        int16_t cy = center_.y();

        return { static_cast<int16_t>(cx + static_cast<int16_t>(xscale_ * rl)),
                 static_cast<int16_t>(cx + static_cast<int16_t>(xscale_ * rr)),
                 static_cast<int16_t>(cy + static_cast<int16_t>(yscale_ * rt)),
                 static_cast<int16_t>(cy + static_cast<int16_t>(yscale_ * rb)) };
    }

private:
    Point<int16_t> pos_;
    Point<int16_t> center_;
    Point<int16_t> stretch_;
    float xscale_;
    float yscale_;
    float angle_;
    Color color_;
};
}  // namespace ms