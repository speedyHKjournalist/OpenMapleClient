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

#include "../../Character/Char.h"

namespace ms {
// Interface for skill effects
class SkillUseEffect {
public:
    virtual ~SkillUseEffect() = default;

    virtual void apply(Char &target) const = 0;

protected:
    class Effect {
    public:
        Effect(const nl::node &src) {
            animation_ = src;
            z_ = src["z"];
        }

        void apply(Char &target) const {
            target.show_attack_effect(animation_, z_);
        }

    private:
        Animation animation_;
        int8_t z_;
    };
};

// No animation
class NoUseEffect : public SkillUseEffect {
public:
    void apply(Char &) const override {}
};

// An effect which displays an animation over the character's position
class SingleUseEffect : public SkillUseEffect {
public:
    SingleUseEffect(const nl::node &src);

    void apply(Char &target) const override;

private:
    Effect effect_;
};

// An effect which displays an animation over the character's position
// The effect changes based on whether the character uses a two-handed weapon
class TwoHandedUseEffect : public SkillUseEffect {
public:
    TwoHandedUseEffect(const nl::node &src);

    void apply(Char &target) const override;

private:
    BoolPair<Effect> effects_;
};

// An effect which displays multiple animations over the character's position
class MultiUseEffect : public SkillUseEffect {
public:
    MultiUseEffect(const nl::node &src);

    void apply(Char &target) const override;

private:
    std::vector<Effect> effects_;
};

// The animation changes with the character level
class ByLevelUseEffect : public SkillUseEffect {
public:
    ByLevelUseEffect(const nl::node &src);

    void apply(Char &target) const override;

private:
    std::map<uint16_t, Effect> effects_;
};

// Use effect for Iron Body
class IronBodyUseEffect : public SkillUseEffect {
public:
    void apply(Char &target) const override;
};
}  // namespace ms