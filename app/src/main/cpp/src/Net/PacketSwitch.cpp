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
#include "PacketSwitch.h"

#include "../Configuration.h"
#include "Handlers/AttackHandlers.h"
#include "Handlers/CashShopHandlers.h"
#include "Handlers/CommonHandlers.h"
#include "Handlers/InventoryHandlers.h"
#include "Handlers/LoginHandlers.h"
#include "Handlers/MapObjectHandlers.h"
#include "Handlers/MessagingHandlers.h"
#include "Handlers/NpcInteractionHandlers.h"
#include "Handlers/PlayerHandlers.h"
#include "Handlers/PlayerInteractionHandlers.h"
#include "Handlers/SetFieldHandlers.h"
#include "Handlers/TestingHandlers.h"

namespace ms {
static const std::unordered_map<uint16_t, std::string_view> op_name_map {
    { 0, "LOGIN_STATUS" },
    { 1, "GUEST_ID_LOGIN" },
    { 2, "ACCOUNT_INFO" },
    { 3, "SERVER_STATUS" },
    { 4, "GENDER_DONE" },
    { 5, "CONFIRM_EULA_RESULT" },
    { 6, "CHECK_PINCODE" },
    { 7, "UPDATE_PINCODE" },
    { 8, "VIEW_ALL_CHAR" },
    { 9, "SELECT_CHARACTER_BY_VAC" },
    { 10, "SERVER_LIST" },
    { 11, "CHAR_LIST" },
    { 12, "SERVER_IP" },
    { 13, "CHAR_NAME_RESPONSE" },
    { 14, "ADD_NEW_CHAR_ENTRY" },
    { 15, "DELETE_CHAR_RESPONSE" },
    { 16, "CHANGE_CHANNEL" },
    { 17, "PING" },
    { 18, "KOREAN_INTERNET_CAFE_SHIT" },
    { 20, "CHANNEL_SELECTED" },
    { 22, "RELOG_RESPONSE" },
    { 25, "CHECK_CRC_RESULT" },
    { 26, "LAST_CONNECTED_WORLD" },
    { 27, "RECOMMENDED_WORLD_MESSAGE" },
    { 28, "CHECK_SPW_RESULT" },
    { 29, "INVENTORY_OPERATION" },
    { 30, "INVENTORY_GROW" },
    { 31, "STAT_CHANGED" },
    { 32, "GIVE_BUFF" },
    { 33, "CANCEL_BUFF" },
    { 34, "FORCED_STAT_SET" },
    { 35, "FORCED_STAT_RESET" },
    { 36, "UPDATE_SKILLS" },
    { 37, "SKILL_USE_RESULT" },
    { 38, "FAME_RESPONSE" },
    { 39, "SHOW_STATUS_INFO" },
    { 40, "OPEN_FULL_CLIENT_DOWNLOAD_LINK" },
    { 41, "MEMO_RESULT" },
    { 42, "MAP_TRANSFER_RESULT" },
    { 43, "WEDDING_PHOTO" },
    { 45, "CLAIM_RESULT" },
    { 46, "CLAIM_AVAILABLE_TIME" },
    { 47, "CLAIM_STATUS_CHANGED" },
    { 48, "SET_TAMING_MOB_INFO" },
    { 49, "QUEST_CLEAR" },
    { 50, "ENTRUSTED_SHOP_CHECK_RESULT" },
    { 51, "SKILL_LEARN_ITEM_RESULT" },
    { 52, "GATHER_ITEM_RESULT" },
    { 53, "SORT_ITEM_RESULT" },
    { 55, "SUE_CHARACTER_RESULT" },
    { 57, "TRADE_MONEY_LIMIT" },
    { 58, "SET_GENDER" },
    { 59, "GUILD_BBS_PACKET" },
    { 61, "CHAR_INFO" },
    { 62, "PARTY_OPERATION" },
    { 63, "BUDDYLIST" },
    { 65, "GUILD_OPERATION" },
    { 66, "ALLIANCE_OPERATION" },
    { 67, "SPAWN_PORTAL" },
    { 68, "SERVERMESSAGE" },
    { 69, "INCUBATOR_RESULT" },
    { 70, "SHOP_SCANNER_RESULT" },
    { 71, "SHOP_LINK_RESULT" },
    { 72, "MARRIAGE_REQUEST" },
    { 73, "MARRIAGE_RESULT" },
    { 74, "WEDDING_GIFT_RESULT" },
    { 75, "NOTIFY_MARRIED_PARTNER_MAP_TRANSFER" },
    { 76, "CASH_PET_FOOD_RESULT" },
    { 77, "SET_WEEK_EVENT_MESSAGE" },
    { 78, "SET_POTION_DISCOUNT_RATE" },
    { 79, "BRIDLE_MOB_CATCH_FAIL" },
    { 80, "IMITATED_NPC_RESULT" },
    { 81, "IMITATED_NPC_DATA" },
    { 82, "LIMITED_NPC_DISABLE_INFO" },
    { 83, "MONSTER_BOOK_SET_CARD" },
    { 84, "MONSTER_BOOK_SET_COVER" },
    { 85, "HOUR_CHANGED" },
    { 86, "MINIMAP_ON_OFF" },
    { 87, "CONSULT_AUTHKEY_UPDATE" },
    { 88, "CLASS_COMPETITION_AUTHKEY_UPDATE" },
    { 89, "WEB_BOARD_AUTHKEY_UPDATE" },
    { 90, "SESSION_VALUE" },
    { 91, "PARTY_VALUE" },
    { 92, "FIELD_SET_VARIABLE" },
    { 93, "BONUS_EXP_CHANGED" },
    { 94, "FAMILY_CHART_RESULT" },
    { 95, "FAMILY_INFO_RESULT" },
    { 96, "FAMILY_RESULT" },
    { 97, "FAMILY_JOIN_REQUEST" },
    { 98, "FAMILY_JOIN_REQUEST_RESULT" },
    { 99, "FAMILY_JOIN_ACCEPTED" },
    { 100, "FAMILY_PRIVILEGE_LIST" },
    { 101, "FAMILY_REP_GAIN" },
    { 102, "FAMILY_NOTIFY_LOGIN_OR_LOGOUT" },
    { 103, "FAMILY_SET_PRIVILEGE" },
    { 104, "FAMILY_SUMMON_REQUEST" },
    { 105, "NOTIFY_LEVELUP" },
    { 106, "NOTIFY_MARRIAGE" },
    { 107, "NOTIFY_JOB_CHANGE" },
    { 109, "MAPLE_TV_USE_RES" },
    { 110, "AVATAR_MEGAPHONE_RESULT" },
    { 111, "SET_AVATAR_MEGAPHONE" },
    { 112, "CLEAR_AVATAR_MEGAPHONE" },
    { 113, "CANCEL_NAME_CHANGE_RESULT" },
    { 114, "CANCEL_TRANSFER_WORLD_RESULT" },
    { 115, "DESTROY_SHOP_RESULT" },
    { 116, "FAKE_GM_NOTICE" },
    { 117, "SUCCESS_IN_USE_GACHAPON_BOX" },
    { 118, "NEW_YEAR_CARD_RES" },
    { 119, "RANDOM_MORPH_RES" },
    { 120, "CANCEL_NAME_CHANGE_BY_OTHER" },
    { 121, "SET_EXTRA_PENDANT_SLOT" },
    { 122, "SCRIPT_PROGRESS_MESSAGE" },
    { 123, "DATA_CRC_CHECK_FAILED" },
    { 124, "MACRO_SYS_DATA_INIT" },
    { 125, "SET_FIELD" },
    { 126, "SET_ITC" },
    { 127, "SET_CASH_SHOP" },
    { 128, "SET_BACK_EFFECT" },
    { 129, "SET_MAP_OBJECT_VISIBLE" },
    { 130, "CLEAR_BACK_EFFECT" },
    { 131, "BLOCKED_MAP" },
    { 132, "BLOCKED_SERVER" },
    { 133, "FORCED_MAP_EQUIP" },
    { 134, "MULTICHAT" },
    { 135, "WHISPER" },
    { 136, "SPOUSE_CHAT" },
    { 137, "SUMMON_ITEM_INAVAILABLE" },
    { 138, "FIELD_EFFECT" },
    { 139, "FIELD_OBSTACLE_ONOFF" },
    { 140, "FIELD_OBSTACLE_ONOFF_LIST" },
    { 141, "FIELD_OBSTACLE_ALL_RESET" },
    { 142, "BLOW_WEATHER" },
    { 143, "PLAY_JUKEBOX" },
    { 144, "ADMIN_RESULT" },
    { 145, "OX_QUIZ" },
    { 146, "GMEVENT_INSTRUCTIONS" },
    { 147, "CLOCK" },
    { 148, "CONTI_MOVE" },
    { 149, "CONTI_STATE" },
    { 150, "SET_QUEST_CLEAR" },
    { 151, "SET_QUEST_TIME" },
    { 152, "ARIANT_RESULT" },
    { 153, "SET_OBJECT_STATE" },
    { 154, "STOP_CLOCK" },
    { 155, "ARIANT_ARENA_SHOW_RESULT" },
    { 157, "PYRAMID_GAUGE" },
    { 158, "PYRAMID_SCORE" },
    { 159, "QUICKSLOT_INIT" },
    { 160, "SPAWN_PLAYER" },
    { 161, "REMOVE_PLAYER_FROM_MAP" },
    { 162, "CHATTEXT" },
    { 163, "CHATTEXT1" },
    { 164, "CHALKBOARD" },
    { 165, "UPDATE_CHAR_BOX" },
    { 166, "SHOW_CONSUME_EFFECT" },
    { 167, "SHOW_SCROLL_EFFECT" },
    { 168, "SPAWN_PET" },
    { 170, "MOVE_PET" },
    { 171, "PET_CHAT" },
    { 172, "PET_NAMECHANGE" },
    { 173, "PET_EXCEPTION_LIST" },
    { 174, "PET_COMMAND" },
    { 175, "SPAWN_SPECIAL_MAPOBJECT" },
    { 176, "REMOVE_SPECIAL_MAPOBJECT" },
    { 177, "MOVE_SUMMON" },
    { 178, "SUMMON_ATTACK" },
    { 179, "DAMAGE_SUMMON" },
    { 180, "SUMMON_SKILL" },
    { 181, "SPAWN_DRAGON" },
    { 182, "MOVE_DRAGON" },
    { 183, "REMOVE_DRAGON" },
    { 185, "MOVE_PLAYER" },
    { 186, "CLOSE_RANGE_ATTACK" },
    { 187, "RANGED_ATTACK" },
    { 188, "MAGIC_ATTACK" },
    { 189, "ENERGY_ATTACK" },
    { 190, "SKILL_EFFECT" },
    { 191, "CANCEL_SKILL_EFFECT" },
    { 192, "DAMAGE_PLAYER" },
    { 193, "FACIAL_EXPRESSION" },
    { 194, "SHOW_ITEM_EFFECT" },
    { 196, "SHOW_CHAIR" },
    { 197, "UPDATE_CHAR_LOOK" },
    { 198, "SHOW_FOREIGN_EFFECT" },
    { 199, "GIVE_FOREIGN_BUFF" },
    { 200, "CANCEL_FOREIGN_BUFF" },
    { 201, "UPDATE_PARTYMEMBER_HP" },
    { 202, "GUILD_NAME_CHANGED" },
    { 203, "GUILD_MARK_CHANGED" },
    { 204, "THROW_GRENADE" },
    { 205, "CANCEL_CHAIR" },
    { 206, "SHOW_ITEM_GAIN_INCHAT" },
    { 207, "DOJO_WARP_UP" },
    { 208, "LUCKSACK_PASS" },
    { 209, "LUCKSACK_FAIL" },
    { 210, "MESO_BAG_MESSAGE" },
    { 211, "UPDATE_QUEST_INFO" },
    { 214, "PLAYER_HINT" },
    { 217, "MAKER_RESULT" },
    { 219, "KOREAN_EVENT" },
    { 220, "OPEN_UI" },
    { 221, "LOCK_UI" },
    { 222, "DISABLE_UI" },
    { 223, "SPAWN_GUIDE" },
    { 224, "TALK_GUIDE" },
    { 225, "SHOW_COMBO" },
    { 234, "COOLDOWN" },
    { 236, "SPAWN_MONSTER" },
    { 237, "KILL_MONSTER" },
    { 238, "SPAWN_MONSTER_CONTROL" },
    { 239, "MOVE_MONSTER" },
    { 240, "MOVE_MONSTER_RESPONSE" },
    { 242, "APPLY_MONSTER_STATUS" },
    { 243, "CANCEL_MONSTER_STATUS" },
    { 244, "RESET_MONSTER_ANIMATION" },
    { 246, "DAMAGE_MONSTER" },
    { 249, "ARIANT_THING" },
    { 250, "SHOW_MONSTER_HP" },
    { 251, "CATCH_MONSTER" },
    { 252, "CATCH_MONSTER_WITH_ITEM" },
    { 253, "SHOW_MAGNET" },
    { 257, "SPAWN_NPC" },
    { 258, "REMOVE_NPC" },
    { 259, "SPAWN_NPC_REQUEST_CONTROLLER" },
    { 260, "NPC_ACTION" },
    { 263, "SET_NPC_SCRIPTABLE" },
    { 265, "SPAWN_HIRED_MERCHANT" },
    { 266, "DESTROY_HIRED_MERCHANT" },
    { 267, "UPDATE_HIRED_MERCHANT" },
    { 268, "DROP_ITEM_FROM_MAPOBJECT" },
    { 269, "REMOVE_ITEM_FROM_MAP" },
    { 270, "CANNOT_SPAWN_KITE" },
    { 271, "SPAWN_KITE" },
    { 272, "REMOVE_KITE" },
    { 273, "SPAWN_MIST" },
    { 274, "REMOVE_MIST" },
    { 275, "SPAWN_DOOR" },
    { 276, "REMOVE_DOOR" },
    { 277, "REACTOR_HIT" },
    { 279, "REACTOR_SPAWN" },
    { 280, "REACTOR_DESTROY" },
    { 281, "SNOWBALL_STATE" },
    { 282, "HIT_SNOWBALL" },
    { 283, "SNOWBALL_MESSAGE" },
    { 284, "LEFT_KNOCK_BACK" },
    { 285, "COCONUT_HIT" },
    { 286, "COCONUT_SCORE" },
    { 287, "GUILD_BOSS_HEALER_MOVE" },
    { 288, "GUILD_BOSS_PULLEY_STATE_CHANGE" },
    { 289, "MONSTER_CARNIVAL_START" },
    { 290, "MONSTER_CARNIVAL_OBTAINED_CP" },
    { 291, "MONSTER_CARNIVAL_PARTY_CP" },
    { 292, "MONSTER_CARNIVAL_SUMMON" },
    { 293, "MONSTER_CARNIVAL_MESSAGE" },
    { 294, "MONSTER_CARNIVAL_DIED" },
    { 295, "MONSTER_CARNIVAL_LEAVE" },
    { 297, "ARIANT_ARENA_USER_SCORE" },
    { 299, "SHEEP_RANCH_INFO" },
    { 300, "SHEEP_RANCH_CLOTHES" },
    { 301, "WITCH_TOWER_SCORE_UPDATE" },
    { 302, "HORNTAIL_CAVE" },
    { 303, "ZAKUM_SHRINE" },
    { 304, "NPC_TALK" },
    { 305, "OPEN_NPC_SHOP" },
    { 306, "CONFIRM_SHOP_TRANSACTION" },
    { 307, "ADMIN_SHOP_MESSAGE" },
    { 308, "ADMIN_SHOP" },
    { 309, "STORAGE" },
    { 310, "FREDRICK_MESSAGE" },
    { 311, "FREDRICK" },
    { 312, "RPS_GAME" },
    { 313, "MESSENGER" },
    { 314, "PLAYER_INTERACTION" },
    { 315, "TOURNAMENT" },
    { 316, "TOURNAMENT_MATCH_TABLE" },
    { 317, "TOURNAMENT_SET_PRIZE" },
    { 318, "TOURNAMENT_UEW" },
    { 319, "TOURNAMENT_CHARACTERS" },
    { 320, "WEDDING_PROGRESS" },
    { 321, "WEDDING_CEREMONY_END" },
    { 322, "PARCEL" },
    { 323, "CHARGE_PARAM_RESULT" },
    { 324, "QUERY_CASH_RESULT" },
    { 325, "CASHSHOP_OPERATION" },
    { 326, "CASHSHOP_PURCHASE_EXP_CHANGED" },
    { 327, "CASHSHOP_GIFT_INFO_RESULT" },
    { 328, "CASHSHOP_CHECK_NAME_CHANGE" },
    { 329, "CASHSHOP_CHECK_NAME_CHANGE_POSSIBLE_RESULT" },
    { 330, "CASHSHOP_REGISTER_NEW_CHARACTER_RESULT" },
    { 331, "CASHSHOP_CHECK_TRANSFER_WORLD_POSSIBLE_RESULT" },
    { 332, "CASHSHOP_GACHAPON_STAMP_RESULT" },
    { 333, "CASHSHOP_CASH_ITEM_GACHAPON_RESULT" },
    { 334, "CASHSHOP_CASH_GACHAPON_OPEN_RESULT" },
    { 335, "KEYMAP" },
    { 336, "AUTO_HP_POT" },
    { 337, "AUTO_MP_POT" },
    { 341, "SEND_TV" },
    { 342, "REMOVE_TV" },
    { 343, "ENABLE_TV" },
    { 347, "MTS_OPERATION2" },
    { 348, "MTS_OPERATION" },
    { 349, "MAPLELIFE_RESULT" },
    { 350, "MAPLELIFE_ERROR" },
    { 354, "VICIOUS_HAMMER" },
    { 358, "VEGA_SCROLL" }
};

// Opcodes for InPackets
enum Opcode : uint16_t {
    /// Login 1
    LOGIN_RESULT = 0,
    SERVER_STATUS = 3,
    SERVER_LIST = 10,
    CHAR_LIST = 11,
    SERVER_IP = 12,
    CHARNAME_RESPONSE = 13,
    ADD_NEWCHAR_ENTRY = 14,
    DELCHAR_RESPONSE = 15,
    PING = 17,

