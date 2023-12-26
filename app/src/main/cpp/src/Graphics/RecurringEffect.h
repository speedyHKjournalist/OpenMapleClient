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

#include <list>
#include <map>

#include "../Constants.h"
#include "Sprite.h"

namespace ms {
class RecurringEffect {
public:
    void drawbelow(Point<int16_t> position, float alpha) const;

    void drawabove(Point<int16_t> position, float alpha) const;

    void update();

    void add(const Animation &effect,
             const DrawArgument &args,
             int8_t z,
             float speed);

    void add(const Animation &effect, const DrawArgument &args, int8_t z);

    void add(const Animation &effect, const DrawArgument &args);

    void add(const Animation &effect);

private:
    class Effect {
    public:
        Effect(const Animation &a, const DrawArgument &args, float s) :
            sprite_(a, args),
            speed_(s) {}

        void draw(Point<int16_t> position, float alpha) const {
            sprite_.draw(position, alpha);
        }

        bool update() {
            bool ended = sprite_.update(
                static_cast<uint16_t>(Constants::TIMESTEP * speed_));

            if (ended) {
                sprite_.reset();
            }

            return ended;
        }

    private:
        Sprite sprite_;
        float speed_;
    };

    std::map<int8_t, std::list<Effect>> effects_;
};
}  // namespace ms