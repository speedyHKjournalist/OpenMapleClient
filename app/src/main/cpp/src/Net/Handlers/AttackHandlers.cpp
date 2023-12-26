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
#include "AttackHandlers.h"

#include <unordered_map>
#include <vector>

#include "../Character/SkillId.h"
#include "../Gameplay/Stage.h"

namespace ms {
AttackHandler::AttackHandler(Attack::Type t) {
    type_ = t;
}

void AttackHandler::handle(InPacket &recv) const {
    int32_t cid = recv.read_int();
    uint8_t count = recv.read_ubyte();

    recv.skip(1);

    AttackResult attack;
    attack.type = type_;
    attack.attacker = cid;

    attack.level = recv.read_ubyte();
    attack.skill = (attack.level > 0) ? recv.read_int() : 0;

    attack.display = recv.read_ubyte();
    attack.toleft = recv.read_bool();
    attack.stance = recv.read_ubyte();
    attack.speed = recv.read_ubyte();

    recv.skip(1);

    attack.bullet = recv.read_int();

    attack.mobcount = (count >> 4) & 0xF;
    attack.hitcount = count & 0xF;

    for (int i = 0; i < attack.mobcount; i++) {
        int32_t oid = recv.read_int();

        recv.skip(1);

        uint8_t length = (attack.skill == SkillId::Id::MESO_EXPLOSION)
                             ? recv.read_ubyte()
                             : attack.hitcount;

        for (int j = 0; j < length; j++) {
            int32_t damage = recv.read_int();
            bool critical = false;  // TODO: ?
            auto single_damage = std::make_pair(damage, critical);
            attack.damage_lines[oid].push_back(single_damage);
        }
    }

    Stage::get().get_combat().push_attack(attack);
}
}  // namespace ms