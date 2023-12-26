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
#include "UIKeyConfig.h"

#include <nlnx/nx.hpp>
#include <string>

#include "../../Data/ItemData.h"
#include "../../Data/SkillData.h"
#include "../../Net/Packets/PlayerPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "../UITypes/Login/UILoginNotice.h"
#include "../UITypes/UINotice.h"

namespace ms {
namespace {
auto fn_change_keymap = []<typename... T>(T && ...args) {
    ChangeKeyMapPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIKeyConfig::UIKeyConfig(const Inventory &in_inventory,
                         const SkillBook &in_skillbook) :
    UIDragElement<PosKEYCONFIG>(),
    inventory_(in_inventory),
    skillbook_(in_skillbook),
    dirty_(false) {
    keyboard_ = &UI::get().get_keyboard();
    staged_mappings_ = keyboard_->get_maplekeys();

    nl::node KeyConfig = nl::nx::ui["StatusBar3.img"]["KeyConfig"];

    icon_ = KeyConfig["icon"];
    key_ = KeyConfig["key"];

    nl::node backgrnd = KeyConfig["backgrnd"];
    Texture bg = backgrnd;
    Point<int16_t> bg_dimensions = bg.get_dimensions();

    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(KeyConfig["backgrnd2"]);
    sprites_.emplace_back(KeyConfig["backgrnd3"]);

    nl::node BtClose3 = nl::nx::ui["Basic.img"]["BtClose3"];
    buttons_[Buttons::CLOSE] = std::make_unique<MapleButton>(
        BtClose3,
        Point<int16_t>(bg_dimensions.x() - 18, 3));
    buttons_[Buttons::CANCEL] =
        std::make_unique<MapleButton>(KeyConfig["button:Cancel"]);
    buttons_[Buttons::DEFAULT] =
        std::make_unique<MapleButton>(KeyConfig["button:Default"]);
    buttons_[Buttons::DELETE] =
        std::make_unique<MapleButton>(KeyConfig["button:Delete"]);
    buttons_[Buttons::KEY_SETTING] =
        std::make_unique<MapleButton>(KeyConfig["button:keySetting"]);
    buttons_[Buttons::OK] =
        std::make_unique<MapleButton>(KeyConfig["button:OK"]);

    dimension_ = bg_dimensions;
    drag_area_ = Point<int16_t>(bg_dimensions.x(), 20);

    load_keys_pos();
    load_unbound_actions_pos();
    load_key_textures();
    load_action_mappings();
    load_action_icons();
    load_item_icons();
    load_skill_icons();

    bind_staged_action_keys();
}

/// Load
void UIKeyConfig::load_keys_pos() {
    int16_t slot_width = 33;
    int16_t slot_width_lg = 98;
    int16_t slot_height = 33;

    int16_t row_y = 126;
    int16_t row_special_y = row_y - slot_height - 5;

    int16_t row_quickslot_x = 535;

    int16_t row_one_x = 31;
    int16_t row_two_x = 80;
    int16_t row_three_x = 96;
    int16_t row_four_x = 55;
    int16_t row_five_x = 39;

    int16_t row_special_x = row_one_x;

    keys_pos_[KeyConfig::Key::ESCAPE] =
        Point<int16_t>(row_one_x, row_special_y);

    row_special_x += slot_width * 2;

    for (size_t i = KeyConfig::Key::F1; i <= KeyConfig::Key::F12; i++) {
        KeyConfig::Key id = KeyConfig::actionbyid(i);

        keys_pos_[id] = Point<int16_t>(row_special_x, row_special_y);

        row_special_x += slot_width;

        if (id == KeyConfig::Key::F4 || id == KeyConfig::Key::F8) {
            row_special_x += 17;
        }
    }

    keys_pos_[KeyConfig::Key::SCROLL_LOCK] =
        Point<int16_t>(row_quickslot_x + (slot_width * 1), row_special_y);

    keys_pos_[KeyConfig::Key::GRAVE_ACCENT] =
        Point<int16_t>(row_one_x + (slot_width * 0), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM1] =
        Point<int16_t>(row_one_x + (slot_width * 1), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM2] =
        Point<int16_t>(row_one_x + (slot_width * 2), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM3] =
        Point<int16_t>(row_one_x + (slot_width * 3), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM4] =
        Point<int16_t>(row_one_x + (slot_width * 4), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM5] =
        Point<int16_t>(row_one_x + (slot_width * 5), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM6] =
        Point<int16_t>(row_one_x + (slot_width * 6), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM7] =
        Point<int16_t>(row_one_x + (slot_width * 7), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM8] =
        Point<int16_t>(row_one_x + (slot_width * 8), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM9] =
        Point<int16_t>(row_one_x + (slot_width * 9), row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::NUM0] =
        Point<int16_t>(row_one_x + (slot_width * 10),
                       row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::MINUS] =
        Point<int16_t>(row_one_x + (slot_width * 11),
                       row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::EQUAL] =
        Point<int16_t>(row_one_x + (slot_width * 12),
                       row_y + (slot_height * 0));

    for (size_t i = KeyConfig::Key::Q; i <= KeyConfig::Key::RIGHT_BRACKET;
         i++) {
        KeyConfig::Key id = KeyConfig::actionbyid(i);

        keys_pos_[id] =
            Point<int16_t>(row_two_x + (slot_width * (i - KeyConfig::Key::Q)),
                           row_y + (slot_height * 1));
    }

    row_two_x += 9;

    keys_pos_[KeyConfig::Key::BACKSLASH] =
        Point<int16_t>(row_two_x + (slot_width * 12),
                       row_y + (slot_height * 1));

    for (size_t i = KeyConfig::Key::A; i <= KeyConfig::Key::APOSTROPHE; i++) {
        KeyConfig::Key id = KeyConfig::actionbyid(i);

        keys_pos_[id] =
            Point<int16_t>(row_three_x + (slot_width * (i - KeyConfig::Key::A)),
                           row_y + (slot_height * 2));
    }

    keys_pos_[KeyConfig::Key::LEFT_SHIFT] =
        Point<int16_t>(row_four_x + (slot_width * 0),
                       row_y + (slot_height * 3));

    row_four_x += 24;

    for (size_t i = KeyConfig::Key::Z; i <= KeyConfig::Key::PERIOD; i++) {
        KeyConfig::Key id = KeyConfig::actionbyid(i);

        keys_pos_[id] = Point<int16_t>(
            row_four_x + (slot_width * (i - KeyConfig::Key::Z + 1)),
            row_y + (slot_height * 3));
    }

    row_four_x += 24;

    keys_pos_[KeyConfig::Key::RIGHT_SHIFT] =
        Point<int16_t>(row_four_x + (slot_width * 11),
                       row_y + (slot_height * 3));

    keys_pos_[KeyConfig::Key::LEFT_CONTROL] =
        Point<int16_t>(row_five_x + (slot_width_lg * 0),
                       row_y + (slot_height * 4));
    keys_pos_[KeyConfig::Key::LEFT_ALT] =
        Point<int16_t>(row_five_x + (slot_width_lg * 1),
                       row_y + (slot_height * 4));

    row_five_x += 24;

    keys_pos_[KeyConfig::Key::SPACE] =
        Point<int16_t>(row_five_x + (slot_width_lg * 2),
                       row_y + (slot_height * 4));

    row_five_x += 27;

    keys_pos_[KeyConfig::Key::RIGHT_ALT] =
        Point<int16_t>(row_five_x + (slot_width_lg * 3),
                       row_y + (slot_height * 4));

    row_five_x += 2;

    keys_pos_[KeyConfig::Key::RIGHT_CONTROL] =
        Point<int16_t>(row_five_x + (slot_width_lg * 4),
                       row_y + (slot_height * 4));

    keys_pos_[KeyConfig::Key::INSERT] =
        Point<int16_t>(row_quickslot_x + (slot_width * 0),
                       row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::HOME] =
        Point<int16_t>(row_quickslot_x + (slot_width * 1),
                       row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::PAGE_UP] =
        Point<int16_t>(row_quickslot_x + (slot_width * 2),
                       row_y + (slot_height * 0));
    keys_pos_[KeyConfig::Key::DELETE] =
        Point<int16_t>(row_quickslot_x + (slot_width * 0),
                       row_y + (slot_height * 1));
    keys_pos_[KeyConfig::Key::END] =
        Point<int16_t>(row_quickslot_x + (slot_width * 1),
                       row_y + (slot_height * 1));
    keys_pos_[KeyConfig::Key::PAGE_DOWN] =
        Point<int16_t>(row_quickslot_x + (slot_width * 2),
                       row_y + (slot_height * 1));
}

void UIKeyConfig::load_unbound_actions_pos() {
    int16_t row_x = 26;
    int16_t row_y = 307;

    int16_t slot_width = 36;
    int16_t slot_height = 36;

    unbound_actions_pos_[KeyAction::Id::MAPLE_CHAT] =
        Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::TOGGLE_CHAT] =
        Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::WHISPER] =
        Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::MEDALS] =
        Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::BOSS_PARTY] =
        Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::PROFESSION] =
        Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::EQUIPMENT] =
        Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::ITEMS] =
        Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::CHAR_INFO] =
        Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::MENU] =
        Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::QUICK_SLOTS] =
        Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::PICKUP] =
        Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::SIT] =
        Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::ATTACK] =
        Point<int16_t>(row_x + (slot_width * 13), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::JUMP] =
        Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::INTERACT_HARVEST] =
        Point<int16_t>(row_x + (slot_width * 15), row_y + (slot_height * 0));
    unbound_actions_pos_[KeyAction::Id::SOUL_WEAPON] =
        Point<int16_t>(row_x + (slot_width * 16), row_y + (slot_height * 0));

    unbound_actions_pos_[KeyAction::Id::SAY] =
        Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::PARTY_CHAT] =
        Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::FRIENDS_CHAT] =
        Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::ITEM_POT] =
        Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::EVENT] =
        Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::SILENT_CRUSADE] =
        Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::STATS] =
        Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::SKILLS] =
        Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::QUEST_LOG] =
        Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::CHANGE_CHANNEL] =
        Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::GUILD] =
        Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::PARTY] =
        Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::NOTIFIER] =
        Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::FRIENDS] =
        Point<int16_t>(row_x + (slot_width * 13), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::WORLD_MAP] =
        Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::MINI_MAP] =
        Point<int16_t>(row_x + (slot_width * 15), row_y + (slot_height * 1));
    unbound_actions_pos_[KeyAction::Id::KEY_BINDINGS] =
        Point<int16_t>(row_x + (slot_width * 16), row_y + (slot_height * 1));

    unbound_actions_pos_[KeyAction::Id::GUILD_CHAT] =
        Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::ALLIANCE_CHAT] =
        Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::BATTLE_ANALYSIS] =
        Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::GUIDE] =
        Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::ENHANCE_EQUIP] =
        Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::MONSTER_COLLECTION] =
        Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::MANAGE_LEGION] =
        Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 2));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 7), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::MAPLE_NEWS] =
        Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::CASHSHOP] =
        Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::MAIN_MENU] =
        Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::SCREENSHOT] =
        Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::PICTURE_MODE] =
        Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 2));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 13), row_y + (slot_height * 2));
    unbound_actions_pos_[KeyAction::Id::MUTE] =
        Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 2));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 15), row_y + (slot_height * 2));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 16), row_y + (slot_height * 2));

    unbound_actions_pos_[KeyAction::Id::FACE1] =
        Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::FACE2] =
        Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::FACE3] =
        Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::FACE4] =
        Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::FACE5] =
        Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::FACE6] =
        Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::FACE7] =
        Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::MAPLE_ACHIEVEMENT] =
        Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::MONSTER_BOOK] =
        Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 3));
    unbound_actions_pos_[KeyAction::Id::TO_SPOUSE] =
        Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 10), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 11), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 12), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 13), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 14), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 15), row_y + (slot_height * 3));
    // unbound_actions_pos_[KeyAction::Id::LENGTH] = Point<int16_t>(row_x +
    // (slot_width * 16), row_y + (slot_height * 3));
}

