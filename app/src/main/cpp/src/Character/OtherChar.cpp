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
#include "OtherChar.h"

#include "../Constants.h"

namespace ms {
OtherChar::OtherChar(int32_t id,
                     const CharLook &lk,
                     uint8_t lvl,
                     int16_t jb,
                     const std::string &nm,
                     int8_t st,
                     Point<int16_t> pos) :
    Char(id, lk, nm) {
    level_ = lvl;
    job_ = jb;
    set_position(pos);

    last_move_.xpos = pos.x();
    last_move_.ypos = pos.y();
    last_move_.newstate = st;
    timer_ = 0;

    attack_speed_ = 6;
    attacking_ = false;
}

int8_t OtherChar::update(const Physics &physics) {
    if (timer_ > 1) {
        timer_--;
    } else if (timer_ == 1) {
        if (!movements_.empty()) {
            last_move_ = movements_.front();
            movements_.pop();
        } else {
            timer_ = 0;
        }
    }

    if (!attacking_) {
        uint8_t laststate = last_move_.newstate;
        set_state(laststate);
    }

    phobj_.hspeed = last_move_.xpos - phobj_.crnt_x();
    phobj_.vspeed = last_move_.ypos - phobj_.crnt_y();
    phobj_.move();

    physics.get_fht().update_fh(phobj_);

    bool aniend = Char::update(physics, get_stancespeed());

    if (aniend && attacking_) {
        attacking_ = false;
    }

    return get_layer();
}

void OtherChar::send_movement(const std::vector<Movement> &newmoves) {
    movements_.push(newmoves.back());

    if (timer_ == 0) {
        constexpr uint16_t DELAY = 50;
        timer_ = DELAY;
    }
}

void OtherChar::update_skill(int32_t skillid, uint8_t skilllevel) {
    skill_levels_[skillid] = skilllevel;
}

void OtherChar::update_speed(uint8_t as) {
    attack_speed_ = as;
}

void OtherChar::update_look(const LookEntry &newlook) {
    look_ = newlook;

    uint8_t laststate = last_move_.newstate;
    set_state(laststate);
}

int8_t OtherChar::get_integer_attackspeed() const {
    return attack_speed_;
}

uint16_t OtherChar::get_level() const {
    return level_;
}

int32_t OtherChar::get_skilllevel(int32_t skillid) const {
    auto iter = skill_levels_.find(skillid);

    if (iter == skill_levels_.end()) {
        return 0;
    }

    return iter->second;
}
}  // namespace ms