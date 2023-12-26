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
#include "CharLook.h"

#include "../../Data/WeaponData.h"

namespace ms {
CharLook::CharLook(const LookEntry &entry) {
    reset();

    set_body(entry.skin);
    set_hair(entry.hairid);
    set_face(entry.faceid);

    for (const auto &equip : entry.equips) {
        add_equip(equip.second);
    }
}

CharLook::CharLook() {
    reset();

    body_ = nullptr;
    hair_ = nullptr;
    face_ = nullptr;
}

void CharLook::reset() {
    flip_ = true;

    action_ = nullptr;
    actionstr_ = "";
    actframe_ = 0;

    set_stance(Stance::Id::STAND1);
    st_frame_.set(0);
    st_elapsed_ = 0;

    set_expression(Expression::Id::DEFAULT);
    exp_frame_.set(0);
    exp_elapsed_ = 0;
}

void CharLook::draw(const DrawArgument &args,
                    Stance::Id interstance,
                    Expression::Id interexpression,
                    uint8_t interframe,
                    uint8_t interexpframe) const {
    Point<int16_t> faceshift = draw_info_.get_face_pos(interstance, interframe);
    DrawArgument faceargs =
        args + DrawArgument { faceshift, false, Point<int16_t> {} };

    if (Stance::Id::DEAD == interstance) {
        Point<int16_t> faceshift =
            draw_info_.get_face_pos(Stance::Id::STAND1, 1);
        DrawArgument faceargs =
            args + DrawArgument { faceshift, false, Point<int16_t> {} };

        hair_->draw(interstance, Hair::Layer::BELOW_BODY, interframe, args);
        equips_.draw(EquipSlot::Id::HAT,
                     interstance,
                     Clothing::Layer::CAP_BELOW_BODY,
                     interframe,
                     args);
        body_->draw(interstance, Body::Layer::BODY, interframe, args);
        hair_->draw(interstance, Hair::Layer::DEFAULT, interframe, args);
        body_->draw(Stance::Id::STAND1, Body::Layer::HEAD, 1, args);
        hair_->draw(interstance, Hair::Layer::SHADE, interframe, args);

        hair_->draw(interstance, Hair::Layer::DEFAULT, interframe, args);
        body_->draw(interstance, Body::Layer::HEAD, interframe, args);
        hair_->draw(interstance, Hair::Layer::SHADE, interframe, args);
        face_->draw(interexpression, interexpframe, faceargs);

        switch (equips_.getcaptype()) {
            case CharEquips::CapType::NONE:
                hair_->draw(interstance,
                            Hair::Layer::OVER_HEAD,
                            interframe,
                            args);
                break;
            case CharEquips::CapType::HEADBAND:
                equips_.draw(EquipSlot::Id::HAT,
                             Stance::Id::STAND1,
                             Clothing::Layer::CAP,
                             1,
                             args);
                hair_->draw(Stance::Id::STAND1, Hair::Layer::DEFAULT, 1, args);
                hair_->draw(Stance::Id::STAND1,
                            Hair::Layer::OVER_HEAD,
                            1,
                            args);
                equips_.draw(EquipSlot::Id::HAT,
                             Stance::Id::STAND1,
                             Clothing::Layer::CAP_OVER_HAIR,
                             1,
                             args);
                break;
            case CharEquips::CapType::HALF_COVER:
                hair_->draw(Stance::Id::STAND1, Hair::Layer::DEFAULT, 1, args);
                equips_.draw(EquipSlot::Id::HAT,
                             Stance::Id::STAND1,
                             Clothing::Layer::CAP,
                             1,
                             args);
                break;
            case CharEquips::CapType::FULL_COVER:
                equips_.draw(EquipSlot::Id::HAT,
                             Stance::Id::STAND1,
                             Clothing::Layer::CAP,
                             1,
                             args);
                break;
        }

        return;
    }

    if (Stance::is_climbing(interstance)) {
        body_->draw(interstance, Body::Layer::BODY, interframe, args);
        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::GLOVE,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::SHOES,
                     interstance,
                     Clothing::Layer::SHOES,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::BOTTOM,
                     interstance,
                     Clothing::Layer::PANTS,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::TOP,
                     interstance,
                     Clothing::Layer::TOP,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::TOP,
                     interstance,
                     Clothing::Layer::MAIL,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::CAPE,
                     interstance,
                     Clothing::Layer::CAPE,
                     interframe,
                     args);
        body_->draw(interstance, Body::Layer::HEAD, interframe, args);
        equips_.draw(EquipSlot::Id::EARACC,
                     interstance,
                     Clothing::Layer::EARRINGS,
                     interframe,
                     args);

        switch (equips_.getcaptype()) {
            case CharEquips::CapType::NONE:
                hair_->draw(interstance, Hair::Layer::BACK, interframe, args);
                break;
            case CharEquips::CapType::HEADBAND:
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP,
                             interframe,
                             args);
                hair_->draw(interstance, Hair::Layer::BACK, interframe, args);
                break;
            case CharEquips::CapType::HALF_COVER:
                hair_->draw(interstance,
                            Hair::Layer::BELOW_CAP,
                            interframe,
                            args);
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP,
                             interframe,
                             args);
                break;
            case CharEquips::CapType::FULL_COVER:
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP,
                             interframe,
                             args);
                break;
        }

        equips_.draw(EquipSlot::Id::SHIELD,
                     interstance,
                     Clothing::Layer::BACK_SHIELD,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::WEAPON,
                     interstance,
                     Clothing::Layer::BACK_WEAPON,
                     interframe,
                     args);
    } else {
        hair_->draw(interstance, Hair::Layer::BELOW_BODY, interframe, args);
        equips_.draw(EquipSlot::Id::CAPE,
                     interstance,
                     Clothing::Layer::CAPE,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::SHIELD,
                     interstance,
                     Clothing::Layer::SHIELD_BELOW_BODY,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::WEAPON,
                     interstance,
                     Clothing::Layer::WEAPON_BELOW_BODY,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::HAT,
                     interstance,
                     Clothing::Layer::CAP_BELOW_BODY,
                     interframe,
                     args);
        body_->draw(interstance, Body::Layer::BODY, interframe, args);
        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::WRIST_OVER_BODY,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::GLOVE_OVER_BODY,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::SHOES,
                     interstance,
                     Clothing::Layer::SHOES,
                     interframe,
                     args);
        body_->draw(interstance, Body::Layer::ARM_BELOW_HEAD, interframe, args);

        if (equips_.has_overall()) {
            equips_.draw(EquipSlot::Id::TOP,
                         interstance,
                         Clothing::Layer::MAIL,
                         interframe,
                         args);
        } else {
            equips_.draw(EquipSlot::Id::BOTTOM,
                         interstance,
                         Clothing::Layer::PANTS,
                         interframe,
                         args);
            equips_.draw(EquipSlot::Id::TOP,
                         interstance,
                         Clothing::Layer::TOP,
                         interframe,
                         args);
        }

        body_->draw(interstance,
                    Body::Layer::ARM_BELOW_HEAD_OVER_MAIL,
                    interframe,
                    args);
        hair_->draw(interstance, Hair::Layer::DEFAULT, interframe, args);
        equips_.draw(EquipSlot::Id::SHIELD,
                     interstance,
                     Clothing::Layer::SHIELD_OVER_HAIR,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::EARACC,
                     interstance,
                     Clothing::Layer::EARRINGS,
                     interframe,
                     args);
        body_->draw(interstance, Body::Layer::HEAD, interframe, args);
        hair_->draw(interstance, Hair::Layer::SHADE, interframe, args);
        face_->draw(interexpression, interexpframe, faceargs);
        equips_.draw(EquipSlot::Id::FACE,
                     interstance,
                     Clothing::Layer::FACE_ACC,
                     0,
                     faceargs);
        equips_.draw(EquipSlot::Id::EYE_ACC,
                     interstance,
                     Clothing::Layer::EYE_ACC,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::SHIELD,
                     interstance,
                     Clothing::Layer::SHIELD,
                     interframe,
                     args);

        switch (equips_.getcaptype()) {
            case CharEquips::CapType::NONE:
                hair_->draw(interstance,
                            Hair::Layer::OVER_HEAD,
                            interframe,
                            args);
                break;
            case CharEquips::CapType::HEADBAND:
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP,
                             interframe,
                             args);
                hair_->draw(interstance,
                            Hair::Layer::DEFAULT,
                            interframe,
                            args);
                hair_->draw(interstance,
                            Hair::Layer::OVER_HEAD,
                            interframe,
                            args);
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP_OVER_HAIR,
                             interframe,
                             args);
                break;
            case CharEquips::CapType::HALF_COVER:
                hair_->draw(interstance,
                            Hair::Layer::DEFAULT,
                            interframe,
                            args);
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP,
                             interframe,
                             args);
                break;
            case CharEquips::CapType::FULL_COVER:
                equips_.draw(EquipSlot::Id::HAT,
                             interstance,
                             Clothing::Layer::CAP,
                             interframe,
                             args);
                break;
        }

        equips_.draw(EquipSlot::Id::WEAPON,
                     interstance,
                     Clothing::Layer::WEAPON_BELOW_ARM,
                     interframe,
                     args);
        bool twohanded = is_twohanded(interstance);

        if (twohanded) {
            equips_.draw(EquipSlot::Id::TOP,
                         interstance,
                         Clothing::Layer::MAILARM,
                         interframe,
                         args);
            body_->draw(interstance, Body::Layer::ARM, interframe, args);
            equips_.draw(EquipSlot::Id::WEAPON,
                         interstance,
                         Clothing::Layer::WEAPON,
                         interframe,
                         args);
        } else {
            equips_.draw(EquipSlot::Id::WEAPON,
                         interstance,
                         Clothing::Layer::WEAPON,
                         interframe,
                         args);
            body_->draw(interstance, Body::Layer::ARM, interframe, args);
            equips_.draw(EquipSlot::Id::TOP,
                         interstance,
                         Clothing::Layer::MAILARM,
                         interframe,
                         args);
        }

        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::WRIST,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::GLOVE,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::WEAPON,
                     interstance,
                     Clothing::Layer::WEAPON_OVER_GLOVE,
                     interframe,
                     args);

        body_->draw(interstance,
                    Body::Layer::HAND_BELOW_WEAPON,
                    interframe,
                    args);

        body_->draw(interstance, Body::Layer::ARM_OVER_HAIR, interframe, args);
        body_->draw(interstance,
                    Body::Layer::ARM_OVER_HAIR_BELOW_WEAPON,
                    interframe,
                    args);
        equips_.draw(EquipSlot::Id::WEAPON,
                     interstance,
                     Clothing::Layer::WEAPON_OVER_HAND,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::WEAPON,
                     interstance,
                     Clothing::Layer::WEAPON_OVER_BODY,
                     interframe,
                     args);
        body_->draw(interstance, Body::Layer::HAND_OVER_HAIR, interframe, args);
        body_->draw(interstance,
                    Body::Layer::HAND_OVER_WEAPON,
                    interframe,
                    args);

        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::WRIST_OVER_HAIR,
                     interframe,
                     args);
        equips_.draw(EquipSlot::Id::GLOVES,
                     interstance,
                     Clothing::Layer::GLOVE_OVER_HAIR,
                     interframe,
                     args);
    }
}

