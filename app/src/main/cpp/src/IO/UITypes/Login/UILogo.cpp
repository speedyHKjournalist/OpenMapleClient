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
#include "UILogo.h"

#include <nlnx/nx.hpp>

#include "../Audio/Audio.h"
#include "../Configuration.h"
#include "UILogin.h"

namespace ms {
UILogo::UILogo() :
    UIElement(Point<int16_t>(0, 0), Point<int16_t>(800, 600)),
    nexon_ended_(false),
    wizet_ended_(false),
    user_clicked_(false) {
    Music("BgmUI.img/NxLogo").play_once();

    nl::node Logo = nl::nx::ui["Logo.img"];

    nexon_ = Logo["Nexon"];
    wizet_ = Logo["Wizet"];
    wizet_end_ = Logo["Wizet"]["40"];
}

void UILogo::draw(float inter) const {
    UIElement::draw(inter);

    if (!user_clicked_) {
        if (!nexon_ended_) {
            nexon_.draw(position_ + Point<int16_t>(440, 360), inter);
        } else {
            if (!wizet_ended_) {
                wizet_.draw(position_ + Point<int16_t>(263, 195), inter);
            } else {
                wizet_end_.draw(position_ + Point<int16_t>(263, 195));
            }
        }
    } else {
        wizet_end_.draw(position_ + Point<int16_t>(263, 195));
    }
}

void UILogo::update() {
    UIElement::update();

    if (!nexon_ended_) {
        nexon_ended_ = nexon_.update();
    } else {
        if (!wizet_ended_) {
            wizet_ended_ = wizet_.update();
        } else {
            Configuration::get().set_start_shown(true);

            UI::get().remove(UIElement::Type::START);
            UI::get().emplace<UILogin>();
        }
    }
}

Cursor::State UILogo::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

    if (clicked && !user_clicked_) {
        user_clicked_ = true;
    }

    return ret;
}

UIElement::Type UILogo::get_type() const {
    return TYPE;
}
}  // namespace ms