void UIKeyConfig::load_key_textures() {
    key_textures_[KeyConfig::Key::ESCAPE] = key_[1];
    key_textures_[KeyConfig::Key::NUM1] = key_[2];
    key_textures_[KeyConfig::Key::NUM2] = key_[3];
    key_textures_[KeyConfig::Key::NUM3] = key_[4];
    key_textures_[KeyConfig::Key::NUM4] = key_[5];
    key_textures_[KeyConfig::Key::NUM5] = key_[6];
    key_textures_[KeyConfig::Key::NUM6] = key_[7];
    key_textures_[KeyConfig::Key::NUM7] = key_[8];
    key_textures_[KeyConfig::Key::NUM8] = key_[9];
    key_textures_[KeyConfig::Key::NUM9] = key_[10];
    key_textures_[KeyConfig::Key::NUM0] = key_[11];
    key_textures_[KeyConfig::Key::MINUS] = key_[12];
    key_textures_[KeyConfig::Key::EQUAL] = key_[13];

    key_textures_[KeyConfig::Key::Q] = key_[16];
    key_textures_[KeyConfig::Key::W] = key_[17];
    key_textures_[KeyConfig::Key::E] = key_[18];
    key_textures_[KeyConfig::Key::R] = key_[19];
    key_textures_[KeyConfig::Key::T] = key_[20];
    key_textures_[KeyConfig::Key::Y] = key_[21];
    key_textures_[KeyConfig::Key::U] = key_[22];
    key_textures_[KeyConfig::Key::I] = key_[23];
    key_textures_[KeyConfig::Key::O] = key_[24];
    key_textures_[KeyConfig::Key::P] = key_[25];
    key_textures_[KeyConfig::Key::LEFT_BRACKET] = key_[26];
    key_textures_[KeyConfig::Key::RIGHT_BRACKET] = key_[27];

    key_textures_[KeyConfig::Key::LEFT_CONTROL] = key_[29];
    key_textures_[KeyConfig::Key::RIGHT_CONTROL] = key_[29];

    key_textures_[KeyConfig::Key::A] = key_[30];
    key_textures_[KeyConfig::Key::S] = key_[31];
    key_textures_[KeyConfig::Key::D] = key_[32];
    key_textures_[KeyConfig::Key::F] = key_[33];
    key_textures_[KeyConfig::Key::G] = key_[34];
    key_textures_[KeyConfig::Key::H] = key_[35];
    key_textures_[KeyConfig::Key::J] = key_[36];
    key_textures_[KeyConfig::Key::K] = key_[37];
    key_textures_[KeyConfig::Key::L] = key_[38];
    key_textures_[KeyConfig::Key::SEMICOLON] = key_[39];
    key_textures_[KeyConfig::Key::APOSTROPHE] = key_[40];
    key_textures_[KeyConfig::Key::GRAVE_ACCENT] = key_[41];

    key_textures_[KeyConfig::Key::LEFT_SHIFT] = key_[42];
    key_textures_[KeyConfig::Key::RIGHT_SHIFT] = key_[42];

    key_textures_[KeyConfig::Key::BACKSLASH] = key_[43];
    key_textures_[KeyConfig::Key::Z] = key_[44];
    key_textures_[KeyConfig::Key::X] = key_[45];
    key_textures_[KeyConfig::Key::C] = key_[46];
    key_textures_[KeyConfig::Key::V] = key_[47];
    key_textures_[KeyConfig::Key::B] = key_[48];
    key_textures_[KeyConfig::Key::N] = key_[49];
    key_textures_[KeyConfig::Key::M] = key_[50];
    key_textures_[KeyConfig::Key::COMMA] = key_[51];
    key_textures_[KeyConfig::Key::PERIOD] = key_[52];

    key_textures_[KeyConfig::Key::LEFT_ALT] = key_[56];
    key_textures_[KeyConfig::Key::RIGHT_ALT] = key_[56];

    key_textures_[KeyConfig::Key::SPACE] = key_[57];

    key_textures_[KeyConfig::Key::F1] = key_[59];
    key_textures_[KeyConfig::Key::F2] = key_[60];
    key_textures_[KeyConfig::Key::F3] = key_[61];
    key_textures_[KeyConfig::Key::F4] = key_[62];
    key_textures_[KeyConfig::Key::F5] = key_[63];
    key_textures_[KeyConfig::Key::F6] = key_[64];
    key_textures_[KeyConfig::Key::F7] = key_[65];
    key_textures_[KeyConfig::Key::F8] = key_[66];
    key_textures_[KeyConfig::Key::F9] = key_[67];
    key_textures_[KeyConfig::Key::F10] = key_[68];

    key_textures_[KeyConfig::Key::SCROLL_LOCK] = key_[70];
    key_textures_[KeyConfig::Key::HOME] = key_[71];

    key_textures_[KeyConfig::Key::PAGE_UP] = key_[73];

    key_textures_[KeyConfig::Key::END] = key_[79];

    key_textures_[KeyConfig::Key::PAGE_DOWN] = key_[81];
    key_textures_[KeyConfig::Key::INSERT] = key_[82];
    key_textures_[KeyConfig::Key::DELETE] = key_[83];

    key_textures_[KeyConfig::Key::F11] = key_[87];
    key_textures_[KeyConfig::Key::F12] = key_[88];
}

