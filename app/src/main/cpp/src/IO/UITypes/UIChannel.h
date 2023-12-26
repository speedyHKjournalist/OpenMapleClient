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

#include "../Template/BoolPair.h"
#include "../UIDragElement.h"

namespace ms {
class UIChannel : public UIDragElement<PosCHANNEL> {
public:
    static constexpr Type TYPE = UIElement::Type::CHANNEL;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UIChannel(uint8_t wid, uint8_t ch, uint8_t ch_count);

    void draw(float inter) const override;

    void update() override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    UIElement::Type get_type() const override;

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    void cancel();

    void change_channel();

    void update_selected_channel_position();

    enum Buttons : uint16_t { CANCEL, CHANGE, CH };

    struct OldState {
        uint8_t current_channel;
        int16_t current_channel_x;
        int16_t current_channel_y;
    };

    OldState old_state_;

    uint8_t current_channel_;
    uint8_t selected_channel_;
    uint8_t channel_count_;
    BoolPair<Texture> channel_;
    std::vector<Sprite> ch_;
    int16_t current_channel_x_;
    int16_t current_channel_y_;
    int16_t selected_channel_x_;
    int16_t selected_channel_y_;
};
}  // namespace ms