    /// Login 2
    RECOMMENDED_WORLDS = 27,
    CHECK_SPW_RESULT = 28,

    /// Inventory 1
    MODIFY_INVENTORY = 29,

    /// Player 2
    CHANGE_CHANNEL = 16,
    CHANGE_STATS = 31,
    GIVE_BUFF = 32,
    CANCEL_BUFF = 33,
    RECALCULATE_STATS = 35,
    UPDATE_SKILL = 36,
    FAME_RESPONSE = 38,

    /// Messaging 1
    SHOW_STATUS_INFO = 39,

    /// Inventory 2
    GATHER_RESULT = 52,
    SORT_RESULT = 53,

    /// Player 3

    /// Messaging 2
    SERVER_MESSAGE = 68,
    WEEK_EVENT_MESSAGE = 77,

    SKILL_MACROS = 124,
    SET_FIELD = 125,
    WHISPER = 135,
    FIELD_EFFECT = 138,

    /// MapObject
    SPAWN_CHAR = 160,
    REMOVE_CHAR = 161,

    /// Messaging
    CHAT_RECEIVED = 162,
    SCROLL_RESULT = 167,

    /// MapObject
    SPAWN_PET = 168,
    CHAR_MOVED = 185,

    /// Attack
    ATTACKED_CLOSE = 186,
    ATTACKED_RANGED = 187,
    ATTACKED_MAGIC = 188,

