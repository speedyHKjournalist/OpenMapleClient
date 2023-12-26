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
#include "SkillUseEffect.h"

#include "StringHandling.h"

namespace ms {
SingleUseEffect::SingleUseEffect(const nl::node &src) :
    effect_(src["effect"]) {}

void SingleUseEffect::apply(Char &target) const {
    effect_.apply(target);
}

TwoHandedUseEffect::TwoHandedUseEffect(const nl::node &src) :
    effects_(src["effect"]["0"], src["effect"]["1"]) {}

void TwoHandedUseEffect::apply(Char &target) const {
    bool twohanded = target.is_twohanded();
    effects_[twohanded].apply(target);
}

MultiUseEffect::MultiUseEffect(const nl::node &src) {
    int8_t no = -1;
    nl::node sub = src["effect"];

    while (sub) {
        effects_.push_back(sub);

        no++;
        sub = src["effect" + std::to_string(no)];
    }
}

void MultiUseEffect::apply(Char &target) const {
    for (const auto &effect : effects_) {
        effect.apply(target);
    }
}

ByLevelUseEffect::ByLevelUseEffect(const nl::node &src) {
    for (const auto &sub : src["CharLevel"]) {
        auto level = string_conversion::or_zero<uint16_t>(sub.name());
        effects_.emplace(level, sub["effect"]);
    }
}

void ByLevelUseEffect::apply(Char &target) const {
    if (effects_.empty()) {
        return;
    }

    uint16_t level = target.get_level();
    auto iter = effects_.begin();
    for (; iter != effects_.end() && level > iter->first; ++iter) {
    }

    if (iter != effects_.begin()) {
        iter--;
    }

    iter->second.apply(target);
}

void IronBodyUseEffect::apply(Char &target) const {
    target.show_iron_body();
}
}  // namespace ms