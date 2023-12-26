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
#include "Hair.h"

#include <array>
#include <iostream>
#include <nlnx/nx.hpp>
#include <string>

namespace ms {
Hair::Hair(int32_t hairid, const BodyDrawInfo &drawinfo) {
    nl::node hairnode =
        nl::nx::character["Hair"]["000" + std::to_string(hairid) + ".img"];

    for (auto stance_iter : Stance::names) {
        Stance::Id stance = stance_iter.first;
        const std::string &stancename = stance_iter.second;

        nl::node stancenode = hairnode[stancename];

        if (!stancenode) {
            continue;
        }

        for (int frame = 0; nl::node framenode = stancenode[frame]; ++frame) {
            for (const nl::node &layernode : framenode) {
                std::string layername = layernode.name();
                auto layer_iter = layers_by_name_.find(layername);

                if (layer_iter == layers_by_name_.end()) {
                    std::cout << "Unknown Hair::Layer name: [" << layername
                              << "]" << std::endl;
                    continue;
                }

                Layer layer = layer_iter->second;

                Point<int16_t> brow = layernode["map"]["brow"];
                Point<int16_t> shift =
                    drawinfo.get_hair_pos(stance, frame) - brow;

                stances_[stance][layer]
                    .emplace(frame, layernode)
                    .first->second.shift(shift);
            }
        }
    }

    name_ = std::string(nl::nx::string["Eqp.img"]["Eqp"]["Hair"]
                                      [std::to_string(hairid)]["name"]);

    const std::array<std::string, 8> haircolors = { "Black",  "Red",   "Orange",
                                                    "Blonde", "Green", "Blue",
                                                    "Violet", "Brown" };

    size_t index = hairid % 10;
    color_ = (index < haircolors.size()) ? haircolors[index] : "";
}

void Hair::draw(Stance::Id stance,
                Layer layer,
                uint8_t frame,
                const DrawArgument &args) const {
    auto frameit = stances_[stance][layer].find(frame);

    if (frameit == stances_[stance][layer].end()) {
        return;
    }

    frameit->second.draw(args);
}

const std::string &Hair::get_name() const {
    return name_;
}

const std::string &Hair::getcolor() const {
    return color_;
}

const std::unordered_map<std::string, Hair::Layer> Hair::layers_by_name_ = {
    { "hair", Hair::Layer::DEFAULT },
    { "hairBelowBody", Hair::Layer::BELOW_BODY },
    { "hairOverHead", Hair::Layer::OVER_HEAD },
    { "hairShade", Hair::Layer::SHADE },
    { "backHair", Hair::Layer::BACK },
    { "backHairBelowCap", Hair::Layer::BELOW_CAP }
};
}  // namespace ms