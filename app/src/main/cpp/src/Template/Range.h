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

namespace ms {
template<class T>
class Range {
public:
    // Construct a range from the specified values
    constexpr Range(const T &first, const T &second) : a_(first), b_(second) {}

    // Construct a range of (0, 0)
    constexpr Range() : Range(0, 0) {}

    // Return the first value
    constexpr const T &first() const { return a_; }

    // Return the second value
    constexpr const T &second() const { return b_; }

    // Return the greater value
    constexpr const T &greater() const { return (a_ > b_) ? a_ : b_; }

    // Return the smaller value
    constexpr const T &smaller() const { return (a_ < b_) ? a_ : b_; }

    // Return the difference between the values
    constexpr T delta() const { return b_ - a_; }

    // Return the absolute difference between the values
    constexpr T length() const { return greater() - smaller(); }

    // Return the mean of both values
    constexpr T center() const { return (a_ + b_) / 2; }

    // Check if both values are equal
    constexpr bool empty() const { return a_ == b_; }

    // Check if the range contains a value
    constexpr bool contains(const T &v) const { return v >= a_ && v <= b_; }

    // Check if the range contains another range
    constexpr bool contains(const Range<T> &v) const {
        return v.a_ >= a_ && v.b_ <= b_;
    }

    // Check if the ranges overlap
    constexpr bool overlaps(const Range<T> &v) const {
        return contains(v.a_) || contains(v.b_) || v.contains(a_)
               || v.contains(b_);
    }

    // Check whether the range is equivalent to another range
    constexpr bool operator==(const Range<T> &v) const {
        return a_ == v.a_ && b_ == v.b_;
    }

    // Check whether the range is not equivalent to another range
    constexpr bool operator!=(const Range<T> &v) const { return !(*this == v); }

    // Shift this range by the amounts defined by another range
    constexpr Range<T> operator+(const Range<T> &v) const {
        return { a_ + v.a_, b_ + v.b_ };
    }

    // Shift this range by the negative amounts defined by another range
    constexpr Range<T> operator-(const Range<T> &v) const {
        return { a_ - v.a_, b_ - v.b_ };
    }

    // Return the negative of this range
    constexpr Range<T> operator-() const { return { -a_, -b_ }; }

    // Construct a symmetric range around mid
    static Range<T> symmetric(const T &mid, const T &tail) {
        return { mid - tail, mid + tail };
    }

private:
    T a_;
    T b_;
};
}  // namespace ms