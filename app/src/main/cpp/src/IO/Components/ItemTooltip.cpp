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
#include "ItemTooltip.h"

#include <nlnx/nx.hpp>

#include "../Constants.h"
#include "../Data/ItemData.h"

namespace ms {
// TODO: Add blue dot next to name
ItemTooltip::ItemTooltip() {
    nl::node Item = nl::nx::ui["UIToolTip.img"]["Item"];
    nl::node Frame = Item["Frame2"];
    nl::node ItemIcon = Item["ItemIcon"];

    frame_ = Frame;
    cover_ = Frame["cover"];
    base_ = ItemIcon["base"];
    item_cover_ = ItemIcon["cover"];
    type_[true] = ItemIcon["new"];
    type_[false] = ItemIcon["old"];

    item_id_ = 0;
}

bool ItemTooltip::set_item(int32_t iid) {
    if (item_id_ == iid) {
        return false;
    }

    item_id_ = iid;

    if (item_id_ == 0) {
        return false;
    }

    const ItemData &idata = ItemData::get(item_id_);

    item_icon_ = idata.get_icon(false);
    untradable_ = idata.is_untradable();
    unique_ = idata.is_unique();

    std::string quality = "";

    if (unique_ && untradable_) {
        quality = "One-of-a-kind Item, Untradable";
    } else if (unique_ && !untradable_) {
        quality = "One-of-a-kind Item";
    } else if (!unique_ && untradable_) {
        quality = "Untradable";
    } else {
        quality = "";
    }

    name_ = Text(Text::Font::A12B,
                 Text::Alignment::LEFT,
                 Color::Name::WHITE,
                 idata.get_name(),
                 240);
    desc_ = Text(Text::Font::A12M,
                 Text::Alignment::LEFT,
                 Color::Name::WHITE,
                 idata.get_desc(),
                 185);
    qual_ = Text(Text::Font::A12M,
                 Text::Alignment::CENTER,
                 Color::Name::ORANGE,
                 quality,
                 185);

    fill_width_ = 264;
    fill_height_ = 83 + name_.height();
    desc_delta_ = desc_.height() - 80;

    if (desc_delta_ > 0) {
        fill_height_ += desc_delta_;
    }

    return true;
}

void ItemTooltip::draw(Point<int16_t> pos) const {
    if (item_id_ == 0) {
        return;
    }

    int16_t max_width = Constants::Constants::get().get_viewwidth();
    int16_t max_height = Constants::Constants::get().get_viewheight();
    int16_t cur_width = pos.x() + fill_width_ + 32;
    int16_t cur_height = pos.y() + fill_height_ + 40;

    int16_t adj_x = cur_width - max_width;
    int16_t adj_y = cur_height - max_height;

    int16_t adj_d = desc_delta_ > 0 ? desc_delta_ : 0;
    int16_t adj_t = (untradable_ || unique_) ? 19 : 0;

    if (adj_x > 0) {
        pos.shift_x(adj_x * -1);
    }

    if (adj_y > 0) {
        pos.shift_y(adj_y * -1);
    }

    frame_.draw(pos + Point<int16_t>(150, 118 + adj_d + adj_t),
                fill_width_,
                fill_height_ + adj_t);
    cover_.draw(pos + Point<int16_t>(4, 4));
    name_.draw(pos + Point<int16_t>(22, 8));

    if (untradable_ || unique_) {
        qual_.draw(pos + Point<int16_t>(148, 27));
    }

    pos.shift(14, 18 + name_.height() + adj_t);

    base_.draw(pos);
    type_[true].draw(pos);
    item_icon_.draw(DrawArgument(pos + Point<int16_t>(8, 72), 2.0f, 2.0f));
    item_cover_.draw(pos);
    desc_.draw(pos + Point<int16_t>(90, -6));
}
}  // namespace ms