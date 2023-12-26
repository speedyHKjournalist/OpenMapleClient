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
#include "NameTag.h"

namespace ms {
NameTag::NameTag(const nl::node &src, Text::Font f, const std::string &n) {
    name_ = OutlinedText(f,
                         Text::Alignment::CENTER,
                         Color::Name::EAGLE,
                         Color::Name::JAMBALAYA);
    name_.change_text(n);

    textures_[false].emplace_back(src["0"]["0"]);
    textures_[false].emplace_back(src["0"]["1"]);
    textures_[false].emplace_back(src["0"]["2"]);
    textures_[true].emplace_back(src["1"]["0"]);
    textures_[true].emplace_back(src["1"]["1"]);
    textures_[true].emplace_back(src["1"]["2"]);

    selected_ = false;
}

void NameTag::draw(Point<int16_t> position) const {
    position = position + Point<int16_t>(1, 2);

    const auto &tag = textures_[selected_];

    int16_t width = name_.width();
    auto startpos = position - Point<int16_t>(6 + width / 2, -1);

    tag[0].draw(startpos);
    tag[1].draw(DrawArgument(startpos + Point<int16_t>(6, 0),
                             Point<int16_t>(width, 0)));
    tag[2].draw(DrawArgument(startpos + Point<int16_t>(width + 6, 0)));

    name_.draw(position);
}

void NameTag::set_selected(bool s) {
    selected_ = s;

    if (s) {
        name_.change_color(Color::Name::WHITE);
    } else {
        name_.change_color(Color::Name::EAGLE);
    }
}
}  // namespace ms