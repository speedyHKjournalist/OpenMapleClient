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
#include "EquipTooltip.h"

#include <nlnx/nx.hpp>

#include "../../Data/WeaponData.h"
#include "../../Gameplay/Stage.h"

namespace ms {
EquipTooltip::EquipTooltip() {
    nl::node Item = nl::nx::ui["UIToolTip.img"]["Item"];
    nl::node Frame = Item["Frame"];
    nl::node ItemIcon = Item["ItemIcon"];
    nl::node Equip = Item["Equip"];
    nl::node EquipCan = Equip["Can"];
    nl::node EquipCannot = Equip["Cannot"];

    top_ = Frame["top"];
    mid_ = Frame["line"];
    line_ = Frame["dotline"];
    bot_ = Frame["bottom"];
    base_ = ItemIcon["base"];
    cover_ = Frame["cover"];
    item_cover_ = ItemIcon["cover"];
    type_[true] = ItemIcon["new"];
    type_[false] = ItemIcon["old"];

    potential_[Equip::Potential::POT_NONE] = Texture();
    potential_[Equip::Potential::POT_HIDDEN] = ItemIcon["0"];
    potential_[Equip::Potential::POT_RARE] = ItemIcon["1"];
    potential_[Equip::Potential::POT_EPIC] = ItemIcon["2"];
    potential_[Equip::Potential::POT_UNIQUE] = ItemIcon["3"];
    potential_[Equip::Potential::POT_LEGENDARY] = ItemIcon["4"];

    requirements_.push_back(MapleStat::Id::LEVEL);
    requirements_.push_back(MapleStat::Id::STR);
    requirements_.push_back(MapleStat::Id::DEX);
    requirements_.push_back(MapleStat::Id::INT);
    requirements_.push_back(MapleStat::Id::LUK);

    req_stat_textures_[MapleStat::Id::LEVEL][false] = EquipCannot["reqLEV"];
    req_stat_textures_[MapleStat::Id::LEVEL][true] = EquipCan["reqLEV"];
    req_stat_textures_[MapleStat::Id::FAME][false] = EquipCannot["reqPOP"];
    req_stat_textures_[MapleStat::Id::FAME][true] = EquipCan["reqPOP"];
    req_stat_textures_[MapleStat::Id::STR][false] = EquipCannot["reqSTR"];
    req_stat_textures_[MapleStat::Id::STR][true] = EquipCan["reqSTR"];
    req_stat_textures_[MapleStat::Id::DEX][false] = EquipCannot["reqDEX"];
    req_stat_textures_[MapleStat::Id::DEX][true] = EquipCan["reqDEX"];
    req_stat_textures_[MapleStat::Id::INT][false] = EquipCannot["reqINT"];
    req_stat_textures_[MapleStat::Id::INT][true] = EquipCan["reqINT"];
    req_stat_textures_[MapleStat::Id::LUK][false] = EquipCannot["reqLUK"];
    req_stat_textures_[MapleStat::Id::LUK][true] = EquipCan["reqLUK"];

    req_stat_positions_[MapleStat::Id::LEVEL] = Point<int16_t>(97, 47);
    req_stat_positions_[MapleStat::Id::STR] = Point<int16_t>(97, 62);
    req_stat_positions_[MapleStat::Id::LUK] = Point<int16_t>(177, 62);
    req_stat_positions_[MapleStat::Id::DEX] = Point<int16_t>(97, 71);
    req_stat_positions_[MapleStat::Id::INT] = Point<int16_t>(177, 71);

    reqset_[false] = Charset(EquipCannot, Charset::Alignment::LEFT);
    reqset_[true] = Charset(EquipCan, Charset::Alignment::LEFT);

    lvset_[false] = Charset(EquipCannot, Charset::Alignment::LEFT);
    lvset_[true] = Charset(Equip["YellowNumber"], Charset::Alignment::LEFT);

    atkincset_[false] =
        Charset(Equip["Summary"]["decline"], Charset::Alignment::RIGHT);
    atkincset_[true] =
        Charset(Equip["Summary"]["incline"], Charset::Alignment::RIGHT);

    jobs_back_ = Equip["Job"]["normal"];
    jobs_[false][0] = Equip["Job"]["disable"]["0"];
    jobs_[false][1] = Equip["Job"]["disable"]["1"];
    jobs_[false][2] = Equip["Job"]["disable"]["2"];
    jobs_[false][3] = Equip["Job"]["disable"]["3"];
    jobs_[false][4] = Equip["Job"]["disable"]["4"];
    jobs_[false][5] = Equip["Job"]["disable"]["5"];
    jobs_[true][0] = Equip["Job"]["enable"]["0"];
    jobs_[true][1] = Equip["Job"]["enable"]["1"];
    jobs_[true][2] = Equip["Job"]["enable"]["2"];
    jobs_[true][3] = Equip["Job"]["enable"]["3"];
    jobs_[true][4] = Equip["Job"]["enable"]["4"];
    jobs_[true][5] = Equip["Job"]["enable"]["5"];

    inv_pos_ = 0;
    inv_pos_preview_ = 0;
}

void EquipTooltip::set_equip(Tooltip::Parent parent, int16_t ivp) {
    if (inv_pos_ == ivp) {
        return;
    }

    inv_pos_ = ivp;
    inv_pos_preview_ = 0;

    const Player &player = Stage::get().get_player();

    InventoryType::Id invtype;

    switch (parent) {
        case Tooltip::Parent::ITEM_INVENTORY:
        case Tooltip::Parent::SHOP: invtype = InventoryType::Id::EQUIP; break;
        case Tooltip::Parent::EQUIP_INVENTORY:
            invtype = InventoryType::Id::EQUIPPED;
            break;
        default: invtype = InventoryType::Id::NONE;
    }

    const Inventory &inventory = player.get_inventory();
    auto oequip = inventory.get_equip(invtype, inv_pos_);
    const CharStats &stats = player.get_stats();

    if (invtype == InventoryType::Id::EQUIP) {
        const int32_t item_id = oequip->get().get_item_id();
        const EquipData &equipdata = EquipData::get(item_id);
        EquipSlot::Id eqslot = equipdata.get_eqslot();

        if (inventory.has_equipped(eqslot)) {
            auto eequip =
                inventory.get_equip(InventoryType::Id::EQUIPPED, eqslot);

            if (eequip) {
                const Equip &equip = *eequip;

                int32_t item_id = equip.get_item_id();

                const EquipData &equipdata = EquipData::get(item_id);
                const ItemData &itemdata = equipdata.get_itemdata();

                height_preview_ = 540;

                item_icon_preview_ = itemdata.get_icon(false);

                for (auto &ms : requirements_) {
                    can_equip_preview_[ms] =
                        stats.get_stat(ms) >= equipdata.get_reqstat(ms);
                    std::string reqstr =
                        std::to_string(equipdata.get_reqstat(ms));

                    if (ms != MapleStat::Id::LEVEL) {
                        reqstr.insert(0, 3 - reqstr.size(), '0');
                    }

                    req_stat_strings_preview_[ms] = reqstr;
                }

                okjobs_preview_.clear();

                switch (equipdata.get_reqstat(MapleStat::Id::JOB)) {
                    case 0:
                        okjobs_preview_.push_back(0);
                        okjobs_preview_.push_back(1);
                        okjobs_preview_.push_back(2);
                        okjobs_preview_.push_back(3);
                        okjobs_preview_.push_back(4);
                        okjobs_preview_.push_back(5);
                        can_equip_preview_[MapleStat::Id::JOB] = true;
                        break;
                    case 1:
                        okjobs_preview_.push_back(1);
                        can_equip_preview_[MapleStat::Id::JOB] =
                            (stats.get_stat(MapleStat::Id::JOB) / 100 == 1)
                            || (stats.get_stat(MapleStat::Id::JOB) / 100 >= 20);
                        break;
                    case 2:
                        okjobs_preview_.push_back(2);
                        can_equip_preview_[MapleStat::Id::JOB] =
                            stats.get_stat(MapleStat::Id::JOB) / 100 == 2;
                        break;
                    case 4:
                        okjobs_preview_.push_back(3);
                        can_equip_preview_[MapleStat::Id::JOB] =
                            stats.get_stat(MapleStat::Id::JOB) / 100 == 3;
                        break;
                    case 8:
                        okjobs_preview_.push_back(4);
                        can_equip_preview_[MapleStat::Id::JOB] =
                            stats.get_stat(MapleStat::Id::JOB) / 100 == 4;
                        break;
                    case 16:
                        okjobs_preview_.push_back(5);
                        can_equip_preview_[MapleStat::Id::JOB] =
                            stats.get_stat(MapleStat::Id::JOB) / 100 == 5;
                        break;
                    default:
                        can_equip_preview_[MapleStat::Id::JOB] = false;
                        break;
                }

                prank_preview_ = equip.get_potrank();

                switch (prank_preview_) {
                    case Equip::Potential::POT_HIDDEN:
                        potflag_preview_ = Text(Text::Font::A11M,
                                                Text::Alignment::CENTER,
                                                Color::Name::RED);
                        potflag_preview_.change_text("(Hidden Potential)");
                        break;
                    case Equip::Potential::POT_RARE:
                        potflag_preview_ = Text(Text::Font::A11M,
                                                Text::Alignment::CENTER,
                                                Color::Name::WHITE);
                        potflag_preview_.change_text("(Rare Item)");
                        break;
                    case Equip::Potential::POT_EPIC:
                        potflag_preview_ = Text(Text::Font::A11M,
                                                Text::Alignment::CENTER,
                                                Color::Name::WHITE);
                        potflag_preview_.change_text("(Epic Item)");
                        break;
                    case Equip::Potential::POT_UNIQUE:
                        potflag_preview_ = Text(Text::Font::A11M,
                                                Text::Alignment::CENTER,
                                                Color::Name::WHITE);
                        potflag_preview_.change_text("(Unique Item)");
                        break;
                    case Equip::Potential::POT_LEGENDARY:
                        potflag_preview_ = Text(Text::Font::A11M,
                                                Text::Alignment::CENTER,
                                                Color::Name::WHITE);
                        potflag_preview_.change_text("(Legendary Item)");
                        break;
                    default: height_preview_ -= 16; break;
                }

                Color::Name namecolor;

                switch (equip.get_quality()) {
                    case EquipQuality::Id::GREY:
                        namecolor = Color::Name::LIGHTGREY;
                        break;
                    case EquipQuality::Id::ORANGE:
                        namecolor = Color::Name::ORANGE;
                        break;
                    case EquipQuality::Id::BLUE:
                        namecolor = Color::Name::MEDIUMBLUE;
                        break;
                    case EquipQuality::Id::VIOLET:
                        namecolor = Color::Name::VIOLET;
                        break;
                    case EquipQuality::Id::GOLD:
                        namecolor = Color::Name::YELLOW;
                        break;
                    default: namecolor = Color::Name::WHITE; break;
                }

                std::string namestr = itemdata.get_name();
                const int8_t reqGender = itemdata.get_gender();
                const bool female = stats.get_female();

                switch (reqGender) {
                    case 0:  // Male
                        namestr += " (M)";
                        break;
                    case 1:  // Female
                        namestr += " (F)";
                        break;
                    case 2:  // Unisex
                    default: break;
                }

                if (equip.get_level() > 0) {
                    namestr.append(" (+");
                    namestr.append(std::to_string(equip.get_level()));
                    namestr.append(")");
                }

                name_preview_ = Text(Text::Font::A12B,
                                     Text::Alignment::LEFT,
                                     namecolor,
                                     namestr,
                                     400);
                atkinc_preview_ = Text(Text::Font::A11M,
                                       Text::Alignment::RIGHT,
                                       Color::Name::DUSTYGRAY,
                                       "ATT INCREASE");

                std::string desctext = itemdata.get_desc();
                has_desc_preview_ = !desctext.empty();

                if (has_desc_preview_) {
                    desc_preview_ = Text(Text::Font::A12M,
                                         Text::Alignment::LEFT,
                                         Color::Name::WHITE,
                                         desctext,
                                         250);
                    height_preview_ += desc_preview_.height() + 10;
                }

                category_preview_ = Text(Text::Font::A11M,
                                         Text::Alignment::LEFT,
                                         Color::Name::WHITE,
                                         "Type: " + equipdata.get_type());

                is_weapon_preview_ = equipdata.is_weapon();

                if (is_weapon_preview_) {
                    const WeaponData &weapon = WeaponData::get(item_id);
                    wepspeed_preview_ =
                        Text(Text::Font::A11M,
                             Text::Alignment::LEFT,
                             Color::Name::WHITE,
                             "Attack Speed: " + weapon.getspeedstring());
                } else {
                    height_preview_ -= 18;
                }

                has_slots_preview_ =
                    (equip.get_slots() > 0) || (equip.get_level() > 0);

                if (has_slots_preview_) {
                    slots_preview_ =
                        Text(Text::Font::A11M,
                             Text::Alignment::LEFT,
                             Color::Name::WHITE,
                             "Remaining Enhancements: "
                                 + std::to_string(equip.get_slots()));

                    std::string vicious = std::to_string(equip.get_vicious());

                    if (equip.get_vicious() > 1) {
                        vicious.append(" (MAX) ");
                    }

                    hammers_preview_ = Text(Text::Font::A11M,
                                            Text::Alignment::LEFT,
                                            Color::Name::WHITE,
                                            "Hammers Applied: " + vicious);
                } else {
                    height_preview_ -= 36;
                }

                stat_labels_preview_.clear();

                for (EquipStat::Id es = EquipStat::Id::STR;
                     es <= EquipStat::Id::JUMP;
                     es = static_cast<EquipStat::Id>(es + 1)) {
                    if (equip.get_stat(es) > 0) {
                        int16_t delta =
                            equip.get_stat(es) - equipdata.get_defstat(es);
                        std::string statstr =
                            std::to_string(equip.get_stat(es));

                        if (delta != 0) {
                            statstr.append(" (");
                            statstr.append((delta < 0) ? "-" : "+");
                            statstr.append(std::to_string(abs(delta)) + ")");
                        }

                        stat_labels_preview_[es] = Text(
                            Text::Font::A11M,
                            Text::Alignment::LEFT,
                            Color::Name::WHITE,
                            EquipStat::names[es] + std::string(": ") + statstr);
                    } else {
                        height_preview_ -= 18;
                    }
                }

                inv_pos_preview_ = 1;
            }
        }
    }

    if (!oequip) {
        return;
    }

    const Equip &equip = *oequip;

    int32_t item_id = equip.get_item_id();

    const EquipData &equipdata = EquipData::get(item_id);
    const ItemData &itemdata = equipdata.get_itemdata();

    height_ = 540;

    item_icon_ = itemdata.get_icon(false);

    for (auto &ms : requirements_) {
        can_equip_[ms] = stats.get_stat(ms) >= equipdata.get_reqstat(ms);
        std::string reqstr = std::to_string(equipdata.get_reqstat(ms));

        if (ms != MapleStat::Id::LEVEL) {
            reqstr.insert(0, 3 - reqstr.size(), '0');
        }

        req_stat_strings_[ms] = reqstr;
    }

    okjobs_.clear();

    switch (equipdata.get_reqstat(MapleStat::Id::JOB)) {
        case 0:
            okjobs_.push_back(0);
            okjobs_.push_back(1);
            okjobs_.push_back(2);
            okjobs_.push_back(3);
            okjobs_.push_back(4);
            okjobs_.push_back(5);
            can_equip_[MapleStat::Id::JOB] = true;
            break;
        case 1:
            okjobs_.push_back(1);
            can_equip_[MapleStat::Id::JOB] =
                (stats.get_stat(MapleStat::Id::JOB) / 100 == 1)
                || (stats.get_stat(MapleStat::Id::JOB) / 100 >= 20);
            break;
        case 2:
            okjobs_.push_back(2);
            can_equip_[MapleStat::Id::JOB] =
                stats.get_stat(MapleStat::Id::JOB) / 100 == 2;
            break;
        case 4:
            okjobs_.push_back(3);
            can_equip_[MapleStat::Id::JOB] =
                stats.get_stat(MapleStat::Id::JOB) / 100 == 3;
            break;
        case 8:
            okjobs_.push_back(4);
            can_equip_[MapleStat::Id::JOB] =
                stats.get_stat(MapleStat::Id::JOB) / 100 == 4;
            break;
        case 16:
            okjobs_.push_back(5);
            can_equip_[MapleStat::Id::JOB] =
                stats.get_stat(MapleStat::Id::JOB) / 100 == 5;
            break;
        default: can_equip_[MapleStat::Id::JOB] = false; break;
    }

    prank_ = equip.get_potrank();

    switch (prank_) {
        case Equip::Potential::POT_HIDDEN:
            potflag_ = Text(Text::Font::A11M,
                            Text::Alignment::CENTER,
                            Color::Name::RED);
            potflag_.change_text("(Hidden Potential)");
            break;
        case Equip::Potential::POT_RARE:
            potflag_ = Text(Text::Font::A11M,
                            Text::Alignment::CENTER,
                            Color::Name::WHITE);
            potflag_.change_text("(Rare Item)");
            break;
        case Equip::Potential::POT_EPIC:
            potflag_ = Text(Text::Font::A11M,
                            Text::Alignment::CENTER,
                            Color::Name::WHITE);
            potflag_.change_text("(Epic Item)");
            break;
        case Equip::Potential::POT_UNIQUE:
            potflag_ = Text(Text::Font::A11M,
                            Text::Alignment::CENTER,
                            Color::Name::WHITE);
            potflag_.change_text("(Unique Item)");
            break;
        case Equip::Potential::POT_LEGENDARY:
            potflag_ = Text(Text::Font::A11M,
                            Text::Alignment::CENTER,
                            Color::Name::WHITE);
            potflag_.change_text("(Legendary Item)");
            break;
        default: height_ -= 16; break;
    }

    Color::Name namecolor;

    switch (equip.get_quality()) {
        case EquipQuality::Id::GREY: namecolor = Color::Name::LIGHTGREY; break;
        case EquipQuality::Id::ORANGE: namecolor = Color::Name::ORANGE; break;
        case EquipQuality::Id::BLUE: namecolor = Color::Name::MEDIUMBLUE; break;
        case EquipQuality::Id::VIOLET: namecolor = Color::Name::VIOLET; break;
        case EquipQuality::Id::GOLD: namecolor = Color::Name::YELLOW; break;
        default: namecolor = Color::Name::WHITE; break;
    }

    std::string namestr = itemdata.get_name();
    const int8_t reqGender = itemdata.get_gender();
    const bool female = stats.get_female();

    switch (reqGender) {
        case 0:  // Male
            namestr += " (M)";
            break;
        case 1:  // Female
            namestr += " (F)";
            break;
        case 2:  // Unisex
        default: break;
    }

    if (equip.get_level() > 0) {
        namestr.append(" (+");
        namestr.append(std::to_string(equip.get_level()));
        namestr.append(")");
    }

    name_ =
        Text(Text::Font::A12B, Text::Alignment::LEFT, namecolor, namestr, 400);
    atkinc_ = Text(Text::Font::A11M,
                   Text::Alignment::RIGHT,
                   Color::Name::DUSTYGRAY,
                   "ATT INCREASE");

    std::string desctext = itemdata.get_desc();
    has_desc_ = !desctext.empty();

    if (has_desc_) {
        desc_ = Text(Text::Font::A12M,
                     Text::Alignment::LEFT,
                     Color::Name::WHITE,
                     desctext,
                     250);
        height_ += desc_.height() + 10;
    }

    category_ = Text(Text::Font::A11M,
                     Text::Alignment::LEFT,
                     Color::Name::WHITE,
                     "Type: " + equipdata.get_type());

    is_weapon_ = equipdata.is_weapon();

    if (is_weapon_) {
        const WeaponData &weapon = WeaponData::get(item_id);
        wepspeed_ = Text(Text::Font::A11M,
                         Text::Alignment::LEFT,
                         Color::Name::WHITE,
                         "Attack Speed: " + weapon.getspeedstring());
    } else {
        height_ -= 18;
    }

    has_slots_ = (equip.get_slots() > 0) || (equip.get_level() > 0);

    if (has_slots_) {
        slots_ = Text(
            Text::Font::A11M,
            Text::Alignment::LEFT,
            Color::Name::WHITE,
            "Remaining Enhancements: " + std::to_string(equip.get_slots()));

        std::string vicious = std::to_string(equip.get_vicious());

        if (equip.get_vicious() > 1) {
            vicious.append(" (MAX) ");
        }

        hammers_ = Text(Text::Font::A11M,
                        Text::Alignment::LEFT,
                        Color::Name::WHITE,
                        "Hammers Applied: " + vicious);
    } else {
        height_ -= 36;
    }

    stat_labels_.clear();

    for (EquipStat::Id es = EquipStat::Id::STR; es <= EquipStat::Id::JUMP;
         es = static_cast<EquipStat::Id>(es + 1)) {
        if (equip.get_stat(es) > 0) {
            int16_t delta = equip.get_stat(es) - equipdata.get_defstat(es);
            std::string statstr = std::to_string(equip.get_stat(es));

            if (delta != 0) {
                statstr.append(" (");
                statstr.append((delta < 0) ? "-" : "+");
                statstr.append(std::to_string(abs(delta)) + ")");
            }

            stat_labels_[es] =
                Text(Text::Font::A11M,
                     Text::Alignment::LEFT,
                     Color::Name::WHITE,
                     EquipStat::names[es] + std::string(": ") + statstr);
        } else {
            height_ -= 18;
        }
    }
}

void EquipTooltip::draw(Point<int16_t> pos) const {
    if (inv_pos_ == 0) {
        return;
    }

    draw_preview(pos);

    int16_t max_width = Constants::Constants::get().get_viewwidth();
    int16_t max_height = Constants::Constants::get().get_viewheight();
    int16_t cur_width = pos.x() + top_.width();
    int16_t cur_height = pos.y() + 36;

    if (inv_pos_preview_ == 1) {
        cur_width += top_.width();
    }

    if (inv_pos_preview_ == 1) {
        cur_height += (height_ > height_preview_) ? height_ : height_preview_;
    } else {
        cur_height += height_;
    }

    int16_t adj_x = cur_width - max_width;
    int16_t adj_y = cur_height - max_height;

    if (adj_x > 0) {
        pos.shift_x(adj_x * -1);
    }

    if (adj_y > 0) {
        pos.shift_y(adj_y * -1);
    }

    top_.draw(pos);
    mid_.draw(
        DrawArgument(pos + Point<int16_t>(0, 13), Point<int16_t>(0, height_)));
    bot_.draw(pos + Point<int16_t>(0, height_ + 13));
    cover_.draw(pos);

    name_.draw(pos + Point<int16_t>(17, 7));

    if (prank_ != Equip::Potential::POT_NONE) {
        potflag_.draw(pos + Point<int16_t>(130, 20));
        pos.shift_y(16);
    }

    pos.shift_y(44);

    line_.draw(pos);

    atkinc_.draw(pos + Point<int16_t>(248, 4));
    base_.draw(pos + Point<int16_t>(12, 10));
    type_[false].draw(pos + Point<int16_t>(12, 10));
    item_icon_.draw(DrawArgument(pos + Point<int16_t>(18, 82), 2.0f, 2.0f));
    potential_[prank_].draw(pos + Point<int16_t>(12, 10));
    item_cover_.draw(pos + Point<int16_t>(12, 10));

    int16_t atkincnum = 0;
    std::string atkincstr = std::to_string(atkincnum);
    bool atkinc_pos = true;

    if (can_equip_[MapleStat::Id::JOB]) {
        if (atkincnum < 0) {
            atkincstr = "m" + atkincstr;
            atkinc_pos = false;
        } else if (atkincnum > 0) {
            atkincstr = "p" + atkincstr;
            atkinc_pos = true;
        } else {
            atkinc_pos = true;
        }
    } else {
        atkincstr = "m";
        atkinc_pos = false;
    }

    atkincset_[atkinc_pos].draw(atkincstr, 11, pos + Point<int16_t>(239, 26));

    pos.shift_y(12);

    for (MapleStat::Id ms : requirements_) {
        Point<int16_t> reqpos = req_stat_positions_[ms];
        bool reqok = can_equip_[ms];
        req_stat_textures_[ms][reqok].draw(pos + reqpos);

        if (ms != MapleStat::Id::LEVEL) {
            reqset_[reqok].draw(req_stat_strings_[ms],
                                6,
                                pos + reqpos + Point<int16_t>(54, 0));
        } else {
            lvset_[reqok].draw(req_stat_strings_[ms],
                               6,
                               pos + reqpos + Point<int16_t>(54, 0));
        }
    }

    pos.shift_y(88);

    Point<int16_t> job_position(pos + Point<int16_t>(10, 14));
    jobs_back_.draw(job_position);

    for (const auto &jbit : okjobs_) {
        jobs_[can_equip_[MapleStat::Id::JOB]].at(jbit).draw(job_position);
    }

    line_.draw(pos + Point<int16_t>(0, 47));

    pos.shift_y(49);

    int16_t stat_x = 13;
    int16_t stat_y = 15;

    category_.draw(pos + Point<int16_t>(stat_x, 0));

    pos.shift_y(stat_y);

    if (is_weapon_) {
        wepspeed_.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
    }

    for (const Text &label : stat_labels_.values()) {
        if (label.empty()) {
            continue;
        }

        label.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
    }

    if (has_slots_) {
        slots_.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
        hammers_.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
    }

    if (has_desc_) {
        pos.shift_y(13);
        line_.draw(pos);
        desc_.draw(pos + Point<int16_t>(9, 3));
    }
}

void EquipTooltip::draw_preview(Point<int16_t> pos) const {
    if (inv_pos_preview_ == 0) {
        return;
    }

    pos.shift_x(top_.width());

    int16_t max_width = Constants::Constants::get().get_viewwidth();
    int16_t max_height = Constants::Constants::get().get_viewheight();
    int16_t cur_width = pos.x() + top_.width();
    int16_t cur_height = pos.y() + height_preview_ + 36;

    int16_t adj_x = cur_width - max_width;
    int16_t adj_y = cur_height - max_height;

    if (adj_x > 0) {
        pos.shift_x(adj_x * -1);
    }

    if (adj_y > 0) {
        pos.shift_y(adj_y * -1);
    }

    top_.draw(pos);
    mid_.draw(DrawArgument(pos + Point<int16_t>(0, 13),
                           Point<int16_t>(0, height_preview_)));
    bot_.draw(pos + Point<int16_t>(0, height_preview_ + 13));
    cover_.draw(pos);

    name_preview_.draw(pos + Point<int16_t>(17, 7));

    if (prank_preview_ != Equip::Potential::POT_NONE) {
        potflag_preview_.draw(pos + Point<int16_t>(130, 20));
        pos.shift_y(16);
    }

    pos.shift_y(44);

    line_.draw(pos);

    atkinc_preview_.draw(pos + Point<int16_t>(248, 4));
    base_.draw(pos + Point<int16_t>(12, 10));
    type_[false].draw(pos + Point<int16_t>(12, 10));
    item_icon_preview_.draw(
        DrawArgument(pos + Point<int16_t>(18, 82), 2.0f, 2.0f));
    potential_[prank_preview_].draw(pos + Point<int16_t>(12, 10));
    item_cover_.draw(pos + Point<int16_t>(12, 10));

    int16_t atkincnum = 0;
    std::string atkincstr = std::to_string(atkincnum);
    bool atkinc_pos = true;

    if (can_equip_preview_[MapleStat::Id::JOB]) {
        if (atkincnum < 0) {
            atkincstr = "m" + atkincstr;
            atkinc_pos = false;
        } else if (atkincnum > 0) {
            atkincstr = "p" + atkincstr;
            atkinc_pos = true;
        } else {
            atkinc_pos = true;
        }
    } else {
        atkincstr = "m";
        atkinc_pos = false;
    }

    atkincset_[atkinc_pos].draw(atkincstr, 11, pos + Point<int16_t>(239, 26));

    pos.shift_y(12);

    for (MapleStat::Id ms : requirements_) {
        Point<int16_t> reqpos = req_stat_positions_[ms];
        bool reqok = can_equip_preview_[ms];
        req_stat_textures_[ms][reqok].draw(pos + reqpos);

        if (ms != MapleStat::Id::LEVEL) {
            reqset_[reqok].draw(req_stat_strings_preview_[ms],
                                6,
                                pos + reqpos + Point<int16_t>(54, 0));
        } else {
            lvset_[reqok].draw(req_stat_strings_preview_[ms],
                               6,
                               pos + reqpos + Point<int16_t>(54, 0));
        }
    }

    pos.shift_y(88);

    Point<int16_t> job_position(pos + Point<int16_t>(10, 14));
    jobs_back_.draw(job_position);

    for (const auto &jbit : okjobs_preview_) {
        jobs_[can_equip_preview_[MapleStat::Id::JOB]].at(jbit).draw(
            job_position);
    }

    line_.draw(pos + Point<int16_t>(0, 47));

    pos.shift_y(49);

    int16_t stat_x = 13;
    int16_t stat_y = 15;

    category_preview_.draw(pos + Point<int16_t>(stat_x, 0));

    pos.shift_y(stat_y);

    if (is_weapon_preview_) {
        wepspeed_preview_.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
    }

    for (const Text &label : stat_labels_preview_.values()) {
        if (label.empty()) {
            continue;
        }

        label.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
    }

    if (has_slots_preview_) {
        slots_preview_.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
        hammers_preview_.draw(pos + Point<int16_t>(stat_x, 0));
        pos.shift_y(stat_y);
    }

    if (has_desc_preview_) {
        pos.shift_y(-4);
        line_.draw(pos);
        desc_preview_.draw(pos + Point<int16_t>(9, 8));
    }
}
}  // namespace ms