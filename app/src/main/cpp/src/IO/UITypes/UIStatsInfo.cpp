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
#include "UIStatsInfo.h"

#include <nlnx/nx.hpp>

#include "../../Gameplay/Stage.h"
#include "../../Net/Packets/PlayerPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "UINotice.h"

namespace ms {
namespace {
auto fn_spend_ap = []<typename... T>(T && ...args) {
    SpendApPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIStatsInfo::UIStatsInfo(const CharStats &st) :
    UIDragElement<PosSTATS>(Point<int16_t>(212, 20)),
    stats_(st) {
    nl::node close = nl::nx::ui["Basic.img"]["BtClose3"];
    nl::node Stat = nl::nx::ui["UIWindow4.img"]["Stat"];
    nl::node main = Stat["main"];
    nl::node detail = Stat["detail"];
    nl::node abilityTitle = detail["abilityTitle"];
    nl::node metierLine = detail["metierLine"];

    sprites_.emplace_back(main["backgrnd"]);
    sprites_.emplace_back(main["backgrnd2"]);
    sprites_.emplace_back(main["backgrnd3"]);

    textures_detail_.emplace_back(detail["backgrnd"]);
    textures_detail_.emplace_back(detail["backgrnd2"]);
    textures_detail_.emplace_back(detail["backgrnd3"]);
    textures_detail_.emplace_back(detail["backgrnd4"]);

    abilities_[Ability::RARE] = abilityTitle["rare"]["0"];
    abilities_[Ability::EPIC] = abilityTitle["epic"]["0"];
    abilities_[Ability::UNIQUE] = abilityTitle["unique"]["0"];
    abilities_[Ability::LEGENDARY] = abilityTitle["legendary"]["0"];
    abilities_[Ability::NONE] = abilityTitle["normal"]["0"];

    inner_ability_[true] = metierLine["activated"]["0"];
    inner_ability_[false] = metierLine["disabled"]["0"];

    buttons_[Buttons::BT_CLOSE] =
        std::make_unique<MapleButton>(close, Point<int16_t>(190, 6));
    buttons_[Buttons::BT_HP] = std::make_unique<MapleButton>(main["BtHpUp"]);
    buttons_[Buttons::BT_MP] = std::make_unique<MapleButton>(
        main["BtHpUp"],
        Point<int16_t>(0,
                       18));  // TODO: "BtMpUp" not Working
    buttons_[Buttons::BT_STR] = std::make_unique<MapleButton>(
        main["BtHpUp"],
        Point<int16_t>(0,
                       87));  // TODO: "BtStrUp" not working
    buttons_[Buttons::BT_DEX] = std::make_unique<MapleButton>(
        main["BtHpUp"],
        Point<int16_t>(0,
                       105));  // TODO: "BtDexUp" not working
    buttons_[Buttons::BT_INT] = std::make_unique<MapleButton>(
        main["BtHpUp"],
        Point<int16_t>(0,
                       123));  // TODO: "BtIntUp" not working
    buttons_[Buttons::BT_LUK] = std::make_unique<MapleButton>(
        main["BtHpUp"],
        Point<int16_t>(0,
                       141));  // TODO: "BtLukUp" not working
    buttons_[Buttons::BT_AUTO] = std::make_unique<MapleButton>(main["BtAuto"]);
    buttons_[Buttons::BT_HYPER_STAT_OPEN] =
        std::make_unique<MapleButton>(main["BtHyperStatOpen"]);
    buttons_[Buttons::BT_HYPER_STAT_CLOSE] =
        std::make_unique<MapleButton>(main["BtHyperStatClose"]);
    buttons_[Buttons::BT_DETAIL_OPEN] =
        std::make_unique<MapleButton>(main["BtDetailOpen"]);
    buttons_[Buttons::BT_DETAIL_CLOSE] =
        std::make_unique<MapleButton>(main["BtDetailClose"]);
    buttons_[Buttons::BT_ABILITY] =
        std::make_unique<MapleButton>(detail["BtAbility"],
                                      Point<int16_t>(212, 0));
    buttons_[Buttons::BT_DETAIL_DETAIL_CLOSE] =
        std::make_unique<MapleButton>(detail["BtHpUp"], Point<int16_t>(212, 0));

    buttons_[Buttons::BT_HYPER_STAT_OPEN]->set_active(false);
    buttons_[Buttons::BT_DETAIL_CLOSE]->set_active(false);
    buttons_[Buttons::BT_ABILITY]->set_active(false);
    buttons_[Buttons::BT_ABILITY]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_DETAIL_DETAIL_CLOSE]->set_active(false);

    update_ap();

    // Normal
    for (size_t i = StatLabel::NAME; i <= LUK; i++) {
        stat_labels_[i] =
            Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    }

    stat_labels_[StatLabel::AP] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);

