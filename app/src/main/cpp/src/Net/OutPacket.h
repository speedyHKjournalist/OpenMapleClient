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

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "../Template/Point.h"
#include "stdio.h"

namespace ms {
static const std::unordered_map<uint16_t, std::string_view> send_op_name_map {
    { 1, "LOGIN_PASSWORD" },
    { 2, "GUEST_LOGIN" },
    { 4, "SERVERLIST_REREQUEST" },
    { 5, "CHAR_LIST_REQUEST" },
    { 6, "SERVER_STATUS_REQUEST" },
    { 7, "ACCEPT_TOS" },
    { 8, "SET_GENDER" },
    { 9, "AFTER_LOGIN" },
    { 10, "REGISTER_PIN" },
    { 11, "SERVERLIST_REQUEST" },
    { 12, "PLAYER_DC" },
    { 13, "VIEW_ALL_CHAR" },
    { 14, "PICK_ALL_CHAR" },
    { 16, "NAME_TRANSFER" },
    { 18, "WORLD_TRANSFER" },
    { 19, "CHAR_SELECT" },
    { 20, "PLAYER_LOGGEDIN" },
    { 21, "CHECK_CHAR_NAME" },
    { 22, "CREATE_CHAR" },
    { 23, "DELETE_CHAR" },
    { 24, "PONG" },
    { 25, "CLIENT_START_ERROR" },
    { 26, "CLIENT_ERROR" },
    { 27, "STRANGE_DATA" },
    { 28, "RELOG" },
    { 29, "REGISTER_PIC" },
    { 30, "CHAR_SELECT_WITH_PIC" },
    { 31, "VIEW_ALL_PIC_REGISTER" },
    { 32, "VIEW_ALL_WITH_PIC" },
    { 38, "CHANGE_MAP" },
    { 39, "CHANGE_CHANNEL" },
    { 40, "ENTER_CASHSHOP" },
    { 41, "MOVE_PLAYER" },
    { 42, "CANCEL_CHAIR" },
    { 43, "USE_CHAIR" },
    { 44, "CLOSE_RANGE_ATTACK" },
    { 45, "RANGED_ATTACK" },
    { 46, "MAGIC_ATTACK" },
    { 47, "TOUCH_MONSTER_ATTACK" },
    { 48, "TAKE_DAMAGE" },
    { 49, "GENERAL_CHAT" },
    { 50, "CLOSE_CHALKBOARD" },
    { 51, "FACE_EXPRESSION" },
    { 52, "USE_ITEMEFFECT" },
    { 53, "USE_DEATHITEM" },
    { 56, "MOB_BANISH_PLAYER" },
    { 57, "MONSTER_BOOK_COVER" },
    { 58, "NPC_TALK" },
    { 59, "REMOTE_STORE" },
    { 60, "NPC_TALK_MORE" },
    { 61, "NPC_SHOP" },
    { 62, "STORAGE" },
    { 63, "HIRED_MERCHANT_REQUEST" },
    { 64, "FREDRICK_ACTION" },
    { 65, "DUEY_ACTION" },
    { 66, "OWL_ACTION" },
    { 67, "OWL_WARP" },
    { 68, "ADMIN_SHOP" },
    { 69, "ITEM_SORT" },
    { 70, "ITEM_SORT2" },
    { 71, "ITEM_MOVE" },
    { 72, "USE_ITEM" },
    { 73, "CANCEL_ITEM_EFFECT" },
    { 75, "USE_SUMMON_BAG" },
    { 76, "PET_FOOD" },
    { 77, "USE_MOUNT_FOOD" },
    { 78, "SCRIPTED_ITEM" },
    { 79, "USE_CASH_ITEM" },
    { 81, "USE_CATCH_ITEM" },
    { 82, "USE_SKILL_BOOK" },
    { 84, "USE_TELEPORT_ROCK" },
    { 85, "USE_RETURN_SCROLL" },
    { 86, "USE_UPGRADE_SCROLL" },
    { 87, "DISTRIBUTE_AP" },
    { 88, "AUTO_DISTRIBUTE_AP" },
    { 89, "HEAL_OVER_TIME" },
    { 90, "DISTRIBUTE_SP" },
    { 91, "SPECIAL_MOVE" },
    { 92, "CANCEL_BUFF" },
    { 93, "SKILL_EFFECT" },
    { 94, "MESO_DROP" },
    { 95, "GIVE_FAME" },
    { 97, "CHAR_INFO_REQUEST" },
    { 98, "SPAWN_PET" },
    { 99, "CANCEL_DEBUFF" },
    { 100, "CHANGE_MAP_SPECIAL" },
    { 101, "USE_INNER_PORTAL" },
    { 102, "TROCK_ADD_MAP" },
    { 106, "REPORT" },
    { 107, "QUEST_ACTION" },
    { 109, "GRENADE_EFFECT" },
    { 110, "SKILL_MACRO" },
    { 112, "USE_ITEM_REWARD" },
    { 113, "MAKER_SKILL" },
    { 116, "USE_REMOTE" },
    { 117, "WATER_OF_LIFE" },
    { 118, "ADMIN_CHAT" },
    { 119, "MULTI_CHAT" },
    { 120, "WHISPER" },
    { 121, "SPOUSE_CHAT" },
    { 122, "MESSENGER" },
    { 123, "PLAYER_INTERACTION" },
    { 124, "PARTY_OPERATION" },
    { 125, "DENY_PARTY_REQUEST" },
    { 126, "GUILD_OPERATION" },
    { 127, "DENY_GUILD_REQUEST" },
    { 128, "ADMIN_COMMAND" },
    { 129, "ADMIN_LOG" },
    { 130, "BUDDYLIST_MODIFY" },
    { 131, "NOTE_ACTION" },
    { 133, "USE_DOOR" },
    { 135, "CHANGE_KEYMAP" },
    { 136, "RPS_ACTION" },
    { 137, "RING_ACTION" },
    { 138, "WEDDING_ACTION" },
    { 139, "WEDDING_TALK" },
    { 143, "ALLIANCE_OPERATION" },
    { 144, "DENY_ALLIANCE_REQUEST" },
    { 145, "OPEN_FAMILY_PEDIGREE" },
    { 146, "OPEN_FAMILY" },
    { 147, "ADD_FAMILY" },
    { 148, "SEPARATE_FAMILY_BY_SENIOR" },
    { 149, "SEPARATE_FAMILY_BY_JUNIOR" },
    { 150, "ACCEPT_FAMILY" },
    { 151, "USE_FAMILY" },
    { 152, "CHANGE_FAMILY_MESSAGE" },
    { 153, "FAMILY_SUMMON_RESPONSE" },
    { 155, "BBS_OPERATION" },
    { 156, "ENTER_MTS" },
    { 157, "USE_SOLOMON_ITEM" },
    { 158, "USE_GACHA_EXP" },
    { 159, "NEW_YEAR_CARD_REQUEST" },
    { 161, "CASHSHOP_SURPRISE" },
    { 162, "CLICK_GUIDE" },
    { 163, "ARAN_COMBO_COUNTER" },
    { 167, "MOVE_PET" },
    { 168, "PET_CHAT" },
    { 169, "PET_COMMAND" },
    { 170, "PET_LOOT" },
    { 171, "PET_AUTO_POT" },
    { 172, "PET_EXCLUDE_ITEMS" },
    { 175, "MOVE_SUMMON" },
    { 176, "SUMMON_ATTACK" },
    { 177, "DAMAGE_SUMMON" },
    { 178, "BEHOLDER" },
    { 181, "MOVE_DRAGON" },
    { 183, "CHANGE_QUICKSLOT" },
    { 188, "MOVE_LIFE" },
    { 189, "AUTO_AGGRO" },
    { 191, "FIELD_DAMAGE_MOB" },
    { 192, "MOB_DAMAGE_MOB_FRIENDLY" },
    { 193, "MONSTER_BOMB" },
    { 194, "MOB_DAMAGE_MOB" },
    { 197, "NPC_ACTION" },
    { 202, "ITEM_PICKUP" },
    { 205, "DAMAGE_REACTOR" },
    { 206, "TOUCHING_REACTOR" },
    { 207, "PLAYER_MAP_TRANSFER" },
    { 65534, "MAPLETV" },
    { 211, "SNOWBALL" },
    { 212, "LEFT_KNOCKBACK" },
    { 213, "COCONUT" },
    { 214, "MATCH_TABLE" },
    { 218, "MONSTER_CARNIVAL" },
    { 220, "PARTY_SEARCH_REGISTER" },
    { 222, "PARTY_SEARCH_START" },
    { 223, "PARTY_SEARCH_UPDATE" },
    { 228, "CHECK_CASH" },
    { 229, "CASHSHOP_OPERATION" },
    { 230, "COUPON_CODE" },
    { 236, "OPEN_ITEMUI" },
    { 237, "CLOSE_ITEMUI" },
    { 238, "USE_ITEMUI" },
    { 253, "MTS_OPERATION" },
    { 256, "USE_MAPLELIFE" },
    { 260, "USE_HAMMER" }
};

// A packet to be sent to the server
// Used as a base class to create specific packets
class OutPacket {
public:
    // Construct a packet by writing its opcode
    OutPacket(int16_t opcode);

