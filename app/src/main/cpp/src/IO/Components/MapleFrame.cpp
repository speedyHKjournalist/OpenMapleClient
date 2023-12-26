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
#include "MapleFrame.h"

#include <algorithm>
#include <nlnx/nx.hpp>

namespace ms {
MapleFrame::MapleFrame(const nl::node &src) {
    center_ = src["c"];
    east_ = src["e"];
    north_east_ = src["ne"];
    north_ = src["n"];
    north_west_ = src["nw"];
    west_ = src["w"];
    south_west_ = src["sw"];
    south_ = src["s"];
    south_east_ = src["se"];

    xtile_ = std::max<int16_t>(north_.width(), 1);
    ytile_ = std::max<int16_t>(west_.height(), 1);
}

MapleFrame::MapleFrame() = default;

void MapleFrame::draw(Point<int16_t> position,
                      int16_t rwidth,
                      int16_t rheight) const {
    int16_t numhor = rwidth / xtile_ + 2;
    int16_t numver = rheight / ytile_;
    int16_t width = numhor * xtile_;
    int16_t height = numver * ytile_;
    int16_t left = position.x() - width / 2;
    int16_t top = position.y() - height;
    int16_t right = left + width;
    int16_t bottom = top + height;

    north_west_.draw(DrawArgument(left, top));
    south_west_.draw(DrawArgument(left, bottom));

    for (int y = top; y < bottom; y += ytile_) {
        west_.draw(DrawArgument(left, y));
        east_.draw(DrawArgument(right, y));
    }

    center_.draw(
        DrawArgument(Point<int16_t>(left, top), Point<int16_t>(width, height)));

    for (int x = left; x < right; x += xtile_) {
        north_.draw(DrawArgument(x, top));
        south_.draw(DrawArgument(x, bottom));
    }

    north_east_.draw(DrawArgument(right, top));
    south_east_.draw(DrawArgument(right, bottom));
}
}  // namespace ms