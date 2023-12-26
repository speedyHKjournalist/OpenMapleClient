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
#include "MobSpecialAttack.h"

#include <nlnx/nx.hpp>

#include "StringHandling.h"
#include "../MapleMap/Mob.h"

namespace ms {
MobSpecialAttack::MobSpecialAttack(int32_t mob_id, int32_t move_id) :
    move_id_(move_id) {
    std::string strid;

    if (mob_id < 10000000) {
        strid = string_format::extend_id(mob_id, 7);
    } else {
        strid = std::to_string(mob_id);
    }

    nl::node src =
        nl::nx::mob[strid + ".img"]["attack" + std::to_string(move_id)]["info"];

    // sound_ = std::make_unique<SingleSkillSound>(strid);

    bool has_multi_effect = src["effect0"].size() > 0;
    bool has_area_warning_effect = src["areaWarning"].size() > 0;

    if (has_multi_effect || has_area_warning_effect) {
        use_effect_ =
            std::make_unique<MobMultiUseEffect>(src, has_area_warning_effect);
    } else {
        use_effect_ = std::make_unique<MobSingleUseEffect>(src);
    }

    // hit_effect_ = std::make_unique<MobSingleHitEffect>(src["level"][4]);

    // hit_effect_ =
    // std::make_unique<MobSingleHitEffect>(src["attack1"]["info"]); hit_effect_
    // = std::make_unique<NoHitEffect>();

    // action_ = std::make_unique<NoAction>();
}

void MobSpecialAttack::apply_useeffects(Mob &mob) const {
    use_effect_->apply(mob);
    // sound_->play_use();
}

void MobSpecialAttack::apply_hiteffects(Mob &mob) const {
    // hit_effect_->apply(mob);
    // sound_->play_hit();
}

bool MobSpecialAttack::is_attack() const {
    return true;
}

bool MobSpecialAttack::is_skill() const {
    return false;
}

int32_t MobSpecialAttack::get_id() const {
    return move_id_;
}
}  // namespace ms