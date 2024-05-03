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

#include <unordered_map>
#include "JumpButton.h"
#include "JoyStickButton.h"
#include "ControlUIButton.h"
#include "Audio.h"
#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>


namespace ms {
    class UIMobileInput : public UIDragElement<PosMobileInput> {
    public:
        enum MenuType { UICONTROL };
        static constexpr Type TYPE = UIElement::Type::MOBILE_INPUT;
        static constexpr bool FOCUSED = false;
        static constexpr bool TOGGLED = false;

        enum Buttons {
            BT_JOYSTICK,
            BT_JUMP,
            BT_UICONTROL,
            BT_HIDEUI,
            BT_DRAGUI,
            BT_SKILL_USE
        };

        UIMobileInput();

        void draw(float inter) const override;

        void update() override;

        Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

        Button::State button_pressed(uint16_t id) override;

        void toggle_UIControl(bool state);

        void toggle_ui();

        bool is_menu_active();

        void remove_menus();

        UIElement::Type get_type() const override;

        bool is_in_range(Point<int16_t> cursorpos) const override;

    private:
        bool show_ui;
        bool drag_ui;
        Point<int16_t> prev_cursor_pos;
        Point<int16_t> curr_cursor_pos;
    };
}  // namespace ms