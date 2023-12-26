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
#include "Spawn.h"

#include <utility>

#include "../Character/OtherChar.h"
#include "MapleMap/ItemDrop.h"
#include "MapleMap/MesoDrop.h"
#include "MapleMap/Mob.h"
#include "MapleMap/Npc.h"
#include "MapleMap/Reactor.h"

namespace ms {
NpcSpawn::NpcSpawn(int32_t o,
                   int32_t i,
                   Point<int16_t> p,
                   bool fl,
                   uint16_t f) :
    oid_(o),
    id_(i),
    position_(p),
    flip_(fl),
    fh_(f) {}

int32_t NpcSpawn::get_oid() const {
    return oid_;
}

std::unique_ptr<MapObject> NpcSpawn::instantiate(const Physics &physics) const {
    auto spawnposition = physics.get_y_below(position_);
    return std::make_unique<Npc>(id_, oid_, flip_, fh_, false, spawnposition);
}

MobSpawn::MobSpawn(int32_t o,
                   int32_t i,
                   int8_t m,
                   int8_t st,
                   uint16_t f,
                   bool ns,
                   int8_t t,
                   Point<int16_t> p) :
    oid_(o),
    id_(i),
    mode_(m),
    stance_(st),
    fh_(f),
    new_spawn_(ns),
    team_(t),
    position_(p) {}

int8_t MobSpawn::get_mode() const {
    return mode_;
}

int32_t MobSpawn::get_oid() const {
    return oid_;
}

std::unique_ptr<MapObject> MobSpawn::instantiate() const {
    return std::make_unique<Mob>(oid_,
                                 id_,
                                 mode_,
                                 stance_,
                                 fh_,
                                 new_spawn_,
                                 team_,
                                 position_);
}

ReactorSpawn::ReactorSpawn(int32_t o, int32_t r, int8_t s, Point<int16_t> p) :
    oid_(o),
    rid_(r),
    state_(s),
    position_(p) {}

int32_t ReactorSpawn::get_oid() const {
    return oid_;
}

std::unique_ptr<MapObject> ReactorSpawn::instantiate(
    const Physics &physics) const {
    auto spawnposition = physics.get_y_below(position_);
    return std::make_unique<Reactor>(oid_, rid_, state_, spawnposition);
}

DropSpawn::DropSpawn(int32_t o,
                     int32_t i,
                     bool ms,
                     int32_t ow,
                     Point<int16_t> p,
                     Point<int16_t> d,
                     int8_t t,
                     int8_t m,
                     bool pd) :
    oid_(o),
    id_(i),
    meso_(ms),
    owner_(ow),
    start_(p),
    dest_(d),
    drop_type_(t),
    mode_(m),
    player_drop_(pd) {}

bool DropSpawn::is_meso() const {
    return meso_;
}

int32_t DropSpawn::get_itemid() const {
    return id_;
}

int32_t DropSpawn::get_oid() const {
    return oid_;
}

std::unique_ptr<MapObject> DropSpawn::instantiate(const Animation &icon) const {
    return std::make_unique<MesoDrop>(oid_,
                                      owner_,
                                      start_,
                                      dest_,
                                      drop_type_,
                                      mode_,
                                      player_drop_,
                                      icon);
}

std::unique_ptr<MapObject> DropSpawn::instantiate(const Texture &icon) const {
    return std::make_unique<ItemDrop>(oid_,
                                      owner_,
                                      start_,
                                      dest_,
                                      drop_type_,
                                      mode_,
                                      id_,
                                      player_drop_,
                                      icon);
}

CharSpawn::CharSpawn(int32_t c,
                     LookEntry lk,
                     uint8_t l,
                     int16_t j,
                     std::string nm,
                     int8_t st,
                     Point<int16_t> p) :
    cid_(c),
    level_(l),
    job_(j),
    name_(std::move(nm)),
    stance_(st),
    position_(p),
    look_(std::move(lk)) {}

int32_t CharSpawn::get_cid() const {
    return cid_;
}

std::unique_ptr<MapObject> CharSpawn::instantiate() const {
    return std::make_unique<OtherChar>(cid_,
                                       look_,
                                       level_,
                                       job_,
                                       name_,
                                       stance_,
                                       position_);
}
}  // namespace ms