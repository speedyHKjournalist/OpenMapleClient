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
#include "UIStatusBar.h"

#include <nlnx/nx.hpp>

#include "../../Character/ExpTable.h"
#include "../../Gameplay/Stage.h"
#include "../../Net/Packets/GameplayPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "UIChannel.h"
#include "UICharInfo.h"
#include "UIChat.h"
#include "UIEquipInventory.h"
#include "UIEvent.h"
#include "UIItemInventory.h"
#include "UIJoypad.h"
#include "UIKeyConfig.h"
#include "UIOptionMenu.h"
#include "UIQuestLog.h"
#include "UIQuit.h"
#include "UISkillBook.h"
#include "UIStatsInfo.h"
#include "UIUserList.h"
#include "UIWorldSelect.h"

namespace ms {
namespace {
auto fn_enter_cashshop = []<typename... T>(T && ...args) {
    EnterCashShopPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIStatusBar::UIStatusBar(const CharStats &st, uint8_t channel_count) :
    stats_(st),
    channel_count_(channel_count) {
    quickslot_active_ = false;
    quickslot_adj_ = Point<int16_t>(QUICKSLOT_MAX_, 0);
    VWIDTH_ = Constants::Constants::get().get_viewwidth();
    VHEIGHT_ = Constants::Constants::get().get_viewheight();

    menu_active_ = false;
    setting_active_ = false;
    community_active_ = false;
    character_active_ = false;
    event_active_ = false;

    std::string stat = "status";

    if (VWIDTH_ == 800) {
        stat += "800";
    }

    nl::node mainBar = nl::nx::ui["StatusBar3.img"]["mainBar"];
    nl::node status = mainBar[stat];
    nl::node EXPBar = mainBar["EXPBar"];
    nl::node EXPBarRes = EXPBar[VWIDTH_];
    nl::node menu = mainBar["menu"];
    nl::node quickSlot = mainBar["quickSlot"];
    nl::node submenu = mainBar["submenu"];

    exp_pos_ = Point<int16_t>(0, 87);

    sprites_.emplace_back(
        EXPBar["backgrnd"],
        DrawArgument(Point<int16_t>(0, 87), Point<int16_t>(VWIDTH_, 0)));
    sprites_.emplace_back(EXPBarRes["layer:back"], exp_pos_);

    int16_t exp_max = VWIDTH_ - 16;

    exp_bar_ = Gauge(Gauge::Type::GAME,
                     EXPBarRes.resolve("layer:gauge"),
                     EXPBarRes.resolve("layer:cover"),
                     EXPBar.resolve("layer:effect"),
                     exp_max,
                     0.0f);

    int16_t pos_adj = 0;

    if (VWIDTH_ == 1280) {
        pos_adj = 87;
    } else if (VWIDTH_ == 1366) {
        pos_adj = 171;
    } else if (VWIDTH_ == 1920) {
        pos_adj = 448;
    }

    if (VWIDTH_ == 1024) {
        quickslot_min_ = 1;
    } else {
        quickslot_min_ = 0;
    }

    if (VWIDTH_ == 800) {
        hpmp_pos_ = Point<int16_t>(412, 40);
        hpset_pos_ = Point<int16_t>(530, 70);
        mpset_pos_ = Point<int16_t>(528, 86);
        statset_pos_ = Point<int16_t>(427, 111);
        levelset_pos_ = Point<int16_t>(461, 48);
        namelabel_pos_ = Point<int16_t>(487, 40);
        quickslot_pos_ = Point<int16_t>(579, 0);

        // Menu
        menu_pos_ = Point<int16_t>(682, -280);
        setting_pos_ = menu_pos_ + Point<int16_t>(0, 168);
        community_pos_ = menu_pos_ + Point<int16_t>(-26, 196);
        character_pos_ = menu_pos_ + Point<int16_t>(-61, 168);
        event_pos_ = menu_pos_ + Point<int16_t>(-94, 252);
    } else {
        hpmp_pos_ = Point<int16_t>(416 + pos_adj, 40);
        hpset_pos_ = Point<int16_t>(550 + pos_adj, 70);
        mpset_pos_ = Point<int16_t>(546 + pos_adj, 86);
        statset_pos_ = Point<int16_t>(539 + pos_adj, 111);
        levelset_pos_ = Point<int16_t>(465 + pos_adj, 48);
        namelabel_pos_ = Point<int16_t>(493 + pos_adj, 40);
        quickslot_pos_ = Point<int16_t>(628 + pos_adj, 37);

        // Menu
        menu_pos_ = Point<int16_t>(720 + pos_adj, -280);
        setting_pos_ = menu_pos_ + Point<int16_t>(0, 168);
        community_pos_ = menu_pos_ + Point<int16_t>(-26, 196);
        character_pos_ = menu_pos_ + Point<int16_t>(-61, 168);
        event_pos_ = menu_pos_ + Point<int16_t>(-94, 252);
    }

    if (VWIDTH_ == 1280) {
        statset_pos_ = Point<int16_t>(580 + pos_adj, 111);
        quickslot_pos_ = Point<int16_t>(622 + pos_adj, 37);

        // Menu
        menu_pos_ += Point<int16_t>(-7, 0);
        setting_pos_ += Point<int16_t>(-7, 0);
        community_pos_ += Point<int16_t>(-7, 0);
        character_pos_ += Point<int16_t>(-7, 0);
        event_pos_ += Point<int16_t>(-7, 0);
    } else if (VWIDTH_ == 1366) {
        quickslot_pos_ = Point<int16_t>(623 + pos_adj, 37);

        // Menu
        menu_pos_ += Point<int16_t>(-5, 0);
        setting_pos_ += Point<int16_t>(-5, 0);
        community_pos_ += Point<int16_t>(-5, 0);
        character_pos_ += Point<int16_t>(-5, 0);
        event_pos_ += Point<int16_t>(-5, 0);
    } else if (VWIDTH_ == 1920) {
        quickslot_pos_ = Point<int16_t>(900 + pos_adj, 37);

        // Menu
        menu_pos_ += Point<int16_t>(272, 0);
        setting_pos_ += Point<int16_t>(272, 0);
        community_pos_ += Point<int16_t>(272, 0);
        character_pos_ += Point<int16_t>(272, 0);
        event_pos_ += Point<int16_t>(272, 0);
    }

    hpmp_sprites_.emplace_back(status["backgrnd"],
                               hpmp_pos_ - Point<int16_t>(1, 0));
    hpmp_sprites_.emplace_back(status["layer:cover"],
                               hpmp_pos_ - Point<int16_t>(1, 0));

    if (VWIDTH_ == 800) {
        hpmp_sprites_.emplace_back(status["layer:Lv"], hpmp_pos_);
    } else {
        hpmp_sprites_.emplace_back(status["layer:Lv"],
                                   hpmp_pos_ - Point<int16_t>(1, 0));
    }

    int16_t hpmp_max = 139;

    if (VWIDTH_ > 800) {
        hpmp_max += 30;
    }

    hp_bar_ = Gauge(Gauge::Type::GAME,
                    status.resolve("gauge/hp/layer:0"),
                    hpmp_max,
                    0.0f);
    mp_bar_ = Gauge(Gauge::Type::GAME,
                    status.resolve("gauge/mp/layer:0"),
                    hpmp_max,
                    0.0f);

    stat_set_ = Charset(EXPBar["number"], Charset::Alignment::RIGHT);
    hpmp_set_ = Charset(status["gauge"]["number"], Charset::Alignment::RIGHT);
    level_set_ = Charset(status["lvNumber"], Charset::Alignment::LEFT);

    name_label_ = OutlinedText(Text::Font::A13M,
                               Text::Alignment::LEFT,
                               Color::Name::GALLERY,
                               Color::Name::TUNA);

    quickslot_[0] = quickSlot["backgrnd"];
    quickslot_[1] = quickSlot["layer:cover"];

    Point<int16_t> buttonPos = Point<int16_t>(591 + pos_adj, 73);

    if (VWIDTH_ == 1024) {
        buttonPos += Point<int16_t>(38, 0);
    } else if (VWIDTH_ == 1280) {
        buttonPos += Point<int16_t>(31, 0);
    } else if (VWIDTH_ == 1366) {
        buttonPos += Point<int16_t>(33, 0);
    } else if (VWIDTH_ == 1920) {
        buttonPos += Point<int16_t>(310, 0);
    }

    buttons_[Buttons::BT_CASHSHOP] =
        std::make_unique<MapleButton>(menu["button:CashShop"], buttonPos);
    buttons_[Buttons::BT_MENU] =
        std::make_unique<MapleButton>(menu["button:Menu"], buttonPos);
    buttons_[Buttons::BT_OPTIONS] =
        std::make_unique<MapleButton>(menu["button:Setting"], buttonPos);
    buttons_[Buttons::BT_CHARACTER] =
        std::make_unique<MapleButton>(menu["button:Character"], buttonPos);
    buttons_[Buttons::BT_COMMUNITY] =
        std::make_unique<MapleButton>(menu["button:Community"], buttonPos);
    buttons_[Buttons::BT_EVENT] =
        std::make_unique<MapleButton>(menu["button:Event"], buttonPos);

    if (quickslot_active_ && VWIDTH_ > 800) {
        buttons_[Buttons::BT_CASHSHOP]->set_active(false);
        buttons_[Buttons::BT_MENU]->set_active(false);
        buttons_[Buttons::BT_OPTIONS]->set_active(false);
        buttons_[Buttons::BT_CHARACTER]->set_active(false);
        buttons_[Buttons::BT_COMMUNITY]->set_active(false);
        buttons_[Buttons::BT_EVENT]->set_active(false);
    }

    std::string fold = "button:Fold";
    std::string extend = "button:Extend";

    if (VWIDTH_ == 800) {
        fold += "800";
        extend += "800";
    }

    if (VWIDTH_ == 1366) {
        quickslot_qs_adj_ = Point<int16_t>(213, 0);
    } else {
        quickslot_qs_adj_ = Point<int16_t>(211, 0);
    }

    if (VWIDTH_ == 800) {
        Point<int16_t> quickslot_qs = Point<int16_t>(579, 0);

        buttons_[Buttons::BT_FOLD_QS] =
            std::make_unique<MapleButton>(quickSlot[fold], quickslot_qs);
        buttons_[Buttons::BT_EXTEND_QS] =
            std::make_unique<MapleButton>(quickSlot[extend],
                                          quickslot_qs + quickslot_qs_adj_);
    } else if (VWIDTH_ == 1024) {
        Point<int16_t> quickslot_qs = Point<int16_t>(627 + pos_adj, 37);

        buttons_[Buttons::BT_FOLD_QS] =
            std::make_unique<MapleButton>(quickSlot[fold], quickslot_qs);
        buttons_[Buttons::BT_EXTEND_QS] =
            std::make_unique<MapleButton>(quickSlot[extend],
                                          quickslot_qs + quickslot_qs_adj_);
    } else if (VWIDTH_ == 1280) {
        Point<int16_t> quickslot_qs = Point<int16_t>(621 + pos_adj, 37);

        buttons_[Buttons::BT_FOLD_QS] =
            std::make_unique<MapleButton>(quickSlot[fold], quickslot_qs);
        buttons_[Buttons::BT_EXTEND_QS] =
            std::make_unique<MapleButton>(quickSlot[extend],
                                          quickslot_qs + quickslot_qs_adj_);
    } else if (VWIDTH_ == 1366) {
        Point<int16_t> quickslot_qs = Point<int16_t>(623 + pos_adj, 37);

        buttons_[Buttons::BT_FOLD_QS] =
            std::make_unique<MapleButton>(quickSlot[fold], quickslot_qs);
        buttons_[Buttons::BT_EXTEND_QS] =
            std::make_unique<MapleButton>(quickSlot[extend],
                                          quickslot_qs + quickslot_qs_adj_);
    } else if (VWIDTH_ == 1920) {
        Point<int16_t> quickslot_qs = Point<int16_t>(900 + pos_adj, 37);

        buttons_[Buttons::BT_FOLD_QS] =
            std::make_unique<MapleButton>(quickSlot[fold], quickslot_qs);
        buttons_[Buttons::BT_EXTEND_QS] =
            std::make_unique<MapleButton>(quickSlot[extend],
                                          quickslot_qs + quickslot_qs_adj_);
    }

    if (quickslot_active_) {
        buttons_[Buttons::BT_EXTEND_QS]->set_active(false);
    } else {
        buttons_[Buttons::BT_FOLD_QS]->set_active(false);
    }

#pragma region Menu
    menu_background_[0] = submenu["backgrnd"]["0"];
    menu_background_[1] = submenu["backgrnd"]["1"];
    menu_background_[2] = submenu["backgrnd"]["2"];

    buttons_[Buttons::BT_MENU_ACHIEVEMENT] =
        std::make_unique<MapleButton>(submenu["menu"]["button:achievement"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_AUCTION] =
        std::make_unique<MapleButton>(submenu["menu"]["button:auction"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_BATTLE] =
        std::make_unique<MapleButton>(submenu["menu"]["button:battleStats"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_CLAIM] =
        std::make_unique<MapleButton>(submenu["menu"]["button:Claim"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_FISHING] =
        std::make_unique<MapleButton>(submenu["menu"]["button:Fishing"],
                                      menu_pos_ + Point<int16_t>(3, 1));
    buttons_[Buttons::BT_MENU_HELP] =
        std::make_unique<MapleButton>(submenu["menu"]["button:Help"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_MEDAL] =
        std::make_unique<MapleButton>(submenu["menu"]["button:medal"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_MONSTER_COLLECTION] =
        std::make_unique<MapleButton>(
            submenu["menu"]["button:monsterCollection"],
            menu_pos_);
    buttons_[Buttons::BT_MENU_MONSTER_LIFE] =
        std::make_unique<MapleButton>(submenu["menu"]["button:monsterLife"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_QUEST] =
        std::make_unique<MapleButton>(submenu["menu"]["button:quest"],
                                      menu_pos_);
    buttons_[Buttons::BT_MENU_UNION] =
        std::make_unique<MapleButton>(submenu["menu"]["button:union"],
                                      menu_pos_);

    buttons_[Buttons::BT_SETTING_CHANNEL] =
        std::make_unique<MapleButton>(submenu["setting"]["button:channel"],
                                      setting_pos_);
    buttons_[Buttons::BT_SETTING_QUIT] =
        std::make_unique<MapleButton>(submenu["setting"]["button:GameQuit"],
                                      setting_pos_);
    buttons_[Buttons::BT_SETTING_JOYPAD] =
        std::make_unique<MapleButton>(submenu["setting"]["button:JoyPad"],
                                      setting_pos_);
    buttons_[Buttons::BT_SETTING_KEYS] =
        std::make_unique<MapleButton>(submenu["setting"]["button:keySetting"],
                                      setting_pos_);
    buttons_[Buttons::BT_SETTING_OPTION] =
        std::make_unique<MapleButton>(submenu["setting"]["button:option"],
                                      setting_pos_);

    buttons_[Buttons::BT_COMMUNITY_PARTY] =
        std::make_unique<MapleButton>(submenu["community"]["button:bossParty"],
                                      community_pos_);
    buttons_[Buttons::BT_COMMUNITY_FRIENDS] =
        std::make_unique<MapleButton>(submenu["community"]["button:friends"],
                                      community_pos_);
    buttons_[Buttons::BT_COMMUNITY_GUILD] =
        std::make_unique<MapleButton>(submenu["community"]["button:guild"],
                                      community_pos_);
    buttons_[Buttons::BT_COMMUNITY_MAPLECHAT] =
        std::make_unique<MapleButton>(submenu["community"]["button:mapleChat"],
                                      community_pos_);

    buttons_[Buttons::BT_CHARACTER_INFO] =
        std::make_unique<MapleButton>(submenu["character"]["button:character"],
                                      character_pos_);
    buttons_[Buttons::BT_CHARACTER_EQUIP] =
        std::make_unique<MapleButton>(submenu["character"]["button:Equip"],
                                      character_pos_);
    buttons_[Buttons::BT_CHARACTER_ITEM] =
        std::make_unique<MapleButton>(submenu["character"]["button:Item"],
                                      character_pos_);
    buttons_[Buttons::BT_CHARACTER_SKILL] =
        std::make_unique<MapleButton>(submenu["character"]["button:Skill"],
                                      character_pos_);
    buttons_[Buttons::BT_CHARACTER_STAT] =
        std::make_unique<MapleButton>(submenu["character"]["button:Stat"],
                                      character_pos_);

    buttons_[Buttons::BT_EVENT_DAILY] =
        std::make_unique<MapleButton>(submenu["event"]["button:dailyGift"],
                                      event_pos_);
    buttons_[Buttons::BT_EVENT_SCHEDULE] =
        std::make_unique<MapleButton>(submenu["event"]["button:schedule"],
                                      event_pos_);

    for (size_t i = Buttons::BT_MENU_QUEST; i <= Buttons::BT_EVENT_DAILY; i++) {
        buttons_[i]->set_active(false);
    }

    menu_title_[0] = submenu["title"]["character"];
    menu_title_[1] = submenu["title"]["community"];
    menu_title_[2] = submenu["title"]["event"];
    menu_title_[3] = submenu["title"]["menu"];
    menu_title_[4] = submenu["title"]["setting"];
#pragma endregion

    if (VWIDTH_ == 800) {
        position_ = Point<int16_t>(0, 480);
        position_x_ = 410;
        position_y_ = position_.y();
        dimension_ = Point<int16_t>(VWIDTH_ - position_x_, 140);
    } else if (VWIDTH_ == 1024) {
        position_ = Point<int16_t>(0, 648);
        position_x_ = 410;
        position_y_ = position_.y() + 42;
        dimension_ = Point<int16_t>(VWIDTH_ - position_x_, 75);
    } else if (VWIDTH_ == 1280) {
        position_ = Point<int16_t>(0, 600);
        position_x_ = 500;
        position_y_ = position_.y() + 42;
        dimension_ = Point<int16_t>(VWIDTH_ - position_x_, 75);
    } else if (VWIDTH_ == 1366) {
        position_ = Point<int16_t>(0, 648);
        position_x_ = 585;
        position_y_ = position_.y() + 42;
        dimension_ = Point<int16_t>(VWIDTH_ - position_x_, 75);
    } else if (VWIDTH_ == 1920) {
        position_ = Point<int16_t>(0, 960 + (VHEIGHT_ - 1080));
        position_x_ = 860;
        position_y_ = position_.y() + 40;
        dimension_ = Point<int16_t>(VWIDTH_ - position_x_, 80);
    }
}

void UIStatusBar::draw(float alpha) const {
    UIElement::draw_sprites(alpha);

    for (size_t i = 0; i <= Buttons::BT_EVENT; i++) {
        buttons_.at(i)->draw(position_);
    }

    hpmp_sprites_[0].draw(position_, alpha);

    exp_bar_.draw(position_ + exp_pos_);
    hp_bar_.draw(position_ + hpmp_pos_);
    mp_bar_.draw(position_ + hpmp_pos_);

    hpmp_sprites_[1].draw(position_, alpha);
    hpmp_sprites_[2].draw(position_, alpha);

    int16_t level = stats_.get_stat(MapleStat::Id::LEVEL);
    int16_t hp = stats_.get_stat(MapleStat::Id::HP);
    int16_t mp = stats_.get_stat(MapleStat::Id::MP);
    int32_t maxhp = stats_.get_total(EquipStat::Id::HP);
    int32_t maxmp = stats_.get_total(EquipStat::Id::MP);
    int64_t exp = stats_.get_exp();

    std::string expstring = std::to_string(100 * getexppercent());

    stat_set_.draw(std::to_string(exp) + "["
                       + expstring.substr(0, expstring.find('.') + 3) + "%]",
                   position_ + statset_pos_);

    hpmp_set_.draw("[" + std::to_string(hp) + "/" + std::to_string(maxhp) + "]",
                   position_ + hpset_pos_);

    hpmp_set_.draw("[" + std::to_string(mp) + "/" + std::to_string(maxmp) + "]",
                   position_ + mpset_pos_);

    level_set_.draw(std::to_string(level), position_ + levelset_pos_);

    name_label_.draw(position_ + namelabel_pos_);

    buttons_.at(Buttons::BT_FOLD_QS)->draw(position_ + quickslot_adj_);
    buttons_.at(Buttons::BT_EXTEND_QS)
        ->draw(position_ + quickslot_adj_ - quickslot_qs_adj_);

    if (VWIDTH_ > 800 && VWIDTH_ < 1366) {
        quickslot_[0].draw(position_ + quickslot_pos_ + Point<int16_t>(-1, 0)
                           + quickslot_adj_);
        quickslot_[1].draw(position_ + quickslot_pos_ + Point<int16_t>(-1, 0)
                           + quickslot_adj_);
    } else {
        quickslot_[0].draw(position_ + quickslot_pos_ + quickslot_adj_);
        quickslot_[1].draw(position_ + quickslot_pos_ + quickslot_adj_);
    }

#pragma region Menu
    Point<int16_t> pos_adj = Point<int16_t>(0, 0);

    if (quickslot_active_) {
        if (VWIDTH_ == 800) {
            pos_adj += Point<int16_t>(0, -73);
        } else {
            pos_adj += Point<int16_t>(0, -31);
        }
    }

    Point<int16_t> pos;
    uint8_t button_count = 0;
    uint8_t menutitle_index = 0;

    if (character_active_) {
        pos = character_pos_;
        button_count = 5;
        menutitle_index = 0;
    } else if (community_active_) {
        pos = community_pos_;
        button_count = 4;
        menutitle_index = 1;
    } else if (event_active_) {
        pos = event_pos_;
        button_count = 2;
        menutitle_index = 2;
    } else if (menu_active_) {
        pos = menu_pos_;
        button_count = 11;
        menutitle_index = 3;
    } else if (setting_active_) {
        pos = setting_pos_;
        button_count = 5;
        menutitle_index = 4;
    } else {
        return;
    }

    Point<int16_t> mid_pos = Point<int16_t>(0, 29);

    uint16_t end_y = std::floor(28.2 * button_count);

    if (menu_active_) {
        end_y -= 1;
    }

    uint16_t mid_y = end_y - mid_pos.y();

    menu_background_[0].draw(position_ + pos + pos_adj);
    menu_background_[1].draw(DrawArgument(position_ + pos + pos_adj)
                             + DrawArgument(mid_pos, Point<int16_t>(0, mid_y)));
    menu_background_[2].draw(position_ + pos + pos_adj
                             + Point<int16_t>(0, end_y));

    menu_title_[menutitle_index].draw(position_ + pos + pos_adj);

    for (size_t i = Buttons::BT_MENU_QUEST; i <= Buttons::BT_EVENT_DAILY; i++) {
        buttons_.at(i)->draw(position_);
    }
#pragma endregion
}

void UIStatusBar::update() {
    UIElement::update();

    for (auto sprite : hpmp_sprites_) {
        sprite.update();
    }

    exp_bar_.update(getexppercent());
    hp_bar_.update(gethppercent());
    mp_bar_.update(getmppercent());

    name_label_.change_text(stats_.get_name());

    Point<int16_t> pos_adj = get_quickslot_pos();

    if (quickslot_active_) {
        if (quickslot_adj_.x() > quickslot_min_) {
            int16_t new_x = quickslot_adj_.x() - Constants::TIMESTEP;

            if (new_x < quickslot_min_) {
                quickslot_adj_.set_x(quickslot_min_);
            } else {
                quickslot_adj_.shift_x(-Constants::TIMESTEP);
            }
        }
    } else {
        if (quickslot_adj_.x() < QUICKSLOT_MAX_) {
            int16_t new_x = quickslot_adj_.x() + Constants::TIMESTEP;

            if (new_x > QUICKSLOT_MAX_) {
                quickslot_adj_.set_x(QUICKSLOT_MAX_);
            } else {
                quickslot_adj_.shift_x(Constants::TIMESTEP);
            }
        }
    }

    for (size_t i = Buttons::BT_MENU_QUEST; i <= Buttons::BT_MENU_CLAIM; i++) {
        Point<int16_t> menu_adj = Point<int16_t>(0, 0);

        if (i == Buttons::BT_MENU_FISHING) {
            menu_adj = Point<int16_t>(3, 1);
        }

        buttons_[i]->set_position(menu_pos_ + menu_adj + pos_adj);
    }

    for (size_t i = Buttons::BT_SETTING_CHANNEL; i <= Buttons::BT_SETTING_QUIT;
         i++) {
        buttons_[i]->set_position(setting_pos_ + pos_adj);
    }

    for (size_t i = Buttons::BT_COMMUNITY_FRIENDS;
         i <= Buttons::BT_COMMUNITY_MAPLECHAT;
         i++) {
        buttons_[i]->set_position(community_pos_ + pos_adj);
    }

    for (size_t i = Buttons::BT_CHARACTER_INFO; i <= Buttons::BT_CHARACTER_ITEM;
         i++) {
        buttons_[i]->set_position(character_pos_ + pos_adj);
    }

    for (size_t i = Buttons::BT_EVENT_SCHEDULE; i <= Buttons::BT_EVENT_DAILY;
         i++) {
        buttons_[i]->set_position(event_pos_ + pos_adj);
    }
}

Button::State UIStatusBar::button_pressed(uint16_t id) {
    switch (id) {
        case Buttons::BT_CASHSHOP: fn_enter_cashshop(); break;
        case Buttons::BT_MENU: toggle_menu(); break;
        case Buttons::BT_OPTIONS: toggle_setting(); break;
        case Buttons::BT_CHARACTER: toggle_character(); break;
        case Buttons::BT_COMMUNITY: toggle_community(); break;
        case Buttons::BT_EVENT: toggle_event(); break;
        case Buttons::BT_FOLD_QS: toggle_qs(false); break;
        case Buttons::BT_EXTEND_QS: toggle_qs(true); break;
        case Buttons::BT_MENU_QUEST:
            UI::get().emplace<UIQuestLog>(
                Stage::get().get_player().get_quests());

            remove_menus();
            break;
        case Buttons::BT_MENU_MEDAL:
        case Buttons::BT_MENU_UNION:
        case Buttons::BT_MENU_MONSTER_COLLECTION:
        case Buttons::BT_MENU_AUCTION:
        case Buttons::BT_MENU_MONSTER_LIFE:
        case Buttons::BT_MENU_BATTLE:
        case Buttons::BT_MENU_ACHIEVEMENT:
        case Buttons::BT_MENU_FISHING:
        case Buttons::BT_MENU_HELP:
        case Buttons::BT_MENU_CLAIM: remove_menus(); break;
        case Buttons::BT_SETTING_CHANNEL:
            UI::get().emplace<UIChannel>(
                Stage::get().get_player().get_world_id(),
                Stage::get().get_player().get_channel_id(),
                channel_count_);

            remove_menus();
            break;
        case Buttons::BT_SETTING_OPTION:
            UI::get().emplace<UIOptionMenu>();

            remove_menus();
            break;
        case Buttons::BT_SETTING_KEYS:
            UI::get().emplace<UIKeyConfig>(
                Stage::get().get_player().get_inventory(),
                Stage::get().get_player().get_skills());

            remove_menus();
            break;
        case Buttons::BT_SETTING_JOYPAD:
            UI::get().emplace<UIJoypad>();

            remove_menus();
            break;
        case Buttons::BT_SETTING_QUIT:
            UI::get().emplace<UIQuit>(stats_);

            remove_menus();
            break;
        case Buttons::BT_COMMUNITY_FRIENDS:
        case Buttons::BT_COMMUNITY_PARTY: {
            auto userlist = UI::get().get_element<UIUserList>();
            auto tab = (id == Buttons::BT_COMMUNITY_FRIENDS)
                           ? UIUserList::Tab::FRIEND
                           : UIUserList::Tab::PARTY;

            if (!userlist) {
                UI::get().emplace<UIUserList>(tab);
            } else {
                auto cur_tab = userlist->get().get_tab();
                auto is_active = userlist->get().is_active();

                if (cur_tab == tab) {
                    if (is_active) {
                        userlist->get().deactivate();
                    } else {
                        userlist->get().makeactive();
                    }
                } else {
                    if (!is_active) {
                        userlist->get().makeactive();
                    }

                    userlist->get().change_tab(tab);
                }
            }

            remove_menus();
        } break;
        case Buttons::BT_COMMUNITY_GUILD: remove_menus(); break;
        case Buttons::BT_COMMUNITY_MAPLECHAT:
            UI::get().emplace<UIChat>();

            remove_menus();
            break;
        case Buttons::BT_CHARACTER_INFO:
            UI::get().emplace<UICharInfo>(Stage::get().get_player().get_oid());

            remove_menus();
            break;
        case Buttons::BT_CHARACTER_STAT:
            UI::get().emplace<UIStatsInfo>(
                Stage::get().get_player().get_stats());

            remove_menus();
            break;
        case Buttons::BT_CHARACTER_SKILL:
            UI::get().emplace<UISkillBook>(
                Stage::get().get_player().get_stats(),
                Stage::get().get_player().get_skills());

            remove_menus();
            break;
        case Buttons::BT_CHARACTER_EQUIP:
            UI::get().emplace<UIEquipInventory>(
                Stage::get().get_player().get_inventory());

            remove_menus();
            break;
        case Buttons::BT_CHARACTER_ITEM:
            UI::get().emplace<UIItemInventory>(
                Stage::get().get_player().get_inventory());

            remove_menus();
            break;
        case Buttons::BT_EVENT_SCHEDULE:
            UI::get().emplace<UIEvent>();

            remove_menus();
            break;
        case Buttons::BT_EVENT_DAILY: remove_menus(); break;
    }

    return Button::State::NORMAL;
}

void UIStatusBar::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            if (!menu_active_ && !setting_active_ && !community_active_
                && !character_active_ && !event_active_) {
                toggle_setting();
            } else {
                remove_menus();
            }
        } else if (keycode == KeyAction::Id::RETURN) {
            for (size_t i = Buttons::BT_MENU_QUEST;
                 i <= Buttons::BT_EVENT_DAILY;
                 i++) {
                if (buttons_[i]->get_state() == Button::State::MOUSEOVER) {
                    button_pressed(i);
                }
            }
        } else if (keycode == KeyAction::Id::UP
                   || keycode == KeyAction::Id::DOWN) {
            uint16_t min_id, max_id;

            if (menu_active_) {
                min_id = Buttons::BT_MENU_QUEST;
                max_id = Buttons::BT_MENU_CLAIM;
            } else if (setting_active_) {
                min_id = Buttons::BT_SETTING_CHANNEL;
                max_id = Buttons::BT_SETTING_QUIT;
            } else if (community_active_) {
                min_id = Buttons::BT_COMMUNITY_FRIENDS;
                max_id = Buttons::BT_COMMUNITY_MAPLECHAT;
            } else if (character_active_) {
                min_id = Buttons::BT_CHARACTER_INFO;
                max_id = Buttons::BT_CHARACTER_ITEM;
            } else if (event_active_) {
                min_id = Buttons::BT_EVENT_SCHEDULE;
                max_id = Buttons::BT_EVENT_DAILY;
            }

            uint16_t id = min_id;

            for (size_t i = min_id; i <= max_id; i++) {
                if (buttons_[i]->get_state() != Button::State::NORMAL) {
                    id = i;

                    buttons_[i]->set_state(Button::State::NORMAL);
                    break;
                }
            }

            if (keycode == KeyAction::Id::DOWN) {
                if (id < max_id) {
                    id++;
                } else {
                    id = min_id;
                }
            } else if (keycode == KeyAction::Id::UP) {
                if (id > min_id) {
                    id--;
                } else {
                    id = max_id;
                }
            }

            buttons_[id]->set_state(Button::State::MOUSEOVER);
        }
    }
}

bool UIStatusBar::is_in_range(Point<int16_t> cursorpos) const {
    Point<int16_t> pos;
    Rectangle<int16_t> bounds;

    if (!character_active_ && !community_active_ && !event_active_
        && !menu_active_ && !setting_active_) {
        pos = Point<int16_t>(position_x_, position_y_);
        bounds = Rectangle<int16_t>(pos, pos + dimension_);
    } else {
        uint8_t button_count;
        int16_t pos_y_adj;

        if (character_active_) {
            pos = character_pos_;
            button_count = 5;
            pos_y_adj = 248;
        } else if (community_active_) {
            pos = community_pos_;
            button_count = 4;
            pos_y_adj = 301;
        } else if (event_active_) {
            pos = event_pos_;
            button_count = 2;
            pos_y_adj = 417;
        } else if (menu_active_) {
            pos = menu_pos_;
            button_count = 11;
            pos_y_adj = -90;
        } else if (setting_active_) {
            pos = setting_pos_;
            button_count = 5;
            pos_y_adj = 248;
        }

        pos_y_adj += VHEIGHT_ - 600;

        Point<int16_t> pos_adj = get_quickslot_pos();
        pos = Point<int16_t>(pos.x(), std::abs(pos.y()) + pos_y_adj) + pos_adj;

        uint16_t end_y = std::floor(28.2 * button_count);

        bounds = Rectangle<int16_t>(pos, pos + Point<int16_t>(113, end_y + 35));
    }

    return bounds.contains(cursorpos);
}

UIElement::Type UIStatusBar::get_type() const {
    return TYPE;
}

void UIStatusBar::toggle_qs() {
    if (!menu_active_ && !setting_active_ && !community_active_
        && !character_active_ && !event_active_) {
        toggle_qs(!quickslot_active_);
    }
}

void UIStatusBar::toggle_qs(bool quick_slot_active) {
    if (quickslot_active_ == quick_slot_active) {
        return;
    }

    quickslot_active_ = quick_slot_active;
    buttons_[Buttons::BT_FOLD_QS]->set_active(quickslot_active_);
    buttons_[Buttons::BT_EXTEND_QS]->set_active(!quickslot_active_);

    if (VWIDTH_ > 800) {
        buttons_[Buttons::BT_CASHSHOP]->set_active(!quickslot_active_);
        buttons_[Buttons::BT_MENU]->set_active(!quickslot_active_);
        buttons_[Buttons::BT_OPTIONS]->set_active(!quickslot_active_);
        buttons_[Buttons::BT_CHARACTER]->set_active(!quickslot_active_);
        buttons_[Buttons::BT_COMMUNITY]->set_active(!quickslot_active_);
        buttons_[Buttons::BT_EVENT]->set_active(!quickslot_active_);
    }
}

void UIStatusBar::toggle_menu() {
    remove_active_menu(MenuType::MENU);

    menu_active_ = !menu_active_;

    buttons_[Buttons::BT_MENU_ACHIEVEMENT]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_AUCTION]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_BATTLE]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_CLAIM]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_FISHING]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_HELP]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_MEDAL]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_MONSTER_COLLECTION]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_MONSTER_LIFE]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_QUEST]->set_active(menu_active_);
    buttons_[Buttons::BT_MENU_UNION]->set_active(menu_active_);

    if (menu_active_) {
        buttons_[Buttons::BT_MENU_QUEST]->set_state(Button::State::MOUSEOVER);

        Sound(Sound::Name::DLG_NOTICE).play();
    }
}

