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
#include "MobSkillHitEffect.h"

#include "../MapleMap/Mob.h"

namespace ms {
MobSingleHitEffect::MobSingleHitEffect(const nl::node &src) :
    effect_(src["mob"]) {}

void MobSingleHitEffect::apply(Mob &mob) const {
    const auto func = [&mob](const Animation &a, int8_t b, int8_t c, bool d) {
        mob.show_effect(a, b, c, d);
    };

    effect_.apply(func);
}

MobBySkillLevelHitEffect::MobBySkillLevelHitEffect(const nl::node &src) {
    // for (auto sub : src["level"]) {
    //     auto level = string_conversion::or_zero<int32_t>(sub.name());
    //     effects_.emplace(level, sub["hit"]["0"]);
    // }
}

void MobBySkillLevelHitEffect::apply(Mob &mob) const {
    // auto iter = effects_.find(user.skilllevel);

    // if (iter != effects_.end())
    //     iter->second.apply(target, user.flip);
}
}  // namespace ms