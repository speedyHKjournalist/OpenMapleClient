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
#include "LoginHandlers.h"

#include "../../IO/UI.h"
#include "../../IO/UITypes/Login/UICharSelect.h"
#include "../../IO/UITypes/Login/UIGender.h"
#include "../../IO/UITypes/Login/UILoginNotice.h"
#include "../../IO/UITypes/Login/UILoginWait.h"
#include "../../IO/UITypes/Login/UIRaceSelect.h"
#include "../../IO/UITypes/Login/UITermsOfService.h"
#include "../../IO/UITypes/Login/UIWorldSelect.h"
#include "../PacketProcessor.h"
#include "../Packets/LoginPackets.h"
#include "Helpers/LoginParser.h"

namespace ms {
namespace {
auto fn_server_req = []<typename... T>(T && ...args) {
    ServerRequestPacket(std::forward<T>(args)...).dispatch();
};
auto fn_player_login = []<typename... T>(T && ...args) {
    PlayerLoginPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

void LoginResultHandler::handle(InPacket &recv) const {
    auto loginwait = UI::get().get_element<UILoginWait>();

    if (loginwait && loginwait->get().is_active()) {
        std::function<void()> okhandler = loginwait->get().get_handler();

        // Remove previous UIs
        UI::get().remove(UIElement::Type::LOGIN_NOTICE);
        UI::get().remove(UIElement::Type::LOGIN_WAIT);
        UI::get().remove(UIElement::Type::TOS);
        UI::get().remove(UIElement::Type::GENDER);

        // The packet should contain a 'reason' integer which can signify
        // various things
        if (int32_t reason = recv.read_int()) {
            // Login unsuccessful
            // The LoginNotice displayed will contain the specific information
            switch (reason) {
                case 2:
                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::BLOCKED_ID,
                        okhandler);
                    break;
                case 5:
                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::NOT_REGISTERED,
                        okhandler);
                    break;
                case 7:
                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::ALREADY_LOGGED_IN,
                        okhandler);
                    break;
                case 13:
                    UI::get().emplace<UILoginNotice>(
                        UILoginNotice::Message::UNABLE_TO_LOGIN_WITH_IP,
                        okhandler);
                    break;
                case 23: UI::get().emplace<UITermsOfService>(okhandler); break;
                default:
                    // Other reasons
                    if (reason > 0) {
                        auto reasonbyte = static_cast<int8_t>(reason - 1);

                        UI::get().emplace<UILoginNotice>(reasonbyte, okhandler);
                    }

                    break;
            }
        } else {
            // Login successful
            // The packet contains information on the account, so we initialize
            // the account with it.
            Account account = LoginParser::parse_account(recv);

            Configuration::get().set_admin(account.admin);

            if (account.female == 10) {
                UI::get().emplace<UIGender>(okhandler);
            } else {
                // Save the "Login ID" if the box for it on the login screen is
                // checked
                if (Setting<SaveLogin>::get().load()) {
                    Setting<DefaultAccount>::get().save(account.name);
                }

                // AfterLoginPacket("1111").dispatch();

                // Request the list of worlds and channels online.
                fn_server_req();
            }
        }
    }
}

void ServerStatusHandler::handle(InPacket &recv) const {
    // Possible values for status:
    // 0 - Normal
    // 1 - Highly populated
    // 2 - Full
    recv.read_short();  // status

    // TODO: I believe it shows a warning message if it's 1 and blocks enter
    // into the world if it's 2. Need to find those messages.
}

void ServerlistHandler::handle(InPacket &recv) const {
    auto worldselect = UI::get().get_element<UIWorldSelect>();

    if (!worldselect) {
        worldselect = UI::get().emplace<UIWorldSelect>();
    }

    // Parse all worlds
    while (recv.available()) {
        World world = LoginParser::parse_world(recv);

        if (world.wid != -1) {
            worldselect->get().add_world(world);
        } else {
            // Remove previous UIs
            UI::get().remove(UIElement::Type::LOGIN);

            // Add the world selection screen to the UI
            worldselect->get().draw_world();

            // End of packet
            return;
        }
    }
}

void CharlistHandler::handle(InPacket &recv) const {
    auto loginwait = UI::get().get_element<UILoginWait>();

    if (loginwait && loginwait->get().is_active()) {
        uint8_t channel_id = recv.read_ubyte();

        // Parse all characters
        std::vector<CharEntry> characters;
        int8_t charcount = recv.read_byte();

        for (int i = 0; i < charcount; ++i) {
            characters.emplace_back(LoginParser::parse_charentry(recv));
        }

        int8_t pic = recv.read_byte();
        int32_t slots = recv.read_int();

        // Remove previous UIs
        UI::get().remove(UIElement::Type::LOGIN_NOTICE);
        UI::get().remove(UIElement::Type::LOGIN_WAIT);

        // Remove the world selection screen
        if (auto worldselect = UI::get().get_element<UIWorldSelect>()) {
            worldselect->get().remove_selected();
        }

        // Add the character selection screen
        UI::get().emplace<UICharSelect>(characters, charcount, slots, pic);
    }
}

void ServerIPHandler::handle(InPacket &recv) const {
    recv.skip_byte();

    auto [ip, port] = LoginParser::parse_login(recv);
    // Attempt to reconnect to the server
    PacketProcessor::get().reconnect(ip, port);

    int32_t cid = recv.read_int();
    fn_player_login(cid);
}

void CharnameResponseHandler::handle(InPacket &recv) const {
    // Read the name and if it is already in use
    std::string name = recv.read_string();
    bool used = recv.read_bool();

    // Notify the character creation screen
    if (auto raceselect = UI::get().get_element<UIRaceSelect>()) {
        raceselect->get().send_naming_result(used);
    }
}

void AddNewCharEntryHandler::handle(InPacket &recv) const {
    recv.skip(1);

    // Parse info on the new character
    CharEntry character = LoginParser::parse_charentry(recv);

    // Read the updated character selection
    if (auto charselect = UI::get().get_element<UICharSelect>()) {
        charselect->get().add_character(std::move(character));
    }
}

void DeleteCharResponseHandler::handle(InPacket &recv) const {
    // Read the character id and if deletion was successful (PIC was correct)
    int32_t cid = recv.read_int();
    uint8_t state = recv.read_ubyte();

    // Extract information from the state byte
    if (state) {
        UILoginNotice::Message message = UILoginNotice::Message::UNKNOWN_ERROR;

        switch (state) {
            case 10:
                message = UILoginNotice::Message::BIRTHDAY_INCORRECT;
                break;
            case 20: message = UILoginNotice::Message::INCORRECT_PIC; break;
            default: message = UILoginNotice::Message::UNKNOWN_ERROR;
        }

        UI::get().emplace<UILoginNotice>(message);
    } else {
        if (auto charselect = UI::get().get_element<UICharSelect>()) {
            charselect->get().remove_character(cid);
        }
    }
}

void RecommendedWorldsHandler::handle(InPacket &recv) const {
    if (auto worldselect = UI::get().get_element<UIWorldSelect>()) {
        int16_t count = recv.read_ubyte();

        for (int i = 0; i < count; i++) {
            RecommendedWorld world = LoginParser::parse_recommended_world(recv);

            if (world.wid != -1 && !world.message.empty()) {
                worldselect->get().add_recommended_world(world);
            }
        }
    }
}
}  // namespace ms