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
#include "../Character/Char.h"
#include "nlnx/nx.hpp"

namespace ms {
// Interface for affected effects
class SkillAffectedEffect {
public:
    virtual ~SkillAffectedEffect() = default;

    virtual void apply(Char &target) const = 0;

protected:
    class Effect {
    public:
        Effect(const nl::node &src) {
            animation_ = src;
            pos_ = src["pos"];
            z_ = src["z"];
        }

        void apply(Char &target) const {
            target.show_attack_effect(animation_, z_);
        }

    private:
        Animation animation_;
        int8_t pos_;
        int8_t z_;
    };
};

// No animation
class NoAffectedEffect : public SkillAffectedEffect {
public:
    void apply(Char &) const override {}
};

// An effect which displays an animation over the character's position
class SingleAffectedEffect : public SkillAffectedEffect {
public:
    SingleAffectedEffect(const nl::node &src);

    void apply(Char &target) const override;

private:
    Effect effect_;
};
}  // namespace ms