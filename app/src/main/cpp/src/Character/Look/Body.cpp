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
#include "Body.h"

#include <array>
#include <nlnx/nx.hpp>
#include <string>

#include "StringHandling.h"

namespace ms {
Body::Body(int32_t skin, const BodyDrawInfo &drawinfo) {
    std::string strid = string_format::extend_id(skin, 2);
    nl::node bodynode = nl::nx::character["000020" + strid + ".img"];
    nl::node headnode = nl::nx::character["000120" + strid + ".img"];

    for (const auto &[stance, stance_name] : Stance::names) {
        nl::node stancenode = bodynode[stance_name];

        if (!stancenode) {
            continue;
        }

        for (int frame = 0; nl::node framenode = stancenode[frame]; ++frame) {
            for (const auto &partnode : framenode) {
                std::string part = partnode.name();

                if (part != "delay" && part != "face") {
                    std::string z = partnode["z"];
                    Body::Layer layer = layer_by_name(z);

                    if (layer == Body::Layer::NONE) {
                        continue;
                    }

                    Point<int16_t> shift;

                    switch (layer) {
                        case Body::Layer::HAND_BELOW_WEAPON:
                            shift = drawinfo.get_hand_position(stance, frame);
                            shift -= partnode["map"]["handMove"];
                            break;
                        default:
                            shift = drawinfo.get_body_position(stance, frame);
                            if (Stance::Id::DEAD == stance) {
                                shift -= Point<int16_t>(5, 4);
                            }

                            shift -= partnode["map"]["navel"];
                            break;
                    }

                    stances_[stance][layer]
                        .emplace(frame, partnode)
                        .first->second.shift(shift);
                }
            }

            if (nl::node headsfnode = headnode[stance_name][frame]["head"]) {
                Point<int16_t> shift =
                    drawinfo.get_head_position(stance, frame);

                stances_[stance][Body::Layer::HEAD]
                    .emplace(frame, headsfnode)
                    .first->second.shift(shift);
            }
        }
    }

    const std::array<std::string, 12> skintypes = { "Light", "Tan",  "Dark",
                                                    "Pale",  "Blue", "Green",
                                                    "",      "",     "",
                                                    "Grey",  "Pink", "Red" };

    size_t index = skin;
    name_ = (index < skintypes.size()) ? skintypes[index] : "";
}

void Body::draw(Stance::Id stance,
                Layer layer,
                uint8_t frame,
                const DrawArgument &args) const {
    auto frameit = stances_[stance][layer].find(frame);

    if (frameit == stances_[stance][layer].end()) {
        return;
    }

    frameit->second.draw(args);
}

const std::string &Body::get_name() const {
    return name_;
}

Body::Layer Body::layer_by_name(const std::string &name) {
    auto layer_iter = layers_by_name_.find(name);

    if (layer_iter == layers_by_name_.end()) {
        if (!name.empty()) {
            std::cout << "Unknown Body::Layer name: [" << name << "]"
                      << std::endl;
        }

        return Body::Layer::NONE;
    }

    return layer_iter->second;
}

const std::unordered_map<std::string, Body::Layer> Body::layers_by_name_ = {
    { "body", Body::Layer::BODY },
    { "backBody", Body::Layer::BODY },
    { "arm", Body::Layer::ARM },
    { "armBelowHead", Body::Layer::ARM_BELOW_HEAD },
    { "armBelowHeadOverMailChest", Body::Layer::ARM_BELOW_HEAD_OVER_MAIL },
    { "armOverHair", Body::Layer::ARM_OVER_HAIR },
    { "armOverHairBelowWeapon", Body::Layer::ARM_OVER_HAIR_BELOW_WEAPON },
    { "handBelowWeapon", Body::Layer::HAND_BELOW_WEAPON },
    { "handOverHair", Body::Layer::HAND_OVER_HAIR },
    { "handOverWeapon", Body::Layer::HAND_OVER_WEAPON },
    { "head", Body::Layer::HEAD }
};
}  // namespace ms