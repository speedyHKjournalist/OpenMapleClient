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
#include "CharStats.h"

#include "StatCaps.h"

namespace ms {
CharStats::CharStats(const StatsEntry &s) :
    name_(s.name),
    pet_ids_(s.petids),
    exp_(s.exp),
    map_id_(s.mapid),
    portal_(s.portal),
    rank_(s.rank),
    job_rank_(s.jobrank),
    base_stats_(s.stats),
    female_(s.female) {
    job_ = base_stats_[MapleStat::Id::JOB];

    init_totalstats();
}

CharStats::CharStats() = default;

void CharStats::init_totalstats() {
    total_stats_.clear();
    buff_deltas_.clear();
    percentages_.clear();

    total_stats_[EquipStat::Id::HP] = get_stat(MapleStat::Id::MAXHP);
    total_stats_[EquipStat::Id::MP] = get_stat(MapleStat::Id::MAXMP);
    total_stats_[EquipStat::Id::STR] = get_stat(MapleStat::Id::STR);
    total_stats_[EquipStat::Id::DEX] = get_stat(MapleStat::Id::DEX);
    total_stats_[EquipStat::Id::INT] = get_stat(MapleStat::Id::INT);
    total_stats_[EquipStat::Id::LUK] = get_stat(MapleStat::Id::LUK);
    total_stats_[EquipStat::Id::SPEED] = 100;
    total_stats_[EquipStat::Id::JUMP] = 100;

    max_damage_ = 0;
    min_damage_ = 0;
    honor_ = 0;
    attack_speed_ = 0;
    projectile_range_ = 400;
    mastery_ = 0.0f;
    critical_ = 0.05f;
    min_crit_ = 0.5f;
    max_crit_ = 0.75f;
    damage_percent_ = 0.0f;
    boss_dmg_ = 0.0f;
    ignore_def_ = 0.0f;
    stance_ = 0.0f;
    resist_status_ = 0.0f;
    reduce_damage_ = 0.0f;
}

void CharStats::close_totalstats() {
    total_stats_[EquipStat::Id::ACC] += calculate_accuracy();

    for (auto iter : percentages_) {
        EquipStat::Id stat = iter.first;
        int32_t total = total_stats_[stat];
        total += static_cast<int32_t>(total * iter.second);
        set_total(stat, total);
    }

    int32_t primary = get_primary_stat();
    int32_t secondary = get_secondary_stat();
    int32_t attack = get_total(EquipStat::Id::WATK);
    float multiplier = damage_percent_ + static_cast<float>(attack) / 100;
    max_damage_ = static_cast<int32_t>((primary + secondary) * multiplier);
    min_damage_ = static_cast<int32_t>(((primary * 0.9f * mastery_) + secondary)
                                       * multiplier);
}

int32_t CharStats::calculate_accuracy() const {
    int32_t totaldex = get_total(EquipStat::Id::DEX);
    int32_t totalluk = get_total(EquipStat::Id::LUK);

    return static_cast<int32_t>(totaldex * 0.8f + totalluk * 0.5f);
}

int32_t CharStats::get_primary_stat() const {
    EquipStat::Id primary = job_.get_primary(weapon_type_);

    return static_cast<int32_t>(get_multiplier() * get_total(primary));
}

int32_t CharStats::get_secondary_stat() const {
    EquipStat::Id secondary = job_.get_secondary(weapon_type_);

    return get_total(secondary);
}

float CharStats::get_multiplier() const {
    switch (weapon_type_) {
        case Weapon::Type::SWORD_1H: return 4.0f;
        case Weapon::Type::AXE_1H:
        case Weapon::Type::MACE_1H:
        case Weapon::Type::WAND:
        case Weapon::Type::STAFF: return 4.4f;
        case Weapon::Type::DAGGER:
        case Weapon::Type::CROSSBOW:
        case Weapon::Type::CLAW:
        case Weapon::Type::GUN: return 3.6f;
        case Weapon::Type::SWORD_2H: return 4.6f;
        case Weapon::Type::AXE_2H:
        case Weapon::Type::MACE_2H:
        case Weapon::Type::KNUCKLE: return 4.8f;
        case Weapon::Type::SPEAR:
        case Weapon::Type::POLEARM: return 5.0f;
        case Weapon::Type::BOW: return 3.4f;
        default: return 0.0f;
    }
}

void CharStats::set_stat(MapleStat::Id stat, uint16_t value) {
    base_stats_[stat] = value;
}

void CharStats::set_total(EquipStat::Id stat, int32_t value) {
    auto iter = EQSTAT_CAPS.find(stat);

    if (iter != EQSTAT_CAPS.end()) {
        int32_t cap_value = iter->second;

        if (value > cap_value) {
            value = cap_value;
        }
    }

    total_stats_[stat] = value;
}

void CharStats::add_buff(EquipStat::Id stat, int32_t value) {
    int32_t current = get_total(stat);
    set_total(stat, current + value);
    buff_deltas_[stat] += value;
}

void CharStats::add_value(EquipStat::Id stat, int32_t value) {
    int32_t current = get_total(stat);
    set_total(stat, current + value);
}

void CharStats::add_percent(EquipStat::Id stat, float percent) {
    percentages_[stat] += percent;
}

void CharStats::set_weapontype(Weapon::Type w) {
    weapon_type_ = w;
}

void CharStats::set_exp(int64_t e) {
    exp_ = e;
}

void CharStats::set_portal(uint8_t p) {
    portal_ = p;
}

void CharStats::set_mastery(float m) {
    mastery_ = 0.5f + m;
}

void CharStats::set_damagepercent(float d) {
    damage_percent_ = d;
}

void CharStats::set_reducedamage(float r) {
    reduce_damage_ = r;
}

void CharStats::change_job(uint16_t id) {
    base_stats_[MapleStat::Id::JOB] = id;
    job_.change_job(id);
}

int32_t CharStats::calculate_damage(int32_t mobatk) const {
    // TODO: Random stuff, need to find the actual formula somewhere.
    auto weapon_def = get_total(EquipStat::Id::WDEF);

    if (weapon_def == 0) {
        return mobatk;
    }

    int32_t reduceatk = mobatk / 2 + mobatk / weapon_def;

    return reduceatk - static_cast<int32_t>(reduceatk * reduce_damage_);
}

bool CharStats::is_damage_buffed() const {
    return get_buffdelta(EquipStat::Id::WATK) > 0
           || get_buffdelta(EquipStat::Id::MAGIC) > 0;
}

uint16_t CharStats::get_stat(MapleStat::Id stat) const {
    return base_stats_[stat];
}

int32_t CharStats::get_total(EquipStat::Id stat) const {
    return total_stats_[stat];
}

int32_t CharStats::get_buffdelta(EquipStat::Id stat) const {
    return buff_deltas_[stat];
}

Rectangle<int16_t> CharStats::get_range() const {
    return Rectangle<int16_t>(-projectile_range_, -5, -50, 50);
}

void CharStats::set_mapid(int32_t id) {
    map_id_ = id;
}

int32_t CharStats::get_mapid() const {
    return map_id_;
}

uint8_t CharStats::get_portal() const {
    return portal_;
}

int64_t CharStats::get_exp() const {
    return exp_;
}

const std::string &CharStats::get_name() const {
    return name_;
}

const std::string &CharStats::get_jobname() const {
    return job_.get_name();
}

Weapon::Type CharStats::get_weapontype() const {
    return weapon_type_;
}

float CharStats::get_mastery() const {
    return mastery_;
}

float CharStats::get_critical() const {
    return critical_;
}

float CharStats::get_mincrit() const {
    return min_crit_;
}

float CharStats::get_maxcrit() const {
    return max_crit_;
}

float CharStats::get_reducedamage() const {
    return reduce_damage_;
}

float CharStats::get_bossdmg() const {
    return boss_dmg_;
}

float CharStats::get_ignoredef() const {
    return ignore_def_;
}

void CharStats::set_stance(float s) {
    stance_ = s;
}

float CharStats::get_stance() const {
    return stance_;
}

float CharStats::get_resistance() const {
    return resist_status_;
}

int32_t CharStats::get_maxdamage() const {
    return max_damage_;
}

int32_t CharStats::get_mindamage() const {
    return min_damage_;
}

uint16_t CharStats::get_honor() const {
    return honor_;
}

void CharStats::set_attackspeed(int8_t as) {
    attack_speed_ = as;
}

int8_t CharStats::get_attackspeed() const {
    return attack_speed_;
}

const Job &CharStats::get_job() const {
    return job_;
}

bool CharStats::get_female() const {
    return female_;
}
}  // namespace ms
