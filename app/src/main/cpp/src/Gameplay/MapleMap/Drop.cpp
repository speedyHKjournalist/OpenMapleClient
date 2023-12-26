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
#include "Drop.h"

namespace ms {
Drop::Drop(int32_t id,
           int32_t own,
           Point<int16_t> start,
           Point<int16_t> dst,
           int8_t type,
           int8_t mode,
           bool pldrp) :
    MapObject(id) {
    owner_ = own;
    set_position(start.x(), start.y() - 4);
    dest_ = dst;
    pickup_type_ = type;
    player_drop_ = pldrp;

    angle_.set(0.0f);
    opacity_.set(1.0f);
    moved_ = 0.0f;
    looter_ = nullptr;

    switch (mode) {
        case 0:
        case 1:
            state_ = Drop::State::DROPPED;
            base_y_ = static_cast<double>(dest_.y() - 4);
            phobj_.vspeed = -5.0f;
            phobj_.hspeed = static_cast<double>(dest_.x() - start.x()) / 48;
            break;
        case 2:
            state_ = Drop::State::FLOATING;
            base_y_ = phobj_.crnt_y();
            phobj_.type = PhysicsObject::Type::FIXATED;
            break;
        case 3:
            state_ = Drop::State::PICKED_UP;
            phobj_.vspeed = -5.0f;
            break;
    }
}

int8_t Drop::update(const Physics &physics) {
    physics.move_object(phobj_);

    if (state_ == Drop::State::DROPPED) {
        if (phobj_.onground) {
            phobj_.hspeed = 0.0;
            phobj_.type = PhysicsObject::Type::FIXATED;
            state_ = Drop::State::FLOATING;
            angle_.set(0.0f);
            set_position(dest_.x(), dest_.y() - 4);
        } else {
            static const float SPINSTEP = 0.2f;
            angle_ += SPINSTEP;
        }
    }

    if (state_ == Drop::State::FLOATING) {
        phobj_.y = base_y_ + 5.0f + (cos(moved_) - 1.0f) * 2.5f;
        moved_ = (moved_ < 360.0f) ? moved_ + 0.025f : 0.0f;
    }

    if (state_ == Drop::State::PICKED_UP) {
        static const uint16_t PICKUPTIME = 48;
        static const float OPCSTEP = 1.0f / PICKUPTIME;

        if (looter_) {
            double hdelta = looter_->x - phobj_.x;
            phobj_.hspeed =
                looter_->hspeed / 2.0 + (hdelta - 16.0) / PICKUPTIME;
        }

        opacity_ -= OPCSTEP;

        if (opacity_.last() <= OPCSTEP) {
            opacity_.set(1.0f);

            MapObject::deactivate();
            return -1;
        }
    }

    return phobj_.fhlayer;
}

void Drop::expire(int8_t type, const PhysicsObject *lt) {
    switch (type) {
        case 0: state_ = Drop::State::PICKED_UP; break;
        case 1: deactivate(); break;
        case 2:
            angle_.set(0.0f);
            state_ = Drop::State::PICKED_UP;
            looter_ = lt;
            phobj_.vspeed = -4.5f;
            phobj_.type = PhysicsObject::Type::NORMAL;
            break;
    }
}

Rectangle<int16_t> Drop::bounds() const {
    auto lt = get_position();
    auto rb = lt + Point<int16_t>(32, 32);

    return Rectangle<int16_t>(lt, rb);
}
}  // namespace ms