    void dispatch();

    std::vector<int8_t> build() { return std::move(bytes_); }

    // Opcodes for OutPackets associated with version 83 of the game
    enum Opcode : uint16_t {
        /// Login
        LOGIN = 1,
        CHAR_LIST_REQUEST = 5,
        SERVER_STATUS_REQUEST = 6,
        ACCEPT_TOS = 7,
        SET_GENDER = 8,
        SERVERLIST_REQUEST = 11,
        SELECT_CHAR = 19,
        PLAYER_LOGIN = 20,
        NAME_CHAR = 21,
        CREATE_CHAR = 22,
        DELETE_CHAR = 23,
        PONG = 24,
        REGISTER_PIC = 29,
        SELECT_CHAR_PIC = 30,
        LOGIN_START = 35,  // Custom name

        /// Gameplay 1
        CHANGE_MAP = 38,
        CHANGE_CHANNEL = 39,
        ENTER_CASHSHOP = 40,
        MOVE_PLAYER = 41,
        CLOSE_ATTACK = 44,
        RANGED_ATTACK = 45,
        MAGIC_ATTACK = 46,
        TAKE_DAMAGE = 48,

        /// Messaging
        GENERAL_CHAT = 49,

        /// NPC Interaction
        TALK_TO_NPC = 58,
        NPC_TALK_MORE = 60,
        NPC_SHOP_ACTION = 61,

