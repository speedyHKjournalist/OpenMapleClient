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
#include "MobSkill.h"

#include <nlnx/nx.hpp>

#include "../MapleMap/Mob.h"

namespace ms {
MobSkill::MobSkill(int32_t id, int32_t level) : skill_id_(id), level_(level) {
    nl::node src = nl::nx::skill["MobSkill.img"][skill_id_];

    // sound_ = std::make_unique<SingleSkillSound>(strid);

    // nl::node src = nl::nx::mob["8510000.img"];
    // bool multi_effect = src["attack1"]["info"]["effect0"].size() > 0;

    use_effect_ = std::make_unique<MobSingleUseEffect>(src["level"][level]);

    // hit_effect_ = std::make_unique<MobSingleHitEffect>(src["level"][4]);

    bool is_buff = src["level"][level]["mob"].size() > 0;

    if (is_buff) {
        buff_ = { src["level"][level]["time"].get_integer(),
                  src["level"][level]["mob"] };
    } else {
        buff_ = {};
    }

    // hit_effect_ =
    // std::make_unique<MobSingleHitEffect>(src["attack1"]["info"]); hit_effect_
    // = std::make_unique<NoHitEffect>();

    // action_ = std::make_unique<NoAction>();
}

void MobSkill::apply_useeffects(Mob &mob) const {
    use_effect_->apply(mob);
    // sound_->play_use();
}

void MobSkill::apply_hiteffects(Mob &mob) const {
    // hit_effect_->apply(mob);
    // sound_->play_hit();
}

bool MobSkill::is_attack() const {
    return false;
}

bool MobSkill::is_skill() const {
    return true;
}

bool MobSkill::is_buff() const {
    return buff_.time > 5;
}

int32_t MobSkill::get_id() const {
    return skill_id_;
}
}  // namespace ms