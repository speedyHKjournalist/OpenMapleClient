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
#include "EffectLayer.h"

namespace ms {
void EffectLayer::drawbelow(Point<int16_t> position, float alpha) const {
    for (auto iter = effects_.begin(); iter != effects_.upper_bound(-1);
         ++iter) {
        for (const auto &effect : iter->second) {
            effect.draw(position, alpha);
        }
    }
}

void EffectLayer::drawabove(Point<int16_t> position, float alpha) const {
    for (auto iter = effects_.upper_bound(-1); iter != effects_.end(); ++iter) {
        for (const auto &effect : iter->second) {
            effect.draw(position, alpha);
        }
    }
}

void EffectLayer::update() {
    for (auto &effectlist : effects_) {
        effectlist.second.remove_if(
            [](Effect &effect) { return effect.update(); });
    }
}

void EffectLayer::add(const Animation &animation,
                      const DrawArgument &args,
                      int8_t z,
                      float speed) {
    effects_[z].emplace_back(animation, args, speed);
}

void EffectLayer::add(const Animation &animation,
                      const DrawArgument &args,
                      int8_t z) {
    add(animation, args, z, 1.0f);
}

void EffectLayer::add(const Animation &animation, const DrawArgument &args) {
    add(animation, args, 0, 1.0f);
}

void EffectLayer::add(const Animation &animation) {
    add(animation, {}, 0, 1.0f);
}
}  // namespace ms