        /// Player Interaction
        GIVE_FAME = 95,
        CHAR_INFO_REQUEST = 97,

        /// Inventory
        GATHER_ITEMS = 69,
        SORT_ITEMS = 70,
        MOVE_ITEM = 71,
        USE_ITEM = 72,
        SCROLL_EQUIP = 86,
        DROP_MESOS = 94,

        /// Player
        SPEND_AP = 87,
        SPEND_SP = 90,
        CHANGE_KEYMAP = 135,

        /// Skill
        USE_SKILL = 91,

        /// Gameplay 2
        PLAYER_INTERACTION = 123,
        PARTY_OPERATION = 124,
        ADMIN_COMMAND = 128,
        MOVE_MONSTER = 188,
        PICKUP_ITEM = 202,
        DAMAGE_REACTOR = 205,
        PLAYER_MAP_TRANSFER = 207,
        PLAYER_UPDATE = 223
    };

protected:
    // Skip a number of bytes (filled with zeros)
    void skip(size_t count);
    // Write a byte
    void write_byte(int8_t ch);
    // Write a short
    void write_short(int16_t sh);
    // Write an int
    void write_int(int32_t in);
    // Write a long
    void write_long(int64_t lg);

    // Write a point
    // One short for x and one for y
    void write_point(Point<int16_t> point);
    // Write a timestamp as an integer
    void write_time();
    // Write a string
    // Writes the length as a short and then each individual character as a byte
    void write_string(const std::string &str);
    // Write a random int
    void write_random();

    // Write the MACS and then write the HWID of the computer
    void write_hardware_info();
    // Function to convert hexadecimal to decimal
    int32_t hex_to_dec(std::string hexVal);

private:
    std::vector<int8_t> bytes_;
    int16_t opcode_;
};
}  // namespace ms