    stat_offsets_[StatLabel::NAME] = Point<int16_t>(73, 26);
    stat_offsets_[StatLabel::JOB] = Point<int16_t>(74, 44);
    stat_offsets_[StatLabel::GUILD] = Point<int16_t>(74, 63);
    stat_offsets_[StatLabel::FAME] = Point<int16_t>(74, 80);
    stat_offsets_[StatLabel::DAMAGE] = Point<int16_t>(74, 98);
    stat_offsets_[StatLabel::HP] = Point<int16_t>(74, 116);
    stat_offsets_[StatLabel::MP] = Point<int16_t>(74, 134);
    stat_offsets_[StatLabel::AP] = Point<int16_t>(91, 175);
    stat_offsets_[StatLabel::STR] = Point<int16_t>(73, 204);
    stat_offsets_[StatLabel::DEX] = Point<int16_t>(73, 222);
    stat_offsets_[StatLabel::INT] = Point<int16_t>(73, 240);
    stat_offsets_[StatLabel::LUK] = Point<int16_t>(73, 258);

    // Detailed
    stat_labels_[StatLabel::DAMAGE_DETAILED] =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::DAMAGE_BONUS] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::BOSS_DAMAGE] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::FINAL_DAMAGE] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::IGNORE_DEFENSE] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::CRITICAL_RATE] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::CRITICAL_DAMAGE] =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::STATUS_RESISTANCE] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::KNOCKBACK_RESISTANCE] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::DEFENSE] =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::SPEED] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::JUMP] =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::EMPEROR);
    stat_labels_[StatLabel::HONOR] =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::EMPEROR);

    stat_offsets_[StatLabel::DAMAGE_DETAILED] = Point<int16_t>(73, 38);
    stat_offsets_[StatLabel::DAMAGE_BONUS] = Point<int16_t>(100, 56);
    stat_offsets_[StatLabel::BOSS_DAMAGE] = Point<int16_t>(196, 56);
    stat_offsets_[StatLabel::FINAL_DAMAGE] = Point<int16_t>(100, 74);
    stat_offsets_[StatLabel::IGNORE_DEFENSE] = Point<int16_t>(196, 74);
    stat_offsets_[StatLabel::CRITICAL_RATE] = Point<int16_t>(100, 92);
    stat_offsets_[StatLabel::CRITICAL_DAMAGE] = Point<int16_t>(73, 110);
    stat_offsets_[StatLabel::STATUS_RESISTANCE] = Point<int16_t>(100, 128);
    stat_offsets_[StatLabel::KNOCKBACK_RESISTANCE] = Point<int16_t>(196, 128);
    stat_offsets_[StatLabel::DEFENSE] = Point<int16_t>(73, 146);
    stat_offsets_[StatLabel::SPEED] = Point<int16_t>(100, 164);
    stat_offsets_[StatLabel::JUMP] = Point<int16_t>(196, 164);
    stat_offsets_[StatLabel::HONOR] = Point<int16_t>(73, 283);

    update_all_stats();
    update_stat(MapleStat::Id::JOB);
    update_stat(MapleStat::Id::FAME);

    dimension_ = Point<int16_t>(212, 318);
    show_detail_ = false;
}

