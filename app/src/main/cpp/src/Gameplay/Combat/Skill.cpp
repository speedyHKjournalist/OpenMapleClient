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
#include "Skill.h"

#include <nlnx/nx.hpp>

#include "../../Character/SkillId.h"
#include "../../Data/SkillData.h"
#include "StringHandling.h"

namespace ms {
Skill::Skill(int32_t id) : skill_id_(id) {
    const SkillData &data = SkillData::get(skill_id_);

    std::string strid;

    if (skill_id_ < 10000000) {
        strid = string_format::extend_id(skill_id_, 7);
    } else {
        strid = std::to_string(skill_id_);
    }

    nl::node src = nl::nx::skill[strid.substr(0, 3) + ".img"]["skill"][strid];

    projectile_ = true;
    over_regular_ = false;

    sound_ = std::make_unique<SingleSkillSound>(strid);

    bool has_affected_effect = src["affected"].size() > 0;
    bool by_level_effect = src["CharLevel"]["10"]["effect"].size() > 0;
    bool multi_effect = src["effect0"].size() > 0;

    if (has_affected_effect) {
        affected_effects_ = std::make_unique<SingleAffectedEffect>(src);
    } else {
        affected_effects_ = std::make_unique<NoAffectedEffect>();
    }

    if (by_level_effect) {
        use_effect_ = std::make_unique<ByLevelUseEffect>(src);
    } else if (multi_effect) {
        use_effect_ = std::make_unique<MultiUseEffect>(src);
    } else {
        bool is_animation =
            src["effect"]["0"].data_type() == nl::node::type::bitmap;
        bool has_effect1 = src["effect"]["1"].size() > 0;

        if (is_animation) {
            use_effect_ = std::make_unique<SingleUseEffect>(src);
        } else if (has_effect1) {
            use_effect_ = std::make_unique<TwoHandedUseEffect>(src);
        } else {
            switch (skill_id_) {
                case SkillId::IRON_BODY:
                case SkillId::MAGIC_ARMOR:
                    use_effect_ = std::make_unique<IronBodyUseEffect>();
                    break;
                default: use_effect_ = std::make_unique<NoUseEffect>(); break;
            }
        }
    }

    bool by_level_hit = src["CharLevel"]["10"]["hit"].size() > 0;
    bool by_skill_level_hit = src["level"]["1"]["hit"].size() > 0;
    bool has_hit0 = src["hit"]["0"].size() > 0;
    bool has_hit1 = src["hit"]["1"].size() > 0;

    if (by_level_hit) {
        if (has_hit0 && has_hit1) {
            hit_effect_ = std::make_unique<ByLevelTwoHandedHitEffect>(src);
        } else {
            hit_effect_ = std::make_unique<ByLevelHitEffect>(src);
        }
    } else if (by_skill_level_hit) {
        hit_effect_ = std::make_unique<BySkillLevelHitEffect>(src);
    } else if (has_hit0 && has_hit1) {
        hit_effect_ = std::make_unique<TwoHandedHitEffect>(src);
    } else if (has_hit0) {
        hit_effect_ = std::make_unique<SingleHitEffect>(src);
    } else {
        hit_effect_ = std::make_unique<NoHitEffect>();
    }

    bool has_action0 = src["action"]["0"].data_type() == nl::node::type::string;
    bool has_action1 = src["action"]["1"].data_type() == nl::node::type::string;

    if (has_action0 && has_action1) {
        action_ = std::make_unique<TwoHandedAction>(src);
    } else if (has_action0) {
        action_ = std::make_unique<SingleAction>(src);
    } else if (data.is_attack()) {
        bool by_level =
            src["level"]["1"]["action"].data_type() == nl::node::type::string;

        if (by_level) {
            action_ = std::make_unique<ByLevelAction>(src, skill_id_);
        } else {
            action_ = std::make_unique<RegularAction>();
            over_regular_ = true;
        }
    } else {
        action_ = std::make_unique<NoAction>();
    }

    bool has_ball = src["ball"].size() > 0;
    bool by_level_ball = src["level"]["1"]["ball"].size() > 0;

    if (by_level_ball) {
        bullet_ = std::make_unique<BySkillLevelBullet>(src, skill_id_);
    } else if (has_ball) {
        bullet_ = std::make_unique<SingleBullet>(src);
    } else {
        bullet_ = std::make_unique<RegularBullet>();
        projectile_ = false;
    }
}

void Skill::apply_useeffects(Char &user) const {
    use_effect_->apply(user);

    sound_->play_use();
}

void Skill::apply_actions(Char &user, Attack::Type type) const {
    action_->apply(user, type);
}

void Skill::apply_stats(const Char &user, Attack &attack) const {
    attack.skill = skill_id_;

    int32_t level = user.get_skilllevel(skill_id_);
    const SkillData::Stats stats = SkillData::get(skill_id_).get_stats(level);

    if (stats.fixdamage) {
        attack.fixdamage = stats.fixdamage;
        attack.damagetype = Attack::DMG_FIXED;
    } else if (stats.matk) {
        attack.matk += stats.matk;
        attack.damagetype = Attack::DMG_MAGIC;
    } else {
        attack.mindamage *= stats.damage;
        attack.maxdamage *= stats.damage;
        attack.damagetype = Attack::DMG_WEAPON;
    }

    attack.critical += stats.critical;
    attack.ignoredef += stats.ignoredef;
    attack.mobcount = stats.mobcount;
    attack.hrange = stats.hrange;

    switch (attack.type) {
        case Attack::RANGED: attack.hitcount = stats.bulletcount; break;
        default: attack.hitcount = stats.attackcount; break;
    }

    if (!stats.range.empty()) {
        attack.range = stats.range;
    }

    if (projectile_ && !attack.bullet) {
        switch (skill_id_) {
            case SkillId::THREE_SNAILS:
                switch (level) {
                    case 1: attack.bullet = 4000019; break;
                    case 2: attack.bullet = 4000000; break;
                    case 3: attack.bullet = 4000016; break;
                }
                break;
            default: attack.bullet = skill_id_; break;
        }
    }

    if (over_regular_) {
        attack.stance = user.get_look().get_stance();

        if (attack.type == Attack::CLOSE && !projectile_) {
            attack.range = user.get_afterimage().get_range();
        }
    }
}

void Skill::apply_hiteffects(const AttackUser &user, Mob &target) const {
    hit_effect_->apply(user, target);

    sound_->play_hit();
}

void Skill::apply_affected_effects(Char &user) const {
    affected_effects_->apply(user);
}

Animation Skill::get_bullet(const Char &user, int32_t bulletid) const {
    return bullet_->get(user, bulletid);
}

bool Skill::is_attack() const {
    return SkillData::get(skill_id_).is_attack();
}

bool Skill::is_skill() const {
    return true;
}

int32_t Skill::get_id() const {
    return skill_id_;
}

SpecialMove::ForbidReason Skill::can_use(int32_t level,
                                         Weapon::Type weapon,
                                         const Job &job,
                                         uint16_t hp,
                                         uint16_t mp,
                                         uint16_t bullets) const {
    if (level <= 0 || level > SkillData::get(skill_id_).get_masterlevel()) {
        return FBR_OTHER;
    }

    if (job.can_use(skill_id_) == false) {
        return FBR_OTHER;
    }

    const SkillData::Stats stats = SkillData::get(skill_id_).get_stats(level);

    if (hp <= stats.hpcost) {
        return FBR_HPCOST;
    }

    if (mp < stats.mpcost) {
        return FBR_MPCOST;
    }

    Weapon::Type reqweapon = SkillData::get(skill_id_).get_required_weapon();

    if (weapon != reqweapon && reqweapon != Weapon::NONE) {
        return FBR_WEAPONTYPE;
    }

    switch (weapon) {
        case Weapon::BOW:
        case Weapon::CROSSBOW:
        case Weapon::CLAW:
        case Weapon::GUN:
            return (bullets >= stats.bulletcost) ? FBR_NONE : FBR_BULLETCOST;
        default: return FBR_NONE;
    }
}
}  // namespace ms