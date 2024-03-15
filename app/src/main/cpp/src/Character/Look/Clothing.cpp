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
#include "Clothing.h"

#include <nlnx/nx.hpp>
#include <unordered_set>

#include "../../Data/WeaponData.h"

namespace ms {
Clothing::Clothing(int32_t id, const BodyDrawInfo &drawinfo) : item_id_(id) {
    const EquipData &equipdata = EquipData::get(item_id_);

    eq_slot_ = equipdata.get_eqslot();

    if (item_id_ == TOP_DEFAULT_ID)
        eq_slot_ = EquipSlot::Id::TOP_DEFAULT;
    else if (item_id_ == BOTTOM_DEFAULT_ID)
        eq_slot_ = EquipSlot::Id::BOTTOM_DEFAULT;

    if (eq_slot_ == EquipSlot::Id::WEAPON) {
        two_handed_ = WeaponData::get(item_id_).is_twohanded();
    } else {
        two_handed_ = false;
    }

    constexpr size_t NON_WEAPON_TYPES = 15;
    constexpr size_t WEAPON_OFFSET = NON_WEAPON_TYPES + 15;
    constexpr size_t WEAPON_TYPES = 20;

    constexpr Clothing::Layer layers[NON_WEAPON_TYPES] = {
        Clothing::Layer::CAP,     Clothing::Layer::FACE_ACC,
        Clothing::Layer::EYE_ACC, Clothing::Layer::EARRINGS,
        Clothing::Layer::TOP,     Clothing::Layer::MAIL,
        Clothing::Layer::PANTS,   Clothing::Layer::SHOES,
        Clothing::Layer::GLOVE,   Clothing::Layer::SHIELD,
        Clothing::Layer::CAPE,    Clothing::Layer::RING,
        Clothing::Layer::PENDANT, Clothing::Layer::BELT,
        Clothing::Layer::MEDAL
    };

    Clothing::Layer chlayer;
    size_t index = (item_id_ / 10000) - 100;

    if (item_id_ == TOP_DEFAULT_ID) {
        chlayer = Clothing::Layer::TOP_DEFAULT;
    } else if (item_id_ == BOTTOM_DEFAULT_ID) {
        chlayer = Clothing::Layer::PANTS_DEFAULT;
    } else if (index < NON_WEAPON_TYPES) {
        chlayer = layers[index];
    } else if (index >= WEAPON_OFFSET && index < WEAPON_OFFSET + WEAPON_TYPES) {
        chlayer = Clothing::Layer::WEAPON;
    } else {
        chlayer = Clothing::Layer::CAPE;
    }

    std::string strid = "0" + std::to_string(item_id_);
    std::string category = equipdata.get_itemdata().get_category();
    nl::node src = nl::nx::character[category][strid + ".img"];
    nl::node info = src["info"];

    vslot_ = std::string(info["vslot"]);

    switch (int32_t standno = info["stand"]) {
        case 1: stand_ = Stance::Id::STAND1; break;
        case 2: stand_ = Stance::Id::STAND2; break;
        default:
            stand_ = two_handed_ ? Stance::Id::STAND2 : Stance::Id::STAND1;
            break;
    }

    switch (int32_t walkno = info["walk"]) {
        case 1: walk_ = Stance::Id::WALK1; break;
        case 2: walk_ = Stance::Id::WALK2; break;
        default:
            walk_ = two_handed_ ? Stance::Id::WALK2 : Stance::Id::WALK1;
            break;
    }

    for (auto iter : Stance::names) {
        Stance::Id stance = iter.first;
        const std::string &stancename = iter.second;

        nl::node stancenode = src[stancename];

        if (!stancenode) {
            continue;
        }

        for (int frame = 0; nl::node framenode = stancenode[frame]; ++frame) {
            for (const nl::node &partnode : framenode) {
                std::string part = partnode.name();

                if (!partnode
                    || partnode.data_type() != nl::node::type::bitmap) {
                    continue;
                }

                Clothing::Layer z = chlayer;
                std::string zs = partnode["z"];

                if (part == "mailArm") {
                    z = Clothing::Layer::MAILARM;
                } else {
                    auto sublayer_iter = sub_layer_names_.find(zs);

                    if (sublayer_iter != sub_layer_names_.end()) {
                        z = sublayer_iter->second;
                    }
                }

                std::string parent;
                Point<int16_t> parentpos;

                for (const auto &mapnode : partnode["map"]) {
                    if (mapnode.data_type() == nl::node::type::vector) {
                        parent = mapnode.name();
                        parentpos = mapnode;
                    }
                }

                nl::node mapnode = partnode["map"];
                Point<int16_t> shift;

                switch (eq_slot_) {
                    case EquipSlot::Id::FACE: shift -= parentpos; break;
                    case EquipSlot::Id::SHOES:
                    case EquipSlot::Id::GLOVES:
                    case EquipSlot::Id::TOP:
                    case EquipSlot::Id::TOP_DEFAULT:
                    case EquipSlot::Id::BOTTOM:
                    case EquipSlot::Id::BOTTOM_DEFAULT:
                    case EquipSlot::Id::CAPE:
                        shift = drawinfo.get_body_position(stance, frame)
                                - parentpos;
                        break;
                    case EquipSlot::Id::HAT:
                    case EquipSlot::Id::EARACC:
                    case EquipSlot::Id::EYE_ACC:
                        shift =
                            drawinfo.get_face_pos(stance, frame) - parentpos;
                        break;
                    case EquipSlot::Id::SHIELD:
                    case EquipSlot::Id::WEAPON:
                        if (parent == "handMove") {
                            shift += drawinfo.get_hand_position(stance, frame);
                        } else if (parent == "hand") {
                            shift += drawinfo.get_arm_position(stance, frame);
                        } else if (parent == "navel") {
                            shift += drawinfo.get_body_position(stance, frame);
                        }

                        shift -= parentpos;
                        break;
                    default:
                        break;
                }

                stances_[stance][z]
                    .emplace(frame, partnode)
                    ->second.shift(shift);
            }
        }
    }

    static const std::unordered_set<int32_t> transparents = { 1002186 };

    transparent_ = transparents.count(item_id_) > 0;
}

void Clothing::draw(Stance::Id stance,
                    Layer layer,
                    uint8_t frame,
                    const DrawArgument &args) const {
    auto range = stances_[stance][layer].equal_range(frame);

    for (auto iter = range.first; iter != range.second; ++iter) {
        iter->second.draw(args);
    }
}

bool Clothing::contains_layer(Stance::Id stance, Layer layer) const {
    return !stances_[stance][layer].empty();
}

bool Clothing::is_transparent() const {
    return transparent_;
}

bool Clothing::is_twohanded() const {
    return two_handed_;
}

int32_t Clothing::get_id() const {
    return item_id_;
}

Stance::Id Clothing::get_stand() const {
    return stand_;
}

Stance::Id Clothing::get_walk() const {
    return walk_;
}

EquipSlot::Id Clothing::get_eqslot() const {
    return eq_slot_;
}

const std::string &Clothing::get_vslot() const {
    return vslot_;
}

const std::unordered_map<std::string, Clothing::Layer>
    Clothing::sub_layer_names_ = {
        // WEAPON
        { "weaponOverHand", Clothing::Layer::WEAPON_OVER_HAND },
        { "weaponOverGlove", Clothing::Layer::WEAPON_OVER_GLOVE },
        { "weaponOverBody", Clothing::Layer::WEAPON_OVER_BODY },
        { "weaponBelowArm", Clothing::Layer::WEAPON_BELOW_ARM },
        { "weaponBelowBody", Clothing::Layer::WEAPON_BELOW_BODY },
        { "backWeaponOverShield", Clothing::Layer::BACK_WEAPON },
        // SHIELD
        { "shieldOverHair", Clothing::Layer::SHIELD_OVER_HAIR },
        { "shieldBelowBody", Clothing::Layer::SHIELD_BELOW_BODY },
        { "backShield", Clothing::Layer::BACK_SHIELD },
        // GLOVE
        { "gloveWrist", Clothing::Layer::WRIST },
        { "gloveOverHair", Clothing::Layer::GLOVE_OVER_HAIR },
        { "gloveOverBody", Clothing::Layer::GLOVE_OVER_BODY },
        { "gloveWristOverHair", Clothing::Layer::WRIST_OVER_HAIR },
        { "gloveWristOverBody", Clothing::Layer::WRIST_OVER_BODY },
        // CAP
        { "capOverHair", Clothing::Layer::CAP_OVER_HAIR },
        { "capBelowBody", Clothing::Layer::CAP_BELOW_BODY },
    };
}  // namespace ms