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
#include "Pet.h"

#include <utility>

namespace ms {
Pet::Pet(int32_t item_id,
         int64_t expiration,
         std::string petname,
         uint8_t level,
         uint16_t closeness,
         uint8_t fullness) :
//    item_id_(item_id),
//    expiration_(expiration),
    pet_name_(std::move(petname))
//    pet_level_(level),
//    closeness_(closeness),
    /*fullness_(fullness)*/ {}
}  // namespace ms