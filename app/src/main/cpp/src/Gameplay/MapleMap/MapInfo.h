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

#include <functional>
#include <optional>
#include <vector>

#include "../../Template/Point.h"
#include "../../Template/Range.h"

namespace ms {
class Seat {
public:
    Seat(const nl::node &source);

    bool inrange(Point<int16_t> position) const;

    Point<int16_t> getpos() const;

private:
    Point<int16_t> pos_;
};

class Ladder {
public:
    Ladder(const nl::node &source);

    bool is_ladder() const;

    bool inrange(Point<int16_t> position, bool upwards) const;

    bool felloff(int16_t y, bool downwards) const;

    int16_t get_x() const;

private:
    int16_t x_;
    int16_t y1_;
    int16_t y2_;
    bool ladder_;
};

class MapInfo {
public:
    MapInfo(const nl::node &src, Range<int16_t> walls, Range<int16_t> borders);

    MapInfo();

    bool is_underwater() const;

    std::string get_bgm() const;

    Range<int16_t> get_walls() const;

    Range<int16_t> get_borders() const;

    // Find a seat the player's position
    std::optional<std::reference_wrapper<const Seat>> findseat(
        Point<int16_t> position) const;
    // Find a ladder at the player's position
    // !upwards - implies downwards
    std::optional<std::reference_wrapper<const Ladder>> findladder(
        Point<int16_t> position,
        bool upwards) const;

private:
    int32_t field_limit_;
    bool cloud_;
    std::string bgm_;
    std::string map_desc_;
    std::string map_name_;
    std::string street_name_;
    std::string map_mark_;
    bool swim_;
    bool town_;
    bool hide_minimap_;
    Range<int16_t> map_walls_;
    Range<int16_t> map_borders_;
    std::vector<Seat> seats_;
    std::vector<Ladder> ladders_;
};
}  // namespace ms