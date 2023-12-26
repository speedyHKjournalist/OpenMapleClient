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

#include <cmath>
#include <nlnx/node.hpp>

namespace ms {
template<class T>
class Point {
public:
    // Construct a point from a vector property
    Point(const nl::node &src) :
        a_(static_cast<T>(src.x())),
        b_(static_cast<T>(src.y())) {}

    // Construct a point from the specified coordinates
    constexpr Point(T first, T second) : a_(first), b_(second) {}

    // Construct a point with coordinates (0, 0)
    constexpr Point() : Point(0, 0) {}

    // Return the x-coordinate
    constexpr T x() const { return a_; }

    // Return the y-coordinate
    constexpr T y() const { return b_; }

    // Return the inner product
    constexpr T length() const {
        return static_cast<T>(std::sqrt(a_ * a_ + b_ * b_));
    }

    // Check whether the coordinates are equal
    constexpr bool straight() const { return a_ == b_; }

    // Return a string representation of the point
    std::string to_string() const {
        return "(" + std::to_string(a_) + "," + std::to_string(b_) + ")";
    }

    // Return the distance to another point
    constexpr T distance(Point<T> v) const {
        return Point<T>(a_ - v.a_, b_ - v.b_).length();
    }

    // Set the x-coordinate
    void set_x(T v) { a_ = v; }

    // Set the y-coordinate
    void set_y(T v) { b_ = v; }

    // Shift the x-coordinate by the specified amount
    void shift_x(T v) { a_ += v; }

    // Shift the y-coordinate by the specified amount
    void shift_y(T v) { b_ += v; }

    // Shift the coordinates by the specified amounts
    void shift(T x, T y) {
        a_ += x;
        b_ += y;
    }

    // Shift the this point by the amounts defined by another point
    // Equivalent to += operator
    void shift(Point<T> v) {
        a_ += v.a_;
        b_ += v.b_;
    }

    // Take the absolute value of the point
    Point<T> abs() {
        return { static_cast<T>(std::abs(a_)), static_cast<T>(std::abs(b_)) };
    }

    // Check whether point is equivalent to the specified point
    constexpr bool operator==(const Point<T> &v) const {
        return a_ == v.a_ && b_ == v.b_;
    }

    // Check whether point is not equivalent to the specified point
    constexpr bool operator!=(const Point<T> &v) const { return !(*this == v); }

    // Shift the this point by the amounts defined by another point
    void operator+=(Point<T> v) {
        a_ += v.a_;
        b_ += v.b_;
    }

    // Shift the this point in reverse direction by the amounts defined by
    // another point
    void operator-=(Point<T> v) {
        a_ -= v.a_;
        b_ -= v.b_;
    }

    // Return a point whose coordinates are the negation of this point's
    // coordinates
    constexpr Point<T> operator-() const { return { static_cast<short>(-a_), static_cast<short>(-b_) }; }

    // Return a point whose coordinates have been added the specified amount
    constexpr Point<T> operator+(T v) const { return { static_cast<T>(a_ + v), static_cast<T>(b_ + v) }; }

    // Return a point whose coordinates have been subtracted the specified
    // amount
    constexpr Point<T> operator-(T v) const { return { static_cast<T>(a_ - v), static_cast<T>(b_ - v) }; }

    // Return a point whose coordinates have been multiplied by the specified
    // amount
    constexpr Point<T> operator*(T v) const { return { a_ * v, b_ * v }; }

    // Return a point whose coordinates have been divided by the specified
    // amount
    constexpr Point<T> operator/(T v) const { return { static_cast<T>(a_ / v), static_cast<T>(b_ / v) }; }

    // Return a point whose coordinates are the sum of this and another points
    // coordinates
    constexpr Point<T> operator+(Point<T> v) const {
        return { static_cast<T>(a_ + v.a_), static_cast<T>(b_ + v.b_) };
    }

    // Return a point whose coordinates are the difference of this and another
    // points coordinates
    constexpr Point<T> operator-(Point<T> v) const {
        return { static_cast<T>(a_ - v.a_), static_cast<T>(b_ - v.b_) };
    }

    // Return a point whose coordinates are the product of this and another
    // points coordinates
    constexpr Point<T> operator*(Point<T> v) const {
        return { a_ / v.a_, b_ / v.b_ };
    }

    // Return a point whose coordinates are the division of this and another
    // points coordinates
    constexpr Point<T> operator/(Point<T> v) const {
        return { static_cast<T>(a_ / (v.a_ == 0 ? 1 : v.a_)), static_cast<int16_t>(b_ / (v.b_ == 0 ? 1 : v.b_)) };
    }

private:
    T a_;
    T b_;
};
}  // namespace ms