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

#include <array>

#include "../Components/Slider.h"
#include "../Components/Textfield.h"
#include "../Messages.h"
#include "../UIDragElement.h"

namespace ms {
class UIChatBar : public UIDragElement<PosCHAT> {
public:
    static constexpr Type TYPE = UIElement::Type::CHAT_BAR;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    enum LineType { UNK0, WHITE, RED, BLUE, YELLOW, GREEN };

    UIChatBar();

    void draw(float inter) const override;

    void update() override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    bool is_in_range(Point<int16_t> cursorpos) const override;

    Cursor::State send_cursor(bool clicking, Point<int16_t> cursorpos) override;

    UIElement::Type get_type() const override;

    Cursor::State check_drag_top(bool clicking, Point<int16_t> cursorpos);

    void send_chatline(const std::string &line, LineType type);

    void display_message(Messages::Type line, UIChatBar::LineType type);

    void toggle_chat();

    void toggle_chat(bool chat_open);

    void toggle_chatfield();

    void toggle_chatfield(bool chatfield_open);

    bool is_chat_open();

    bool is_chatfield_open();

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    bool is_in_drag_range(Point<int16_t> cursorpos) const override;

    int16_t get_chat_top(bool chat_open) const;

    int16_t get_chatbar_height() const;

    Rectangle<int16_t> get_bounds(Point<int16_t> additional_area) const;

    static constexpr int16_t CHAT_ROW_HEIGHT_ = 13;
    static constexpr int16_t MIN_CHAT_ROWS_ = 1;
    static constexpr int16_t MAX_CHAT_ROWS_ = 16;
    static constexpr int16_t DIMENSION_Y_ = 17;
    static constexpr time_t MESSAGE_COOLDOWN_ = 1'000;

    enum Buttons : uint16_t {
        BT_OPEN_CHAT,
        BT_CLOSE_CHAT,
        BT_CHAT,
        BT_HELP,
        BT_LINK,
        BT_TAB_0,
        BT_TAB_1,
        BT_TAB_2,
        BT_TAB_3,
        BT_TAB_4,
        BT_TAB_5,
        BT_TAB_ADD,
        BT_CHAT_TARGET
    };

    enum ChatTab {
        CHT_ALL,
        CHT_BATTLE,
        CHT_PARTY,
        CHT_FRIEND,
        CHT_GUILD,
        CHT_ALLIANCE,
        NUM_CHATTAB
    };

    std::vector<std::string> chat_tab_text_ = {
        "All", "Battle", "Party", "Friend", "Guild", "Alliance"
    };

    bool is_chat_open_;
    bool should_chat_open_persist_;
    bool is_chatfield_open_;
    std::array<Texture, 4> chatspace_;
    Texture chatenter_;
    Texture chatcover_;
    Textfield chatfield_;
    Point<int16_t> closechat_;

    std::array<Text, UIChatBar::ChatTab::NUM_CHATTAB> chattab_text_;
    int16_t chattab_x_;
    int16_t chattab_y_;
    int16_t chattab_span_;

    Slider slider_;

    EnumMap<Messages::Type, time_t> message_cooldowns_;
    std::vector<std::string> last_entered_;
    size_t last_pos_;

    int16_t chat_rows_;
    int16_t row_pos_;
    int16_t row_max_;
    std::unordered_map<int16_t, Text> row_texts_;

    bool should_drag_chat_top_;
};
}  // namespace ms