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
#pragma once

#include <cstdint>
#include <nlnx/node.hpp>
#include <string>
#include <unordered_map>

namespace ms::NxHelper::Map {
struct MapInfo {
    std::string description;
    std::string name;
    std::string street_name;
    std::string full_name;
};

// Returns all relative map info
MapInfo get_map_info_by_id(int32_t mapid);

// Returns the category of a map
std::string get_map_category(int32_t mapid);

// Returns a list of all life on a map (Mobs and NPCs)
std::unordered_map<int64_t, std::pair<std::string, std::string>>
get_life_on_map(int32_t mapid);

// Returns the name of the node, under which the argument map id is in
nl::node get_map_node_name(int32_t mapid);
}  // namespace ms::NxHelper::Map