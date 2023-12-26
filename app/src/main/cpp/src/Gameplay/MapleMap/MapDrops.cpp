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
#include "MapDrops.h"

#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>

#include "../Constants.h"
#include "../Data/ItemData.h"
#include "Drop.h"
#include "ItemDrop.h"
#include "MesoDrop.h"
#include "OptionalCreator.h"

namespace ms {
MapDrops::MapDrops() : loot_enabled_(false) {}

void MapDrops::init() {
    nl::node src = nl::nx::item["Special"]["0900.img"];

    meso_icons_[MesoIcon::BRONZE] = src["09000000"]["iconRaw"];
    meso_icons_[MesoIcon::GOLD] = src["09000001"]["iconRaw"];
    meso_icons_[MesoIcon::BUNDLE] = src["09000002"]["iconRaw"];
    meso_icons_[MesoIcon::BAG] = src["09000003"]["iconRaw"];
}

void MapDrops::draw(Layer::Id layer,
                    double viewx,
                    double viewy,
                    float alpha) const {
    drops_.draw(layer, viewx, viewy, alpha);
}

void MapDrops::update(const Physics &physics) {
    for (; !spawns_.empty(); spawns_.pop()) {
        const DropSpawn &spawn = spawns_.front();

        int32_t oid = spawn.get_oid();

        if (auto drop = drops_.get<MapObject>(oid)) {
            drop->get().makeactive();
        } else {
            int32_t itemid = spawn.get_itemid();
            bool meso = spawn.is_meso();

            if (meso) {
                MesoIcon mesotype = (itemid > 999)
                                        ? BAG
                                        : (itemid > 99)
                                              ? BUNDLE
                                              : (itemid > 49) ? GOLD : BRONZE;

                const Animation &icon = meso_icons_[mesotype];
                drops_.add(spawn.instantiate(icon));
            } else if (const ItemData &itemdata = ItemData::get(itemid)) {
                const Texture &icon = itemdata.get_icon(true);
                drops_.add(spawn.instantiate(icon));
            }
        }
    }

    for (auto &mesoicon : meso_icons_) {
        mesoicon.update();
    }

    drops_.update(physics);

    loot_enabled_ = true;
}

void MapDrops::spawn(DropSpawn &&spawn) {
    spawns_.emplace(std::move(spawn));
}

void MapDrops::remove(int32_t oid, int8_t mode, const PhysicsObject *looter) {
    if (auto drop = drops_.get<Drop>(oid)) {
        drop->get().expire(mode, looter);
    }
}

void MapDrops::clear() {
    drops_.clear();
}

MapDrops::Loot MapDrops::find_loot_at(Point<int16_t> playerpos) {
    if (!loot_enabled_) {
        return { 0, {} };
    }

    for (auto &mmo : drops_) {
        // auto drop = mmo.second.get();
        auto drop = create_optional<const Drop>(mmo.second.get());

        if (drop && drop->get().bounds().contains(playerpos)) {
            loot_enabled_ = false;

            int32_t oid = mmo.first;
            Point<int16_t> position = drop->get().get_position();

            return { oid, position };
        }
    }

    return { 0, {} };
}
}  // namespace ms