    UPDATE_CHARLOOK = 197,
    SHOW_FOREIGN_EFFECT = 198,
    GIVE_FOREIGN_BUFF = 199,
    CANCEL_FOREIGN_BUFF = 200,
    SHOW_ITEM_GAIN_INCHAT = 206,  // TODO: Rename this (Terribly named)

    /// Player
    ADD_COOLDOWN = 234,

    /// MapObject
    SPAWN_MOB = 236,
    KILL_MOB = 237,
    SPAWN_MOB_C = 238,
    MOB_MOVED = 239,
    MOB_MOVE_RESPONSE = 240,
    APPLY_MONSTER_STATUS = 242,
    CANCEL_MONSTER_STATUS = 243,
    SHOW_MOB_HP = 250,
    SPAWN_NPC = 257,
    SPAWN_NPC_C = 259,
    SET_NPC_SCRIPTABLE = 263,
    DROP_LOOT = 268,
    REMOVE_LOOT = 269,
    HIT_REACTOR = 277,
    SPAWN_REACTOR = 279,
    REMOVE_REACTOR = 280,

    /// NPC Interaction
    NPC_DIALOGUE = 304,
    OPEN_NPC_SHOP = 305,
    CONFIRM_SHOP_TRANSACTION = 306,
    KEYMAP = 335,

