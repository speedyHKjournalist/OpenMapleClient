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

#include <memory>

#include "../../Graphics/Animation.h"
#include "MobSkillHitEffect.h"
#include "MobSkillUseEffect.h"

namespace ms {
class Mob;

struct MobBuff {
    int64_t time;
    Animation anim;
};

// The skill implementation of special move
class MobSkill {
public:
    MobSkill(int32_t skillid, int32_t level);

    void apply_useeffects(Mob &mob) const;

    void apply_hiteffects(Mob &mob) const;

    bool is_attack() const;

    bool is_skill() const;

    bool is_buff() const;

    int32_t get_id() const;

    uint8_t get_level() const { return level_; }

    MobBuff get_buff() const { return buff_; }

private:
    std::unique_ptr<MobSkillUseEffect> use_effect_;
    std::unique_ptr<MobSkillHitEffect> hit_effect_;

    int32_t skill_id_;
    uint8_t level_;

    MobBuff buff_;
};
}  // namespace ms