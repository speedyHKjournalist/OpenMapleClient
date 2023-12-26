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
#include "MapObjectHandlers.h"

#include <functional>
#include <optional>

#include "../../Gameplay/Stage.h"
#include "Helpers/LoginParser.h"
#include "Helpers/MovementParser.h"

namespace ms {
void SpawnCharHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();

    // We don't need to spawn the player twice
    if (Stage::get().is_player(cid)) {
        return;
    }

    uint8_t level = recv.read_ubyte();
    std::string name = recv.read_string();

    recv.read_string();  // guildname
    recv.read_short();   // guildlogobg
    recv.read_byte();    // guildlogobgcolor
    recv.read_short();   // guildlogo
    recv.read_byte();    // guildlogocolor

    recv.skip(8);

    bool morphed = recv.read_int() == 2;
    int32_t buffmask1 = recv.read_int();
    int16_t buffvalue = 0;

    if (buffmask1 != 0) {
        buffvalue = morphed ? recv.read_short() : recv.read_byte();
    }

    recv.read_int();  // buffmask 2

    recv.skip(43);

    recv.read_int();  // 'mount'

    recv.skip(61);

    int16_t job = recv.read_short();
    LookEntry look = LoginParser::parse_look(recv);

    recv.read_int();  // count of 5110000
    recv.read_int();  // 'itemeffect'
    recv.read_int();  // 'chair'

    Point<int16_t> position = recv.read_point();
    int8_t stance = recv.read_byte();

    recv.skip(3);

    for (size_t i = 0; i < 3; i++) {
        int8_t available = recv.read_byte();

        if (available == 1) {
            recv.read_byte();    // 'byte2'
            recv.read_int();     // petid
            recv.read_string();  // name
            recv.read_int();     // unique id
            recv.read_int();
            recv.read_point();  // pos
            recv.read_byte();   // stance
            recv.read_int();    // fhid
        } else {
            break;
        }
    }

    recv.read_int();  // mountlevel
    recv.read_int();  // mountexp
    recv.read_int();  // mounttiredness

    // TODO: Shop stuff
    recv.read_byte();
    // TODO: Shop stuff end

    bool chalkboard = recv.read_bool();
    std::string chalktext = chalkboard ? recv.read_string() : "";

    recv.skip(3);
    recv.read_byte();  // team

    Stage::get().get_chars().spawn(
        { cid, look, level, job, name, stance, position });
}

void RemoveCharHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();

    Stage::get().get_chars().remove(cid);
}

void SpawnPetHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();
    auto character = Stage::get().get_character(cid);

    if (!character) {
        return;
    }

    uint8_t petindex = recv.read_ubyte();
    int8_t mode = recv.read_byte();

    if (mode == 1) {
        recv.skip(1);

        int32_t itemid = recv.read_int();
        std::string name = recv.read_string();
        int32_t uniqueid = recv.read_int();

        recv.skip(4);

        Point<int16_t> pos = recv.read_point();
        uint8_t stance = recv.read_ubyte();
        int32_t fhid = recv.read_int();

        character->get()
            .add_pet(petindex, itemid, name, uniqueid, pos, stance, fhid);
    } else if (mode == 0) {
        bool hunger = recv.read_bool();

        character->get().remove_pet(petindex, hunger);
    }
}

void CharMovedHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();
    recv.skip(4);
    std::vector<Movement> movements = MovementParser::parse_movements(recv);

    Stage::get().get_chars().send_movement(cid, movements);
}

void UpdateCharLookHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();
    recv.read_byte();
    LookEntry look = LoginParser::parse_look(recv);

    Stage::get().get_chars().update_look(cid, look);
}

void ShowForeignEffectHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();
    int8_t effect = recv.read_byte();

    if (effect == 10)  // recovery
    {
        recv.read_byte();     // 'amount'
    } else if (effect == 13)  // card effect
    {
        Stage::get().show_character_effect(cid, CharEffect::MONSTER_CARD);
    } else if (recv.available())  // skill
    {
        int32_t skillid = recv.read_int();
        recv.read_byte();  // 'direction'
        // 9 more bytes after this

        if (effect == 1) {
            Stage::get().get_combat().show_buff(cid, skillid, effect);
        } else {
            Stage::get().get_combat().show_affected_by_buff(cid,
                                                            skillid,
                                                            effect);
        }
    } else {
        // TODO: Blank
    }
}

void ForeignBuffHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();
    uint64_t firstmask = recv.read_long();
    uint64_t secondmask = recv.read_long();

    switch (secondmask) {
        case Buffstat::BATTLESHIP:
            handle_buff(recv, cid, Buffstat::BATTLESHIP);
            return;
    }

    for (const auto &[buff_id, mask] : Buffstat::first_codes) {
        if (firstmask & mask) {
            handle_buff(recv, cid, buff_id);
        }
    }

    for (const auto &[buff_id, mask] : Buffstat::second_codes) {
        if (secondmask & mask) {
            handle_buff(recv, cid, buff_id);
        }
    }
}

