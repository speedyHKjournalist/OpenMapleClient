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
#include "MobCombat.h"

#include "../../Character/SkillId.h"
#include "../../IO/Messages.h"
#include "../../Net/Packets/AttackAndSkillPackets.h"
#include "../../Net/Packets/GameplayPackets.h"

namespace ms {
MobCombat::MobCombat(Player &in_player, MapChars &in_chars, MapMobs &in_mobs) :
    player_(in_player),
    chars_(in_chars),
    mobs_(in_mobs),
    attack_results_([&](const AttackResult &attack) { apply_attack(attack); }),
    damage_effects_(
        [&](const DamageEffect &effect) { apply_damage_effect(effect); }) {}

void MobCombat::draw(double viewx, double viewy, float alpha) const {}

void MobCombat::update() {}

void MobCombat::use_move(int oid, int move_id, int skill_id, uint8_t level) {
    int at = (move_id - 24);
    int action_id = at % 2 == 0 ? at / 2 : (at + 1) / 2;

    if (auto mob = mobs_.get_mobs()->get<Mob>(oid)) {
        if (0 < action_id && action_id < 7) {
            const auto &move = mob->get().get_move(action_id);
            apply_move(move, *mob);
        } else if (skill_id != 0) {
            const auto &move = mob->get().get_move(skill_id, level);
            apply_move(move, *mob);
        }
    }
}

void MobCombat::apply_move(const MobSkill &move, Mob &mob) {
    mob.update_movement(1, 1, 47, move.get_id(), move.get_level(), 0);
    if (move.is_buff()) {
        mob.give_buff(move.get_buff());
    }
    mob.use_skill(move);
    move.apply_useeffects(mob);
    // move.apply_hiteffects(mob);
}

void MobCombat::apply_move(const MobSpecialAttack &move, Mob &mob) {
    mob.update_movement(1, 1, 2 * (move.get_id() + 24), 0, 0, 0);
    mob.use_attack(move);
    move.apply_useeffects(mob);
    // move.apply_hiteffects(mob);
}

void MobCombat::use_some_attack(int oid) {
    if (auto mob = mobs_.get_mobs()->get<Mob>(oid)) {
        mob->get().use_some_attack();
    }
}

std::vector<int32_t> MobCombat::find_closest(MapObjects *objs,
                                             Rectangle<int16_t> range,
                                             Point<int16_t> origin,
                                             uint8_t objcount,
                                             bool use_mobs) const {
    std::vector<int32_t> targets;

    return targets;
}

void MobCombat::apply_use_movement(const SpecialMove &move) {}

void MobCombat::apply_result_movement(const SpecialMove &move,
                                      const AttackResult &result) {}

void MobCombat::apply_rush(const AttackResult &result) {}

void MobCombat::apply_bullet_effect(const BulletEffect &effect) {}

void MobCombat::apply_damage_effect(const DamageEffect &effect) {}

void MobCombat::push_attack(const AttackResult &attack) {}

void MobCombat::apply_attack(const AttackResult &attack) {}

void MobCombat::extract_effects(const Char &user,
                                const SpecialMove &move,
                                const AttackResult &result) {}

std::vector<DamageNumber> MobCombat::place_numbers(
    int32_t oid,
    const std::vector<std::pair<int32_t, bool>> &damagelines) {
    std::vector<DamageNumber> numbers;

    return numbers;
}

void MobCombat::show_buff(int32_t cid, int32_t skillid, int8_t level) {}

void MobCombat::show_player_buff(int32_t skillid) {}

const SpecialMove &MobCombat::get_move(int32_t move_id) {
    if (move_id == 0) {
        return regular_attack_;
    }

    auto iter = skills_.find(move_id);

    if (iter == skills_.end()) {
        iter = skills_.emplace(move_id, move_id).first;
    }

    return iter->second;
}
}  // namespace ms