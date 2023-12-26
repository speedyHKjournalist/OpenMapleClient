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
#include "UIJoypad.h"

#include <nlnx/nx.hpp>

#include "../Components/MapleButton.h"

namespace ms {
// TODO: Add combo boxes nl::nx::ui["Basic.img"]["ComboBox"] / ["ComboBox5"];
UIJoypad::UIJoypad() : UIDragElement<PosJOYPAD>() {
    alternative_settings_ = false;  // TODO: Get user's key settings type

    nl::node JoyPad = nl::nx::ui["UIWindow.img"]["JoyPad"];
    nl::node Basic = nl::nx::ui["Basic.img"];

    backgrnd_[true] = JoyPad["backgrnd_alternative"];
    backgrnd_[false] = JoyPad["backgrnd_classic"];

    buttons_[Buttons::DEFAULT] =
        std::make_unique<MapleButton>(JoyPad["BtDefault"]);
    buttons_[Buttons::CANCEL] =
        std::make_unique<MapleButton>(Basic["BtCancel4"],
                                      Point<int16_t>(124, 303));
    buttons_[Buttons::OK] =
        std::make_unique<MapleButton>(Basic["BtOK4"], Point<int16_t>(82, 303));

    for (auto &text : key_text_) {
        text = Text(Text::Font::A12M,
                    Text::Alignment::LEFT,
                    Color::Name::BLACK,
                    "None");
    }

    dimension_ = backgrnd_[true].get_dimensions();
}

void UIJoypad::draw(float inter) const {
    backgrnd_[alternative_settings_].draw(position_);

    int16_t x = 79;
    int16_t y = 24;
    int16_t y_adj = 18;

    for (size_t i = 0; i < key_text_.size(); i++) {
        if (i == 0) {
            key_text_[i].draw(position_ + Point<int16_t>(x, y));
        } else if (i > 0 && i < 4) {
            key_text_[i].draw(
                position_ + Point<int16_t>(x - 16, y + 44 + y_adj * (i - 1)));
        } else {
            key_text_[i].draw(
                position_ + Point<int16_t>(x - 16, y + 123 + y_adj * (i - 4)));
        }
    }

    UIElement::draw(inter);
}

void UIJoypad::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            cancel();
        } else if (keycode == KeyAction::Id::RETURN) {
            save();
        }
    }
}

UIElement::Type UIJoypad::get_type() const {
    return TYPE;
}

Button::State UIJoypad::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::DEFAULT: break;
        case Buttons::CANCEL: cancel(); break;
        case Buttons::OK: save(); break;
        default: break;
    }

    return Button::State::NORMAL;
}

void UIJoypad::cancel() {
    deactivate();
}

void UIJoypad::save() {
    cancel();
}
}  // namespace ms