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
#include "BulletData.h"

#include <nlnx/nx.hpp>

namespace ms {
BulletData::BulletData(int32_t itemid) : item_data_(ItemData::get(itemid)) {
    std::string prefix = "0" + std::to_string(itemid / 10000);
    std::string strid = "0" + std::to_string(itemid);
    nl::node src = nl::nx::item["Consume"][prefix + ".img"][strid];

    bullet_ = src["bullet"];
    watk_ = src["info"]["incPAD"];
}

bool BulletData::is_valid() const {
    return item_data_.is_valid();
}

BulletData::operator bool() const {
    return is_valid();
}

int16_t BulletData::get_watk() const {
    return watk_;
}

const Animation &BulletData::get_animation() const {
    return bullet_;
}

const ItemData &BulletData::get_itemdata() const {
    return item_data_;
}
}  // namespace ms