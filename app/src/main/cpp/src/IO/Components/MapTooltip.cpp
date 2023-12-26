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
#include "MapTooltip.h"

#include <nlnx/nx.hpp>

#include "../Constants.h"
#include "GameInfo.h"

namespace ms {
MapTooltip::MapTooltip() :
    name_(""),
    description_(""),
    fill_width_(MIN_WIDTH),
    fill_height_(0) {
    nl::node Frame = nl::nx::ui["UIToolTip.img"]["Item"]["Frame2"];
    nl::node WorldMap = nl::nx::ui["UIWindow2.img"]["ToolTip"]["WorldMap"];

    frame_ = Frame;
    cover_ = Frame["cover"];
    mob_ = WorldMap["Mob"];
    npc_ = WorldMap["Npc"];
    party_ = WorldMap["Party"];
}

void MapTooltip::draw(Point<int16_t> pos) const {
    if (name_label_.empty()) {
        return;
    }

    int16_t max_width = Constants::Constants::get().get_viewwidth();
    int16_t max_height = Constants::Constants::get().get_viewheight();

    if (parent_ == Tooltip::Parent::MINI_MAP && mob_labels_->empty()
        && npc_labels_->empty()) {
        if (desc_label_.empty()) {
            int16_t new_width = name_simple_.width();
            int16_t new_height = name_simple_.height();

            int16_t cur_width = pos.x() + new_width + 21;
            int16_t cur_height = pos.y() + new_height + 40;

            int16_t adj_x = cur_width - max_width;
            int16_t adj_y = cur_height - max_height;

            if (adj_x > 0) {
                pos.shift_x(adj_x * -1);
            }

            if (adj_y > 0) {
                pos.shift_y(adj_y * -1);
            }

            frame_.draw(pos + Point<int16_t>(new_width / 2 + 2, new_height - 7),
                        new_width - 14,
                        new_height - 18);

            if (new_height > 18) {
                name_simple_.draw(pos);
            } else {
                name_simple_.draw(pos + Point<int16_t>(1, -3));
            }
        } else {
            int16_t name_width = name_label_.width();
            int16_t name_height = name_label_.height();

            int16_t desc_width = desc_simple_.width();

            int16_t new_width =
                (name_width > desc_width) ? name_width : desc_width;
            int16_t new_height = name_height + desc_simple_.height() - 11;

            int16_t cur_width = pos.x() + new_width + 21;
            int16_t cur_height = pos.y() + new_height + 40;

            int16_t adj_x = cur_width - max_width;
            int16_t adj_y = cur_height - max_height;

            if (adj_x > 0) {
                pos.shift_x(adj_x * -1);
            }

            if (adj_y > 0) {
                pos.shift_y(adj_y * -1);
            }

            int16_t half_width = new_width / 2;

            frame_.draw(pos
                            + Point<int16_t>(half_width + 2,
                                             new_height - 7 + BOTTOM_PADDING),
                        new_width - 14,
                        new_height - 18 + BOTTOM_PADDING);
            cover_.draw(pos + Point<int16_t>(-5, -2));
            name_label_.draw(pos + Point<int16_t>(half_width, -2));

            pos.shift_y(name_height);

            separator_.draw(pos + SEPARATOR_ADJ);
            desc_simple_.draw(pos + Point<int16_t>(half_width, -3));
        }
    } else {
        int16_t cur_width = pos.x() + fill_width_ + 21;
        int16_t cur_height = pos.y() + fill_height_ + 40;

        int16_t adj_x = cur_width - max_width;
        int16_t adj_y = cur_height - max_height;

        if (adj_x > 0) {
            pos.shift_x(adj_x * -1);
        }

        if (adj_y > 0) {
            pos.shift_y(adj_y * -1);
        }

        int16_t half_width = fill_width_ / 2;

        frame_.draw(pos
                        + Point<int16_t>(half_width + 2,
                                         fill_height_ - 7 + BOTTOM_PADDING),
                    fill_width_ - 14,
                    fill_height_ - 18 + BOTTOM_PADDING);
        cover_.draw(pos + Point<int16_t>(-5, -2));
        name_label_.draw(pos + Point<int16_t>(half_width, 0));

        int16_t name_height = name_label_.height();

        if (!desc_label_.empty()) {
            pos.shift_y(name_height + 4);

            desc_label_.draw(pos + Point<int16_t>(4, 0));

            pos.shift_y(desc_label_.height() + BOTTOM_PADDING);
        } else {
            pos.shift_y(name_height + BOTTOM_PADDING);
        }

        if (mob_labels_->length() > 0) {
            separator_.draw(pos + SEPARATOR_ADJ);

            for (size_t i = 0; i < MAX_LIFE; i++) {
                if (!mob_labels_[i].empty()) {
                    mob_labels_[i].draw(pos + LIFE_LABEL_ADJ);

                    if (i == 0) {
                        mob_.draw(pos + LIFE_ICON_ADJ);
                    }

                    pos.shift_y(mob_labels_[i].height());
                }
            }

            pos.shift_y(8);
        }

        if (npc_labels_->length() > 0) {
            separator_.draw(pos + SEPARATOR_ADJ);

            for (size_t i = 0; i < MAX_LIFE; i++) {
                if (!npc_labels_[i].empty()) {
                    npc_labels_[i].draw(pos + LIFE_LABEL_ADJ);

                    if (i == 0) {
                        npc_.draw(pos + LIFE_ICON_ADJ);
                    }

                    pos.shift_y(npc_labels_[i].height());
                }
            }
        }
    }
}

void MapTooltip::set_name(Tooltip::Parent p,
                          const std::string &n,
                          bool bolded) {
    if (name_ == n || parent_ == p) {
        return;
    }

    name_ = n;
    parent_ = p;

    if (name_.empty()
        || (parent_ != Tooltip::Parent::WORLD_MAP
            && parent_ != Tooltip::Parent::MINI_MAP)) {
        return;
    }

    name_label_ = Text(bolded ? Text::Font::A12B : Text::Font::A12M,
                       Text::Alignment::CENTER,
                       Color::Name::WHITE,
                       name_);
    name_simple_ = Text(bolded ? Text::Font::A12B : Text::Font::A12M,
                        Text::Alignment::LEFT,
                        Color::Name::WHITE,
                        name_);

    int16_t width = name_label_.width();
    int16_t height = name_label_.height();

    if (width > fill_width_) {
        fill_width_ = width;
    }

    separator_ = ColorLine(fill_width_ - 6, Color::Name::WHITE, 0.40f);

    if (height > fill_height_) {
        fill_height_ = height;
    }
}

void MapTooltip::set_desc(const std::string &d) {
    if (description_ == d) {
        return;
    }

    description_ = d;

    if (description_.empty()) {
        return;
    }

    desc_label_ = Text(Text::Font::A12M,
                       Text::Alignment::LEFT,
                       Color::Name::WHITE,
                       description_,
                       fill_width_);
    desc_simple_ = Text(Text::Font::A12M,
                        Text::Alignment::CENTER,
                        Color::Name::WHITE,
                        description_,
                        fill_width_);

    fill_width_ += 17;

    if (parent_ == Tooltip::Parent::MINI_MAP) {
        int16_t name_width = name_label_.width();
        int16_t desc_width = desc_simple_.width();
        int16_t new_width = (name_width > desc_width) ? name_width : desc_width;

        separator_ = ColorLine(new_width - 6, Color::Name::WHITE, 0.40f);
    } else {
        separator_ = ColorLine(fill_width_ - 6, Color::Name::WHITE, 0.40f);
    }

    fill_height_ += desc_label_.height() + 4;
}

void MapTooltip::set_mapid(int32_t mapid) {
    size_t m = 0;
    size_t n = 0;
    auto life = NxHelper::Map::get_life_on_map(mapid);

    for (const auto &l : life) {
        auto [life_type, life_info] = l.second;

        if (life_type == "m" && m < MAX_LIFE) {
            mob_labels_[m] = Text(Text::Font::A12M,
                                  Text::Alignment::LEFT,
                                  Color::Name::CHARTREUSE,
                                  life_info);
            fill_height_ += mob_labels_->height() + 2;
            m++;
        } else if (life_type == "n" && n < MAX_LIFE) {
            npc_labels_[n] = Text(Text::Font::A12M,
                                  Text::Alignment::LEFT,
                                  Color::Name::MALIBU,
                                  life_info);
            fill_height_ += npc_labels_->height() + 2;
            n++;
        }
    }

    if (desc_label_.empty()) {
        if (mob_labels_->length() > 0 || npc_labels_->length() > 0) {
            fill_height_ += BOTTOM_PADDING;
        }
    }
}

void MapTooltip::reset() {
    set_name(Tooltip::Parent::NONE, "", false);
    set_desc("");

    desc_label_.change_text("");
    name_label_.change_text("");

    for (size_t i = 0; i < MAX_LIFE; i++) {
        mob_labels_[i].change_text("");
        npc_labels_[i].change_text("");
    }

    fill_width_ = MIN_WIDTH;
    fill_height_ = 0;
}
}  // namespace ms