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
#include "IconCover.h"

namespace ms {
IconCover::IconCover(Type t, int32_t duration) {
    cover_ = ColorBox(30, 30, Color::Name::BLACK, 0.6f);

    if (duration <= Constants::TIMESTEP) {
        scale_step_ = 1.0f;
    } else {
        scale_step_ = Constants::TIMESTEP * 1.0f / duration;
    }

    type_ = t;

    switch (type_) {
        case Type::BUFF: yscale_.set(0.0f); break;
        case Type::COOLDOWN: yscale_.set(1.0f); break;
    }
}

void IconCover::draw(Point<int16_t> position, float alpha) const {
    float interyscale = yscale_.get(alpha);
    auto interheight = static_cast<int16_t>(30 * interyscale);

    if (interheight == 0) {
        return;
    }

    cover_.draw(DrawArgument(position + Point<int16_t>(0, 30 - interheight),
                             Point<int16_t>(30, interheight)));
}

void IconCover::update() {
    switch (type_) {
        case Type::BUFF:
            yscale_ += scale_step_;

            if (yscale_.last() >= 1.0f) {
                yscale_.set(1.0f);
                scale_step_ = 0.0f;
            }

            break;
        case Type::COOLDOWN:
            yscale_ -= scale_step_;

            if (yscale_.last() <= 0.0f) {
                yscale_.set(0.0f);
                scale_step_ = 0.0f;
            }

            break;
    }
}
}  // namespace ms