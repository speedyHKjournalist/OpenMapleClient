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
#include "Obj.h"

#include <nlnx/nx.hpp>

namespace ms {
Obj::Obj(const nl::node &src) {
    animation_ = Animation(nl::nx::map["Obj"][src["oS"] + ".img"][src["l0"]]
                                      [src["l1"]][src["l2"]]);
    pos_ = Point<int16_t>(src["x"], src["y"]);
    flip_ = src["f"].get_bool();
    z_ = src["z"];
}

void Obj::update() {
    animation_.update();
}

void Obj::draw(Point<int16_t> viewpos, float inter) const {
    animation_.draw(DrawArgument(pos_ + viewpos, flip_), inter);
}

uint8_t Obj::getz() const {
    return z_;
}
}  // namespace ms