void UIStatusBar::toggle_setting() {
    remove_active_menu(MenuType::SETTING);

    setting_active_ = !setting_active_;

    buttons_[Buttons::BT_SETTING_CHANNEL]->set_active(setting_active_);
    buttons_[Buttons::BT_SETTING_QUIT]->set_active(setting_active_);
    buttons_[Buttons::BT_SETTING_JOYPAD]->set_active(setting_active_);
    buttons_[Buttons::BT_SETTING_KEYS]->set_active(setting_active_);
    buttons_[Buttons::BT_SETTING_OPTION]->set_active(setting_active_);

    if (setting_active_) {
        buttons_[Buttons::BT_SETTING_CHANNEL]->set_state(
            Button::State::MOUSEOVER);

        Sound(Sound::Name::DLG_NOTICE).play();
    }
}

void UIStatusBar::toggle_community() {
    remove_active_menu(MenuType::COMMUNITY);

    community_active_ = !community_active_;

    buttons_[Buttons::BT_COMMUNITY_PARTY]->set_active(community_active_);
    buttons_[Buttons::BT_COMMUNITY_FRIENDS]->set_active(community_active_);
    buttons_[Buttons::BT_COMMUNITY_GUILD]->set_active(community_active_);
    buttons_[Buttons::BT_COMMUNITY_MAPLECHAT]->set_active(community_active_);

    if (community_active_) {
        buttons_[Buttons::BT_COMMUNITY_FRIENDS]->set_state(
            Button::State::MOUSEOVER);

        Sound(Sound::Name::DLG_NOTICE).play();
    }
}

