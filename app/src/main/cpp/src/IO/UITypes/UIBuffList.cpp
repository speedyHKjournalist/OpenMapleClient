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
#include "UIBuffList.h"

#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>

#include "../Data/ItemData.h"
#include "StringHandling.h"

namespace ms {
BuffIcon::BuffIcon(int32_t buff, int32_t dur) :
    cover_(IconCover::BUFF, dur - FLASH_TIME_) {
    buffid_ = buff;
    duration_ = dur;
    opacity_.set(1.0f);
    opcstep_ = -0.05f;

    if (buffid_ >= 0) {
        std::string strid = string_format::extend_id(buffid_, 7);
        nl::node src =
            nl::nx::skill[strid.substr(0, 3) + ".img"]["skill"][strid];
        icon_ = src["icon"];
    } else {
        icon_ = ItemData::get(-buffid_).get_icon(true);
    }
}

void BuffIcon::draw(Point<int16_t> position, float alpha) const {
    icon_.draw(DrawArgument(position, opacity_.get(alpha)));
    cover_.draw(position + Point<int16_t>(1, -31), alpha);
}

bool BuffIcon::update() {
    if (duration_ <= FLASH_TIME_) {
        opacity_ += opcstep_;

        bool fadedout = opcstep_ < 0.0f && opacity_.last() <= 0.0f;
        bool fadedin = opcstep_ > 0.0f && opacity_.last() >= 1.0f;

        if (fadedout || fadedin) {
            opcstep_ = -opcstep_;
        }
    }

    cover_.update();

    duration_ -= Constants::TIMESTEP;

    return duration_ < Constants::TIMESTEP;
}

UIBuffList::UIBuffList() {
    int16_t height = Constants::Constants::get().get_viewheight();
    int16_t width = Constants::Constants::get().get_viewwidth();

    update_screen(width, height);
}

void UIBuffList::draw(float alpha) const {
    Point<int16_t> icpos = position_;

    for (const auto &icon : icons_) {
        icon.second.draw(icpos, alpha);
        icpos.shift_x(-32);
    }
}

void UIBuffList::update() {
    for (auto iter = icons_.begin(); iter != icons_.end();) {
        bool expired = iter->second.update();

        if (expired) {
            iter = icons_.erase(iter);
        } else {
            iter++;
        }
    }
}

void UIBuffList::update_screen(int16_t new_width, int16_t) {
    position_ = Point<int16_t>(new_width - 35, 55);
    dimension_ = Point<int16_t>(position_.x(), 32);
}

Cursor::State UIBuffList::send_cursor(bool pressed,
                                      Point<int16_t> cursorposition) {
    return UIElement::send_cursor(pressed, cursorposition);
}

UIElement::Type UIBuffList::get_type() const {
    return TYPE;
}

void UIBuffList::add_buff(int32_t buffid, int32_t duration) {
    icons_.emplace(std::piecewise_construct,
                   std::forward_as_tuple(buffid),
                   std::forward_as_tuple(buffid, duration));
}
}  // namespace ms