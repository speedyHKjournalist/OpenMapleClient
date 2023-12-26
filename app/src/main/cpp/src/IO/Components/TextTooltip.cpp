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
#include "TextTooltip.h"

#include <nlnx/nx.hpp>

#include "../Constants.h"

namespace ms {
TextTooltip::TextTooltip() {
    nl::node Frame = nl::nx::ui["UIToolTip.img"]["Item"]["Frame2"];

    frame_ = Frame;
    cover_ = Frame["cover"];
    text_ = "";
}

void TextTooltip::draw(Point<int16_t> pos) const {
    if (text_label_.empty()) {
        return;
    }

    int16_t fillwidth = text_label_.width();
    int16_t fillheight = text_label_.height();

    int16_t max_width = Constants::Constants::get().get_viewwidth();
    int16_t max_height = Constants::Constants::get().get_viewheight();
    int16_t cur_width = pos.x() + fillwidth + 21;
    int16_t cur_height = pos.y() + fillheight + 40;

    int16_t adj_x = cur_width - max_width;
    int16_t adj_y = cur_height - max_height;

    if (adj_x > 0) {
        pos.shift_x(adj_x * -1);
    }

    if (adj_y > 0) {
        pos.shift_y(adj_y * -1);
    }

    frame_.draw(pos + Point<int16_t>(fillwidth / 2 + 2, fillheight - 7),
                fillwidth - 14,
                fillheight - 18);

    if (fillheight > 18) {
        cover_.draw(pos + Point<int16_t>(-5, -2));
        text_label_.draw(pos);
    } else {
        cover_.draw(DrawArgument(pos + Point<int16_t>(-5, -2), 0.5f, 0.5f));
        text_label_.draw(pos + Point<int16_t>(1, -3));
    }
}

bool TextTooltip::set_text(const std::string &t) {
    if (text_ == t) {
        return false;
    }

    text_ = t;

    if (text_.empty()) {
        return false;
    }

    text_label_ = Text(Text::Font::A12M,
                       Text::Alignment::LEFT,
                       Color::Name::WHITE,
                       text_,
                       340,
                       true,
                       2);

    return true;
}
}  // namespace ms