void UIKeyConfig::load_action_mappings() {
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::EQUIPMENT),
                          KeyAction::Id::EQUIPMENT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::ITEMS),
                          KeyAction::Id::ITEMS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::STATS),
                          KeyAction::Id::STATS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::SKILLS),
                          KeyAction::Id::SKILLS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FRIENDS),
                          KeyAction::Id::FRIENDS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::WORLD_MAP),
                          KeyAction::Id::WORLD_MAP));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MAPLE_CHAT),
                          KeyAction::Id::MAPLE_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MINI_MAP),
                          KeyAction::Id::MINI_MAP));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::QUEST_LOG),
                          KeyAction::Id::QUEST_LOG));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::KEY_BINDINGS),
                          KeyAction::Id::KEY_BINDINGS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::SAY), KeyAction::Id::SAY));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::WHISPER),
                          KeyAction::Id::WHISPER));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::PARTY_CHAT),
                          KeyAction::Id::PARTY_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FRIENDS_CHAT),
                          KeyAction::Id::FRIENDS_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MENU),
                          KeyAction::Id::MENU));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::QUICK_SLOTS),
                          KeyAction::Id::QUICK_SLOTS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::TOGGLE_CHAT),
                          KeyAction::Id::TOGGLE_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::GUILD),
                          KeyAction::Id::GUILD));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::GUILD_CHAT),
                          KeyAction::Id::GUILD_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::PARTY),
                          KeyAction::Id::PARTY));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::NOTIFIER),
                          KeyAction::Id::NOTIFIER));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MAPLE_NEWS),
                          KeyAction::Id::MAPLE_NEWS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::CASHSHOP),
                          KeyAction::Id::CASHSHOP));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::ALLIANCE_CHAT),
                          KeyAction::Id::ALLIANCE_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MANAGE_LEGION),
                          KeyAction::Id::MANAGE_LEGION));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MEDALS),
                          KeyAction::Id::MEDALS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::BOSS_PARTY),
                          KeyAction::Id::BOSS_PARTY));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::PROFESSION),
                          KeyAction::Id::PROFESSION));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::ITEM_POT),
                          KeyAction::Id::ITEM_POT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::EVENT),
                          KeyAction::Id::EVENT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::SILENT_CRUSADE),
                          KeyAction::Id::SILENT_CRUSADE));
    // action_mappings_.push_back(Keyboard::Mapping(get_keytype(KeyAction::Id::BITS),
    // KeyAction::Id::BITS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::BATTLE_ANALYSIS),
                          KeyAction::Id::BATTLE_ANALYSIS));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::GUIDE),
                          KeyAction::Id::GUIDE));
    // action_mappings_.push_back(Keyboard::Mapping(get_keytype(KeyAction::Id::VIEWERS_CHAT),
    // KeyAction::Id::VIEWERS_CHAT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::ENHANCE_EQUIP),
                          KeyAction::Id::ENHANCE_EQUIP));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MONSTER_COLLECTION),
                          KeyAction::Id::MONSTER_COLLECTION));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::SOUL_WEAPON),
                          KeyAction::Id::SOUL_WEAPON));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::CHAR_INFO),
                          KeyAction::Id::CHAR_INFO));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::CHANGE_CHANNEL),
                          KeyAction::Id::CHANGE_CHANNEL));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MAIN_MENU),
                          KeyAction::Id::MAIN_MENU));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::SCREENSHOT),
                          KeyAction::Id::SCREENSHOT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::PICTURE_MODE),
                          KeyAction::Id::PICTURE_MODE));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MAPLE_ACHIEVEMENT),
                          KeyAction::Id::MAPLE_ACHIEVEMENT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::PICKUP),
                          KeyAction::Id::PICKUP));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::SIT), KeyAction::Id::SIT));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::ATTACK),
                          KeyAction::Id::ATTACK));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::JUMP),
                          KeyAction::Id::JUMP));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::INTERACT_HARVEST),
                          KeyAction::Id::INTERACT_HARVEST));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE1),
                          KeyAction::Id::FACE1));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE2),
                          KeyAction::Id::FACE2));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE3),
                          KeyAction::Id::FACE3));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE4),
                          KeyAction::Id::FACE4));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE5),
                          KeyAction::Id::FACE5));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE6),
                          KeyAction::Id::FACE6));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::FACE7),
                          KeyAction::Id::FACE7));
    // action_mappings_.push_back(Keyboard::Mapping(get_keytype(KeyAction::Id::MAPLE_STORAGE),
    // KeyAction::Id::MAPLE_STORAGE));
    // action_mappings_.push_back(Keyboard::Mapping(get_keytype(KeyAction::Id::SAFE_MODE),
    // KeyAction::Id::SAFE_MODE));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MUTE),
                          KeyAction::Id::MUTE));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::MONSTER_BOOK),
                          KeyAction::Id::MONSTER_BOOK));
    action_mappings_.push_back(
        Keyboard::Mapping(get_keytype(KeyAction::Id::TO_SPOUSE),
                          KeyAction::Id::TO_SPOUSE));
}

