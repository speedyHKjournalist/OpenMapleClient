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
#include "Gauge.h"

namespace ms {
Gauge::Gauge(Type type, Texture front, int16_t max, float percent) :
    type_(type),
    bar_front_(front),
    maximum_(max),
    percentage_(percent),
    target_(percent) {}

Gauge::Gauge(Type type,
             Texture front,
             Texture mid,
             int16_t max,
             float percent) :
    type_(type),
    bar_front_(front),
    bar_mid_(mid),
    maximum_(max),
    percentage_(percent),
    target_(percent) {}

Gauge::Gauge(Type type,
             Texture front,
             Texture mid,
             Texture end,
             int16_t max,
             float percent) :
    type_(type),
    bar_front_(front),
    bar_mid_(mid),
    bar_end_(end),
    maximum_(max),
    percentage_(percent),
    target_(percent) {}

Gauge::Gauge() = default;

void Gauge::draw(const DrawArgument &args) const {
    int16_t length = static_cast<int16_t>(percentage_ * maximum_);

    if (length > 0) {
        if (type_ == Type::GAME) {
            bar_front_.draw(args
                            + DrawArgument(Point<int16_t>(0, 0),
                                           Point<int16_t>(length, 0)));
            bar_mid_.draw(args);
            bar_end_.draw(args + Point<int16_t>(length + 8, 20));
        } else if (type_ == Type::CASHSHOP) {
            Point<int16_t> pos_adj = Point<int16_t>(45, 1);

            bar_front_.draw(args - pos_adj);
            bar_mid_.draw(args
                          + DrawArgument(Point<int16_t>(0, 0),
                                         Point<int16_t>(length, 0)));
            bar_end_.draw(args - pos_adj
                          + Point<int16_t>(length + bar_front_.width(), 0));
        }
    }
}

void Gauge::update(float t) {
    if (target_ != t) {
        target_ = t;
        step_ = (target_ - percentage_) / 24;
    }

    if (percentage_ != target_) {
        percentage_ += step_;

        if (step_ < 0.0f) {
            if (target_ - percentage_ >= step_) {
                percentage_ = target_;
            }
        } else if (step_ > 0.0f) {
            if (target_ - percentage_ <= step_) {
                percentage_ = target_;
            }
        }
    }
}
}  // namespace ms