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
#include "UISoftKey.h"

#include <nlnx/nx.hpp>
#include <utility>

#include "../../Constants.h"
#include "../Components/MapleButton.h"
#include "../Components/TwoSpriteButton.h"

namespace ms {
UISoftKey::UISoftKey(OkCallback ok,
                     CancelCallback cancel,
                     std::string tooltip_text,
                     Point<int16_t> tooltip_pos) :
    ok_callback_(std::move(ok)),
    cancel_callback_(std::move(cancel)),
    tooltip_position_(tooltip_pos) {
    Point<int16_t> screen_adj = Point<int16_t>(-1, 0);

    nl::node SoftKey = nl::nx::ui["Login.img"]["Common"]["SoftKey"];
    nl::node backgrnd = SoftKey["backgrnd"];

    nl::node Tab = SoftKey["Tab"];
    nl::node TabNormal = Tab["normal"];
    nl::node TabSelected = Tab["selected"];

    sprites_.emplace_back(backgrnd, screen_adj);
    sprites_.emplace_back(SoftKey["backgrnd2"]);
    sprites_.emplace_back(SoftKey["backgrnd3"]);

    buttons_[Buttons::NEXT] = std::make_unique<MapleButton>(SoftKey["BtNext"]);
    buttons_[Buttons::DEL] = std::make_unique<MapleButton>(SoftKey["BtDel"]);
    buttons_[Buttons::CANCEL] =
        std::make_unique<MapleButton>(SoftKey["BtCancel"]);
    buttons_[Buttons::OK] = std::make_unique<MapleButton>(SoftKey["BtOK"]);

    for (size_t i = 0; i < 3; i++) {
        buttons_[Buttons::TAB0 + i] =
            std::make_unique<TwoSpriteButton>(TabNormal[i],
                                              TabSelected[i],
                                              screen_adj);

        if (i == 0) {
            buttons_[Buttons::TAB0 + i]->set_state(Button::State::PRESSED);
        } else {
            buttons_[Buttons::TAB0 + i]->set_state(Button::State::DISABLED);
        }
    }

    for (size_t i = 0; i < NUM_KEYS_; i++) {
        buttons_[Buttons::NUM0 + i] =
            std::make_unique<MapleButton>(SoftKey["BtNum"][i]);
    }

    entry_ = Textfield(
        Text::Font::A11M,
        Text::Alignment::LEFT,
        Color::Name::LIGHTGREY,
        Rectangle<int16_t>(Point<int16_t>(-3, -4), Point<int16_t>(150, 24)),
        MAX_SIZE_);
    entry_.set_cryptchar('*');

    shufflekeys();
    show_text(std::move(tooltip_text));

    position_ = Point<int16_t>(130, 160);
    dimension_ = Texture(backgrnd).get_dimensions();
}

UISoftKey::UISoftKey(OkCallback ok_callback,
                     CancelCallback cancel_callback,
                     std::string tooltip_text) :
    UISoftKey(std::move(ok_callback),
              std::move(cancel_callback),
              std::move(tooltip_text),
              Point<int16_t>(0, 0)) {}

UISoftKey::UISoftKey(OkCallback ok_callback, CancelCallback cancel_callback) :
    UISoftKey(std::move(ok_callback), std::move(cancel_callback), "") {}

UISoftKey::UISoftKey(OkCallback ok_callback) :
    UISoftKey(std::move(ok_callback), []() {}) {}

void UISoftKey::draw(float inter) const {
    UIElement::draw(inter);

    entry_.draw(position_ + Point<int16_t>(210, 65));

    if (tooltip_) {
        tooltip_->get().draw(position_ + Point<int16_t>(200, 30)
                             + tooltip_position_);
    }
}

void UISoftKey::update() {
    UIElement::update();

    if (tooltip_) {
        if (timestamp_ > 0) {
            timestamp_ -= Constants::TIMESTEP;
        } else {
            clear_tooltip();
        }
    }
}

void UISoftKey::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            button_pressed(Buttons::CANCEL);
        } else if (keycode == KeyAction::Id::RETURN) {
            button_pressed(Buttons::OK);
        }
    }
}

UIElement::Type UISoftKey::get_type() const {
    return TYPE;
}

Button::State UISoftKey::button_pressed(uint16_t buttonid) {
    std::string entered = entry_.get_text();
    size_t size = entered.size();

    if (buttonid == Buttons::DEL) {
        if (size > 0) {
            entered.pop_back();
            entry_.change_text(entered);
        }
    } else if (buttonid == Buttons::CANCEL) {
        if (cancel_callback_) {
            cancel_callback_();
            deactivate();
        }
    } else if (buttonid == Buttons::OK) {
        if (size >= MIN_SIZE_) {
            if (ok_callback_) {
                ok_callback_(entered);
                deactivate();
            }
        } else {
            clear_tooltip();
            show_text("The PIC needs to be at least 6 characters long.");
        }
    } else if (buttonid >= Buttons::NUM0) {
        if (size < MAX_SIZE_) {
            entered.append(std::to_string(buttonid - Buttons::NUM0));
            entry_.change_text(entered);
        }
    }

    return Button::State::NORMAL;
}

void UISoftKey::shufflekeys() {
    std::vector<uint8_t> reserve;

    for (size_t i = 0; i < NUM_KEYS_; i++) {
        reserve.push_back(i);
    }

    for (size_t i = 0; i < NUM_KEYS_; i++) {
        size_t rand = random_.next_int(reserve.size());
        Point<int16_t> pos = keypos(reserve[rand]);

        buttons_[Buttons::NUM0 + i]->set_position(pos);

        reserve.erase(reserve.begin() + rand);
    }
}

void UISoftKey::show_text(const std::string &text) {
    tetooltip_.set_text(text);

    if (!text.empty()) {
        tooltip_ = tetooltip_;
        timestamp_ = 6 * 1000;
    }
}

void UISoftKey::clear_tooltip() {
    tooltip_position_ = Point<int16_t>(0, 0);
    tetooltip_.set_text("");
    tooltip_ = {};
}

Point<int16_t> UISoftKey::keypos(uint8_t num) const {
    auto x = (num % 3) * 45;
    auto y = (num / 3) * 43;

    return Point<int16_t>(230 + x, 94 + y);
}
}  // namespace ms