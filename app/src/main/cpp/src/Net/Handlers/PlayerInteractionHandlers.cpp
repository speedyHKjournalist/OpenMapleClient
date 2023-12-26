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
#include "PlayerInteractionHandlers.h"

#include "../../IO/UI.h"
#include "../../IO/UITypes/UICharInfo.h"
#include "../../IO/UITypes/UINotification.h"
#include "../../IO/UITypes/UITrade.h"

namespace ms {
void CharInfoHandler::handle(InPacket &recv) const {
    int32_t character_id = recv.read_int();
    uint8_t character_level = recv.read_ubyte();
    int16_t character_job_id = recv.read_short();
    int16_t character_fame = recv.read_short();
    recv.skip_byte();  // character_marriage_ring

    std::string guild_name = recv.read_string();
    std::string alliance_name = recv.read_string();

    recv.skip_byte();

    int8_t pet_unique_id = recv.read_byte();

    while (pet_unique_id != 0) {
        recv.skip_int();     // pet_id
        recv.skip_string();  // pet_name
        recv.skip_byte();    // pet_level
        recv.skip_short();   // pet_closeness
        recv.skip_byte();    // pet_fullness

        recv.skip_short();

        recv.skip_int();  // pet_inventory_id

        pet_unique_id = recv.read_byte();
    }

    int8_t mount = recv.read_byte();

    if (mount != 0) {
        recv.skip_int();  // mount_level
        recv.skip_int();  // mount_exp
        recv.skip_int();  // mount_tiredness
    }

    int8_t wishlist_size = recv.read_byte();

    for (int sn = 0; sn < wishlist_size; sn++) {
        recv.skip_int();  // wishlist_item
    }

    recv.skip_int();  // monster_book_level
    recv.skip_int();  // monster_book_card_normal
    recv.skip_int();  // monster_book_card_special
    recv.skip_int();  // monster_book_cards_total
    recv.skip_int();  // monster_book_cover

    recv.skip_int();  // medal

    int16_t medal_quests_size = recv.read_short();

    for (int s = 0; s < medal_quests_size; s++) {
        recv.skip_short();  // medal_quest
    }

    // Update the character information window
    if (auto charinfo = UI::get().get_element<UICharInfo>()) {
        charinfo->get().update_stats(character_id,
                                     character_job_id,
                                     character_level,
                                     character_fame,
                                     guild_name,
                                     alliance_name);
        charinfo->get().makeactive();
    } else {
        UI::get().emplace<UICharInfo>(character_id);
    }
}

void PlayerInteractionHandler::handle(InPacket &recv) const {
    int8_t mode = recv.read_byte();
//    int8_t mode_specific = recv.read_byte();

    switch (mode) {
        case mode::INVITE: {
            std::string char_name = recv.read_string();
            UI::get().emplace<UINotification>(char_name);
        } break;
        case mode::EXIT:
            UI::get().remove(UIElement::Type::NOTIFICATION);
            UI::get().remove(UIElement::Type::TRADE);
            break;
        // TODO: below
        case mode::CREATE:
        case mode::DECLINE:
        case mode::VISIT:
        case mode::ROOM:  // open trade ui
            UI::get().emplace<UITrade>();
            break;
        case mode::CHAT:
        case mode::CHAT_THING:
        case mode::OPEN_STORE:
        case mode::OPEN_CASH:
        case mode::SET_ITEMS:
        case mode::SET_MESO:
        case mode::CONFIRM:
        case mode::TRANSACTION:
        case mode::ADD_ITEM:
        case mode::BUY:
        case mode::UPDATE_MERCHANT:
        case mode::UPDATE_PLAYERSHOP:
        case mode::REMOVE_ITEM:
        case mode::BAN_PLAYER:
        case mode::MERCHANT_THING:
        case mode::OPEN_THING:
        case mode::PUT_ITEM:
        case mode::MERCHANT_BUY:
        case mode::TAKE_ITEM_BACK:
        case mode::MAINTENANCE_OFF:
        case mode::MERCHANT_ORGANIZE:
        case mode::CLOSE_MERCHANT:
        case mode::REAL_CLOSE_MERCHANT:
        case mode::MERCHANT_MESO:
        case mode::SOMETHING:
        case mode::VIEW_VISITORS:
        case mode::BLACKLIST:
        case mode::REQUEST_TIE:
        case mode::ANSWER_TIE:
        case mode::GIVE_UP:
        case mode::EXIT_AFTER_GAME:
        case mode::CANCEL_EXIT_AFTER_GAME:
        case mode::READY:
        case mode::UN_READY:
        case mode::EXPEL:
        case mode::START:
        case mode::GET_RESULT:
        case mode::MOVE_OMOK:
        case mode::SELECT_CARD: break;
        default: break;
    }
}
}  // namespace ms