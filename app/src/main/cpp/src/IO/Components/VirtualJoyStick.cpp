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
#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>

#include "VirtualJoyStick.h"
#include "Singleton.h"
#include "KeyAction.h"
#include "Stage.h"
#include "UI.h"

namespace ms {
    VirtualJoyStick::VirtualJoyStick(Point<int16_t> position, int16_t radius) :
            position_(position),
            background_(position.x(), position.y(), radius, Color::Name::BLACK, 0.535f) {
        cursor_in_range_ = false;
        radius_ = radius;
        angle_ = 0;
    }

    void VirtualJoyStick::draw() const {
        background_.draw(position_);
    }

    void VirtualJoyStick::update() {
        GLFMTouchPhase current_phase = UI::get().get_touch_phase();
        if (cursor_in_range_) {
            if (current_phase == GLFMTouchPhaseBegan || current_phase == GLFMTouchPhaseMoved) {
                if (angle_ < 135.0 && angle_ >= 45.0) {
                    UI::get().send_key(GLFMKeyCodeArrowDown, true);
                } else if (angle_ < 225.0 && angle_ >= 135.0) {
                    UI::get().send_key(GLFMKeyCodeArrowLeft, true);
                } else if (angle_ < 325.0 && angle_ >= 225.0) {
                    UI::get().send_key(GLFMKeyCodeArrowUp, true);
                } else {
                    UI::get().send_key(GLFMKeyCodeArrowRight, true);
                }
            }
            cursor_in_range_ = false;
        }
        if (current_phase == GLFMTouchPhaseEnded) {
            UI::get().send_key(GLFMKeyCodeArrowUp, false);
            UI::get().send_key(GLFMKeyCodeArrowDown, false);
            UI::get().send_key(GLFMKeyCodeArrowLeft, false);
            UI::get().send_key(GLFMKeyCodeArrowRight, false);
        }
    }

    bool VirtualJoyStick::set_state(Point<double_t> cursor_pos) {
        int16_t distance_x = cursor_pos.x() - position_.x();
        int16_t distance_y = cursor_pos.y() - position_.y();
        if (pow(distance_x, 2) + pow(distance_y, 2) <
            pow(radius_, 2)) {
            cursor_in_range_ = true;
            angle_ = atan2(distance_y, distance_x) * 180.0 / M_PI;
            if (angle_ < 0)
                angle_ += 360.0;
            return true;
        }
        cursor_in_range_ = false;
        return false;
    }
}  // namespace ms