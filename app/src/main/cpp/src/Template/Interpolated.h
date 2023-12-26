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
#include "Template.h"
namespace ms {
template<typename T>
class Nominal {
public:
    constexpr Nominal() : now_(T()), before_(T()), threshold_(0.0f) {}

    T get() const { return now_; }

    T get(float alpha) const { return alpha >= threshold_ ? now_ : before_; }

    T last() const { return before_; }

    void set(T value) {
        now_ = value;
        before_ = value;
    }

    void normalize() { before_ = now_; }

    bool normalized() const { return before_ == now_; }

    void next(T value, float thrs) {
        before_ = now_;
        now_ = value;
        threshold_ = thrs;
    }

    bool operator==(T value) const { return now_ == value; }

    bool operator!=(T value) const { return now_ != value; }

    T operator+(T value) const { return now_ + value; }

    T operator-(T value) const { return now_ - value; }

    T operator*(T value) const { return now_ * value; }

    T operator/(T value) const { return now_ / value; }

private:
    T now_;
    T before_;
    float threshold_;
};

template<typename T>
class Linear {
    static_assert(
        std::is_floating_point<T>::value,
        "Template parameter 'T' for Linear must be a floating point type.");

public:
    T get() const { return now_; }

    T get(T alpha) const { return lerp(before_, now_, alpha); }

    T last() const { return before_; }

    void set(T value) {
        now_ = value;
        before_ = value;
    }

    void normalize() { before_ = now_; }

    bool normalized() const { return before_ == now_; }

    void operator=(T value) {
        before_ = now_;
        now_ = value;
    }

    void operator+=(T value) {
        before_ = now_;
        now_ += value;
    }

    void operator-=(T value) {
        before_ = now_;
        now_ -= value;
    }

    bool operator==(T value) const { return now_ == value; }

    bool operator!=(T value) const { return now_ != value; }

    bool operator<(T value) const { return now_ < value; }

    bool operator<=(T value) const { return now_ <= value; }

    bool operator>(T value) const { return now_ > value; }

    bool operator>=(T value) const { return now_ >= value; }

    T operator+(T value) const { return now_ + value; }

    T operator-(T value) const { return now_ - value; }

    T operator*(T value) const { return now_ * value; }

    T operator/(T value) const { return now_ / value; }

    T operator+(Linear<T> value) const { return now_ + value.get(); }

    T operator-(Linear<T> value) const { return now_ - value.get(); }

    T operator*(Linear<T> value) const { return now_ * value.get(); }

    T operator/(Linear<T> value) const { return now_ / value.get(); }

private:
    T now_;
    T before_;
};
}  // namespace ms