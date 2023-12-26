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
#include "MapInfo.h"

#include "../Constants.h"

namespace ms {
MapInfo::MapInfo(const nl::node &src,
                 Range<int16_t> walls,
                 Range<int16_t> borders) {
    nl::node info = src["info"];

    if (info["VRLeft"].data_type() == nl::node::type::integer) {
        map_walls_ = Range<int16_t>(info["VRLeft"], info["VRRight"]);
        map_borders_ = Range<int16_t>(info["VRTop"], info["VRBottom"]);
    } else {
        map_walls_ = walls;
        map_borders_ = borders;
    }

    std::string bgmpath = info["bgm"];
    size_t split = bgmpath.find('/');
    bgm_ = bgmpath.substr(0, split) + ".img/" + bgmpath.substr(split + 1);

    cloud_ = info["cloud"].get_bool();
    field_limit_ = info["fieldLimit"];
    hide_minimap_ = info["hideMinimap"].get_bool();
    map_mark_ = std::string(info["mapMark"]);
    swim_ = info["swim"].get_bool();
    town_ = info["town"].get_bool();

    for (const auto &seat : src["seat"]) {
        seats_.push_back(seat);
    }

    for (const auto &ladder : src["ladderRope"]) {
        ladders_.push_back(ladder);
    }
}

MapInfo::MapInfo() = default;

bool MapInfo::is_underwater() const {
    return swim_;
}

std::string MapInfo::get_bgm() const {
    return bgm_;
}

Range<int16_t> MapInfo::get_walls() const {
    return map_walls_;
}

Range<int16_t> MapInfo::get_borders() const {
    return map_borders_;
}

std::optional<std::reference_wrapper<const Seat>> MapInfo::findseat(
    Point<int16_t> position) const {
    for (const auto &seat : seats_) {
        if (seat.inrange(position)) {
            return seat;
        }
    }

    return {};
}

std::optional<std::reference_wrapper<const Ladder>> MapInfo::findladder(
    Point<int16_t> position,
    bool upwards) const {
    for (const auto &ladder : ladders_) {
        if (ladder.inrange(position, upwards)) {
            return ladder;
        }
    }

    return {};
}

Seat::Seat(const nl::node &src) {
    pos_ = src;
}

bool Seat::inrange(Point<int16_t> position) const {
    auto hor = Range<int16_t>::symmetric(position.x(), 10);
    auto ver = Range<int16_t>::symmetric(position.y(), 10);

    return hor.contains(pos_.x()) && ver.contains(pos_.y());
}

Point<int16_t> Seat::getpos() const {
    return pos_;
}

Ladder::Ladder(const nl::node &src) {
    x_ = src["x"];
    y1_ = src["y1"];
    y2_ = src["y2"];
    ladder_ = src["l"].get_bool();
}

bool Ladder::is_ladder() const {
    return ladder_;
}

bool Ladder::inrange(Point<int16_t> position, bool upwards) const {
    auto hor = Range<int16_t>::symmetric(position.x(), 10);
    auto ver = Range<int16_t>(y1_, y2_);

    int16_t y = upwards ? position.y() - 5 : position.y() + 5;

    return hor.contains(x_) && ver.contains(y);
}

bool Ladder::felloff(int16_t y, bool downwards) const {
    int16_t dy = downwards ? y + 1 : y - 1;

    return dy > y2_ || y + 1 < y1_;
}

int16_t Ladder::get_x() const {
    return x_;
}
}  // namespace ms