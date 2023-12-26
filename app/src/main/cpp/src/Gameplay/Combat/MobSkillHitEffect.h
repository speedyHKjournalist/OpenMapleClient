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

#include <cstdint>
#include <functional>
#include <map>
#include <nlnx/nx.hpp>
#include <vector>

#include "../../Graphics/Animation.h"
#include "../../Template/BoolPair.h"

namespace ms {
class Mob;

// Interface for hit effects, animations applied to a character for each hit.
class MobSkillHitEffect {
public:
    virtual ~MobSkillHitEffect() = default;

    virtual void apply(Mob &mob) const = 0;

protected:
    class Effect {
    public:
        Effect(const nl::node &src) {
            animation_ = src;
            pos_ = src["pos"];
            z_ = src["z"];
        }

        void apply(const std::function<void(Animation, int8_t, int8_t, bool)>
                       &func_show_effect) const {
            func_show_effect(animation_, pos_, z_, false);
        }

    private:
        Animation animation_;
        int8_t pos_;
        int8_t z_;
    };
};

// No animation
class MobNoHitEffect : public MobSkillHitEffect {
public:
    void apply(Mob & /*mob*/) const override {}
};

// A single animation
class MobSingleHitEffect : public MobSkillHitEffect {
public:
    MobSingleHitEffect(const nl::node &src);

    void apply(Mob &mob) const override;

private:
    Effect effect_;
};

// The animation changes with the skill level
class MobBySkillLevelHitEffect : public MobSkillHitEffect {
public:
    MobBySkillLevelHitEffect(const nl::node &src);

    void apply(Mob &mob) const override;

private:
    std::map<int32_t, Effect> effects_;
};
}  // namespace ms