void UIKeyConfig::load_action_icons() {
    action_icons_[KeyAction::Id::EQUIPMENT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::EQUIPMENT),
        icon_[0],
        -1);
    action_icons_[KeyAction::Id::ITEMS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::ITEMS),
        icon_[1],
        -1);
    action_icons_[KeyAction::Id::STATS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::STATS),
        icon_[2],
        -1);
    action_icons_[KeyAction::Id::SKILLS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::SKILLS),
        icon_[3],
        -1);
    action_icons_[KeyAction::Id::FRIENDS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FRIENDS),
        icon_[4],
        -1);
    action_icons_[KeyAction::Id::WORLD_MAP] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::WORLD_MAP),
        icon_[5],
        -1);
    action_icons_[KeyAction::Id::MAPLE_CHAT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MAPLE_CHAT),
        icon_[6],
        -1);
    action_icons_[KeyAction::Id::MINI_MAP] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MINI_MAP),
        icon_[7],
        -1);
    action_icons_[KeyAction::Id::QUEST_LOG] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::QUEST_LOG),
        icon_[8],
        -1);
    action_icons_[KeyAction::Id::KEY_BINDINGS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::KEY_BINDINGS),
        icon_[9],
        -1);
    action_icons_[KeyAction::Id::SAY] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::SAY),
        icon_[10],
        -1);
    action_icons_[KeyAction::Id::WHISPER] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::WHISPER),
        icon_[11],
        -1);
    action_icons_[KeyAction::Id::PARTY_CHAT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::PARTY_CHAT),
        icon_[12],
        -1);
    action_icons_[KeyAction::Id::FRIENDS_CHAT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FRIENDS_CHAT),
        icon_[13],
        -1);
    action_icons_[KeyAction::Id::MENU] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MENU),
        icon_[14],
        -1);
    action_icons_[KeyAction::Id::QUICK_SLOTS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::QUICK_SLOTS),
        icon_[15],
        -1);
    action_icons_[KeyAction::Id::TOGGLE_CHAT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::TOGGLE_CHAT),
        icon_[16],
        -1);
    action_icons_[KeyAction::Id::GUILD] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::GUILD),
        icon_[17],
        -1);
    action_icons_[KeyAction::Id::GUILD_CHAT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::GUILD_CHAT),
        icon_[18],
        -1);
    action_icons_[KeyAction::Id::PARTY] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::PARTY),
        icon_[19],
        -1);
    action_icons_[KeyAction::Id::NOTIFIER] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::NOTIFIER),
        icon_[20],
        -1);
    action_icons_[KeyAction::Id::MAPLE_NEWS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MAPLE_NEWS),
        icon_[21],
        -1);
    action_icons_[KeyAction::Id::CASHSHOP] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::CASHSHOP),
        icon_[22],
        -1);
    action_icons_[KeyAction::Id::ALLIANCE_CHAT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::ALLIANCE_CHAT),
        icon_[23],
        -1);
    action_icons_[KeyAction::Id::MANAGE_LEGION] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MANAGE_LEGION),
        icon_[25],
        -1);
    action_icons_[KeyAction::Id::MEDALS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MEDALS),
        icon_[26],
        -1);
    action_icons_[KeyAction::Id::BOSS_PARTY] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::BOSS_PARTY),
        icon_[27],
        -1);
    action_icons_[KeyAction::Id::PROFESSION] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::PROFESSION),
        icon_[29],
        -1);
    action_icons_[KeyAction::Id::ITEM_POT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::ITEM_POT),
        icon_[30],
        -1);
    action_icons_[KeyAction::Id::EVENT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::EVENT),
        icon_[31],
        -1);
    action_icons_[KeyAction::Id::SILENT_CRUSADE] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::SILENT_CRUSADE),
        icon_[33],
        -1);
    // action_icons_[KeyAction::Id::BITS] =
    // std::make_unique<Icon>(std::make_unique<KeyMapIcon>(KeyAction::Id::BITS),
    // icon[34], -1);
    action_icons_[KeyAction::Id::BATTLE_ANALYSIS] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::BATTLE_ANALYSIS),
        icon_[35],
        -1);
    action_icons_[KeyAction::Id::GUIDE] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::GUIDE),
        icon_[39],
        -1);
    // action_icons_[KeyAction::Id::VIEWERS_CHAT] =
    // std::make_unique<Icon>(std::make_unique<KeyMapIcon>(KeyAction::Id::VIEWERS_CHAT),
    // icon[40], -1);
    action_icons_[KeyAction::Id::ENHANCE_EQUIP] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::ENHANCE_EQUIP),
        icon_[41],
        -1);
    action_icons_[KeyAction::Id::MONSTER_COLLECTION] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MONSTER_COLLECTION),
        icon_[42],
        -1);
    action_icons_[KeyAction::Id::SOUL_WEAPON] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::SOUL_WEAPON),
        icon_[43],
        -1);
    action_icons_[KeyAction::Id::CHAR_INFO] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::CHAR_INFO),
        icon_[44],
        -1);
    action_icons_[KeyAction::Id::CHANGE_CHANNEL] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::CHANGE_CHANNEL),
        icon_[45],
        -1);
    action_icons_[KeyAction::Id::MAIN_MENU] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MAIN_MENU),
        icon_[46],
        -1);
    action_icons_[KeyAction::Id::SCREENSHOT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::SCREENSHOT),
        icon_[47],
        -1);
    action_icons_[KeyAction::Id::PICTURE_MODE] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::PICTURE_MODE),
        icon_[48],
        -1);
    action_icons_[KeyAction::Id::MAPLE_ACHIEVEMENT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MAPLE_ACHIEVEMENT),
        icon_[49],
        -1);
    action_icons_[KeyAction::Id::PICKUP] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::PICKUP),
        icon_[50],
        -1);
    action_icons_[KeyAction::Id::SIT] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::SIT),
        icon_[51],
        -1);
    action_icons_[KeyAction::Id::ATTACK] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::ATTACK),
        icon_[52],
        -1);
    action_icons_[KeyAction::Id::JUMP] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::JUMP),
        icon_[53],
        -1);
    action_icons_[KeyAction::Id::INTERACT_HARVEST] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::INTERACT_HARVEST),
        icon_[54],
        -1);
    action_icons_[KeyAction::Id::FACE1] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE1),
        icon_[100],
        -1);
    action_icons_[KeyAction::Id::FACE2] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE2),
        icon_[101],
        -1);
    action_icons_[KeyAction::Id::FACE3] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE3),
        icon_[102],
        -1);
    action_icons_[KeyAction::Id::FACE4] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE4),
        icon_[103],
        -1);
    action_icons_[KeyAction::Id::FACE5] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE5),
        icon_[104],
        -1);
    action_icons_[KeyAction::Id::FACE6] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE6),
        icon_[105],
        -1);
    action_icons_[KeyAction::Id::FACE7] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::FACE7),
        icon_[106],
        -1);
    // action_icons_[KeyAction::Id::MAPLE_STORAGE] =
    // std::make_unique<Icon>(std::make_unique<KeyMapIcon>(KeyAction::Id::MAPLE_STORAGE),
    // icon[200], -1); action_icons_[KeyAction::Id::SAFE_MODE] =
    // std::make_unique<Icon>(std::make_unique<KeyMapIcon>(KeyAction::Id::SAFE_MODE),
    // icon[201], -1);
    action_icons_[KeyAction::Id::MUTE] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MUTE),
        icon_[202],
        -1);
    action_icons_[KeyAction::Id::MONSTER_BOOK] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::MONSTER_BOOK),
        icon_[1000],
        -1);
    action_icons_[KeyAction::Id::TO_SPOUSE] = std::make_unique<Icon>(
        std::make_unique<MappingIcon>(KeyAction::Id::TO_SPOUSE),
        icon_[1001],
        -1);
}

