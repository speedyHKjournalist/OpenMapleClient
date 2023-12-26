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

#include <map>
#include <vector>

#include "../Gameplay/Physics/Physics.h"
#include "../Graphics/Animation.h"
#include "MapObject.h"

namespace ms {
class Reactor : public MapObject {
public:
    Reactor(int32_t oid, int32_t rid, int8_t state, Point<int16_t> position);

    void draw(double viewx, double viewy, float alpha) const override;

    int8_t update(const Physics &physics) override;

    void set_state(int8_t state);

    void destroy(int8_t state, Point<int16_t> position);

    bool is_hittable() const;

    // Check if this mob collides with the specified rectangle.
    bool is_in_range(const Rectangle<int16_t> &range) const;

private:
//    int32_t oid_;
    int32_t rid_;
    int8_t state_;
    // int8_t stance; // TODO: ??
    // TODO: These are in the GMS client
    // bool movable; // TODO: Snowball?
    // int32_t questid;
    // bool activates_by_touch;

    nl::node src_;
    std::map<int8_t, Animation> animations_;
    bool animation_ended_;

    bool hit_table_;
    bool dead_;

    Animation normal_;
};
}  // namespace ms