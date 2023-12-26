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

#include "../Graphics/Animation.h"
#include "../Template/Point.h"

namespace ms {
// Represents an obj (map decoration) on a map.
class Obj {
public:
    Obj(const nl::node &source);

    // Update animation.
    void update();

    // Draw the obj at the specified position.
    void draw(Point<int16_t> viewpos, float inter) const;

    // Return depth of the obj.
    uint8_t getz() const;

private:
    Animation animation_;
    Point<int16_t> pos_;
    uint8_t z_;
    bool flip_;
};
}  // namespace ms