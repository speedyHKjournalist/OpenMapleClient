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
#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>

#include "StringHandling.h"
#include "TouchButton.h"
#include "UI.h"
#include "Stage.h"

namespace ms {
    TouchButton::TouchButton(Point<int16_t> position, ActionType action_type,
                             const std::string &text) :
            position_(position),
            background_(100, 100, Color::Name::BLACK, 0.535f),
            bind_key_(GLFMKeyCodeUnknown),
            action_type_(action_type),
            cursor_in_range_(false),
            text_(Text::Font::A18M, Text::Alignment::CENTER, Color::Name::YELLOW) {
        text_.change_text(text);
    }

    TouchButton::TouchButton(Point<int16_t> position, ActionType action_type, GLFMKeyCode bind_key,
                             const std::string &text) :
            position_(position),
            background_(100, 100, Color::Name::BLACK, 0.535f),
            bind_key_(bind_key),
            action_type_(action_type),
            text_(Text::Font::A18M, Text::Alignment::CENTER, Color::Name::YELLOW) {
        text_.change_text(text);
    }

    void TouchButton::draw() const {
        background_.draw(position_);
        text_.draw(position_ + Point<int16_t>(50, 50));
    }

    void TouchButton::update() {
        if (cursor_in_range_) {
            GLFMTouchPhase current_phase = UI::get().get_touch_phase();
            if (current_phase == GLFMTouchPhaseBegan || current_phase == GLFMTouchPhaseMoved) {
                if (action_type_ == ActionType::Jump) {
                    Stage::get().get_player().send_action(KeyAction::Id::JUMP, true);
                } else if (action_type_ == ActionType::Potion) {

                } else if (action_type_ == ActionType::Skill) {

                } else {
                    UI::get().send_key(bind_key_, true);
                }
            }
            cursor_in_range_ = false;
        }
    }

    bool TouchButton::set_state(Point<double_t> cursor_pos) {
        if (cursor_pos.x() > position_.x() && cursor_pos.x() < (position_.x() + 100) &&
            cursor_pos.y() > position_.y() && cursor_pos.y() < (position_.y() + 100)) {
            cursor_in_range_ = true;
            return true;
        }
        cursor_in_range_ = false;
        return false;
    }
}  // namespace ms