void CharLook::draw(const DrawArgument &args, float alpha) const {
    if (!body_ || !hair_ || !face_) {
        return;
    }

    Point<int16_t> acmove;

    if (action_) {
        acmove = action_->get_move();
    }

    DrawArgument relargs = { acmove, flip_ };

    Stance::Id interstance = stance_.get(alpha);
    Expression::Id interexpression = expression_.get(alpha);
    uint8_t interframe = st_frame_.get(alpha);
    uint8_t interexpframe = exp_frame_.get(alpha);

    switch (interstance) {
        case Stance::Id::STAND1:
        case Stance::Id::STAND2:
            if (alerted_) {
                interstance = Stance::Id::ALERT;
            }

            break;
    }

    draw(relargs + args,
         interstance,
         interexpression,
         interframe,
         interexpframe);
}

void CharLook::draw(Point<int16_t> position,
                    bool flipped,
                    Stance::Id interstance,
                    Expression::Id interexpression) const {
    interstance = equips_.adjust_stance(interstance);
    draw({ position, flipped }, interstance, interexpression, 0, 0);
}

bool CharLook::update(uint16_t timestep) {
    if (timestep == 0) {
        stance_.normalize();
        st_frame_.normalize();
        expression_.normalize();
        exp_frame_.normalize();
        return false;
    }

    alerted_.update();

    bool aniend = false;

    if (action_ == nullptr) {
        uint16_t delay = get_delay(stance_.get(), st_frame_.get());
        uint16_t delta = delay - st_elapsed_;

        if (timestep >= delta) {
            st_elapsed_ = timestep - delta;

            uint8_t nextframe = getnextframe(stance_.get(), st_frame_.get());
            float threshold = static_cast<float>(delta) / timestep;
            st_frame_.next(nextframe, threshold);

            if (st_frame_ == 0) {
                aniend = true;
            }
        } else {
            stance_.normalize();
            st_frame_.normalize();

            st_elapsed_ += timestep;
        }
    } else {
        uint16_t delay = action_->get_delay();
        uint16_t delta = delay - st_elapsed_;

        if (timestep >= delta) {
            st_elapsed_ = timestep - delta;
            actframe_ = draw_info_.next_action_frame(actionstr_, actframe_);

            if (actframe_ > 0) {
                action_ = draw_info_.get_action(actionstr_, actframe_);

                float threshold = static_cast<float>(delta) / timestep;
                stance_.next(action_->get_stance(), threshold);
                st_frame_.next(action_->get_frame(), threshold);
            } else {
                aniend = true;
                action_ = nullptr;
                actionstr_ = "";
                set_stance(Stance::Id::STAND1);
            }
        } else {
            stance_.normalize();
            st_frame_.normalize();

            st_elapsed_ += timestep;
        }
    }

    uint16_t expdelay = face_->get_delay(expression_.get(), exp_frame_.get());
    uint16_t expdelta = expdelay - exp_elapsed_;

    if (timestep >= expdelta) {
        exp_elapsed_ = timestep - expdelta;

        uint8_t nextexpframe =
            face_->nextframe(expression_.get(), exp_frame_.get());
        float fcthreshold = static_cast<float>(expdelta) / timestep;
        exp_frame_.next(nextexpframe, fcthreshold);

        if (exp_frame_ == 0) {
            if (expression_ == Expression::Id::DEFAULT) {
                expression_.next(Expression::Id::BLINK, fcthreshold);
            } else {
                expression_.next(Expression::Id::DEFAULT, fcthreshold);
            }
        }
    } else {
        expression_.normalize();
        exp_frame_.normalize();

        exp_elapsed_ += timestep;
    }

    return aniend;
}

