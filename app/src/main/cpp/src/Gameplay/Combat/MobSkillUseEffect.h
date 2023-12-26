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
#include <nlnx/nx.hpp>
#include <vector>

#include "../../Graphics/Animation.h"

namespace ms {
class Mob;
class MobSkillUseEffect {
public:
    virtual ~MobSkillUseEffect() = default;

    virtual void apply(Mob &mob) const = 0;

protected:
    class Effect {
    public:
        Effect(const nl::node &src) {
            animation_ = src;
            z_ = src["z"];
        }

        void apply(const std::function<void(Animation, int8_t, int8_t, bool)>
                       &func_show_effect) const {
            func_show_effect(animation_, 1, z_, false);
            // mob.show_effect(animation_, 0, z_, true);
        }

    private:
        Animation animation_;
        int8_t z_;
    };
};

// An effect which displays an animation over the mob's position
class MobSingleUseEffect : public MobSkillUseEffect {
public:
    MobSingleUseEffect(const nl::node &src);

    void apply(Mob &mob) const override;

private:
    Effect effect_;
};

// An effect which displays multiple animations over the mob's position
class MobMultiUseEffect : public MobSkillUseEffect {
public:
    MobMultiUseEffect(const nl::node &src, bool area_warning = false);

    void apply(Mob &mob) const override;

private:
    std::vector<Effect> effects_;
};
}  // namespace ms