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
#include "MobSkillUseEffect.h"

#include "../MapleMap/Mob.h"

namespace ms {
MobSingleUseEffect::MobSingleUseEffect(const nl::node &src) :
    effect_(src["effect"]) {}

void MobSingleUseEffect::apply(Mob &mob) const {
    const auto func = [&mob](const Animation &a, int8_t b, int8_t c, bool d) {
        mob.show_effect(a, b, c, d);
    };
    effect_.apply(func);
}

MobMultiUseEffect::MobMultiUseEffect(const nl::node &src, bool area_warning) {
    if (area_warning) {
        effects_.push_back(src["areaWarning"]);
    }

    int8_t no = -1;
    nl::node sub = src["effect"];
    effects_.push_back(sub);

    while (sub) {
        no++;
        sub = src["effect" + std::to_string(no)];
        effects_.push_back(sub[no]);
    }
}

void MobMultiUseEffect::apply(Mob &mob) const {
    const auto func = [&mob](const Animation &a, int8_t b, int8_t c, bool d) {
        mob.show_effect(a, b, c, d);
    };

    for (const auto &effect : effects_) {
        effect.apply(func);
    }
}
}  // namespace ms