void UIStatsInfo::draw(float alpha) const {
    UIElement::draw_sprites(alpha);

    if (show_detail_) {
        Point<int16_t> detail_pos(position_ + Point<int16_t>(212, 0));

        textures_detail_[0].draw(detail_pos + Point<int16_t>(0, -1));
        textures_detail_[1].draw(detail_pos);
        textures_detail_[2].draw(detail_pos);
        textures_detail_[3].draw(detail_pos);

        abilities_[Ability::NONE].draw(DrawArgument(detail_pos));

        inner_ability_[false].draw(detail_pos);
        inner_ability_[false].draw(detail_pos + Point<int16_t>(0, 19));
        inner_ability_[false].draw(detail_pos + Point<int16_t>(0, 38));
    }

    size_t last = show_detail_ ? StatLabel::NUM_LABELS : StatLabel::NUM_NORMAL;

    for (size_t i = 0; i < last; i++) {
        Point<int16_t> labelpos = position_ + stat_offsets_[i];

        if (i >= StatLabel::NUM_NORMAL) {
            labelpos.shift_x(213);
        }

        stat_labels_[i].draw(labelpos);
    }

    UIElement::draw_buttons(alpha);
}

void UIStatsInfo::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        deactivate();
    }
}

bool UIStatsInfo::is_in_range(Point<int16_t> cursorpos) const {
    Point<int16_t> pos_adj;

    if (show_detail_) {
        pos_adj = Point<int16_t>(211, 25);
    } else {
        pos_adj = Point<int16_t>(0, 0);
    }

    auto bounds =
        Rectangle<int16_t>(position_, position_ + dimension_ + pos_adj);
    return bounds.contains(cursorpos);
}

UIElement::Type UIStatsInfo::get_type() const {
    return TYPE;
}

void UIStatsInfo::update_all_stats() {
    update_simple(AP, MapleStat::Id::AP);

    if (has_ap_ ^ (stats_.get_stat(MapleStat::Id::AP) > 0)) {
        update_ap();
    }

    stat_labels_[StatLabel::NAME].change_text(stats_.get_name());
    stat_labels_[StatLabel::GUILD].change_text("-");
    stat_labels_[StatLabel::HP].change_text(
        std::to_string(stats_.get_stat(MapleStat::Id::HP)) + " / "
        + std::to_string(stats_.get_total(EquipStat::Id::HP)));
    stat_labels_[StatLabel::MP].change_text(
        std::to_string(stats_.get_stat(MapleStat::Id::MP)) + " / "
        + std::to_string(stats_.get_total(EquipStat::Id::MP)));

    update_basevstotal(StatLabel::STR, MapleStat::Id::STR, EquipStat::Id::STR);
    update_basevstotal(StatLabel::DEX, MapleStat::Id::DEX, EquipStat::Id::DEX);
    update_basevstotal(StatLabel::INT, MapleStat::Id::INT, EquipStat::Id::INT);
    update_basevstotal(StatLabel::LUK, MapleStat::Id::LUK, EquipStat::Id::LUK);

    stat_labels_[StatLabel::DAMAGE].change_text(
        std::to_string(stats_.get_mindamage()) + " ~ "
        + std::to_string(stats_.get_maxdamage()));

    if (stats_.is_damage_buffed()) {
        stat_labels_[StatLabel::DAMAGE].change_color(Color::Name::RED);
    } else {
        stat_labels_[StatLabel::DAMAGE].change_color(Color::Name::EMPEROR);
    }

    stat_labels_[StatLabel::DAMAGE_DETAILED].change_text(
        std::to_string(stats_.get_mindamage()) + " ~ "
        + std::to_string(stats_.get_maxdamage()));
    stat_labels_[StatLabel::DAMAGE_BONUS].change_text("0%");
    stat_labels_[StatLabel::BOSS_DAMAGE].change_text(
        std::to_string(static_cast<int32_t>(stats_.get_bossdmg() * 100)) + "%");
    stat_labels_[StatLabel::FINAL_DAMAGE].change_text("0%");
    stat_labels_[StatLabel::IGNORE_DEFENSE].change_text(
        std::to_string(static_cast<int32_t>(stats_.get_ignoredef())) + "%");
    stat_labels_[StatLabel::CRITICAL_RATE].change_text(
        std::to_string(static_cast<int32_t>(stats_.get_critical() * 100))
        + "%");
    stat_labels_[StatLabel::CRITICAL_DAMAGE].change_text("0.00%");
    stat_labels_[StatLabel::STATUS_RESISTANCE].change_text(
        std::to_string(static_cast<int32_t>(stats_.get_resistance())));
    stat_labels_[StatLabel::KNOCKBACK_RESISTANCE].change_text("0%");

    update_buffed(StatLabel::DEFENSE, EquipStat::Id::WDEF);

    stat_labels_[StatLabel::SPEED].change_text(
        std::to_string(stats_.get_total(EquipStat::Id::SPEED)) + "%");
    stat_labels_[StatLabel::JUMP].change_text(
        std::to_string(stats_.get_total(EquipStat::Id::JUMP)) + "%");
    stat_labels_[StatLabel::HONOR].change_text(
        std::to_string(stats_.get_honor()));
}

