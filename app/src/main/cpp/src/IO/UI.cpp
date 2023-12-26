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
#include "UI.h"

#include <optional>
#include <utility>

#include "../Gameplay/Stage.h"
#include "../../Audio/Audio.h"
#include "../../Configuration.h"
#include "OptionalCreator.h"
#include "UIStateCashShop.h"
#include "UIStateGame.h"
#include "UIStateLogin.h"
#include "UITypes/UIChannel.h"
#include "UITypes/UIChat.h"
#include "UITypes/UIChatBar.h"
#include "UITypes/UIJoypad.h"
#include "UITypes/UINpcTalk.h"
#include "UITypes/UIOptionMenu.h"
#include "UITypes/UIQuit.h"
#include "UITypes/UIShop.h"
#include "UITypes/UIStatusBar.h"
#include "UITypes/UIWorldMap.h"
#include "UIWorldSelect.h"
#include "Window.h"

namespace ms {
UI::UI() :
    state_(std::make_unique<UIStateNull>()),
    enabled_(true),
    quitted_(false),
    caps_lock_enabled_(false) {}

void UI::init() {
    cursor_.init();
    change_state(State::LOGIN);
}

void UI::draw(float alpha) const {
    state_->draw(alpha, cursor_.get_position());
    scrolling_notice_.draw(alpha);
    cursor_.draw(alpha);
}

void UI::update() {
    state_->update();
    scrolling_notice_.update();
    cursor_.update();
}

void UI::enable() {
    enabled_ = true;
}

void UI::disable() {
    enabled_ = false;
}

void UI::change_state(State id) {
    switch (id) {
        case State::LOGIN: state_ = std::make_unique<UIStateLogin>(); break;
        case State::GAME:
            state_ = std::make_unique<UIStateGame>(
                UI::get().get_element<UIWorldSelect>()
                    ? UI::get()
                          .get_element<UIWorldSelect>()
                          ->get()
                          .get_channel_count(
                              Stage::get().get_player().get_world_id())
                    : 20);
            break;
        case State::CASHSHOP:
            state_ = std::make_unique<UIStateCashShop>();
            break;
    }
}

void UI::quit() {
    quitted_ = true;
}

bool UI::not_quitted() const {
    return !quitted_;
}

void UI::send_cursor(Point<int16_t> cursorpos, Cursor::State cursorstate) {
    Cursor::State nextstate = state_->send_cursor(cursorstate, cursorpos);
    cursor_.set_state(nextstate);
    cursor_.set_position(cursorpos);
}

void UI::send_focus(int focused) {
    if (focused) {
        // The window gained input focus
        uint8_t sfxvolume = Setting<SFXVolume>::get().load();
        Sound::set_sfxvolume(sfxvolume);

        uint8_t bgmvolume = Setting<BGMVolume>::get().load();
        Music::set_bgmvolume(bgmvolume);
    } else {
        // The window lost input focus
        Sound::set_sfxvolume(0);
        Music::set_bgmvolume(0);
    }
}

void UI::send_scroll(double yoffset) {
    state_->send_scroll(yoffset);
}

void UI::send_close() {
    state_->send_close();
}

void UI::send_cursor(bool pressed) {
    Cursor::State cursorstate =
        (pressed && enabled_) ? Cursor::State::CLICKING : Cursor::State::IDLE;
    Point<int16_t> cursorpos = cursor_.get_position();
    send_cursor(cursorpos, cursorstate);

    if (focused_text_field_ && pressed) {
        Cursor::State tstate =
            focused_text_field_->get().send_cursor(cursorpos, pressed);

        switch (tstate) {
            case Cursor::State::IDLE: focused_text_field_ = {}; break;
        }
    }
}

void UI::send_cursor(Point<int16_t> pos) {
    send_cursor(pos, cursor_.get_state());
}

void UI::rightclick() {
    Point<int16_t> pos = cursor_.get_position();
    state_->rightclick(pos);
}

void UI::doubleclick() {
    Point<int16_t> pos = cursor_.get_position();
    state_->doubleclick(pos);
}

void UI::send_key(int32_t keycode, bool pressed) {
    if ((is_key_down_[AKEYCODE_ALT_LEFT] || is_key_down_[AKEYCODE_ALT_RIGHT])
        && (is_key_down_[AKEYCODE_ENTER] || is_key_down_[AKEYCODE_NUMPAD_ENTER])) {
        Window::get().toggle_fullscreen();

        is_key_down_[AKEYCODE_ALT_LEFT] = false;
        is_key_down_[AKEYCODE_ALT_RIGHT] = false;
        is_key_down_[AKEYCODE_ENTER] = false;
        is_key_down_[AKEYCODE_NUMPAD_ENTER] = false;

        return;
    }

    if (is_key_down_[keyboard_.capslock_code()]) {
        caps_lock_enabled_ = !caps_lock_enabled_;
    }

    if (focused_text_field_) {
        bool ctrl = is_key_down_[keyboard_.left_ctrl_code()]
                    || is_key_down_[keyboard_.right_ctrl_code()];

        if (ctrl) {
            if (!pressed) {
                KeyAction::Id action = keyboard_.get_ctrl_action(keycode);

                switch (action) {
                    case KeyAction::Id::COPY:
                        Window::get().set_clipboard(
                            focused_text_field_->get().get_text());
                        break;
                    case KeyAction::Id::PASTE:
                        focused_text_field_->get().add_string(
                            Window::get().get_clipboard());
                        break;
                }
            }
        } else {
            bool shift = is_key_down_[keyboard_.left_shift_code()]
                         || is_key_down_[keyboard_.right_shift_code()]
                         || caps_lock_enabled_;
            Keyboard::Mapping mapping =
                keyboard_.get_text_mapping(keycode, shift);
            if (mapping.type != KeyType::Id::TEXT) {
                focused_text_field_->get().send_key(mapping.type,
                                                    mapping.action,
                                                    pressed);
            }
        }
    } else {
        Keyboard::Mapping mapping = keyboard_.get_mapping(keycode);

        bool sent = false;
        std::list<UIElement::Type> types;

        bool escape = keycode == AKEYCODE_ESCAPE;
        bool tab = keycode == AKEYCODE_TAB;
        bool enter = keycode == AKEYCODE_ENTER || keycode == AKEYCODE_NUMPAD_ENTER;
        bool up_down = keycode == AKEYCODE_DPAD_UP || keycode == AKEYCODE_DPAD_DOWN;
        bool left_right = keycode == AKEYCODE_DPAD_LEFT || keycode == AKEYCODE_DPAD_RIGHT;
        bool arrows = up_down || left_right;

        auto statusbar = UI::get().get_element<UIStatusBar>();
        auto channel = UI::get().get_element<UIChannel>();
        auto worldmap = UI::get().get_element<UIWorldMap>();
        auto optionmenu = UI::get().get_element<UIOptionMenu>();
        auto shop = UI::get().get_element<UIShop>();
        auto joypad = UI::get().get_element<UIJoypad>();
        auto rank = UI::get().get_element<UIRank>();
        auto quit = UI::get().get_element<UIQuit>();
        auto npctalk = UI::get().get_element<UINpcTalk>();
        // auto report = UI::get().get_element<UIReport>();
        // auto whisper = UI::get().get_element<UIWhisper>();

        if (npctalk && npctalk->get().is_active()) {
            npctalk->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (statusbar && statusbar->get().is_menu_active()) {
            statusbar->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (channel && channel->get().is_active()
                   && mapping.action != KeyAction::Id::CHANGE_CHANNEL) {
            channel->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (worldmap && worldmap->get().is_active()
                   && mapping.action != KeyAction::Id::WORLD_MAP) {
            worldmap->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (optionmenu && optionmenu->get().is_active()) {
            optionmenu->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (shop && shop->get().is_active()) {
            shop->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (joypad && joypad->get().is_active()) {
            joypad->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (rank && rank->get().is_active()) {
            rank->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else if (quit && quit->get().is_active()) {
            quit->get().send_key(mapping.action, pressed, escape);
            sent = true;
        } else {
            // All
            if (escape || tab || enter || arrows) {
                // Login
                types.emplace_back(UIElement::Type::WORLD_SELECT);
                types.emplace_back(UIElement::Type::CHAR_SELECT);
                types.emplace_back(UIElement::Type::RACE_SELECT);  // No tab
                types.emplace_back(
                    UIElement::Type::CLASS_CREATION);  // No tab (No arrows,
                                                       // but
                                                       // shouldn't send else
                                                       // where)
                types.emplace_back(
                    UIElement::Type::LOGIN_NOTICE);  // No tab (No arrows,
                                                     // but
                                                     // shouldn't send else
                                                     // where)
                types.emplace_back(
                    UIElement::Type::LOGIN_NOTICE_CONFIRM);  // No tab (No
                                                             // arrows, but
                                                             // shouldn't
                                                             //  send
                                                             // else where)
                types.emplace_back(
                    UIElement::Type::LOGIN_WAIT);  // No tab (No arrows, but
                                                   // shouldn't send else
                                                   //    where)
            }

            if (escape) {
                // Login
                types.emplace_back(UIElement::Type::SOFT_KEYBOARD);

                // Game
                types.emplace_back(UIElement::Type::NOTICE);
                types.emplace_back(UIElement::Type::KEY_CONFIG);
                types.emplace_back(UIElement::Type::CHAT);
                types.emplace_back(UIElement::Type::EVENT);
                types.emplace_back(UIElement::Type::STATS_INFO);
                types.emplace_back(UIElement::Type::ITEM_INVENTORY);
                types.emplace_back(UIElement::Type::EQUIP_INVENTORY);
                types.emplace_back(UIElement::Type::SKILL_BOOK);
                types.emplace_back(UIElement::Type::QUEST_LOG);
                types.emplace_back(UIElement::Type::USER_LIST);
                types.emplace_back(UIElement::Type::NPC_TALK);
                types.emplace_back(UIElement::Type::CHAR_INFO);
            } else if (enter) {
                // Login
                types.emplace_back(UIElement::Type::SOFT_KEYBOARD);

                // Game
                types.emplace_back(UIElement::Type::NOTICE);
            } else if (tab) {
                // Game
                types.emplace_back(UIElement::Type::ITEM_INVENTORY);
                types.emplace_back(UIElement::Type::EQUIP_INVENTORY);
                types.emplace_back(UIElement::Type::SKILL_BOOK);
                types.emplace_back(UIElement::Type::QUEST_LOG);
                types.emplace_back(UIElement::Type::USER_LIST);
            }

            if (!types.empty()) {
                auto *element = state_->get_front(types);

                if (element && element != nullptr) {
                    element->send_key(mapping.action, pressed, escape);
                    sent = true;
                }
            }
            // TODO: (rich) fix
        }

        if (!sent) {
            auto chatbar = UI::get().get_element<UIChatBar>();

            if (escape) {
                if (chatbar && chatbar->get().is_chat_open()) {
                    chatbar->get().send_key(mapping.action, pressed, escape);
                } else {
                    state_->send_key(mapping.type,
                                     mapping.action,
                                     pressed,
                                     escape);
                }
            } else if (enter) {
                if (chatbar) {
                    chatbar->get().send_key(mapping.action, pressed, escape);
                } else {
                    state_->send_key(mapping.type,
                                     mapping.action,
                                     pressed,
                                     escape);
                }
            } else {
                state_->send_key(mapping.type, mapping.action, pressed, escape);
            }
        }
    }

    is_key_down_[keycode] = pressed;
}

void UI::send_key(uint32_t unicode) {
    if (focused_text_field_) {
        focused_text_field_->get().send_key(KeyType::Id::TEXT, unicode, true);
    }

    is_key_down_[unicode] = true;  // maybe remove
}

void UI::set_scrollnotice(const std::string &notice) {
    scrolling_notice_.set_notice(notice);
}

void UI::focus_textfield(Textfield *tofocus) {
    if (focused_text_field_) {
        focused_text_field_->get().set_state(Textfield::State::NORMAL);
    }

    if (tofocus) {
        focused_text_field_ = *tofocus;
    } else {
        focused_text_field_ = {};
    }
}

void UI::remove_textfield() {
    if (focused_text_field_) {
        focused_text_field_->get().set_state(Textfield::State::NORMAL);
    }

    focused_text_field_ = {};
}

void UI::drag_icon(Icon *icon) {
    state_->drag_icon(icon);
}

void UI::add_keymapping(uint8_t no, uint8_t type, int32_t action) {
    keyboard_.assign(no, type, action);
}

void UI::clear_tooltip(Tooltip::Parent parent) {
    state_->clear_tooltip(parent);
}

void UI::show_equip(Tooltip::Parent parent, int16_t slot) {
    state_->show_equip(parent, slot);
}

void UI::show_item(Tooltip::Parent parent, int32_t item_id) {
    state_->show_item(parent, item_id);
}

void UI::show_skill(Tooltip::Parent parent,
                    int32_t skill_id,
                    int32_t level,
                    int32_t masterlevel,
                    int64_t expiration) {
    state_->show_skill(parent, skill_id, level, masterlevel, expiration);
}

void UI::show_text(Tooltip::Parent parent, std::string text) {
    state_->show_text(parent, std::move(text));
}

void UI::show_map(Tooltip::Parent parent,
                  std::string name,
                  std::string description,
                  int32_t mapid,
                  bool bolded) {
    state_->show_map(parent,
                     std::move(name),
                     std::move(description),
                     mapid,
                     bolded);
}

Keyboard &UI::get_keyboard() {
    return keyboard_;
}

void UI::remove(UIElement::Type type) {
    focused_text_field_ = {};
    state_->remove(type);
}
}  // namespace ms