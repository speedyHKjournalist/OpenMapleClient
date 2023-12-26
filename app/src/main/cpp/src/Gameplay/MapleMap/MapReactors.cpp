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
#include "MapReactors.h"

namespace ms {
void MapReactors::draw(Layer::Id layer,
                       double viewx,
                       double viewy,
                       float alpha) const {
    reactors_.draw(layer, viewx, viewy, alpha);
}

// Spawns all reactors to map with proper footholds
void MapReactors::update(const Physics &physics) {
    for (; !spawns_.empty(); spawns_.pop()) {
        const ReactorSpawn &spawn = spawns_.front();

        int32_t oid = spawn.get_oid();

        if (auto reactor = reactors_.get<MapObject>(oid)) {
            reactor->get().makeactive();
        } else {
            reactors_.add(spawn.instantiate(physics));
        }
    }

    reactors_.update(physics);
}

void MapReactors::trigger(int32_t oid, int8_t state) {
    if (auto reactor = reactors_.get<Reactor>(oid)) {
        reactor->get().set_state(state);
    }
}

void MapReactors::spawn(ReactorSpawn &&spawn) {
    spawns_.emplace(std::move(spawn));
}

void MapReactors::remove(int32_t oid, int8_t state, Point<int16_t> position) {
    if (auto reactor = reactors_.get<Reactor>(oid)) {
        reactor->get().destroy(state, position);
    }
}

void MapReactors::clear() {
    reactors_.clear();
}

MapObjects *MapReactors::get_reactors() {
    return &reactors_;
}
}  // namespace ms