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
#include "UIGender.h"

#include <nlnx/nx.hpp>
#include <utility>

#include "../../Net/Packets/LoginPackets.h"
#include "../Components/MapleButton.h"
#include "../Components/MapleComboBox.h"
#include "../UI.h"
#include "UILoginWait.h"

namespace ms {
namespace {
auto fn_gender = []<typename... T>(T && ...args) {
    GenderPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIGender::UIGender(std::function<void()> oh) :
    UIElement(Point<int16_t>(0, 15), Point<int16_t>(0, 0)),
    okhandler_(std::move(oh)) {
    cur_timestep_ = 0;

    nl::node Login = nl::nx::ui["Login.img"];
    nl::node Gender = Login["Gender"];
    nl::node scroll = Gender["scroll"][0];

    for (size_t i = 0; i < gender_sprites_.size(); i++) {
        gender_sprites_[i] = scroll[i];
    }

    sprites_.emplace_back(Gender["text"][0], Point<int16_t>(601, 326));

    std::vector<std::string> options;
    options.push_back("Male");
    options.push_back("Female");

    uint16_t default_option = 0;

    buttons_[Buttons::NO] = std::make_unique<MapleButton>(
        Login["BtCancel"],
        Point<int16_t>(650,
                       349));  // TODO: _inlink issue: Original: Gender["BtNo"]
    buttons_[Buttons::YES] =
        std::make_unique<MapleButton>(Gender["BtYes"],
                                      Point<int16_t>(578, 349));
    buttons_[Buttons::SELECT] =
        std::make_unique<MapleComboBox>(MapleComboBox::Type::DEFAULT,
                                        options,
                                        default_option,
                                        position_,
                                        Point<int16_t>(510, 283),
                                        65);

    dimension_ = Texture(gender_sprites_[2]).get_dimensions();
}

void UIGender::draw(float inter) const {
    Point<int16_t> gender_pos = Point<int16_t>(355, 185);

    if (cur_timestep_ == 0) {
        gender_sprites_[0].draw(position_ + gender_pos);
    } else if (cur_timestep_ == Constants::TIMESTEP * 3) {
        gender_sprites_[1].draw(position_ + gender_pos);
    } else if (cur_timestep_ >= Constants::TIMESTEP * 6) {
        gender_sprites_[2].draw(position_ + gender_pos);

        UIElement::draw(inter);
    }
}

void UIGender::update() {
    UIElement::update();

    if (cur_timestep_ <= Constants::TIMESTEP * 6) {
        cur_timestep_ += Constants::TIMESTEP;
    }
}

Cursor::State UIGender::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    auto &combobox = buttons_[Buttons::SELECT];

    if (combobox->is_pressed() && combobox->in_combobox(cursorpos)) {
        if (Cursor::State new_state =
                combobox->send_cursor(clicked, cursorpos)) {
            return new_state;
        }
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

UIElement::Type UIGender::get_type() const {
    return TYPE;
}

Button::State UIGender::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::NO:
            deactivate();
            okhandler_();
            break;
        case Buttons::YES: {
            UI::get().emplace<UILoginWait>();

            uint16_t selected_value = buttons_[Buttons::SELECT]->get_selected();
            fn_gender(selected_value);
        } break;
        case Buttons::SELECT:
            buttons_[Buttons::SELECT]->toggle_pressed();
            break;
        default: break;
    }

    return Button::State::NORMAL;
}
}  // namespace ms