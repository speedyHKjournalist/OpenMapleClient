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

#include <cstdint>
#include <nlnx/node.hpp>
#include <string>

#include "../Graphics/Texture.h"
#include "../Template/Point.h"

namespace ms {
// Represents a tile on a map.
class Tile {
public:
    Tile(const nl::node &src, const std::string &tileset);

    // Draw the tile.
    void draw(Point<int16_t> viewpos) const;

    // Returns depth of the tile.
    uint8_t getz() const;

private:
    Texture texture_;
    Point<int16_t> pos_;
    uint8_t z_;
};
}  // namespace ms