void UIKeyConfig::load_item_icons() {
    for (auto const &it : staged_mappings_) {
        Keyboard::Mapping mapping = it.second;

        if (mapping.type == KeyType::Id::ITEM) {
            int32_t item_id = mapping.action;
            int16_t count = inventory_.get_total_item_count(item_id);
            Texture tx = get_item_texture(item_id);

            item_icons_[item_id] = std::make_unique<Icon>(
                std::make_unique<CountableMappingIcon>(mapping, count),
                tx,
                count);
        }
    }
}

void UIKeyConfig::load_skill_icons() {
    for (auto const &it : staged_mappings_) {
        Keyboard::Mapping mapping = it.second;

        if (mapping.type == KeyType::Id::SKILL) {
            int32_t skill_id = mapping.action;
            Texture tx = get_skill_texture(skill_id);

            skill_icons_[skill_id] =
                std::make_unique<Icon>(std::make_unique<MappingIcon>(mapping),
                                       tx,
                                       -1);
        }
    }
}

/// UI: General
void UIKeyConfig::draw(float inter) const {
    UIElement::draw(inter);

    for (auto const &[maplekey, mapping] : staged_mappings_) {
        Icon *ficon = nullptr;

        if (mapping.type == KeyType::Id::ITEM) {
            int32_t item_id = mapping.action;
            ficon = item_icons_.at(item_id).get();
        } else if (mapping.type == KeyType::Id::SKILL) {
            int32_t skill_id = mapping.action;
            ficon = skill_icons_.at(skill_id).get();
        } else if (is_action_mapping(mapping)) {
            KeyAction::Id action = KeyAction::get_action_by_id(mapping.action);

            if (action) {
                for (auto const &[key_actionid, action_icon] : action_icons_) {
                    if (key_actionid == action) {
                        ficon = action_icon.get();
                        break;
                    }
                }
            }
        } else {
            std::cout << "Invalid key mapping: (" << mapping.type << ", "
                      << mapping.action << ")" << std::endl;
        }

        if (ficon) {
            if (maplekey != -1) {
                KeyConfig::Key fkey = KeyConfig::actionbyid(maplekey);

                if (maplekey == KeyConfig::Key::SPACE) {
                    ficon->draw(position_ + keys_pos_[fkey]
                                - Point<int16_t>(0, 3));
                } else {
                    if (fkey == KeyConfig::Key::LEFT_CONTROL
                        || fkey == KeyConfig::Key::RIGHT_CONTROL) {
                        ficon->draw(position_
                                    + keys_pos_[KeyConfig::Key::LEFT_CONTROL]
                                    - Point<int16_t>(2, 3));
                        ficon->draw(position_
                                    + keys_pos_[KeyConfig::Key::RIGHT_CONTROL]
                                    - Point<int16_t>(2, 3));
                    } else if (fkey == KeyConfig::Key::LEFT_ALT
                               || fkey == KeyConfig::Key::RIGHT_ALT) {
                        ficon->draw(position_
                                    + keys_pos_[KeyConfig::Key::LEFT_ALT]
                                    - Point<int16_t>(2, 3));
                        ficon->draw(position_
                                    + keys_pos_[KeyConfig::Key::RIGHT_ALT]
                                    - Point<int16_t>(2, 3));
                    } else if (fkey == KeyConfig::Key::LEFT_SHIFT
                               || fkey == KeyConfig::Key::RIGHT_SHIFT) {
                        ficon->draw(position_
                                    + keys_pos_[KeyConfig::Key::LEFT_SHIFT]
                                    - Point<int16_t>(2, 3));
                        ficon->draw(position_
                                    + keys_pos_[KeyConfig::Key::RIGHT_SHIFT]
                                    - Point<int16_t>(2, 3));
                    } else {
                        ficon->draw(position_ + keys_pos_[fkey]
                                    - Point<int16_t>(2, 3));
                    }
                }
            }
        }
    }

    for (auto [key_actionid, ubaction_icon] : action_icons_) {
        if (ubaction_icon) {
            if (std::find(bound_actions_.begin(),
                          bound_actions_.end(),
                          key_actionid)
                == bound_actions_.end()) {
                ubaction_icon->draw(position_
                                    + unbound_actions_pos_[key_actionid]);
            }
        }
    }

    for (auto [key, tx] : key_textures_) {
        tx.draw(position_ + keys_pos_[key]);
    }
}

void UIKeyConfig::close() {
    clear_tooltip();
    deactivate();
    reset();
}

