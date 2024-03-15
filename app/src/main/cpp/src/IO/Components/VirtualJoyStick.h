//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2024  Daniel Allendorf, Ryan Payton, Bizhou Xing
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
#include "IconCover.h"
#include "Button.h"
#include "TouchInfo.h"

namespace ms {
class VirtualJoyStick {
public:
    VirtualJoyStick(Point<int16_t> position, int16_t radius);

    void draw() const;

    void update();

    bool set_state(TouchInfo touchInfo);

    void bind_touch_id(int16_t touch_id);

    int16_t get_bind_touch_id();

private:
    Point<int16_t> position_;
    ColorCircle background_;
    int16_t radius_;
    int16_t bind_touch_id_;
    double angle_;
};
}  // namespace ms