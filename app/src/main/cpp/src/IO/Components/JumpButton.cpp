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

#include "StringHandling.h"
#include "JumpButton.h"
#include "UI.h"
#include "Stage.h"

namespace ms {
    JumpButton::JumpButton(Point<int16_t> position) :
            dimension_(100, 100),
            background_(dimension_.x(), dimension_.y(), Color::Name::BLACK, 0.535f),
            text_(Text::Font::A18M, Text::Alignment::CENTER, Color::Name::YELLOW) {
        position_ = position;
        text_.change_text("JUMP");
        jump_ = false;

        active_ = true;
        pressed_ = false;
        draggable_ = true;
        state_ = Button::State::NORMAL;
    }

    void JumpButton::draw(Point<int16_t> parentpos) const {
        if (active_) {
            background_.draw(position_);
            text_.draw(position_ + Point<int16_t>(50, 50));
        }
    }

    void JumpButton::update() {
        if (jump_) {
            Stage::get().get_player().send_action(KeyAction::Id::JUMP, true);
        } else {
            Stage::get().get_player().send_action(KeyAction::Id::JUMP, false);
        }
    }

    Cursor::State JumpButton::send_cursor(bool clicked, Point<int16_t> cursorpos) {
        if (in_combobox(cursorpos) && active_) {
            if (clicked) {
                jump_ = true;
            } else {
                jump_ = false;
            }
        }
        return Cursor::State::IDLE;
    }

    Rectangle<int16_t> JumpButton::bounds(Point<int16_t> parentpos) const {
        Point<int16_t> absp = position_ + parentpos;

        return {absp, absp + dimension_};
    }

    int16_t JumpButton::width() const {
        return dimension_.x();
    }

    Point<int16_t> JumpButton::origin() const {
        return position_;
    }

    bool JumpButton::in_combobox(Point<int16_t> cursorpos) {
        if (active_) {
            if (bounds(Point<int16_t>()).contains(cursorpos)) {
                return true;
            }
        }
        return false;
    }
}  // namespace ms