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
#include "Cursor.h"

#include <nlnx/nx.hpp>

#include "../Constants.h"

namespace ms {
Cursor::Cursor() {
    state_ = Cursor::State::IDLE;
    hide_counter_ = 0;
}

void Cursor::init() {
    nl::node src = nl::nx::ui["Basic.img"]["Cursor"];

    for (auto iter : animations_) {
        iter.second = src[iter.first];
    }
}

void Cursor::draw(float alpha) const {
    constexpr int64_t HIDE_AFTER = HIDE_TIME / Constants::TIMESTEP;

    if (hide_counter_ < HIDE_AFTER) {
        animations_[state_].draw(position_, alpha);
    }
}

void Cursor::update() {
    animations_[state_].update();

    switch (state_) {
        case Cursor::State::CAN_CLICK:
        case Cursor::State::CAN_CLICK2:
        case Cursor::State::CAN_GRAB:
        case Cursor::State::CLICKING:
        case Cursor::State::GRABBING: hide_counter_ = 0; break;
        default: hide_counter_++; break;
    }
}

void Cursor::set_state(State s) {
    if (state_ != s) {
        state_ = s;

        animations_[state_].reset();
        hide_counter_ = 0;
    }
}

void Cursor::set_position(Point<int16_t> pos) {
    position_ = pos;
    hide_counter_ = 0;
}

Cursor::State Cursor::get_state() const {
    return state_;
}

Point<int16_t> Cursor::get_position() const {
    return position_;
}
}  // namespace ms