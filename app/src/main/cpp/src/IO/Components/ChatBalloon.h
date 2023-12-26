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
#pragma once

#include "../Graphics/Text.h"
#include "MapleFrame.h"

namespace ms {
class ChatBalloon {
public:
    ChatBalloon(int8_t type);

    ChatBalloon();

    void draw(Point<int16_t> position) const;

    void update();

    void change_text(const std::string &text);

    void expire();

private:
    // How long a line stays on screen
    static constexpr int16_t DURATION = 4000;  // 4 seconds

    MapleFrame frame_;
    Text text_label_;
    Texture arrow_;
    int16_t duration_;
};

class ChatBalloonHorizontal {
public:
    ChatBalloonHorizontal();

    void draw(Point<int16_t> position) const;

    void change_text(const std::string &text);

private:
    Text text_label_;
    Texture arrow_;
    Texture center_;
    Texture east_;
    Texture north_east_;
    Texture north_;
    Texture north_west_;
    Texture west_;
    Texture south_west_;
    Texture south_;
    Texture south_east_;
    int16_t xtile_;
    int16_t ytile_;
};
}  // namespace ms