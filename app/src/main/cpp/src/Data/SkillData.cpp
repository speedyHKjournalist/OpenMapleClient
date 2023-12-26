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
#include "SkillData.h"

#include <nlnx/nx.hpp>

#include "../Character/SkillId.h"
#include "StringHandling.h"

namespace ms {
SkillData::SkillData(int32_t id) {
    /// Locate sources
    std::string strid = string_format::extend_id(id, 7);
    std::string jobid = strid.substr(0, 3);
    nl::node src = nl::nx::skill[jobid + ".img"]["skill"][strid];
    nl::node strsrc = nl::nx::string["Skill.img"][strid];

    // Load icons
    icons_ = { src["icon"], src["iconDisabled"], src["iconMouseOver"] };

    // Load strings
    name_ = std::string(strsrc["name"]);
    desc_ = std::string(strsrc["desc"]);

    for (int level = 1; nl::node sub = strsrc["h" + std::to_string(level)];
         level++) {
        levels_.emplace(level, sub);
    }

    /// Load stats
    nl::node levelsrc = src["level"];

    for (const auto &sub : levelsrc) {
        float damage = (float)sub["damage"] / 100;
        int32_t matk = sub["mad"];
        int32_t fixdamage = sub["fixdamage"];
        int32_t mastery = sub["mastery"];
        uint8_t attackcount = (uint8_t)sub["attackCount"].get_integer(1);
        uint8_t mobcount = (uint8_t)sub["mobCount"].get_integer(1);
        uint8_t bulletcount = (uint8_t)sub["bulletCount"].get_integer(1);
        int16_t bulletcost =
            (int16_t)sub["bulletConsume"].get_integer(bulletcount);
        int32_t hpcost = sub["hpCon"];
        int32_t mpcost = sub["mpCon"];
        float chance = (float)sub["prop"].get_real(100.0) / 100;
        float critical = 0.0f;
        float ignoredef = 0.0f;
        float hrange = (float)sub["range"].get_real(100.0) / 100;
        Rectangle<int16_t> range = sub;
        int32_t level = string_conversion::or_default<int32_t>(sub.name(), -1);

        stats_.emplace(std::piecewise_construct,
                       std::forward_as_tuple(level),
                       std::forward_as_tuple(damage,
                                             matk,
                                             fixdamage,
                                             mastery,
                                             attackcount,
                                             mobcount,
                                             bulletcount,
                                             bulletcost,
                                             hpcost,
                                             mpcost,
                                             chance,
                                             critical,
                                             ignoredef,
                                             hrange,
                                             range));
    }

    element_ = std::string(src["elemAttr"]);

    if (jobid == "900" || jobid == "910") {
        req_weapon_ = Weapon::Type::NONE;
    } else {
        req_weapon_ = Weapon::by_value(100 + (int32_t)src["weapon"]);
    }

    master_level_ = static_cast<int32_t>(stats_.size());
    passive_ = (id % 10000) / 1000 == 0;
    flags_ = flags_of(id);
    invisible_ = src["invisible"].get_bool();

    /// Load required skills
    nl::node reqsrc = src["req"];

    for (const auto &sub : reqsrc) {
        int32_t skillid =
            string_conversion::or_default<int32_t>(sub.name(), -1);
        int32_t reqlv = sub.get_integer();

        req_skills_.emplace(skillid, reqlv);
    }
}

int32_t SkillData::flags_of(int32_t id) const {
    static const std::unordered_map<int32_t, int32_t> skill_flags = {
        // Beginner
        { SkillId::THREE_SNAILS, ATTACK },
        // Warrior
        { SkillId::POWER_STRIKE, ATTACK },
        { SkillId::SLASH_BLAST, ATTACK },
        // Fighter
        // Page
        // Crusader
        { SkillId::SWORD_PANIC, ATTACK },
        { SkillId::AXE_PANIC, ATTACK },
        { SkillId::SWORD_COMA, ATTACK },
        { SkillId::AXE_COMA, ATTACK },
        // Hero
        { SkillId::RUSH_HERO, ATTACK },
        { SkillId::BRANDISH, ATTACK },
        // Page
        // White Knight
        { SkillId::CHARGE, ATTACK },
        // Paladin
        { SkillId::RUSH_PALADIN, ATTACK },
        { SkillId::BLAST, ATTACK },
        { SkillId::HEAVENS_HAMMER, ATTACK },
        // Spearman
        // Dragon Knight
        { SkillId::DRAGON_BUSTER, ATTACK },
        { SkillId::DRAGON_FURY, ATTACK },
        { SkillId::PA_BUSTER, ATTACK },
        { SkillId::PA_FURY, ATTACK },
        { SkillId::SACRIFICE, ATTACK },
        { SkillId::DRAGONS_ROAR, ATTACK },
        // Dark Knight
        { SkillId::RUSH_DK, ATTACK },
        // Mage
        { SkillId::ENERGY_BOLT, ATTACK | RANGED },
        { SkillId::MAGIC_CLAW, ATTACK | RANGED },
        // F/P Mage
        { SkillId::SLOW_FP, ATTACK },
        { SkillId::FIRE_ARROW, ATTACK | RANGED },
        { SkillId::POISON_BREATH, ATTACK | RANGED },
        // F/P ArchMage
        { SkillId::EXPLOSION, ATTACK },
        { SkillId::POISON_BREATH, ATTACK },
        { SkillId::SEAL_FP, ATTACK },
        { SkillId::ELEMENT_COMPOSITION_FP, ATTACK | RANGED },
        //
        { SkillId::FIRE_DEMON, ATTACK },
        { SkillId::PARALYZE, ATTACK | RANGED },
        { SkillId::METEOR_SHOWER, ATTACK }
    };

    auto iter = skill_flags.find(id);

    if (iter == skill_flags.end()) {
        return NONE;
    }

    return iter->second;
}

bool SkillData::is_passive() const {
    return passive_;
}

bool SkillData::is_attack() const {
    return !passive_ && (flags_ & ATTACK);
}

bool SkillData::is_invisible() const {
    return invisible_;
}

int32_t SkillData::get_masterlevel() const {
    return master_level_;
}

Weapon::Type SkillData::get_required_weapon() const {
    return req_weapon_;
}

const SkillData::Stats &SkillData::get_stats(int32_t level) const {
    auto iter = stats_.find(level);

    if (iter == stats_.end()) {
        static constexpr Stats null_stats = Stats(0.0f,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0,
                                                  0.0f,
                                                  0.0f,
                                                  0.0f,
                                                  0.0f,
                                                  Rectangle<int16_t>());

        return null_stats;
    }

    return iter->second;
}

const std::string &SkillData::get_name() const {
    return name_;
}

const std::string &SkillData::get_desc() const {
    return desc_;
}

const std::string &SkillData::get_level_desc(int32_t level) const {
    auto iter = levels_.find(level);

    if (iter == levels_.end()) {
        static const std::string null_level = "Missing level description.";

        return null_level;
    }
    return iter->second;
}

const Texture &SkillData::get_icon(Icon icon) const {
    return icons_[icon];
}

const std::unordered_map<int32_t, int32_t> &SkillData::get_reqskills() const {
    return req_skills_;
}
}  // namespace ms