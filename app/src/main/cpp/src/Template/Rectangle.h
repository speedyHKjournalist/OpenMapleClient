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

#include "Point.h"
#include "Range.h"

namespace ms {
template<class T>
class Rectangle {
public:
    Rectangle(const nl::node &sourceLeftTop,
              const nl::node &sourceRightBottom) :
        left_top_(sourceLeftTop),
        right_bottom_(sourceRightBottom) {}
    Rectangle(const nl::node &source) :
        left_top_(source["lt"]),
        right_bottom_(source["rb"]) {}

    constexpr Rectangle(Point<T> leftTop, Point<T> rightBottom) :
        left_top_(leftTop),
        right_bottom_(rightBottom) {}
    constexpr Rectangle(T left, T right, T top, T bottom) :
        left_top_(left, top),
        right_bottom_(right, bottom) {}
    constexpr Rectangle() = default;

    constexpr T width() const { return std::abs(left() - right()); }

    constexpr T height() const { return std::abs(top() - bottom()); }

    constexpr T left() const { return left_top_.x(); }

    constexpr T top() const { return left_top_.y(); }

    constexpr T right() const { return right_bottom_.x(); }

    constexpr T bottom() const { return right_bottom_.y(); }

    constexpr bool contains(const Point<T> &v) const {
        return !straight() && v.x() >= left() && v.x() <= right()
               && v.y() >= top() && v.y() <= bottom();
    }

    constexpr bool overlaps(const Rectangle<T> &ar) const {
        return get_horizontal().overlaps(Range<T>(ar.left(), ar.right()))
               && get_vertical().overlaps(Range<T>(ar.top(), ar.bottom()));
    }

    constexpr bool straight() const { return left_top_ == right_bottom_; }

    constexpr bool empty() const {
        return left_top_.straight() && right_bottom_.straight() && straight();
    }

    constexpr const Point<T> &get_left_top() const { return left_top_; }

    constexpr const Point<T> &get_right_bottom() const { return right_bottom_; }

    constexpr Range<T> get_horizontal() const { return { left(), right() }; }

    constexpr Range<T> get_vertical() const { return { top(), bottom() }; }

    void shift(const Point<T> &v) {
        left_top_ = left_top_ + v;
        right_bottom_ = right_bottom_ + v;
    }

private:
    Point<T> left_top_;
    Point<T> right_bottom_;
};
}  // namespace ms