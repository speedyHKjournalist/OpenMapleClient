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
#include "UIMobileInput.h"

namespace ms {
    UIMobileInput::UIMobileInput() : UIDragElement<PosMobileInput>() {
        // Controllers
        buttons_[Buttons::BT_JUMP] = std::make_unique<JumpButton>(Point<int16_t>(1050, 550));
        buttons_[Buttons::BT_JOYSTICK] = std::make_unique<JoyStickButton>(Point<int16_t>(250, 550));
        // UI Control
        buttons_[Buttons::BT_UICONTROL] = std::make_unique<ControlUIButton>(Point<int16_t>(600, 25),
                                                                            "UI CONF");
        buttons_[Buttons::BT_HIDEUI] = std::make_unique<ControlUIButton>(Point<int16_t>(600, 125),
                                                                         "TOGGLE UI");
        buttons_[Buttons::BT_DRAGUI] = std::make_unique<ControlUIButton>(Point<int16_t>(600, 225),
                                                                         "DRAG UI");
//        mobile_buttons_.emplace(std::make_unique<SkillUseButton>(Point<int16_t>(1050, 100), ACTION_TYPE::ATTACK);
//        mobile_buttons_.emplace(std::make_unique<SkillUseButton>(Point<int16_t>(1050, 250), ACTION_TYPE::NONE);
//        mobile_buttons_.emplace(std::make_unique<SkillUseButton>(Point<int16_t>(1050, 400), ACTION_TYPE::NONE);
//        mobile_buttons_.emplace(std::make_unique<SkillUseButton>(Point<int16_t>(1200, 100), ACTION_TYPE::NONE);
//        mobile_buttons_.emplace(std::make_unique<SkillUseButton>(Point<int16_t>(1200, 250), ACTION_TYPE::NONE);
//        mobile_buttons_.emplace(std::make_unique<SkillUseButton>(Point<int16_t>(1200, 400), ACTION_TYPE::NONE);

        buttons_[Buttons::BT_JUMP]->set_active(true);
        buttons_[Buttons::BT_JOYSTICK]->set_active(true);
        buttons_[Buttons::BT_UICONTROL]->set_active(true);
        buttons_[Buttons::BT_HIDEUI]->set_active(false);
        buttons_[Buttons::BT_DRAGUI]->set_active(false);

        show_ui = true;
        drag_ui = false;

        prev_cursor_pos = Point<int16_t>(0, 0);
        curr_cursor_pos = Point<int16_t>(0, 0);
    }

    void UIMobileInput::draw(float inter) const {
        for (const auto &iter: buttons_) {
            iter.second->draw(position_);
        }
    }

    void UIMobileInput::update() {
        if (drag_ui) {
            for (const auto &iter: buttons_) {
                if (iter.second->in_combobox(prev_cursor_pos) &&
                    iter.second->in_combobox(curr_cursor_pos) && iter.second->is_draggable()) {
                    Point<int16_t> pos = iter.second->get_position();
                    iter.second->set_position(pos + curr_cursor_pos - prev_cursor_pos);
                    prev_cursor_pos = curr_cursor_pos;
                    break;
                }
            }
        }

        for (const auto &iter: buttons_) {
            iter.second->update();
        }
    }

    Cursor::State UIMobileInput::send_cursor(bool clicked, Point<int16_t> cursorpos) {
        if (drag_ui) {
            if (curr_cursor_pos != cursorpos) {
                if (prev_cursor_pos.x() == 0 && prev_cursor_pos.y() == 0) {
                    prev_cursor_pos = curr_cursor_pos = cursorpos;
                } else {
                    prev_cursor_pos = curr_cursor_pos;
                    curr_cursor_pos = cursorpos;
                }
            }
        } else {
            prev_cursor_pos = curr_cursor_pos = Point<int16_t>(0, 0);
        }

        if (!drag_ui) {
            buttons_[Buttons::BT_JUMP]->send_cursor(clicked, cursorpos);
            buttons_[Buttons::BT_JOYSTICK]->send_cursor(clicked, cursorpos);
        }

        Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

        for (auto &btit: buttons_) {
            if (btit.second->is_active()
                && btit.second->bounds(position_).contains(cursorpos)) {
                if (btit.second->get_state() == Button::State::NORMAL) {
                    btit.second->set_state(Button::State::MOUSEOVER);
                    ret = Cursor::State::CAN_CLICK;
                } else if (btit.second->get_state() == Button::State::MOUSEOVER) {
                    if (clicked) {
                        btit.second->set_state(button_pressed(btit.first));

                        ret = Cursor::State::IDLE;
                    } else {
                        ret = Cursor::State::CAN_CLICK;
                    }
                }
            } else if (btit.second->get_state() == Button::State::MOUSEOVER) {
                btit.second->set_state(Button::State::NORMAL);
            }
        }

        return ret;
    }

    Button::State UIMobileInput::button_pressed(uint16_t id) {
        switch (id) {
            case Buttons::BT_UICONTROL: {
                bool UI_control_active_ = buttons_[Buttons::BT_HIDEUI]->is_active();
                toggle_UIControl(!UI_control_active_);
                break;
            }
            case Buttons::BT_HIDEUI: {
                toggle_ui();
                break;
            }
            case Buttons::BT_DRAGUI: {
                drag_ui = !drag_ui;
                buttons_[Buttons::BT_HIDEUI]->set_active(false);
                buttons_[Buttons::BT_DRAGUI]->set_active(false);
                break;
            }
        }
        return Button::State::NORMAL;
    }

    void UIMobileInput::toggle_UIControl(bool state) {
        buttons_[Buttons::BT_UICONTROL]->set_active(true);
        buttons_[Buttons::BT_HIDEUI]->set_active(state);
        buttons_[Buttons::BT_DRAGUI]->set_active(state);

        if (buttons_[Buttons::BT_UICONTROL]->is_active()) {
            buttons_[Buttons::BT_UICONTROL]->set_state(
                    Button::State::MOUSEOVER);

            Sound(Sound::Name::DLG_NOTICE).play();
        }
    }

    void UIMobileInput::toggle_ui() {
        if (show_ui) {
            buttons_[Buttons::BT_JUMP]->set_active(false);
            buttons_[Buttons::BT_JOYSTICK]->set_active(false);
            buttons_[Buttons::BT_HIDEUI]->set_active(false);
            buttons_[Buttons::BT_DRAGUI]->set_active(false);
        } else {
            buttons_[Buttons::BT_JUMP]->set_active(true);
            buttons_[Buttons::BT_JOYSTICK]->set_active(true);
            buttons_[Buttons::BT_HIDEUI]->set_active(false);
            buttons_[Buttons::BT_DRAGUI]->set_active(false);
        }
        show_ui = !show_ui;
    }

    bool UIMobileInput::is_menu_active() {
        return buttons_[Buttons::BT_HIDEUI]->is_active();
    }

    void UIMobileInput::remove_menus() {
        if (buttons_[Buttons::BT_HIDEUI]->is_active()) {
            toggle_UIControl(false);
        }
    }

    UIElement::Type UIMobileInput::get_type() const {
        return TYPE;
    }

    bool UIMobileInput::is_in_range(Point<int16_t> cursorpos) const {
        bool in_range = false;
        for (const auto &iter: buttons_) {
            in_range = in_range || iter.second->in_combobox(position_ + cursorpos);
        }
        return in_range;
    }
}  // namespace ms