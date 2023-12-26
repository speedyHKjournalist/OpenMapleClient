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
#include "UIElement.h"

#include "../Audio/Audio.h"

namespace ms {
UIElement::UIElement(Point<int16_t> p, Point<int16_t> d, bool a) :
    position_(p),
    dimension_(d),
    active_(a) {}

UIElement::UIElement(Point<int16_t> p, Point<int16_t> d) :
    UIElement(p, d, true) {}

UIElement::UIElement() : UIElement(Point<int16_t>(), Point<int16_t>()) {}

void UIElement::draw(float alpha) const {
    draw_sprites(alpha);
    draw_buttons(alpha);
}

void UIElement::draw_sprites(float alpha) const {
    for (const Sprite &sprite : sprites_) {
        sprite.draw(position_, alpha);
    }
}

void UIElement::draw_buttons(float) const {
    for (const auto &iter : buttons_) {
        if (const Button *button = iter.second.get()) {
            button->draw(position_);
        }
    }
}

void UIElement::update() {
    for (auto &sprite : sprites_) {
        sprite.update();
    }

    for (auto &iter : buttons_) {
        if (Button *button = iter.second.get()) {
            button->update();
        }
    }
}

void UIElement::makeactive() {
    active_ = true;
}

void UIElement::deactivate() {
    active_ = false;
}

bool UIElement::is_active() const {
    return active_;
}

void UIElement::toggle_active() {
    if (active_) {
        deactivate();
    } else {
        makeactive();
    }
}

bool UIElement::is_in_range(Point<int16_t> cursorpos) const {
    auto bounds = Rectangle<int16_t>(position_, position_ + dimension_);

    return bounds.contains(cursorpos);
}

void UIElement::remove_cursor() {
    for (auto &btit : buttons_) {
        auto *button = btit.second.get();

        if (button->get_state() == Button::State::MOUSEOVER) {
            button->set_state(Button::State::NORMAL);
        }
    }
}

Cursor::State UIElement::send_cursor(bool down, Point<int16_t> pos) {
    Cursor::State ret = down ? Cursor::State::CLICKING : Cursor::State::IDLE;

    for (auto &btit : buttons_) {
        if (btit.second->is_active()
            && btit.second->bounds(position_).contains(pos)) {
            if (btit.second->get_state() == Button::State::NORMAL) {
                Sound(Sound::Name::BUTTON_OVER).play();

                btit.second->set_state(Button::State::MOUSEOVER);
                ret = Cursor::State::CAN_CLICK;
            } else if (btit.second->get_state() == Button::State::MOUSEOVER) {
                if (down) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

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
}  // namespace ms