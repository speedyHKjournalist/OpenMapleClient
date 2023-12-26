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

#include "../Graphics/Texture.h"

namespace ms {
class Gauge {
public:
    enum Type : uint8_t { GAME, CASHSHOP };

    Gauge(Type type, Texture front, int16_t max, float percent);
    Gauge(Type type, Texture front, Texture mid, int16_t max, float percent);
    Gauge(Type type,
          Texture front,
          Texture mid,
          Texture end,
          int16_t maximum,
          float percentage);
    Gauge();

    void draw(const DrawArgument &args) const;
    void update(float target);

private:
    Type type_;

    Texture bar_front_;
    Texture bar_mid_;
    Texture bar_end_;
    int16_t maximum_;

    float percentage_;
    float target_;
    float step_;
};
}  // namespace ms