void CharLook::set_body(int32_t skin_id) {
    auto iter = body_types_.find(skin_id);

    if (iter == body_types_.end()) {
        iter = body_types_
                   .emplace(std::piecewise_construct,
                            std::forward_as_tuple(skin_id),
                            std::forward_as_tuple(skin_id, draw_info_))
                   .first;
    }

    body_ = &iter->second;
}

void CharLook::set_hair(int32_t hair_id) {
    auto iter = hair_styles_.find(hair_id);

    if (iter == hair_styles_.end()) {
        iter = hair_styles_
                   .emplace(std::piecewise_construct,
                            std::forward_as_tuple(hair_id),
                            std::forward_as_tuple(hair_id, draw_info_))
                   .first;
    }

    hair_ = &iter->second;
}

void CharLook::set_face(int32_t face_id) {
    auto iter = face_types_.find(face_id);

    if (iter == face_types_.end()) {
        iter = face_types_.emplace(face_id, face_id).first;
    }

    face_ = &iter->second;
}

void CharLook::updatetwohanded() {
    Stance::Id basestance = Stance::baseof(stance_.get());
    set_stance(basestance);
}

void CharLook::add_equip(int32_t itemid) {
    equips_.add_equip(itemid, draw_info_);
    updatetwohanded();
}

