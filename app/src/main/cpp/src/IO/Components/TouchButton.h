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
#include "glfm.h"
#include "TouchInfo.h"

namespace ms {
class TouchButton {
public:
    enum ActionType : uint16_t {
        Jump,
        Skill,
        Potion,
        Default
    };

    TouchButton(Point<int16_t> position, ActionType action_type, const std::string &text);

    TouchButton(Point<int16_t> position, ActionType action_type, GLFMKeyCode bind_key, const std::string &text);

    void draw() const;

    void update();

    void bind_touch_id(int16_t touch_id);

    bool set_state(TouchInfo touchInfo);

    int16_t get_bind_touch_id();

private:
    Point<int16_t> position_;
    ColorBox background_;
    GLFMKeyCode bind_key_;
    ActionType action_type_;
    Text text_;
    int16_t bind_touch_id_;
};
}  // namespace ms