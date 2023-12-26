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

#include "../Components/Slider.h"
#include "../Graphics/Text.h"
#include "../UIElement.h"

namespace ms {
class UINpcTalk : public UIElement {
public:
    enum TalkType : int8_t {
        NONE = -1,
        SEND_TEXT_ONLY,
        SEND_YESNO,
        SEND_SIMPLE = 4,
        SEND_ACCEPT_DECLINE = 12,
        // below are determined by 2 style bytes when msgtype = 0
        SEND_OK,
        SEND_NEXT,
        SEND_NEXT_PREV,
        SEND_PREV,
        SEND_GET_TEXT,
        SEND_GET_NUMBER,
        LENGTH
    };

    static constexpr Type TYPE = UIElement::Type::NPC_TALK;
    static constexpr bool FOCUSED = true;
    static constexpr bool TOGGLED = false;

    UINpcTalk();

    void draw(float inter) const override;

    void update() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void change_text(int32_t npcid,
                     int8_t msgtype,
                     uint8_t style_b0,
                     uint8_t style_b1,
                     int8_t speaker,
                     const std::string &text);

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    TalkType get_by_value(int8_t value, uint8_t style_b0, uint8_t style_b1);

    std::string format_text(const std::string &tx, const int32_t &npcid);

    static constexpr int16_t MAX_HEIGHT_ = 248;

    enum Buttons {
        ALL_LEVEL,
        CLOSE,
        MY_LEVEL,
        NEXT,
        NO,
        OK,
        PREV,
        QAFTER,
        QCNO,
        QCYES,
        QGIVEUP,
        QNO,
        QSTART,
        QYES,
        YES
    };

    Texture top_;
    Texture fill_;
    Texture bottom_;
    Texture nametag_;
    Texture speaker_;

    Text text_;
    Text name_;

    int16_t height_;
    int16_t offset_;
    int16_t unit_rows_;
    int16_t row_max_;
    int16_t min_height_;

    bool show_slider_;
    bool draw_text_;
    Slider slider_;
    TalkType type_;
    std::string formatted_text_;
    size_t formatted_text_pos_;
    uint16_t timestep_;

    std::function<void(bool)> onmoved_;
};
}  // namespace ms