void UIStatusBar::toggle_character() {
    remove_active_menu(MenuType::CHARACTER);

    character_active_ = !character_active_;

    buttons_[Buttons::BT_CHARACTER_INFO]->set_active(character_active_);
    buttons_[Buttons::BT_CHARACTER_EQUIP]->set_active(character_active_);
    buttons_[Buttons::BT_CHARACTER_ITEM]->set_active(character_active_);
    buttons_[Buttons::BT_CHARACTER_SKILL]->set_active(character_active_);
    buttons_[Buttons::BT_CHARACTER_STAT]->set_active(character_active_);

    if (character_active_) {
        buttons_[Buttons::BT_CHARACTER_INFO]->set_state(
            Button::State::MOUSEOVER);

        Sound(Sound::Name::DLG_NOTICE).play();
    }
}

void UIStatusBar::toggle_event() {
    remove_active_menu(MenuType::EVENT);

    event_active_ = !event_active_;

    buttons_[Buttons::BT_EVENT_DAILY]->set_active(event_active_);
    buttons_[Buttons::BT_EVENT_SCHEDULE]->set_active(event_active_);

    if (event_active_) {
        buttons_[Buttons::BT_EVENT_SCHEDULE]->set_state(
            Button::State::MOUSEOVER);

        Sound(Sound::Name::DLG_NOTICE).play();
    }
}

