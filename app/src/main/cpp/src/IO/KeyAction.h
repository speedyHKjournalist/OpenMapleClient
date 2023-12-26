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

#include <cstdint>

namespace ms::KeyAction {
// Maple-specific keycodes, sent via the Keymap Packet.
enum Id : int32_t {
    NONE = 0,
    EQUIPMENT = 1,
    ITEMS = 2,
    STATS = 3,
    SKILLS = 4,
    FRIENDS = 5,
    WORLD_MAP = 6,
    MAPLE_CHAT = 7,
    MINI_MAP = 8,
    QUEST_LOG = 9,
    KEY_BINDINGS = 10,
    TOGGLE_CHAT = 11,
    WHISPER = 12,
    SAY = 13,
    PARTY_CHAT = 14,
    MENU = 15,
    QUICK_SLOTS = 16,
    GUILD = 18,
    FRIENDS_CHAT = 19,
    PARTY = 20,
    NOTIFIER = 21,
    CASHSHOP = 24,
    GUILD_CHAT = 25,
    MEDALS = 28,
    BITS,
    ALLIANCE_CHAT,
    MAPLE_NEWS,
    MANAGE_LEGION,
    PROFESSION,
    BOSS_PARTY,
    ITEM_POT,
    EVENT,
    SILENT_CRUSADE,
    BATTLE_ANALYSIS,
    GUIDE,
    VIEWERS_CHAT,
    ENHANCE_EQUIP,
    MONSTER_COLLECTION,
    SOUL_WEAPON,
    CHAR_INFO,
    CHANGE_CHANNEL,
    MAIN_MENU,
    SCREENSHOT,
    PICTURE_MODE,
    MAPLE_ACHIEVEMENT,
    PICKUP = 51,
    SIT = 52,
    ATTACK = 53,
    JUMP = 54,
    INTERACT_HARVEST = 55,
    MAPLE_STORAGE,
    SAFE_MODE,
    MUTE,
    MONSTER_BOOK,
    TO_SPOUSE,
    FACE1 = 100,
    FACE2 = 101,
    FACE3 = 102,
    FACE4 = 103,
    FACE5 = 104,
    FACE6 = 105,
    FACE7 = 106,
    // Static keys
    LEFT,
    RIGHT,
    UP,
    DOWN,
    BACK,
    TAB,
    RETURN,
    ESCAPE,
    SPACE,
    DELETE,
    HOME,
    END,
    COPY,
    PASTE,
    LENGTH
};

inline Id get_action_by_id(int32_t id) {
    return static_cast<Id>(id);
}
}  // namespace ms::KeyAction