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
#include "Portal.h"

#include <utility>

namespace ms {
Portal::Portal(const Animation *a,
               Type t,
               const std::string &nm,
               bool intramap,
               Point<int16_t> p,
               int32_t tid,
               std::string tnm) :
    animation_(a),
    type_(t),
    name_(nm),
    position_(p),
    warp_info_(tid, intramap, std::move(tnm), nm),
    touched_(false) {}

Portal::Portal() : Portal(nullptr, SPAWN, "", false, Point<int16_t>(), 0, "") {}

void Portal::update(Point<int16_t> playerpos) {
    touched_ = bounds().contains(playerpos);
}

void Portal::draw(Point<int16_t> viewpos, float inter) const {
    if (!animation_ || (type_ == HIDDEN && !touched_)) {
        return;
    }

    animation_->draw(position_ + viewpos, inter);
}

std::string Portal::get_name() const {
    return name_;
}

Portal::Type Portal::get_type() const {
    return type_;
}

Point<int16_t> Portal::get_position() const {
    return position_;
}

Rectangle<int16_t> Portal::bounds() const {
    auto lt = position_ + Point<int16_t>(-25, -100);
    auto rb = position_ + Point<int16_t>(25, 25);

    return Rectangle<int16_t>(lt, rb);
}

Portal::WarpInfo Portal::getwarpinfo() const {
    return warp_info_;
}
}  // namespace ms