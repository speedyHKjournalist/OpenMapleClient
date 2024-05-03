//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2024  Daniel Allendorf, Ryan Payton, Bizhou Xing
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
#include "IconCover.h"
#include "Button.h"
#include "TouchInfo.h"

namespace ms {
class JoyStickButton : public Button {
public:
    JoyStickButton(Point<int16_t> position);

    void draw(Point<int16_t> parentpos) const override;

    void update() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    Rectangle<int16_t> bounds(Point<int16_t> parentpos) const override;

    int16_t width() const override;

    Point<int16_t> origin() const override;

    bool in_combobox(Point<int16_t> cursorpos) override;

private:
    void calc_angle(int16_t distance_x, int16_t distance_y);

    ColorCircle background_;
    int16_t radius_;
    double angle_;
};
}  // namespace ms