Button::State UIKeyConfig::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::CLOSE:
        case Buttons::CANCEL: close(); break;
        case Buttons::DEFAULT: {
            const std::string message =
                "Would you like to revert to default settings?";

            auto onok = [&](bool ok) {
                if (ok) {
                    auto keysel_onok = [&](bool alternate) {
                        clear();

                        if (alternate) {
                            staged_mappings_ = alternate_keys_;
                        } else {
                            staged_mappings_ = basic_keys_;
                        }

                        bind_staged_action_keys();
                    };

                    UI::get().emplace<UIKeySelect>(keysel_onok, false);
                }
            };

            UI::get().emplace<UIOk>(message, onok);
            break;
        }
        case Buttons::DELETE: {
            const std::string message =
                "Would you like to clear all key bindings?";

            auto onok = [&](bool ok) {
                if (ok) {
                    clear();
                }
            };

            UI::get().emplace<UIOk>(message, onok);
            break;
        }
        case Buttons::KEY_SETTING: break;
        case Buttons::OK: {
            save_staged_mappings();
            close();
            break;
        }
        default: break;
    }

    return Button::State::NORMAL;
}

Cursor::State UIKeyConfig::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Cursor::State dstate = UIDragElement::send_cursor(clicked, cursorpos);

    if (dragged_) {
        return dstate;
    }

    KeyAction::Id icon_slot = unbound_action_by_position(cursorpos);

    if (icon_slot != KeyAction::Id::LENGTH) {
        if (auto *icon = action_icons_[icon_slot].get()) {
            if (clicked) {
                icon->start_drag(cursorpos - position_
                                 - unbound_actions_pos_[icon_slot]);
                UI::get().drag_icon(icon);

                return Cursor::State::GRABBING;
            }
            return Cursor::State::CAN_GRAB;
        }
    }

    clear_tooltip();

    KeyConfig::Key key_slot = key_by_position(cursorpos);

    if (key_slot != KeyConfig::Key::LENGTH) {
        Keyboard::Mapping mapping = get_staged_mapping(key_slot);

        if (mapping.type != KeyType::Id::NONE) {
            Icon *ficon = nullptr;

            if (mapping.type == KeyType::Id::ITEM) {
                int32_t item_id = mapping.action;
                ficon = item_icons_[item_id].get();

                show_item(item_id);
            } else if (mapping.type == KeyType::Id::SKILL) {
                int32_t skill_id = mapping.action;
                ficon = skill_icons_[skill_id].get();

                show_skill(skill_id);
            } else if (is_action_mapping(mapping)) {
                KeyAction::Id action =
                    KeyAction::get_action_by_id(mapping.action);
                ficon = action_icons_[action].get();
            } else {
                std::cout << "Invalid icon type for key mapping: ("
                          << mapping.type << ", " << mapping.action << ")"
                          << std::endl;
            }

            if (ficon) {
                if (clicked) {
                    clear_tooltip();

                    ficon->start_drag(cursorpos - position_
                                      - keys_pos_[key_slot]);
                    UI::get().drag_icon(ficon);

                    return Cursor::State::GRABBING;
                }
                return Cursor::State::CAN_GRAB;
            }
        }
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

bool UIKeyConfig::send_icon(const Icon &icon, Point<int16_t> cursorpos) {
    for (auto iter : unbound_actions_pos_) {
        Rectangle<int16_t> icon_rect = Rectangle<int16_t>(
            position_ + iter.second,
            position_ + iter.second + Point<int16_t>(32, 32));

        if (icon_rect.contains(cursorpos)) {
            icon.drop_on_bindings(cursorpos, true);
        }
    }

    KeyConfig::Key fkey = key_by_position(cursorpos);

    if (fkey != KeyConfig::Key::LENGTH) {
        icon.drop_on_bindings(cursorpos, false);
    }

    return true;
}

void UIKeyConfig::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        safe_close();
    }
}

UIElement::Type UIKeyConfig::get_type() const {
    return TYPE;
}

void UIKeyConfig::safe_close() {
    if (dirty_) {
        const std::string message = "Do you want to save your changes?";

        auto onok = [&](bool ok) {
            if (ok) {
                save_staged_mappings();
                close();
            } else {
                close();
            }
        };

        UI::get().emplace<UIOk>(message, onok);
    } else {
        close();
    }
}

/// UI: Tooltip
void UIKeyConfig::show_item(int32_t item_id) {
    UI::get().show_item(Tooltip::Parent::KEY_CONFIG, item_id);
}

void UIKeyConfig::show_skill(int32_t skill_id) {
    int32_t level = skillbook_.get_level(skill_id);
    int32_t masterlevel = skillbook_.get_masterlevel(skill_id);
    int64_t expiration = skillbook_.get_expiration(skill_id);

    UI::get().show_skill(Tooltip::Parent::KEY_CONFIG,
                         skill_id,
                         level,
                         masterlevel,
                         expiration);
}

void UIKeyConfig::clear_tooltip() {
    UI::get().clear_tooltip(Tooltip::Parent::KEY_CONFIG);
}

/// Keymap Staging
void UIKeyConfig::stage_mapping(Point<int16_t> cursorposition,
                                Keyboard::Mapping mapping) {
    KeyConfig::Key key = key_by_position(cursorposition);
    Keyboard::Mapping prior_staged = staged_mappings_[key];

    if (prior_staged == mapping) {
        return;
    }

    unstage_mapping(prior_staged);

    if (is_action_mapping(mapping)) {
        KeyAction::Id action = KeyAction::get_action_by_id(mapping.action);
        auto iter =
            std::find(bound_actions_.begin(), bound_actions_.end(), action);

        if (iter == bound_actions_.end()) {
            bound_actions_.emplace_back(action);
        }
    }

    for (auto const &[mkey, staged_mapping] : staged_mappings_) {
        if (staged_mapping == mapping) {
            if (mkey == KeyConfig::Key::LEFT_CONTROL
                || mkey == KeyConfig::Key::RIGHT_CONTROL) {
                staged_mappings_.erase(KeyConfig::Key::LEFT_CONTROL);
                staged_mappings_.erase(KeyConfig::Key::RIGHT_CONTROL);
            } else if (mkey == KeyConfig::Key::LEFT_ALT
                       || mkey == KeyConfig::Key::RIGHT_ALT) {
                staged_mappings_.erase(KeyConfig::Key::LEFT_ALT);
                staged_mappings_.erase(KeyConfig::Key::RIGHT_ALT);
            } else if (mkey == KeyConfig::Key::LEFT_SHIFT
                       || mkey == KeyConfig::Key::RIGHT_SHIFT) {
                staged_mappings_.erase(KeyConfig::Key::LEFT_SHIFT);
                staged_mappings_.erase(KeyConfig::Key::RIGHT_SHIFT);
            } else {
                staged_mappings_.erase(mkey);
            }

            break;
        }
    }

    if (key == KeyConfig::Key::LEFT_CONTROL
        || key == KeyConfig::Key::RIGHT_CONTROL) {
        staged_mappings_[KeyConfig::Key::LEFT_CONTROL] = mapping;
        staged_mappings_[KeyConfig::Key::RIGHT_CONTROL] = mapping;
    } else if (key == KeyConfig::Key::LEFT_ALT
               || key == KeyConfig::Key::RIGHT_ALT) {
        staged_mappings_[KeyConfig::Key::LEFT_ALT] = mapping;
        staged_mappings_[KeyConfig::Key::RIGHT_ALT] = mapping;
    } else if (key == KeyConfig::Key::LEFT_SHIFT
               || key == KeyConfig::Key::RIGHT_SHIFT) {
        staged_mappings_[KeyConfig::Key::LEFT_SHIFT] = mapping;
        staged_mappings_[KeyConfig::Key::RIGHT_SHIFT] = mapping;
    } else {
        staged_mappings_[key] = mapping;
    }

    if (mapping.type == KeyType::Id::ITEM) {
        int32_t item_id = mapping.action;

        if (item_icons_.find(item_id) == item_icons_.end()) {
            int16_t count = inventory_.get_total_item_count(item_id);
            Texture tx = get_item_texture(item_id);
            item_icons_[item_id] = std::make_unique<Icon>(
                std::make_unique<CountableMappingIcon>(mapping, count),
                tx,
                count);
        }
    } else if (mapping.type == KeyType::Id::SKILL) {
        int32_t skill_id = mapping.action;

        if (skill_icons_.find(skill_id) == skill_icons_.end()) {
            Texture tx = get_skill_texture(skill_id);
            skill_icons_[skill_id] =
                std::make_unique<Icon>(std::make_unique<MappingIcon>(mapping),
                                       tx,
                                       -1);
        }
    }

    dirty_ = true;
}

