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
#include "ScrollingNotice.h"

namespace ms {
ScrollingNotice::ScrollingNotice() :
    active_(false),
    width_(800),
    background_(width_, 23, Color::Name::BLACK, 0.535f),
    notice_(Text::Font::A12M, Text::Alignment::LEFT, Color::Name::YELLOW),
    xpos_() {
    xpos_.set(0.0);
}

void ScrollingNotice::set_notice(const std::string &n) {
    notice_.change_text(n);
    xpos_.set(static_cast<double>(width_));
    active_ = !n.empty();
}

void ScrollingNotice::draw(float alpha) const {
    if (active_) {
        auto interx = static_cast<int16_t>(std::round(xpos_.get(alpha)));
        auto position = Point<int16_t>(interx, -1);

        background_.draw(Point<int16_t>(0, 0));
        notice_.draw(position);
    }
}

void ScrollingNotice::update() {
    if (active_) {
        int16_t new_width = Constants::Constants::get().get_viewwidth();

        if (new_width != width_) {
            width_ = new_width;
            background_.set_width(width_);
            xpos_.set(static_cast<double>(width_));
        }

        xpos_ -= 0.5;

        auto xmin = static_cast<double>(-notice_.width());

        if (xpos_.last() < xmin) {
            xpos_.set(static_cast<double>(width_));
        }
    }
}
}  // namespace ms