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

#include "../Template/BoolPair.h"
#include "Button.h"

namespace ms {
class TwoSpriteButton : public Button {
public:
    TwoSpriteButton(const nl::node &nsrc,
                    const nl::node &ssrc,
                    Point<int16_t> npos,
                    Point<int16_t> spos);

    TwoSpriteButton(const nl::node &nsrc,
                    const nl::node &ssrc,
                    Point<int16_t> position);

    TwoSpriteButton(const nl::node &nsrc, const nl::node &ssrc);

    TwoSpriteButton();

    void draw(Point<int16_t> position) const override;

    void update() override {}

    Rectangle<int16_t> bounds(Point<int16_t> position) const override;

    int16_t width() const override;

    Point<int16_t> origin() const override;

    Cursor::State send_cursor(bool, Point<int16_t>) override {
        return Cursor::State::IDLE;
    }

private:
    BoolPair<Texture> textures_;
    Point<int16_t> npos_;
    Point<int16_t> spos_;
};
}  // namespace ms