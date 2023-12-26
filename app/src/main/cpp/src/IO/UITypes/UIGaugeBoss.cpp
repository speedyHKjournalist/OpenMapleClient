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
#include "UIGaugeBoss.h"

#include <nlnx/nx.hpp>

namespace ms {
UIGaugeBoss::UIGaugeBoss(int16_t screen_width,
                         int8_t tag_color,
                         int8_t tag_bg_color,
                         int32_t mob_id,
                         float percent) :
    screen_width_(screen_width),
    percentage_(percent) {
    nl::node boss_gauge = nl::nx::ui["UIWindow2.img"]["MobGage"];

    tag_front_ = boss_gauge["Gage"][tag_color][1];
    should_use_tag_bg_ = (tag_bg_color != 5);
    tag_bg_ =
        should_use_tag_bg_ ? boss_gauge["Gage"][tag_bg_color][1] : Texture();

    sprites_.emplace_back(boss_gauge["backgrnd"], Point<int16_t>(200, 0));
    sprites_.emplace_back(boss_gauge["Mob"][mob_id], Point<int16_t>(205, 6));
    sprites_.emplace_back(boss_gauge["backgrnd2"], Point<int16_t>(235, 0));

    for (int i = 237; i < screen_width - 5; i++) {
        sprites_.emplace_back(boss_gauge["backgrnd3"], Point<int16_t>(i, 0));
    }

    sprites_.emplace_back(boss_gauge["backgrnd4"],
                          Point<int16_t>(screen_width - 5, 0));
}

void UIGaugeBoss::draw(float alpha) const {
    UIElement::draw_sprites(alpha);
    int16_t length =
        static_cast<int16_t>(percentage_ * (screen_width_ - 237 - 5));

    for (int i = 0; i < length; i++) {
        tag_front_.draw(Point<int16_t>(237 + i, 5));
    }

    if (should_use_tag_bg_) {
        for (int i = length; i < (screen_width_ - 237 - 5); i++) {
            tag_bg_.draw(Point<int16_t>(237 + i, 5));
        }
    }
}

void UIGaugeBoss::update(float p) {
    percentage_ = p;
}
}  // namespace ms