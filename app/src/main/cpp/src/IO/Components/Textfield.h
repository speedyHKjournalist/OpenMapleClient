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
#include <vector>

#include "../../Graphics/Text.h"
#include "../Cursor.h"
#include "../Keyboard.h"

namespace ms {
class Textfield {
public:
    enum State { NORMAL, DISABLED, FOCUSED };

    Textfield(Text::Font font,
              Text::Alignment alignment,
              Color::Name color,
              Rectangle<int16_t> bounds,
              size_t limit);

    Textfield();

    void draw(Point<int16_t> position) const;

    void update(Point<int16_t> parentpos);

    void send_key(KeyType::Id type, int32_t code, bool down);

    void add_string(const std::string &str);

    void add_string(const std::vector<uint32_t> &str);

    void set_state(State state);

    void change_text(const std::string &text);

    void set_cryptchar(int8_t character);

    void set_enter_callback(std::function<void(std::string)> onreturn);

    void set_key_callback(KeyAction::Id key, std::function<void(void)> action);

    Cursor::State send_cursor(Point<int16_t> cursorpos, bool clicked);

    bool empty() const;

    State get_state() const;

    Rectangle<int16_t> get_bounds() const;

    const std::string &get_text() const;

private:
    void modify_text(const std::vector<uint32_t> &str);

    bool below_limit() const;

    Text text_label_;
    std::string text_;
    std::vector<uint32_t> text_as_values_;
    Text marker_;
    bool show_marker_;
    uint16_t elapsed_;
    size_t marker_pos_;
    Rectangle<int16_t> bounds_;
    Point<int16_t> parent_pos_;
    size_t limit_;
    int8_t crypt_;
    State state_;

    std::map<int32_t, std::function<void(void)>> callbacks_;
    std::function<void(std::string)> onreturn_;
};
}  // namespace ms