void UIStatsInfo::update_stat(MapleStat::Id stat) {
    switch (stat) {
        case MapleStat::Id::JOB:
            stat_labels_[StatLabel::JOB].change_text(stats_.get_jobname());
            break;
        case MapleStat::Id::FAME:
            update_simple(StatLabel::FAME, MapleStat::Id::FAME);
            break;
    }
}

Button::State UIStatsInfo::button_pressed(uint16_t id) {
    const Player &player = Stage::get().get_player();

    switch (id) {
        case Buttons::BT_CLOSE: deactivate(); break;
        case Buttons::BT_HP: send_apup(MapleStat::Id::HP); break;
        case Buttons::BT_MP: send_apup(MapleStat::Id::MP); break;
        case Buttons::BT_STR: send_apup(MapleStat::Id::STR); break;
        case Buttons::BT_DEX: send_apup(MapleStat::Id::DEX); break;
        case Buttons::BT_INT: send_apup(MapleStat::Id::INT); break;
        case Buttons::BT_LUK: send_apup(MapleStat::Id::LUK); break;
        case Buttons::BT_AUTO: {
            uint16_t autostr = 0;
            uint16_t autodex = 0;
            uint16_t autoint = 0;
            uint16_t autoluk = 0;
            uint16_t nowap = stats_.get_stat(MapleStat::Id::AP);
            EquipStat::Id id = player.get_stats().get_job().get_primary(
                player.get_weapontype());

            switch (id) {
                case EquipStat::Id::STR: autostr = nowap; break;
                case EquipStat::Id::DEX: autodex = nowap; break;
                case EquipStat::Id::INT: autoint = nowap; break;
                case EquipStat::Id::LUK: autoluk = nowap; break;
            }

            std::string message = "Your AP will be distributed as follows:\\r"
                                  "\\nSTR : +"
                                  + std::to_string(autostr) + "\\nDEX : +"
                                  + std::to_string(autodex) + "\\nINT : +"
                                  + std::to_string(autoint) + "\\nLUK : +"
                                  + std::to_string(autoluk) + "\\r\\n";

            std::function<void(bool)> yesnohandler =
                [&, autostr, autodex, autoint, autoluk](bool yes) {
                    if (yes) {
                        if (autostr > 0) {
                            for (size_t i = 0; i < autostr; i++) {
                                send_apup(MapleStat::Id::STR);
                            }
                        }

                        if (autodex > 0) {
                            for (size_t i = 0; i < autodex; i++) {
                                send_apup(MapleStat::Id::DEX);
                            }
                        }

                        if (autoint > 0) {
                            for (size_t i = 0; i < autoint; i++) {
                                send_apup(MapleStat::Id::INT);
                            }
                        }

                        if (autoluk > 0) {
                            for (size_t i = 0; i < autoluk; i++) {
                                send_apup(MapleStat::Id::LUK);
                            }
                        }
                    }
                };

            UI::get().emplace<UIYesNo>(message,
                                       yesnohandler,
                                       Text::Alignment::LEFT);
        } break;
        case Buttons::BT_HYPER_STAT_OPEN: break;
        case Buttons::BT_HYPER_STAT_CLOSE: {
            if (player.get_level() < 140) {
                UI::get().emplace<UIOk>(
                    "You can use the Hyper Stat at Lv. 140 and above.",
                    [](bool) {});
            }
        } break;
        case Buttons::BT_DETAIL_OPEN: set_detail(true); break;
        case Buttons::BT_DETAIL_CLOSE:
        case Buttons::BT_DETAIL_DETAIL_CLOSE: set_detail(false); break;
        case Buttons::BT_ABILITY: break;
        default: break;
    }

    return Button::State::NORMAL;
}

