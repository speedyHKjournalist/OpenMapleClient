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
#include "StatefulIcon.h"

namespace ms {
StatefulIcon::StatefulIcon(std::unique_ptr<Type> type,
                           Texture ntx,
                           Texture dtx,
                           Texture motx) :
    Icon(std::move(type), ntx, -1) {
    ntx.shift({ 0, 32 });
    dtx.shift({ 0, 32 });
    motx.shift({ 0, 32 });
    textures_[State::NORMAL] = ntx;
    textures_[State::DISABLED] = dtx;
    textures_[State::MOUSEOVER] = motx;

    state_ = State::NORMAL;
}

Texture StatefulIcon::get_texture() const {
    return textures_[state_];
}

void StatefulIcon::set_state(State s) {
    state_ = s;
}
}  // namespace ms