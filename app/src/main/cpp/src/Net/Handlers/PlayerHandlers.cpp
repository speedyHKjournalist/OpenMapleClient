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
#include "PlayerHandlers.h"

#include "../../Gameplay/Stage.h"
#include "../../IO/UI.h"
#include "../../IO/UITypes/UIBuffList.h"
#include "../../IO/UITypes/UICashShop.h"
#include "../../IO/UITypes/UISkillBook.h"
#include "../../IO/UITypes/UIStatsInfo.h"
#include "../../IO/Window.h"
#include "Helpers/LoginParser.h"
#include "../../IO/UITypes/UIChatBar.h"
#include "../../Net/Packets/LoginPackets.h"
#include "../../Net/PacketProcessor.h"
#include "../Packets/GameplayPackets.h"

namespace ms {
namespace {
auto fn_player_login = []<typename... T>(T && ...args) {
    PlayerLoginPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

void transition(int32_t mapid, uint8_t portalid) {
    float fadestep = 0.025f;

    Window::get().fadeout(fadestep, [mapid, portalid]() {
        GraphicsGL::get().clear();
        Stage::get().load(mapid, portalid);
        UI::get().enable();
        GraphicsGL::get().unlock();
    });

    UI::get().disable();
    GraphicsGL::get().lock();
    Stage::get().clear_channel_objects();
}

void ChangeChannelHandler::handle(InPacket &recv) const {
    auto [ip, port] = LoginParser::parse_login(recv);

    Player &player = Stage::get().get_player();
    int32_t mapid = player.get_stats().get_mapid();
    uint8_t portalid = player.get_stats().get_portal();

    if (UI::get().get_element<UICashShop>()) {
        UI::get().change_state(UI::State::GAME);
    }

    // Attempt to connect to the server
    PacketProcessor::get().reconnect(ip, port);

    fn_player_login(player.get_cid());

    transition(mapid, portalid);
}

void ChangeStatsHandler::handle(InPacket &recv) const {
    recv.read_bool();  // 'itemreaction'
    int32_t updatemask = recv.read_int();

    bool recalculate = false;

    for (auto [stat_id, stat_mask] : MapleStat::codes) {
        if (updatemask & stat_mask) {
            recalculate |= handle_stat(stat_id, recv);
        }
    }

    if (recalculate) {
        Stage::get().get_player().recalc_stats(false);
    }

    UI::get().enable();
}

bool ChangeStatsHandler::handle_stat(MapleStat::Id stat, InPacket &recv) const {
    Player &player = Stage::get().get_player();

    bool recalculate = false;

    switch (stat) {
        case MapleStat::Id::SKIN:
            player.change_look(stat, recv.read_short());
            break;
        case MapleStat::Id::FACE:
        case MapleStat::Id::HAIR:
            player.change_look(stat, recv.read_int());
            break;
        case MapleStat::Id::LEVEL:
            player.change_level(recv.read_ubyte());
            break;
        case MapleStat::Id::JOB: player.change_job(recv.read_ushort()); break;
        case MapleStat::Id::HP: {
            int16_t current_hp = recv.read_short();
            player.get_stats().set_stat(stat, current_hp);
            recalculate = true;
            if (current_hp <= 0) { 
                player.set_state(Char::State::DIED);
                bool died = true;
                int32_t targetid = 0;
                ChangeMapPacket changeMap(died, targetid, "sp", false);
                changeMap.dispatch();
            }
            break;
        }
        case MapleStat::Id::EXP:
            player.get_stats().set_exp(recv.read_int());
            break;
        case MapleStat::Id::MESO:
            player.get_inventory().set_meso(recv.read_int());
            break;
        default:
            player.get_stats().set_stat(stat, recv.read_short());
            recalculate = true;
            break;
    }

    bool update_statsinfo = need_statsinfo_update(stat);

    if (update_statsinfo && !recalculate) {
        if (auto statsinfo = UI::get().get_element<UIStatsInfo>()) {
            statsinfo->get().update_stat(stat);
        }
    }

    bool update_skillbook = need_skillbook_update(stat);

    if (update_skillbook) {
        int16_t value = player.get_stats().get_stat(stat);

        if (auto skillbook = UI::get().get_element<UISkillBook>()) {
            skillbook->get().update_stat(stat, value);
        }
    }

    return recalculate;
}

bool ChangeStatsHandler::need_statsinfo_update(MapleStat::Id stat) const {
    switch (stat) {
        case MapleStat::Id::JOB:
        case MapleStat::Id::STR:
        case MapleStat::Id::DEX:
        case MapleStat::Id::INT:
        case MapleStat::Id::LUK:
        case MapleStat::Id::HP:
        case MapleStat::Id::MAXHP:
        case MapleStat::Id::MP:
        case MapleStat::Id::MAXMP:
        case MapleStat::Id::AP: return true;
        default: return false;
    }
}

bool ChangeStatsHandler::need_skillbook_update(MapleStat::Id stat) const {
    switch (stat) {
        case MapleStat::Id::JOB:
        case MapleStat::Id::SP: return true;
        default: return false;
    }
}

void BuffHandler::handle(InPacket &recv) const {
    uint64_t firstmask = recv.read_long();
    uint64_t secondmask = recv.read_long();

    switch (secondmask) {
        case Buffstat::BATTLESHIP:
            handle_buff(recv, Buffstat::BATTLESHIP);
            return;
    }

    for (const auto &[buff_id, mask] : Buffstat::first_codes) {
        if (firstmask & mask) {
            handle_buff(recv, buff_id);
        }
    }

    for (const auto &[buff_id, mask] : Buffstat::second_codes) {
        if (secondmask & mask) {
            handle_buff(recv, buff_id);
        }
    }

    Stage::get().get_player().recalc_stats(false);
}

void ApplyBuffHandler::handle_buff(InPacket &recv, Buffstat::Id bs) const {
    if (Buffstat::is_disease(bs)) {
        int16_t value = recv.read_short();
        int16_t skillid = recv.read_short();
        int16_t skill_level = recv.read_short();
        int32_t duration = recv.read_int();

        if (!Stage::get().get_player().has_buff(bs)) {
            Stage::get().get_combat().show_player_disease(skillid, skill_level);
        }

        Stage::get().get_player().give_buff({ bs, value, skillid, duration });
    } else {
        int16_t value = recv.read_short();
        int32_t skillid = recv.read_int();
        int32_t duration = recv.read_int();

        Stage::get().get_player().give_buff({ bs, value, skillid, duration });

        if (auto bufflist = UI::get().get_element<UIBuffList>()) {
            bufflist->get().add_buff(skillid, duration);
        }
    }
}

void CancelBuffHandler::handle_buff(InPacket &, Buffstat::Id bs) const {
    Stage::get().get_player().cancel_buff(bs);
}

void RecalculateStatsHandler::handle(InPacket &) const {
    Stage::get().get_player().recalc_stats(false);
}

void UpdateSkillHandler::handle(InPacket &recv) const {
    recv.skip(3);

    int32_t skillid = recv.read_int();
    int32_t level = recv.read_int();
    int32_t masterlevel = recv.read_int();
    int64_t expire = recv.read_long();

    Stage::get().get_player().change_skill(skillid, level, masterlevel, expire);

    if (auto skillbook = UI::get().get_element<UISkillBook>()) {
        skillbook->get().update_skills(skillid);
    }

    UI::get().enable();
}

void FameResponseHandler::handle(InPacket &recv) const {
    std::string message;

    switch (auto result = recv.read_byte()) {
        case FameResponseHandler::Result::SUCCESS_GIVE: {
            auto char_name = recv.read_string();
            int8_t mode = recv.read_byte();
            auto new_fame = recv.read_ushort();
            std::string mode_str = mode == 1 ? "famed " : "defamed ";
            recv.skip_byte();
            message = "You " + mode_str + char_name + '.';
        } break;
        case FameResponseHandler::Result::SUCCESS_RECV: {
            auto from_char_name = recv.read_string();
            int8_t mode = recv.read_byte();
            std::string mode_str = mode == 1 ? "fame " : "defame ";
            message =
                "You received " + mode_str + "from " + from_char_name + '.';
        } break;
        case FameResponseHandler::Result::ALREADY_FAMED_TODAY:
            message = "You can't raise or drop fame anymore today.";
            break;
        case FameResponseHandler::Result::FAMED_CHAR_TOO_RECENTLY:
            message = "You have famed this character too recently.";
            break;
        case FameResponseHandler::Result::TOO_LOW_LEVEL:
            message = "You must be at least level 15 to fame someone.";
            break;
        case FameResponseHandler::Result::INCORRECT_USERNAME:
            message = "Incorrect user.";
            break;
        case FameResponseHandler::Result::UNEXPECTED_ERROR:
            message = "An unexpected error occurred.";
            break;
        default: break;
    }

    if (auto chatbar = UI::get().get_element<UIChatBar>()) {
        if (!message.empty()) {
            chatbar->get().send_chatline(message, UIChatBar::LineType::RED);
        }
    }
}

void SkillMacrosHandler::handle(InPacket &recv) const {
    uint8_t size = recv.read_ubyte();

    for (int i = 0; i < size; i++) {
        recv.read_string();  // name
        recv.read_byte();    // 'shout' byte
        recv.read_int();     // skill 1
        recv.read_int();     // skill 2
        recv.read_int();     // skill 3
    }
}

void AddCooldownHandler::handle(InPacket &recv) const {
    int32_t skill_id = recv.read_int();
    int16_t cooltime = recv.read_short();

    Stage::get().get_player().add_cooldown(skill_id, cooltime);
}

void KeymapHandler::handle(InPacket &recv) const {
    recv.skip(1);

    for (int i = 0; i < 90; i++) {
        uint8_t type = recv.read_ubyte();
        int32_t action = recv.read_int();

        UI::get().add_keymapping(i, type, action);
    }
}
}  // namespace ms