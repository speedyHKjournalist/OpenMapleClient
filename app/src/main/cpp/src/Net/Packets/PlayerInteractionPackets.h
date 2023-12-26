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

#include "../OutPacket.h"

namespace ms {
// Packet to fame/defame a character
// Opcode: GIVE_FAME(95)
class GiveFamePacket : public OutPacket {
public:
    enum mode { INCREASE = 1, DECREASE = 0 };

    GiveFamePacket(int32_t oid, mode mode) :
        OutPacket(OutPacket::Opcode::GIVE_FAME) {
        write_int(oid);
        write_byte(mode);
    }
};

// Packet to request character info
// Opcode: CHAR_INFO_REQUEST(97)
class CharInfoRequestPacket : public OutPacket {
public:
    CharInfoRequestPacket(int32_t character_id) :
        OutPacket(OutPacket::Opcode::CHAR_INFO_REQUEST) {
        write_random();
        write_int(character_id);
    }
};

// Packet for various player interactions
// Opcode: PLAYER_INTERACTION(123)
class PlayerInteractionPacket : public OutPacket {
public:
    enum mode {
        CREATE = 0,
        INVITE = 2,
        DECLINE = 3,
        VISIT = 4,
        ROOM = 5,
        CHAT = 6,
        CHAT_THING = 8,
        EXIT = 0xA,
        OPEN_STORE = 0xB,
        OPEN_CASH = 0xE,
        SET_ITEMS = 0xF,
        SET_MESO = 0x10,
        CONFIRM = 0x11,
        TRANSACTION = 0x14,
        ADD_ITEM = 0x16,
        BUY = 0x17,
        UPDATE_MERCHANT = 0x19,
        UPDATE_PLAYERSHOP = 0x1A,
        REMOVE_ITEM = 0x1B,
        BAN_PLAYER = 0x1C,
        MERCHANT_THING = 0x1D,
        OPEN_THING = 0x1E,
        PUT_ITEM = 0x21,
        MERCHANT_BUY = 0x22,
        TAKE_ITEM_BACK = 0x26,
        MAINTENANCE_OFF = 0x27,
        MERCHANT_ORGANIZE = 0x28,
        CLOSE_MERCHANT = 0x29,
        REAL_CLOSE_MERCHANT = 0x2A,
        MERCHANT_MESO = 0x2B,
        SOMETHING = 0x2D,
        VIEW_VISITORS = 0x2E,
        BLACKLIST = 0x2F,
        REQUEST_TIE = 0x32,
        ANSWER_TIE = 0x33,
        GIVE_UP = 0x34,
        EXIT_AFTER_GAME = 0x38,
        CANCEL_EXIT_AFTER_GAME = 0x39,
        READY = 0x3A,
        UN_READY = 0x3B,
        EXPEL = 0x3C,
        START = 0x3D,
        GET_RESULT = 0x3E,
        SKIP = 0x3F,
        MOVE_OMOK = 0x40,
        SELECT_CARD = 0x44
    };

    enum CreateType {
        OMOK = 1,
        MATCHCARD = 2,
        TRADE = 3,
        SHOP1 = 4,
        SHOP2 = 5
    };

    PlayerInteractionPacket(mode mode) :
        OutPacket(OutPacket::Opcode::PLAYER_INTERACTION) {
        write_byte(mode);
    }

    PlayerInteractionPacket(mode mode, CreateType type) :
        OutPacket(OutPacket::Opcode::PLAYER_INTERACTION) {
        write_byte(mode);
        write_byte(type);
    }

    PlayerInteractionPacket(mode mode, int32_t cid) :
        OutPacket(OutPacket::Opcode::PLAYER_INTERACTION) {
        write_byte(mode);
        write_int(cid);
    }
};
}  // namespace ms