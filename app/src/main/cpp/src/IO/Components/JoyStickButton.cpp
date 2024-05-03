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

#include "JoyStickButton.h"
#include "Singleton.h"
#include "KeyAction.h"
#include "Stage.h"
#include "UI.h"

namespace ms {
    JoyStickButton::JoyStickButton(Point<int16_t> position) :
            background_(position.x(), position.y(), 130, Color::Name::BLACK, 0.535f),
            radius_(130) {
        position_ = position;
        angle_ = 0;

        active_ = true;
        pressed_ = false;
        draggable_ = true;
        state_ = Button::State::NORMAL;
    }

    void JoyStickButton::draw(Point<int16_t> parentpos) const {
        if (active_) {
            background_.draw(position_);
        }
    }

    void JoyStickButton::update() {}

    Cursor::State JoyStickButton::send_cursor(bool clicked, Point<int16_t> cursorpos) {
        if (in_combobox(cursorpos) && active_) {
            int16_t distance_x = cursorpos.x() - position_.x();
            int16_t distance_y = cursorpos.y() - position_.y();

            calc_angle(distance_x, distance_y);

            if (clicked) {
                if (angle_ < 135.0 && angle_ >= 45.0) {
                    UI::get().send_key(GLFMKeyCodeArrowDown, true);
                } else if (angle_ < 225.0 && angle_ >= 135.0) {
                    UI::get().send_key(GLFMKeyCodeArrowLeft, true);
                } else if (angle_ < 325.0 && angle_ >= 225.0) {
                    UI::get().send_key(GLFMKeyCodeArrowUp, true);
                } else {
                    UI::get().send_key(GLFMKeyCodeArrowRight, true);
                }
            } else {
                UI::get().send_key(GLFMKeyCodeArrowDown, false);
                UI::get().send_key(GLFMKeyCodeArrowLeft, false);
                UI::get().send_key(GLFMKeyCodeArrowUp, false);
                UI::get().send_key(GLFMKeyCodeArrowRight, false);
            }
        }
        return Cursor::State::IDLE;
    }

    Rectangle<int16_t> JoyStickButton::bounds(Point<int16_t> parentpos) const {
        return {};
    }

    int16_t JoyStickButton::width() const {
        return radius_;
    }

    Point<int16_t> JoyStickButton::origin() const {
        return position_;
    }

    bool JoyStickButton::in_combobox(Point<int16_t> cursorpos) {
        if (active_) {
            int16_t distance_x = cursorpos.x() - position_.x();
            int16_t distance_y = cursorpos.y() - position_.y();
            if (pow(distance_x, 2) + pow(distance_y, 2) <
                pow(radius_, 2)) {
                return true;
            }
        }
        return false;
    }

    void JoyStickButton::calc_angle(int16_t distance_x, int16_t distance_y) {
        angle_ = atan2(distance_y, distance_x) * 180.0 / M_PI;
        if (angle_ < 0)
            angle_ += 360.0;
    }
}  // namespace ms