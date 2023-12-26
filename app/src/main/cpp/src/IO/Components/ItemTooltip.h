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

#include "../Components/MapleFrame.h"
#include "../Graphics/Text.h"
#include "../Template/BoolPair.h"
#include "Tooltip.h"

namespace ms {
class ItemTooltip : public Tooltip {
public:
    ItemTooltip();

    void draw(Point<int16_t> position) const override;

    bool set_item(int32_t itemid);

private:
    int32_t item_id_;
    int16_t fill_width_;
    int16_t fill_height_;
    int16_t desc_delta_;
    Texture item_icon_;

    Text name_;
    Text desc_;
    Text qual_;
    MapleFrame frame_;
    Texture cover_;
    Texture base_;
    Texture item_cover_;
    BoolPair<Texture> type_;
    bool untradable_;
    bool unique_;
};
}  // namespace ms