void GiveForeignBuffHandler::handle_buff(InPacket &recv,
                                         int32_t cid,
                                         Buffstat::Id stat) const {
    if (Buffstat::is_disease(stat)) {
        if (stat == Buffstat::POISON) {
            int16_t value = recv.read_short();  // TODO
        }

        int16_t skill_id = recv.read_short();
        int16_t skill_level = recv.read_short();

        /* if (Optional<Char> chr = Stage::get().get_character(cid)) {

         }*/

        Stage::get().get_combat().give_foreign_buff(cid, skill_id, skill_level);
    } else {
        int16_t value = recv.read_short();  // TODO
    }

    std::cerr << std::endl
              << "Opcode [199] Error: Handler exists but is not implemented."
              << std::endl;
}

void CancelForeignBuffHandler::handle_buff(InPacket &recv,
                                           int32_t cid,
                                           Buffstat::Id stat) const {
    if (auto chr = Stage::get().get_character(cid)) {
        chr->get().remove_recurring_effect();
    }

    std::cerr << std::endl
              << "Opcode [200] Error: Handler exists but is not implemented."
              << std::endl;
}

void SpawnMobHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    recv.read_byte();  // 5 if controller == null
    int32_t id = recv.read_int();

    recv.skip(16);

    Point<int16_t> position = recv.read_point();
    int8_t stance = recv.read_byte();

    recv.skip(2);  // origin fh

    uint16_t fh = recv.read_ushort();
    int8_t effect = recv.read_byte();

    if (effect > 0) {
        recv.read_byte();
        recv.read_short();

        if (effect == 15) {
            recv.read_byte();
        }
    }

    int8_t team = recv.read_byte();

    recv.skip(4);  // item effect

    Stage::get().get_mobs().spawn(
        { oid, id, 0, stance, fh, effect == -2, team, position });
}

void KillMobHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int8_t animation = recv.read_byte();

    Stage::get().get_mobs().remove(oid, animation);
}

void SpawnMobControllerHandler::handle(InPacket &recv) const {
    int8_t mode = recv.read_byte();
    int32_t oid = recv.read_int();

    if (mode == 0) {
        Stage::get().get_mobs().set_control(oid, false);
    } else {
        if (recv.available()) {
            recv.skip(1);

            int32_t id = recv.read_int();

            recv.skip(16);

            Point<int16_t> position = recv.read_point();
            int8_t stance = recv.read_byte();

            recv.skip(2);  // origin fh

            uint16_t fh = recv.read_ushort();
            int8_t effect = recv.read_byte();

            if (effect > 0) {
                recv.read_byte();
                recv.read_short();

                if (effect == 15) {
                    recv.read_byte();
                }
            }

            int8_t team = recv.read_byte();

            recv.skip(4);

            Stage::get().get_mobs().spawn(
                { oid, id, mode, stance, fh, effect == -2, team, position });
        } else {
            // TODO: Remove monster invisibility, not used (maybe in an event
            // script?), Check this!
        }
    }
}

void MobMovedHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();

    recv.read_byte();  // ?

    bool use_skills = recv.read_bool();       // useskill
    int8_t action = recv.read_byte();         // action
    uint8_t skillid = recv.read_ubyte();      // skill id
    uint8_t skill_level = recv.read_ubyte();  // skill level
    auto option = recv.read_short();          // option

    Point<int16_t> position = recv.read_point();
    std::vector<Movement> movements = MovementParser::parse_movements(recv);

    Stage::get().get_mobs().send_movement(oid, position, std::move(movements));
    Stage::get().get_mob_combat().use_move(oid, action, skillid, skill_level);
}

void MobMoveResponseHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int16_t moveid = recv.read_short();
    bool use_skills = recv.read_bool();
    uint16_t curr_mp = recv.read_ushort();
    uint8_t skillid = recv.read_ubyte();
    uint8_t skill_level = recv.read_ubyte();

    if (skillid == 0) {
        // Stage::get().get_mob_combat().use_some_attack(oid);
    } else {
        Stage::get().get_mob_combat().use_move(oid,
                                               moveid,
                                               skillid,
                                               skill_level);
    }

    std::cerr << std::endl
              << "Opcode [240] Error: Handler exists but is not implemented."
              << std::endl;
}

void ApplyMobStatusHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();

    recv.read_long();

    int32_t first_mask = recv.read_int();
    int32_t second_mask = recv.read_int();

    // and some more ...

    std::cerr << std::endl
              << "Opcode [242] Error: Handler exists but is not implemented."
              << std::endl;
}

void CancelMobStatusHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();

    recv.read_long();

    int32_t first_mask = recv.read_int();
    int32_t second_mask = recv.read_int();

    recv.read_int();

    if (auto mob = Stage::get().get_mobs().get_mobs()->get<Mob>(oid)) {
        mob->get().cancel_buff(12345);  // TODO
    }

    std::cerr << std::endl
              << "Opcode [243] Error: Handler exists but is not implemented."
              << std::endl;
}

void ShowMobHpHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int8_t hppercent = recv.read_byte();
    uint16_t playerlevel =
        Stage::get().get_player().get_stats().get_stat(MapleStat::Id::LEVEL);

    Stage::get().get_mobs().send_mobhp(oid, hppercent, playerlevel);
}

void SpawnNpcHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int32_t id = recv.read_int();
    Point<int16_t> position = recv.read_point();
    bool flip = recv.read_bool();
    uint16_t fh = recv.read_ushort();

    recv.read_short();  // 'rx'
    recv.read_short();  // 'ry'

    Stage::get().get_npcs().spawn({ oid, id, position, flip, fh });
}

void SpawnNpcControllerHandler::handle(InPacket &recv) const {
    int8_t mode = recv.read_byte();
    int32_t oid = recv.read_int();

    if (mode == 0) {
        Stage::get().get_npcs().remove(oid);
    } else {
        int32_t id = recv.read_int();
        Point<int16_t> position = recv.read_point();
        bool flip = recv.read_bool();
        uint16_t fh = recv.read_ushort();

        recv.read_short();  // 'rx'
        recv.read_short();  // 'ry'
        recv.read_bool();   // 'minimap'

        Stage::get().get_npcs().spawn({ oid, id, position, flip, fh });
    }
}

void SetNpcScriptableHandler::handle(InPacket &recv) const {
    int size = recv.read_byte();

    for (int i = 0; i < size; ++i) {
        int id = recv.read_int();
        std::string name = recv.read_string();
        int start_time = recv.read_int();
        int end_time = recv.read_int();
    }

    std::cerr << std::endl
              << "Opcode [263] Error: Handler exists but is not implemented."
              << std::endl;
}

void DropLootHandler::handle(InPacket &recv) const {
    int8_t mode = recv.read_byte();
    int32_t oid = recv.read_int();
    bool meso = recv.read_bool();
    int32_t itemid = recv.read_int();
    int32_t owner = recv.read_int();
    int8_t pickuptype = recv.read_byte();
    Point<int16_t> dropto = recv.read_point();

    recv.skip(4);

    Point<int16_t> dropfrom;

    if (mode != 2) {
        dropfrom = recv.read_point();

        recv.skip(2);
    } else {
        dropfrom = dropto;
    }

    if (!meso) {
        recv.skip(8);
    }

    bool playerdrop = !recv.read_bool();

    // bug: plays sound when entering a map containing drops owned by self
    if (mode == 0 || (owner == Stage::get().is_player(owner) && !playerdrop)) {
        Sound(Sound::Name::DROP).play();
    }

    Stage::get().get_drops().spawn({ oid,
                                     itemid,
                                     meso,
                                     owner,
                                     dropfrom,
                                     dropto,
                                     pickuptype,
                                     mode,
                                     playerdrop });
}

void RemoveLootHandler::handle(InPacket &recv) const {
    int8_t mode = recv.read_byte();
    int32_t oid = recv.read_int();

    std::optional<std::reference_wrapper<PhysicsObject>> looter = {};

    if (mode > 1) {
        int32_t cid = recv.read_int();

        if (recv.length() > 0) {
            recv.read_byte();  // pet
        } else if (auto character = Stage::get().get_character(cid)) {
            looter = character->get().get_phobj();
        }

        Sound(Sound::Name::PICKUP).play();
    }

    Stage::get().get_drops().remove(oid, mode, &looter->get());
}

void HitReactorHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int8_t state = recv.read_byte();
    Point<int16_t> point = recv.read_point();
    int8_t stance =
        recv.read_byte();  // TODO: When is this different than state?
    recv.skip(2);          // TODO: Unused
    recv.skip(1);          // "frame" delay but this is in the WZ file?

    Stage::get().get_reactors().trigger(oid, state);
}

void SpawnReactorHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int32_t rid = recv.read_int();
    int8_t state = recv.read_byte();
    Point<int16_t> point = recv.read_point();

    // TODO: Unused, Check this!
    // uint16_t fhid = recv.read_short();
    // recv.read_byte()

    Stage::get().get_reactors().spawn({ oid, rid, state, point });
}

void RemoveReactorHandler::handle(InPacket &recv) const {
    int32_t oid = recv.read_int();
    int8_t state = recv.read_byte();
    Point<int16_t> point = recv.read_point();

    Stage::get().get_reactors().remove(oid, state, point);
}
}  // namespace ms