void UIKeyConfig::unstage_mapping(Keyboard::Mapping mapping) {
    if (is_action_mapping(mapping)) {
        KeyAction::Id action = KeyAction::get_action_by_id(mapping.action);
        auto iter =
            std::find(bound_actions_.begin(), bound_actions_.end(), action);

        if (iter != bound_actions_.end()) {
            bound_actions_.erase(iter);
        }
    }

    for (auto [mkey, staged_mapping] : staged_mappings_) {
        if (staged_mapping == mapping) {
            if (mkey == KeyConfig::Key::LEFT_CONTROL
                || mkey == KeyConfig::Key::RIGHT_CONTROL) {
                staged_mappings_.erase(KeyConfig::Key::LEFT_CONTROL);
                staged_mappings_.erase(KeyConfig::Key::RIGHT_CONTROL);
            } else if (mkey == KeyConfig::Key::LEFT_ALT
                       || mkey == KeyConfig::Key::RIGHT_ALT) {
                staged_mappings_.erase(KeyConfig::Key::LEFT_ALT);
                staged_mappings_.erase(KeyConfig::Key::RIGHT_ALT);
            } else if (mkey == KeyConfig::Key::LEFT_SHIFT
                       || mkey == KeyConfig::Key::RIGHT_SHIFT) {
                staged_mappings_.erase(KeyConfig::Key::LEFT_SHIFT);
                staged_mappings_.erase(KeyConfig::Key::RIGHT_SHIFT);
            } else {
                staged_mappings_.erase(mkey);
            }

            if (staged_mapping.type == KeyType::Id::ITEM) {
                int32_t item_id = staged_mapping.action;
                item_icons_.erase(item_id);
            } else if (staged_mapping.type == KeyType::Id::SKILL) {
                int32_t skill_id = staged_mapping.action;
                skill_icons_.erase(skill_id);
            }

            dirty_ = true;

            break;
        }
    }
}

void UIKeyConfig::save_staged_mappings() {
    std::vector<std::tuple<KeyConfig::Key, KeyType::Id, int32_t>>
        updated_actions;

    for (auto [key, staged_mapping] : staged_mappings_) {
        KeyConfig::Key k = KeyConfig::actionbyid(key);
        Keyboard::Mapping saved_mapping = keyboard_->get_maple_mapping(key);

        if (staged_mapping != saved_mapping) {
            updated_actions.emplace_back(
                std::make_tuple(k, staged_mapping.type, staged_mapping.action));
        }
    }

    for (auto key : keyboard_->get_maplekeys()) {
        bool keyFound = false;
        KeyConfig::Key keyConfig = KeyConfig::actionbyid(key.first);

        for (auto tkey : staged_mappings_) {
            KeyConfig::Key tKeyConfig = KeyConfig::actionbyid(tkey.first);

            if (keyConfig == tKeyConfig) {
                keyFound = true;
                break;
            }
        }

        if (!keyFound) {
            updated_actions.emplace_back(
                std::make_tuple(keyConfig,
                                KeyType::Id::NONE,
                                KeyAction::Id::LENGTH));
        }
    }

    if (!updated_actions.empty()) {
        fn_change_keymap(updated_actions);
    }

    for (auto action : updated_actions) {
        KeyConfig::Key key = std::get<0>(action);
        KeyType::Id type = std::get<1>(action);
        int32_t keyAction = std::get<2>(action);

        if (type == KeyType::Id::NONE) {
            keyboard_->remove(key);
        } else {
            keyboard_->assign(key, type, keyAction);
        }
    }

    dirty_ = false;
}

void UIKeyConfig::bind_staged_action_keys() {
    for (auto fkey : key_textures_) {
        Keyboard::Mapping mapping = get_staged_mapping(fkey.first);

        if (mapping.type != KeyType::Id::NONE) {
            KeyAction::Id action = KeyAction::get_action_by_id(mapping.action);

            if (action) {
                bound_actions_.emplace_back(action);
            }
        }
    }
}

void UIKeyConfig::clear() {
    item_icons_.clear();
    skill_icons_.clear();
    bound_actions_.clear();
    staged_mappings_ = {};
    dirty_ = true;
}

void UIKeyConfig::reset() {
    clear();
    staged_mappings_ = keyboard_->get_maplekeys();
    load_item_icons();
    load_skill_icons();
    bind_staged_action_keys();
    dirty_ = false;
}

/// Helpers
Texture UIKeyConfig::get_item_texture(int32_t item_id) const {
    const ItemData &data = ItemData::get(item_id);
    return data.get_icon(false);
}

Texture UIKeyConfig::get_skill_texture(int32_t skill_id) const {
    const SkillData &data = SkillData::get(skill_id);
    return data.get_icon(SkillData::Icon::NORMAL);
}

KeyConfig::Key UIKeyConfig::key_by_position(Point<int16_t> cursorpos) const {
    for (auto [key, key_pos] : keys_pos_) {
        Rectangle<int16_t> icon_rect =
            Rectangle<int16_t>(position_ + key_pos,
                               position_ + key_pos + Point<int16_t>(32, 32));

        if (icon_rect.contains(cursorpos)) {
            return key;
        }
    }

    return KeyConfig::Key::LENGTH;
}

