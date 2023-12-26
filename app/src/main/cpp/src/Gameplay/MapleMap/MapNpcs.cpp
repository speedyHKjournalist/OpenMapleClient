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
#include "MapNpcs.h"

#include "../Net/Packets/NpcInteractionPackets.h"
#include "Npc.h"
#include "UI.h"
#include "UINpcTalk.h"

namespace ms {
namespace {
auto fn_talk_to_npc = []<typename... T>(T && ...args) {
    TalkToNPCPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

void MapNpcs::draw(Layer::Id layer,
                   double viewx,
                   double viewy,
                   float alpha) const {
    npcs_.draw(layer, viewx, viewy, alpha);
}

void MapNpcs::update(const Physics &physics) {
    for (; !spawns_.empty(); spawns_.pop()) {
        const NpcSpawn &spawn = spawns_.front();

        int32_t oid = spawn.get_oid();
        auto npc = npcs_.get<MapObject>(oid);

        if (npc) {
            npc->get().makeactive();
        } else {
            npcs_.add(spawn.instantiate(physics));
        }
    }

    npcs_.update(physics);
}

void MapNpcs::spawn(NpcSpawn &&spawn) {
    spawns_.emplace(std::move(spawn));
}

void MapNpcs::remove(int32_t oid) {
    if (auto npc = npcs_.get<MapObject>(oid)) {
        npc->get().deactivate();
    }
}

void MapNpcs::clear() {
    npcs_.clear();
}

MapObjects *MapNpcs::get_npcs() {
    return &npcs_;
}

Cursor::State MapNpcs::send_cursor(bool pressed,
                                   Point<int16_t> position,
                                   Point<int16_t> viewpos) {
    for (auto &map_object : npcs_) {
        Npc *npc = static_cast<Npc *>(map_object.second.get());

        if (npc && npc->is_active() && npc->inrange(position, viewpos)) {
            if (pressed) {
                npc->talk([](int id,
                             int msgtype,
                             int b0,
                             int b1,
                             int speaker,
                             std::string str) {
                    UI::get().emplace<UINpcTalk>();
                    if (auto npctalk = UI::get().get_element<UINpcTalk>()) {
                        npctalk->get()
                            .change_text(id, msgtype, b0, b1, speaker, str);
                    }
                });

                fn_talk_to_npc(npc->get_oid());

                return Cursor::State::IDLE;
            }
            return Cursor::State::CAN_CLICK;
        }
    }

    return pressed ? Cursor::State::CLICKING : Cursor::State::IDLE;
}
}  // namespace ms