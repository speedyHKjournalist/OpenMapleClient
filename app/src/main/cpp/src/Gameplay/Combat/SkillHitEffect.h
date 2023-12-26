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

#include "../../Template/BoolPair.h"
#include "../MapleMap/Mob.h"

namespace ms {
// Interface for hit effects, animations applied to a mob for each hit.
class SkillHitEffect {
public:
    virtual ~SkillHitEffect() = default;

    virtual void apply(const AttackUser &user, Mob &target) const = 0;

protected:
    class Effect {
    public:
        Effect(const nl::node &src) {
            animation_ = src;
            pos_ = src["pos"];
            z_ = src["z"];
        }

        void apply(Mob &target, bool flip) const {
            target.show_effect(animation_, pos_, z_, flip);
        }

    private:
        Animation animation_;
        int8_t pos_;
        int8_t z_;
    };
};

// No animation
class NoHitEffect : public SkillHitEffect {
public:
    void apply(const AttackUser &, Mob &) const override {}
};

// A single animation
class SingleHitEffect : public SkillHitEffect {
public:
    SingleHitEffect(const nl::node &src);

    void apply(const AttackUser &user, Mob &target) const override;

private:
    Effect effect_;
};

// The animation changes depending on the weapon used
class TwoHandedHitEffect : public SkillHitEffect {
public:
    TwoHandedHitEffect(const nl::node &src);

    void apply(const AttackUser &user, Mob &target) const override;

private:
    BoolPair<Effect> effects_;
};

// The animation changes with the character level
class ByLevelHitEffect : public SkillHitEffect {
public:
    ByLevelHitEffect(const nl::node &src);

    void apply(const AttackUser &user, Mob &target) const override;

private:
    std::map<uint16_t, Effect> effects_;
};

// The animation changes with the character level and weapon used
class ByLevelTwoHandedHitEffect : public SkillHitEffect {
public:
    ByLevelTwoHandedHitEffect(const nl::node &src);

    void apply(const AttackUser &user, Mob &target) const override;

private:
    std::map<uint16_t, BoolPair<Effect>> effects_;
};

// The animation changes with the skill level
class BySkillLevelHitEffect : public SkillHitEffect {
public:
    BySkillLevelHitEffect(const nl::node &src);

    void apply(const AttackUser &user, Mob &target) const override;

private:
    std::map<int32_t, Effect> effects_;
};
}  // namespace ms