KeyAction::Id UIKeyConfig::unbound_action_by_position(
    Point<int16_t> cursorpos) const {
    for (auto [key_ubaction, ubaction_pos] : unbound_actions_pos_) {
        if (std::find(bound_actions_.begin(),
                      bound_actions_.end(),
                      key_ubaction)
            != bound_actions_.end()) {
            continue;
        }

        Rectangle<int16_t> icon_rect = Rectangle<int16_t>(
            position_ + ubaction_pos,
            position_ + ubaction_pos + Point<int16_t>(32, 32));

        if (icon_rect.contains(cursorpos)) {
            return key_ubaction;
        }
    }

    return KeyAction::Id::LENGTH;
}

Keyboard::Mapping UIKeyConfig::get_staged_mapping(int32_t keycode) const {
    auto iter = staged_mappings_.find(keycode);

    if (iter == staged_mappings_.end()) {
        return {};
    }

    return iter->second;
}

bool UIKeyConfig::is_action_mapping(Keyboard::Mapping mapping) const {
    return std::find(action_mappings_.begin(), action_mappings_.end(), mapping)
           != action_mappings_.end();
}

KeyType::Id UIKeyConfig::get_keytype(KeyAction::Id action) {
    switch (action) {
        case KeyAction::Id::EQUIPMENT:
        case KeyAction::Id::ITEMS:
        case KeyAction::Id::STATS:
        case KeyAction::Id::SKILLS:
        case KeyAction::Id::FRIENDS:
        case KeyAction::Id::WORLD_MAP:
        case KeyAction::Id::MAPLE_CHAT:
        case KeyAction::Id::MINI_MAP:
        case KeyAction::Id::QUEST_LOG:
        case KeyAction::Id::KEY_BINDINGS:
        case KeyAction::Id::TOGGLE_CHAT:
        case KeyAction::Id::WHISPER:
        case KeyAction::Id::SAY:
        case KeyAction::Id::PARTY_CHAT:
        case KeyAction::Id::MENU:
        case KeyAction::Id::QUICK_SLOTS:
        case KeyAction::Id::GUILD:
        case KeyAction::Id::FRIENDS_CHAT:
        case KeyAction::Id::PARTY:
        case KeyAction::Id::NOTIFIER:
        case KeyAction::Id::CASHSHOP:
        case KeyAction::Id::GUILD_CHAT:
        case KeyAction::Id::MEDALS:
        case KeyAction::Id::BITS:
        case KeyAction::Id::ALLIANCE_CHAT:
        case KeyAction::Id::MAPLE_NEWS:
        case KeyAction::Id::MANAGE_LEGION:
        case KeyAction::Id::PROFESSION:
        case KeyAction::Id::BOSS_PARTY:
        case KeyAction::Id::ITEM_POT:
        case KeyAction::Id::EVENT:
        case KeyAction::Id::SILENT_CRUSADE:
        case KeyAction::Id::BATTLE_ANALYSIS:
        case KeyAction::Id::GUIDE:
        case KeyAction::Id::VIEWERS_CHAT:
        case KeyAction::Id::ENHANCE_EQUIP:
        case KeyAction::Id::MONSTER_COLLECTION:
        case KeyAction::Id::SOUL_WEAPON:
        case KeyAction::Id::CHAR_INFO:
        case KeyAction::Id::CHANGE_CHANNEL:
        case KeyAction::Id::MAIN_MENU:
        case KeyAction::Id::SCREENSHOT:
        case KeyAction::Id::PICTURE_MODE:
        case KeyAction::Id::MAPLE_ACHIEVEMENT: return KeyType::Id::MENU;
        case KeyAction::Id::PICKUP:
        case KeyAction::Id::SIT:
        case KeyAction::Id::ATTACK:
        case KeyAction::Id::JUMP: return KeyType::Id::ACTION;
        case KeyAction::Id::INTERACT_HARVEST:
        case KeyAction::Id::MAPLE_STORAGE:
        case KeyAction::Id::SAFE_MODE:
        case KeyAction::Id::MUTE:
        case KeyAction::Id::MONSTER_BOOK:
        case KeyAction::Id::TO_SPOUSE: return KeyType::Id::MENU;
        case KeyAction::Id::FACE1:
        case KeyAction::Id::FACE2:
        case KeyAction::Id::FACE3:
        case KeyAction::Id::FACE4:
        case KeyAction::Id::FACE5:
        case KeyAction::Id::FACE6:
        case KeyAction::Id::FACE7: return KeyType::Id::FACE;
        case KeyAction::Id::LEFT:
        case KeyAction::Id::RIGHT:
        case KeyAction::Id::UP:
        case KeyAction::Id::DOWN:
        case KeyAction::Id::BACK:
        case KeyAction::Id::TAB:
        case KeyAction::Id::RETURN:
        case KeyAction::Id::ESCAPE:
        case KeyAction::Id::SPACE:
        case KeyAction::Id::DELETE:
        case KeyAction::Id::HOME:
        case KeyAction::Id::END:
        case KeyAction::Id::COPY:
        case KeyAction::Id::PASTE:
        case KeyAction::Id::LENGTH:
        default: return KeyType::Id::NONE;
    }
}

/// Item count
void UIKeyConfig::update_item_count(InventoryType::Id type,
                                    int16_t slot,
                                    int16_t change) {
    int32_t item_id = inventory_.get_item_id(type, slot);

    if (item_icons_.find(item_id) == item_icons_.end()) {
        return;
    }

    int16_t item_count = item_icons_[item_id]->get_count();
    item_icons_[item_id]->set_count(item_count + change);
}

/// MappingIcon
UIKeyConfig::MappingIcon::MappingIcon(Keyboard::Mapping m) : mapping_(m) {}

UIKeyConfig::MappingIcon::MappingIcon(KeyAction::Id action) {
    KeyType::Id type = UIKeyConfig::get_keytype(action);
    mapping_ = Keyboard::Mapping(type, action);
}

void UIKeyConfig::MappingIcon::drop_on_stage() const {
    if (mapping_.type == KeyType::Id::ITEM
        || mapping_.type == KeyType::Id::SKILL) {
        auto keyconfig = UI::get().get_element<UIKeyConfig>();
        keyconfig->get().unstage_mapping(mapping_);
    }
}

void UIKeyConfig::MappingIcon::drop_on_bindings(Point<int16_t> cursorposition,
                                                bool remove) const {
    auto keyconfig = UI::get().get_element<UIKeyConfig>();

    if (remove) {
        keyconfig->get().unstage_mapping(mapping_);
    } else {
        keyconfig->get().stage_mapping(cursorposition, mapping_);
    }
}

Icon::IconType UIKeyConfig::MappingIcon::get_type() {
    return Icon::IconType::KEY;
}

UIKeyConfig::CountableMappingIcon::CountableMappingIcon(Keyboard::Mapping m,
                                                        int16_t c) :
    UIKeyConfig::MappingIcon(m),
    count_(c) {}

void UIKeyConfig::CountableMappingIcon::set_count(int16_t c) {
    count_ = c;
}
}  // namespace ms