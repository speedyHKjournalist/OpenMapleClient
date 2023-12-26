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

#include <array>

#include "../Graphics/Animation.h"
#include "Button.h"

namespace ms {
// A standard maplestory-button with 4 states and a texture for each state.
class MapleButton : public Button {
public:
    MapleButton(const nl::node &src, Point<int16_t> position);

    MapleButton(const nl::node &src, int16_t x, int16_t y);

    MapleButton(const nl::node &src);

    void draw(Point<int16_t> position) const override;

    void update() override;

    Rectangle<int16_t> bounds(Point<int16_t> parentpos) const override;

    int16_t width() const override;

    Point<int16_t> origin() const override;

    Cursor::State send_cursor(bool, Point<int16_t>) override {
        return Cursor::State::IDLE;
    }

private:
    std::array<Texture, Button::State::NUM_STATES> textures_;
    std::array<Animation, Button::State::NUM_STATES> animations_;
};
}  // namespace ms