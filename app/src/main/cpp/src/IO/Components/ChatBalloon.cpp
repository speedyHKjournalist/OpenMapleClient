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
#include "ChatBalloon.h"

#include <nlnx/nx.hpp>

#include "../Constants.h"

namespace ms {
ChatBalloon::ChatBalloon(int8_t type) {
    std::string typestr;

    if (type < 0) {
        switch (type) {
            case -1: typestr = "dead"; break;
        }
    } else {
        typestr = std::to_string(type);
    }

    nl::node src = nl::nx::ui["ChatBalloon.img"][typestr];

    arrow_ = src["arrow"];
    frame_ = src;

    text_label_ = Text(Text::Font::A11M,
                       Text::Alignment::CENTER,
                       Color::Name::BLACK,
                       "",
                       80);

    duration_ = 0;
}

ChatBalloon::ChatBalloon() : ChatBalloon(0) {}

void ChatBalloon::change_text(const std::string &text) {
    text_label_.change_text(text);

    duration_ = DURATION;
}

void ChatBalloon::draw(Point<int16_t> position) const {
    if (duration_ == 0) {
        return;
    }

    int16_t width = text_label_.width();
    int16_t height = text_label_.height();

    frame_.draw(position, width, height);
    arrow_.draw(position);
    text_label_.draw(position - Point<int16_t>(0, height + 4));
}

void ChatBalloon::update() {
    duration_ -= Constants::TIMESTEP;

    if (duration_ < 0) {
        duration_ = 0;
    }
}

void ChatBalloon::expire() {
    duration_ = 0;
}

ChatBalloonHorizontal::ChatBalloonHorizontal() {
    nl::node Balloon = nl::nx::ui["Login.img"]["WorldNotice"]["Balloon"];

    arrow_ = Balloon["arrow"];
    center_ = Balloon["c"];
    east_ = Balloon["e"];
    north_east_ = Balloon["ne"];
    north_ = Balloon["n"];
    north_west_ = Balloon["nw"];
    west_ = Balloon["w"];
    south_west_ = Balloon["sw"];
    south_ = Balloon["s"];
    south_east_ = Balloon["se"];

    xtile_ = std::max<int16_t>(north_.width(), 1);
    ytile_ = std::max<int16_t>(west_.height(), 1);

    text_label_ = Text(Text::Font::A12B,
                       Text::Alignment::LEFT,
                       Color::Name::BLACK,
                       "",
                       300);
}

void ChatBalloonHorizontal::draw(Point<int16_t> position) const {
    int16_t width = text_label_.width() + 9;
    int16_t height = text_label_.height() - 2;

    int16_t left = position.x() - width / 2;
    int16_t top = position.y() - height;
    int16_t right = left + width;
    int16_t bottom = top + height;

    north_west_.draw(DrawArgument(left, top));
    south_west_.draw(DrawArgument(left, bottom));

    for (int y = top; y < bottom; y += ytile_) {
        west_.draw(DrawArgument(left, y));
        east_.draw(DrawArgument(right, y));
    }

    center_.draw(DrawArgument(Point<int16_t>(left - 8, top),
                              Point<int16_t>(width + 8, height)));

    for (int x = left; x < right; x += xtile_) {
        north_.draw(DrawArgument(x, top));
        south_.draw(DrawArgument(x, bottom));
    }

    north_east_.draw(DrawArgument(right, top));
    south_east_.draw(DrawArgument(right, bottom));

    arrow_.draw(DrawArgument(right + 1, top));
    text_label_.draw(DrawArgument(left + 6, top - 5));
}

void ChatBalloonHorizontal::change_text(const std::string &text) {
    text_label_.change_text(text);
}
}  // namespace ms