void UIStatusBar::remove_menus() {
    if (menu_active_) {
        toggle_menu();
    } else if (setting_active_) {
        toggle_setting();
    } else if (community_active_) {
        toggle_community();
    } else if (character_active_) {
        toggle_character();
    } else if (event_active_) {
        toggle_event();
    }
}

void UIStatusBar::remove_active_menu(MenuType type) {
    for (size_t i = Buttons::BT_MENU_QUEST; i <= Buttons::BT_EVENT_DAILY; i++) {
        buttons_[i]->set_state(Button::State::NORMAL);
    }

    if (menu_active_ && type != MenuType::MENU) {
        toggle_menu();
    } else if (setting_active_ && type != MenuType::SETTING) {
        toggle_setting();
    } else if (community_active_ && type != MenuType::COMMUNITY) {
        toggle_community();
    } else if (character_active_ && type != MenuType::CHARACTER) {
        toggle_character();
    } else if (event_active_ && type != MenuType::EVENT) {
        toggle_event();
    }
}

Point<int16_t> UIStatusBar::get_quickslot_pos() const {
    if (quickslot_active_) {
        if (VWIDTH_ == 800) {
            return Point<int16_t>(0, -73);
        }
        return Point<int16_t>(0, -31);
    }

    return Point<int16_t>(0, 0);
}

bool UIStatusBar::is_menu_active() {
    return menu_active_ || setting_active_ || community_active_
           || character_active_ || event_active_;
}

float UIStatusBar::getexppercent() const {
    int16_t level = stats_.get_stat(MapleStat::Id::LEVEL);

    if (level >= ExpTable::LEVELCAP) {
        return 0.0f;
    }

    int64_t exp = stats_.get_exp();

    return static_cast<float>(static_cast<double>(exp)
                              / ExpTable::values[level]);
}

float UIStatusBar::gethppercent() const {
    int16_t hp = stats_.get_stat(MapleStat::Id::HP);
    int32_t maxhp = stats_.get_total(EquipStat::Id::HP);

    return static_cast<float>(hp) / maxhp;
}

float UIStatusBar::getmppercent() const {
    int16_t mp = stats_.get_stat(MapleStat::Id::MP);
    int32_t maxmp = stats_.get_total(EquipStat::Id::MP);

    return static_cast<float>(mp) / maxmp;
}
}  // namespace ms