void UIStatsInfo::send_apup(MapleStat::Id stat) const {
    fn_spend_ap(stat);
    UI::get().disable();
}

void UIStatsInfo::set_detail(bool enabled) {
    show_detail_ = enabled;

    buttons_[Buttons::BT_DETAIL_OPEN]->set_active(!enabled);
    buttons_[Buttons::BT_DETAIL_CLOSE]->set_active(enabled);
    buttons_[Buttons::BT_ABILITY]->set_active(enabled);
    buttons_[Buttons::BT_DETAIL_DETAIL_CLOSE]->set_active(enabled);
}

void UIStatsInfo::update_ap() {
    bool nowap = stats_.get_stat(MapleStat::Id::AP) > 0;
    Button::State newstate =
        nowap ? Button::State::NORMAL : Button::State::DISABLED;

    for (int i = Buttons::BT_HP; i <= Buttons::BT_AUTO; i++) {
        buttons_[i]->set_state(newstate);
    }

    has_ap_ = nowap;
}

void UIStatsInfo::update_simple(StatLabel label, MapleStat::Id stat) {
    stat_labels_[label].change_text(std::to_string(stats_.get_stat(stat)));
}

void UIStatsInfo::update_basevstotal(StatLabel label,
                                     MapleStat::Id bstat,
                                     EquipStat::Id tstat) {
    int32_t base = stats_.get_stat(bstat);
    int32_t total = stats_.get_total(tstat);
    int32_t delta = total - base;

    std::string stattext = std::to_string(total);

    if (delta) {
        stattext += " (" + std::to_string(base);

        if (delta > 0) {
            stattext += "+" + std::to_string(delta);
        } else if (delta < 0) {
            stattext += "-" + std::to_string(-delta);
        }

        stattext += ")";
    }

    stat_labels_[label].change_text(stattext);
}

void UIStatsInfo::update_buffed(StatLabel label, EquipStat::Id stat) {
    int32_t total = stats_.get_total(stat);
    int32_t delta = stats_.get_buffdelta(stat);

    std::string stattext = std::to_string(total);

    if (delta) {
        stattext += " (" + std::to_string(total - delta);

        if (delta > 0) {
            stattext += "+" + std::to_string(delta);

            stat_labels_[label].change_color(Color::Name::RED);
        } else if (delta < 0) {
            stattext += "-" + std::to_string(-delta);

            stat_labels_[label].change_color(Color::Name::BLUE);
        }

        stattext += ")";
    }

    stat_labels_[label].change_text(stattext);
}
}  // namespace ms