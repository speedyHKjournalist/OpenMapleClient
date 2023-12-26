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
#include "PetLook.h"

#include <nlnx/nx.hpp>
#include <utility>

namespace ms {
PetLook::PetLook(int32_t iid,
                 std::string nm,
                 int32_t uqid,
                 Point<int16_t> pos,
                 uint8_t st,
                 int32_t) {
    item_id_ = iid;
    name_ = std::move(nm);
    unique_id_ = uqid;

    set_position(pos.x(), pos.y());
    set_stance(st);

    name_label_ = Text(Text::Font::A13M,
                       Text::Alignment::CENTER,
                       Color::Name::WHITE,
                       Text::Background::NAMETAG,
                       name_);

    std::string strid = std::to_string(iid);
    nl::node src = nl::nx::item["Pet"][strid + ".img"];

    animations_[Stance::MOVE] = src["move"];
    animations_[Stance::STAND] = src["stand0"];
    animations_[Stance::JUMP] = src["jump"];
    animations_[Stance::ALERT] = src["alert"];
    animations_[Stance::PRONE] = src["prone"];
    animations_[Stance::FLY] = src["fly"];
    animations_[Stance::HANG] = src["hang"];

    nl::node effsrc = nl::nx::effect["PetEff.img"][strid];

    animations_[Stance::WARP] = effsrc["warp"];
}

PetLook::PetLook() {
    item_id_ = 0;
    name_ = "";
    unique_id_ = 0;
    stance_ = Stance::STAND;
}

void PetLook::draw(double viewx, double viewy, float alpha) const {
    Point<int16_t> absp = phobj_.get_absolute(viewx, viewy, alpha);

    animations_[stance_].draw(DrawArgument(absp, flip_), alpha);
    name_label_.draw(absp);
}

void PetLook::update(const Physics &physics, Point<int16_t> charpos) {
    static const double PETWALKFORCE = 0.35;
    static const double PETFLYFORCE = 0.2;

    Point<int16_t> curpos = phobj_.get_position();

    switch (stance_) {
        case Stance::STAND:
        case Stance::MOVE:
            if (curpos.distance(charpos) > 150) {
                set_position(charpos.x(), charpos.y());
            } else {
                if (charpos.x() - curpos.x() > 50) {
                    phobj_.hforce = PETWALKFORCE;
                    flip_ = true;

                    set_stance(Stance::MOVE);
                } else if (charpos.x() - curpos.x() < -50) {
                    phobj_.hforce = -PETWALKFORCE;
                    flip_ = false;

                    set_stance(Stance::MOVE);
                } else {
                    phobj_.hforce = 0.0;

                    set_stance(Stance::STAND);
                }
            }

            phobj_.type = PhysicsObject::Type::NORMAL;
            phobj_.clear_flag(PhysicsObject::Flag::NO_GRAVITY);
            break;
        case Stance::HANG:
            set_position(charpos.x(), charpos.y());
            phobj_.set_flag(PhysicsObject::Flag::NO_GRAVITY);
            break;
        case Stance::FLY:
            if ((charpos - curpos).length() > 250) {
                set_position(charpos.x(), charpos.y());
            } else {
                if (charpos.x() - curpos.x() > 50) {
                    phobj_.hforce = PETFLYFORCE;
                    flip_ = true;
                } else if (charpos.x() - curpos.x() < -50) {
                    phobj_.hforce = -PETFLYFORCE;
                    flip_ = false;
                } else {
                    phobj_.hforce = 0.0f;
                }

                if (charpos.y() - curpos.y() > 50.0f) {
                    phobj_.vforce = PETFLYFORCE;
                } else if (charpos.y() - curpos.y() < -50.0f) {
                    phobj_.vforce = -PETFLYFORCE;
                } else {
                    phobj_.vforce = 0.0f;
                }
            }

            phobj_.type = PhysicsObject::Type::FLYING;
            phobj_.clear_flag(PhysicsObject::Flag::NO_GRAVITY);
            break;
    }

    physics.move_object(phobj_);

    animations_[stance_].update();
}

void PetLook::set_position(int16_t x, int16_t y) {
    phobj_.set_x(x);
    phobj_.set_y(y);
}

void PetLook::set_stance(Stance st) {
    if (stance_ != st) {
        stance_ = st;
        animations_[stance_].reset();
    }
}

void PetLook::set_stance(uint8_t stancebyte) {
    flip_ = stancebyte % 2 == 1;
    stance_ = stancebyvalue(stancebyte);
}

int32_t PetLook::get_itemid() const {
    return item_id_;
}

PetLook::Stance PetLook::get_stance() const {
    return stance_;
}
}  // namespace ms