void CharLook::remove_equip(EquipSlot::Id slot) {
    equips_.remove_equip(slot);

    if (slot == EquipSlot::Id::WEAPON) {
        updatetwohanded();
    }
}

void CharLook::attack(bool degenerate) {
    int32_t weapon_id = equips_.get_weapon();

    if (weapon_id <= 0) {
        return;
    }

    const WeaponData &weapon = WeaponData::get(weapon_id);

    uint8_t attacktype = weapon.get_attack();

    if (attacktype == 9 && !degenerate) {
        stance_.set(Stance::Id::SHOT);
        set_action("handgun");
    } else {
        stance_.set(getattackstance(attacktype, degenerate));
        st_frame_.set(0);
        st_elapsed_ = 0;
    }

    weapon.get_usesound(degenerate).play();
}

void CharLook::attack(Stance::Id newstance) {
    if (action_ || newstance == Stance::Id::NONE) {
        return;
    }

    switch (newstance) {
        case Stance::Id::SHOT: set_action("handgun"); break;
        default: set_stance(newstance); break;
    }
}

void CharLook::set_stance(Stance::Id newstance) {
    if (action_ || newstance == Stance::Id::NONE) {
        return;
    }

    Stance::Id adjstance = equips_.adjust_stance(newstance);

    if (stance_ != adjstance) {
        stance_.set(adjstance);
        st_frame_.set(0);
        st_elapsed_ = 0;
    }
}

