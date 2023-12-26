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
#include "SkillHitEffect.h"

#include "StringHandling.h"

namespace ms {
SingleHitEffect::SingleHitEffect(const nl::node &src) :
    effect_(src["hit"]["0"]) {}

void SingleHitEffect::apply(const AttackUser &user, Mob &target) const {
    effect_.apply(target, user.flip);
}

TwoHandedHitEffect::TwoHandedHitEffect(const nl::node &src) :
    effects_(src["hit"]["0"], src["hit"]["1"]) {}

void TwoHandedHitEffect::apply(const AttackUser &user, Mob &target) const {
    effects_[user.second_weapon].apply(target, user.flip);
}

ByLevelHitEffect::ByLevelHitEffect(const nl::node &src) {
    for (const auto &sub : src["CharLevel"]) {
        uint16_t level = string_conversion::or_zero<uint16_t>(sub.name());
        effects_.emplace(level, sub["hit"]["0"]);
    }
}

void ByLevelHitEffect::apply(const AttackUser &user, Mob &target) const {
    if (effects_.empty()) {
        return;
    }

    auto iter = effects_.begin();
    for (; iter != effects_.end() && user.level > iter->first; ++iter) {
    }

    if (iter != effects_.begin()) {
        iter--;
    }

    iter->second.apply(target, user.flip);
}

ByLevelTwoHandedHitEffect::ByLevelTwoHandedHitEffect(const nl::node &src) {
    for (const auto &sub : src["CharLevel"]) {
        auto level = string_conversion::or_zero<uint16_t>(sub.name());

        effects_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(level),
            std::forward_as_tuple(sub["hit"]["0"], sub["hit"]["1"]));
    }
}

void ByLevelTwoHandedHitEffect::apply(const AttackUser &user,
                                      Mob &target) const {
    if (effects_.empty()) {
        return;
    }

    auto iter = effects_.begin();
    for (; iter != effects_.end() && user.level > iter->first; ++iter) {
    }

    if (iter != effects_.begin()) {
        iter--;
    }

    iter->second[user.second_weapon].apply(target, user.flip);
}

BySkillLevelHitEffect::BySkillLevelHitEffect(const nl::node &src) {
    for (const auto &sub : src["level"]) {
        auto level = string_conversion::or_zero<int32_t>(sub.name());
        effects_.emplace(level, sub["hit"]["0"]);
    }
}

void BySkillLevelHitEffect::apply(const AttackUser &user, Mob &target) const {
    auto iter = effects_.find(user.skill_level);

    if (iter != effects_.end()) {
        iter->second.apply(target, user.flip);
    }
}
}  // namespace ms