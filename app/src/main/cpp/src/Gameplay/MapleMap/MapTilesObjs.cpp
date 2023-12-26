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
#include "MapTilesObjs.h"

namespace ms {
TilesObjs::TilesObjs(const nl::node &src) {
    auto tileset = src["info"]["tS"] + ".img";

    for (const auto &tilenode : src["tile"]) {
        Tile tile { tilenode, tileset };
        int8_t z = tile.getz();
        tiles_.emplace(z, std::move(tile));
    }

    for (const auto &objnode : src["obj"]) {
        Obj obj { objnode };
        int8_t z = obj.getz();
        objs_.emplace(z, std::move(obj));
    }
}

TilesObjs::TilesObjs() = default;

void TilesObjs::update() {
    for (auto &iter : objs_) {
        iter.second.update();
    }
}

void TilesObjs::draw(Point<int16_t> viewpos, float alpha) const {
    for (const auto &iter : objs_) {
        iter.second.draw(viewpos, alpha);
    }

    for (const auto &iter : tiles_) {
        iter.second.draw(viewpos);
    }
}

MapTilesObjs::MapTilesObjs(const nl::node &src) {
    for (auto iter : layers_) {
        iter.second = src[iter.first];
    }
}

MapTilesObjs::MapTilesObjs() = default;

void MapTilesObjs::draw(Layer::Id layer,
                        Point<int16_t> viewpos,
                        float alpha) const {
    layers_[layer].draw(viewpos, alpha);
}

void MapTilesObjs::update() {
    for (auto iter : layers_) {
        iter.second.update();
    }
}
}  // namespace ms