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
#include "TextButton.h"

#include <utility>

namespace ms {
TextButton::TextButton(Text text,  int16_t selection) {
    active_ = true;
    state_ = Button::State::NORMAL;
    text_ = std::move(text);
    option_number_ = selection;
}

void TextButton::draw(Point<int16_t> parentpos) const {
    if (active_) {
        text_.draw(parentpos);
    }
}

void TextButton::draw(Point<int16_t> parentpos, Range<int16_t> range) const {
    if (active_) {
        text_.draw(parentpos, range);
    }
}

void TextButton::update() {

}

Rectangle<int16_t> TextButton::bounds(Point<int16_t> parentpos) const {
    Point<int16_t> lt;
    Point<int16_t> rb;

    lt = parentpos + position_;
    rb = lt + text_.dimensions();

    return {lt, rb};
}

int16_t TextButton::width() const {
    return text_.width();
}

int16_t TextButton::height() const {
    return text_.height();
}

Point<int16_t> TextButton::origin() const {
    return position_;
}

int16_t TextButton::get_option_number() {
    return option_number_;
}
}  // namespace ms