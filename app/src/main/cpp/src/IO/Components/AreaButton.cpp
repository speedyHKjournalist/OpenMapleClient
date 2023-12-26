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
#include "AreaButton.h"

namespace ms {
AreaButton::AreaButton(Point<int16_t> pos, Point<int16_t> dim) {
    position_ = pos;
    dimension_ = dim;
    state_ = Button::State::NORMAL;
    active_ = true;
}

Rectangle<int16_t> AreaButton::bounds(Point<int16_t> parentpos) const {
    Point<int16_t> absp = position_ + parentpos;

    return Rectangle<int16_t>(absp, absp + dimension_);
}

int16_t AreaButton::width() const {
    return dimension_.x();
}

Point<int16_t> AreaButton::origin() const {
    return Point<int16_t>();
}
}  // namespace ms