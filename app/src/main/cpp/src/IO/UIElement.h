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

#include <functional>
#include <map>

#include "../Graphics/Sprite.h"
#include "Components/Button.h"
#include "Components/Icon.h"

namespace ms {
// Base class for all types of user interfaces on screen.
class UIElement {
public:
    using UPtr = std::unique_ptr<UIElement>;

    enum Type {
        NONE,
        START,
        LOGIN,
        TOS,
        GENDER,
        WORLD_SELECT,
        REGION,
        CHAR_SELECT,
        LOGIN_WAIT,
        RACE_SELECT,
        CLASS_CREATION,
        SOFT_KEYBOARD,
        LOGIN_NOTICE,
        LOGIN_NOTICE_CONFIRM,
        STATUS_MESSENGER,
        STATUS_BAR,
        CHAT_BAR,
        BUFF_LIST,
        NOTICE,
        NPC_TALK,
        SHOP,
        STATS_INFO,
        ITEM_INVENTORY,
        EQUIP_INVENTORY,
        SKILL_BOOK,
        QUEST_LOG,
        WORLD_MAP,
        USER_LIST,
        MINI_MAP,
        CHANNEL,
        CHAT,
        CHAT_RANK,
        JOYPAD,
        EVENT,
        KEY_CONFIG,
        OPTION_MENU,
        QUIT,
        CHAR_INFO,
        CASHSHOP,
        GAUGE_BOSS,
        NOTIFICATION,
        TRADE,
        NUM_TYPES
    };

    virtual ~UIElement() = default;

    virtual void draw(float inter) const;

    virtual void update();

    virtual void update_screen(int16_t new_width, int16_t new_height) {}

    void makeactive();

    void deactivate();

    bool is_active() const;

    virtual void toggle_active();

    virtual Button::State button_pressed(uint16_t buttonid) {
        return Button::State::DISABLED;
    }

    virtual bool send_icon(const Icon &icon, Point<int16_t> cursorpos) {
        return true;
    }

    virtual void doubleclick(Point<int16_t> cursorpos) {}

    virtual void rightclick(Point<int16_t> cursorpos) {}

    virtual bool is_in_range(Point<int16_t> cursorpos) const;

    virtual void remove_cursor();

    virtual Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos);

    virtual void send_scroll(double yoffset) {}

    virtual void send_key(int32_t keycode, bool pressed, bool escape) {}

    virtual UIElement::Type get_type() const = 0;

protected:
    UIElement(Point<int16_t> position, Point<int16_t> dimension, bool active);

    UIElement(Point<int16_t> position, Point<int16_t> dimension);

    UIElement();

    void draw_sprites(float alpha) const;

    void draw_buttons(float alpha) const;

    std::map<uint16_t, std::unique_ptr<Button>> buttons_;
    std::vector<Sprite> sprites_;
    Point<int16_t> position_;
    Point<int16_t> dimension_;
    bool active_;
};
}  // namespace ms