    /// Player Interaction
    CHAR_INFO = 61,
    PLAYER_INTERACTION = 314,

    /// Cash Shop
    SET_CASH_SHOP = 127
};

PacketSwitch::PacketSwitch() {
    // Common handlers
    emplace<PING, PingHandler>();

    // Login handlers
    emplace<LOGIN_RESULT, LoginResultHandler>();
    emplace<SERVER_STATUS, ServerStatusHandler>();
    emplace<SERVER_LIST, ServerlistHandler>();
    emplace<CHAR_LIST, CharlistHandler>();
    emplace<SERVER_IP, ServerIPHandler>();
    emplace<CHARNAME_RESPONSE, CharnameResponseHandler>();
    emplace<ADD_NEWCHAR_ENTRY, AddNewCharEntryHandler>();
    emplace<DELCHAR_RESPONSE, DeleteCharResponseHandler>();
    emplace<RECOMMENDED_WORLDS, RecommendedWorldsHandler>();

    // SetField handlers
    emplace<SET_FIELD, SetFieldHandler>();

    // // MapObject handlers
    emplace<SPAWN_CHAR, SpawnCharHandler>();
    emplace<CHAR_MOVED, CharMovedHandler>();
    emplace<UPDATE_CHARLOOK, UpdateCharLookHandler>();
    emplace<SHOW_FOREIGN_EFFECT, ShowForeignEffectHandler>();
    emplace<GIVE_FOREIGN_BUFF, GiveForeignBuffHandler>();
    emplace<CANCEL_FOREIGN_BUFF, CancelForeignBuffHandler>();
    emplace<REMOVE_CHAR, RemoveCharHandler>();
    emplace<SPAWN_PET, SpawnPetHandler>();
    emplace<SPAWN_NPC, SpawnNpcHandler>();
    emplace<SPAWN_NPC_C, SpawnNpcControllerHandler>();
    emplace<SET_NPC_SCRIPTABLE, SetNpcScriptableHandler>();
    emplace<SPAWN_MOB, SpawnMobHandler>();
    emplace<SPAWN_MOB_C, SpawnMobControllerHandler>();
    emplace<MOB_MOVED, MobMovedHandler>();
    emplace<MOB_MOVE_RESPONSE, MobMoveResponseHandler>();
    emplace<APPLY_MONSTER_STATUS, ApplyMobStatusHandler>();
    emplace<CANCEL_MONSTER_STATUS, CancelMobStatusHandler>();
    emplace<SHOW_MOB_HP, ShowMobHpHandler>();
    emplace<KILL_MOB, KillMobHandler>();
    emplace<DROP_LOOT, DropLootHandler>();
    emplace<REMOVE_LOOT, RemoveLootHandler>();
    emplace<HIT_REACTOR, HitReactorHandler>();
    emplace<SPAWN_REACTOR, SpawnReactorHandler>();
    emplace<REMOVE_REACTOR, RemoveReactorHandler>();

    // Attack handlers
    emplace<ATTACKED_CLOSE, CloseAttackHandler>();
    emplace<ATTACKED_RANGED, RangedAttackHandler>();
    emplace<ATTACKED_MAGIC, MagicAttackHandler>();

    // Player handlers
    emplace<CHANGE_CHANNEL, ChangeChannelHandler>();
    emplace<KEYMAP, KeymapHandler>();
    emplace<SKILL_MACROS, SkillMacrosHandler>();
    emplace<CHANGE_STATS, ChangeStatsHandler>();
    emplace<GIVE_BUFF, ApplyBuffHandler>();
    emplace<CANCEL_BUFF, CancelBuffHandler>();
    emplace<RECALCULATE_STATS, RecalculateStatsHandler>();
    emplace<UPDATE_SKILL, UpdateSkillHandler>();
    emplace<FAME_RESPONSE, FameResponseHandler>();
    emplace<ADD_COOLDOWN, AddCooldownHandler>();

    // Messaging handlers
    emplace<SHOW_STATUS_INFO, ShowStatusInfoHandler>();
    emplace<CHAT_RECEIVED, ChatReceivedHandler>();
    emplace<SCROLL_RESULT, ScrollResultHandler>();
    emplace<SERVER_MESSAGE, ServerMessageHandler>();
    emplace<WEEK_EVENT_MESSAGE, WeekEventMessageHandler>();
    emplace<WHISPER, WhisperReceivedHandler>();
    emplace<SHOW_ITEM_GAIN_INCHAT, ShowItemGainInChatHandler>();

    // Inventory Handlers
    emplace<MODIFY_INVENTORY, ModifyInventoryHandler>();
    emplace<GATHER_RESULT, GatherResultHandler>();
    emplace<SORT_RESULT, SortResultHandler>();

    // NPC Interaction Handlers
    emplace<NPC_DIALOGUE, NpcDialogueHandler>();
    emplace<OPEN_NPC_SHOP, OpenNpcShopHandler>();

    // Player Interaction
    emplace<CHAR_INFO, CharInfoHandler>();
    emplace<PLAYER_INTERACTION, PlayerInteractionHandler>();

    // Cash Shop
    emplace<SET_CASH_SHOP, SetCashShopHandler>();

    // TODO: New handlers, they need coded and moved to a proper file.
    emplace<CHECK_SPW_RESULT, CheckSpwResultHandler>();
    emplace<FIELD_EFFECT, FieldEffectHandler>();
}

void PacketSwitch::forward(int8_t *bytes, size_t length) const {
    // Wrap the bytes with a parser
    InPacket recv = { bytes, length };

    // Read the opcode to determine handler responsible
    uint16_t opcode = recv.read_ushort();

    if (Configuration::get().get_show_packets()) {
        if (opcode == PING) {
            std::cout << "Received Packet: PING" << std::endl;
        } else {
            std::cout << "Received Packet: " << std::to_string(opcode) << ' ';
            if (op_name_map.find(opcode) != op_name_map.end()) {
                std::cout << '[' << op_name_map.at(opcode) << ']';
            }
            std::cout << std::endl;
        }
    }

    if (opcode < handlers_.size()) {
        if (const auto &handler = handlers_[opcode]) {
            // Handler is good, packet is passed on

            try {
                std::cout << '[' << opcode << "] ";
                handler->handle(recv);
                std::cout << std::endl;
            } catch (const PacketError &err) {
                // Notice about an error
                warn(err.what(), opcode);
            }
        } else {
            // Warn about an unhandled packet
            warn(MSG_UNHANDLED, opcode);
        }
    } else {
        // Warn about a packet with opcode out of bounds
        warn(MSG_OUT_OF_BOUNDS, opcode);
    }
}

void PacketSwitch::warn(std::string_view message, size_t opcode) const {
    std::cout << "Opcode [" << opcode << "] Error: " << message << std::endl;
}
}  // namespace ms