Stance::Id CharLook::getattackstance(uint8_t attack, bool degenerate) const {
    if (stance_ == Stance::Id::PRONE) {
        return Stance::Id::PRONESTAB;
    }

    enum Attack {
        NONE = 0,
        S1A1M1D = 1,
        SPEAR = 2,
        BOW = 3,
        CROSSBOW = 4,
        S2A2M2 = 5,
        WAND = 6,
        CLAW = 7,
        GUN = 9,
        NUM_ATTACKS
    };

    static const std::array<std::vector<Stance::Id>, Attack::NUM_ATTACKS>
        degen_stances = { { { Stance::Id::NONE },
                            { Stance::Id::NONE },
                            { Stance::Id::NONE },
                            { Stance::Id::SWINGT1, Stance::Id::SWINGT3 },
                            { Stance::Id::SWINGT1, Stance::Id::STABT1 },
                            { Stance::Id::NONE },
                            { Stance::Id::NONE },
                            { Stance::Id::SWINGT1, Stance::Id::STABT1 },
                            { Stance::Id::NONE },
                            { Stance::Id::SWINGP1, Stance::Id::STABT2 } } };

    static const std::array<std::vector<Stance::Id>, NUM_ATTACKS>
        attack_stances = { { { Stance::Id::NONE },
                             { Stance::Id::STABO1,
                               Stance::Id::STABO2,
                               Stance::Id::SWINGO1,
                               Stance::Id::SWINGO2,
                               Stance::Id::SWINGO3 },
                             { Stance::Id::STABT1, Stance::Id::SWINGP1 },
                             { Stance::Id::SHOOT1 },
                             { Stance::Id::SHOOT2 },
                             { Stance::Id::STABO1,
                               Stance::Id::STABO2,
                               Stance::Id::SWINGT1,
                               Stance::Id::SWINGT2,
                               Stance::Id::SWINGT3 },
                             { Stance::Id::SWINGO1, Stance::Id::SWINGO2 },
                             { Stance::Id::SWINGO1, Stance::Id::SWINGO2 },
                             { Stance::Id::NONE },
                             { Stance::Id::SHOT } } };

    if (attack <= Attack::NONE || attack >= Attack::NUM_ATTACKS) {
        return Stance::Id::STAND1;
    }

    const auto &stances =
        degenerate ? degen_stances[attack] : attack_stances[attack];

    if (stances.empty()) {
        return Stance::Id::STAND1;
    }

    size_t index = randomizer_.next_int(stances.size());

    return stances[index];
}

uint16_t CharLook::get_delay(Stance::Id st, uint8_t fr) const {
    return draw_info_.get_delay(st, fr);
}

uint8_t CharLook::getnextframe(Stance::Id st, uint8_t fr) const {
    return draw_info_.next_frame(st, fr);
}

void CharLook::set_expression(Expression::Id newexpression) {
    if (expression_ != newexpression) {
        expression_.set(newexpression);
        exp_frame_.set(0);

        exp_elapsed_ = 0;
    }
}

void CharLook::set_action(const std::string &acstr) {
    if (acstr == actionstr_ || acstr.empty()) {
        return;
    }

    if (Stance::Id ac_stance = Stance::by_string(acstr)) {
        set_stance(ac_stance);
    } else {
        action_ = draw_info_.get_action(acstr, 0);

        if (action_) {
            actframe_ = 0;
            st_elapsed_ = 0;
            actionstr_ = acstr;

            stance_.set(action_->get_stance());
            st_frame_.set(action_->get_frame());
        }
    }
}

void CharLook::set_direction(bool f) {
    flip_ = f;
}

void CharLook::set_alerted(int64_t millis) {
    alerted_.set_for(millis);
}

bool CharLook::get_alerted() const {
    return (bool)alerted_;
}

bool CharLook::is_twohanded(Stance::Id st) const {
    switch (st) {
        case Stance::Id::STAND1:
        case Stance::Id::WALK1: return false;
        case Stance::Id::STAND2:
        case Stance::Id::WALK2: return true;
        default: return equips_.is_twohanded();
    }
}

uint16_t CharLook::get_attackdelay(size_t no, uint8_t first_frame) const {
    if (action_) {
        return draw_info_.get_attack_delay(actionstr_, no);
    }
    uint16_t delay = 0;

    for (int frame = 0; frame < first_frame; frame++) {
        delay += get_delay(stance_.get(), frame);
    }

    return delay;
}

uint8_t CharLook::get_frame() const {
    return st_frame_.get();
}

Stance::Id CharLook::get_stance() const {
    return stance_.get();
}

const Body *CharLook::get_body() const {
    return body_;
}

const Hair *CharLook::get_hair() const {
    return hair_;
}

const Face *CharLook::get_face() const {
    return face_;
}

const CharEquips &CharLook::get_equips() const {
    return equips_;
}

void CharLook::init() {
    draw_info_.init();
}

BodyDrawInfo CharLook::draw_info_;
std::unordered_map<int32_t, Hair> CharLook::hair_styles_;
std::unordered_map<int32_t, Face> CharLook::face_types_;
std::unordered_map<int32_t, Body> CharLook::body_types_;
}  // namespace ms