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
#include "Char.h"

#include <nlnx/nx.hpp>

#include "../Data/WeaponData.h"

namespace ms {
Char::Char(int32_t o, const CharLook &lk, const std::string &name) :
    MapObject(o),
    look_(lk),
    look_preview_(lk),
    name_label_(Text(Text::Font::A13M,
                     Text::Alignment::CENTER,
                     Color::Name::WHITE,
                     Text::Background::NAMETAG,
                     name)) {}

void Char::draw(double viewx, double viewy, float alpha) const {
    Point<int16_t> absp = phobj_.get_absolute(viewx, viewy, alpha);

    effects_.drawbelow(absp, alpha);
    recurring_effects_.drawbelow(absp, alpha);

    Color color;

    if (invincible_) {
        float phi = invincible_.alpha() * 30;
        float rgb = 0.9f - 0.5f * std::abs(std::sin(phi));

        color = Color(rgb, rgb, rgb, 1.0f);
    } else {
        color = Color::Code::CWHITE;
    }

    look_.draw(DrawArgument(absp, color), alpha);

    after_image_.draw(look_.get_frame(),
                      DrawArgument(absp, facing_right_),
                      alpha);

    if (iron_body_) {
        float ibalpha = iron_body_.alpha();
        float scale = 1.0f + ibalpha;
        float opacity = 1.0f - ibalpha;

        look_.draw(DrawArgument(absp, scale, scale, opacity), alpha);
    }

    for (const auto &pet : pets_) {
        if (pet.get_itemid()) {
            pet.draw(viewx, viewy, alpha);
        }
    }

    // If ever changing code for namelabel confirm placements with map 10000
    name_label_.draw(absp + Point<int16_t>(0, -4));
    chat_balloon_.draw(absp - Point<int16_t>(0, 85));

    effects_.drawabove(absp, alpha);
    recurring_effects_.drawabove(absp, alpha);

    for (const auto &number : damage_numbers_) {
        number.draw(viewx, viewy, alpha);
    }
}

void Char::draw_preview(Point<int16_t> position, float alpha) const {
    look_preview_.draw(position,
                       false,
                       Stance::Id::STAND1,
                       Expression::Id::DEFAULT);
}

bool Char::update(const Physics &physics, float speed) {
    damage_numbers_.remove_if(
        [](DamageNumber &number) { return number.update(); });

    effects_.update();
    recurring_effects_.update();
    chat_balloon_.update();
    invincible_.update();
    iron_body_.update();

    for (auto &pet : pets_) {
        if (pet.get_itemid()) {
            switch (state_) {
                case State::LADDER:
                case State::ROPE: pet.set_stance(PetLook::Stance::HANG); break;
                case State::SWIM: pet.set_stance(PetLook::Stance::FLY); break;
                default:
                    if (pet.get_stance() == PetLook::Stance::HANG
                        || pet.get_stance() == PetLook::Stance::FLY) {
                        pet.set_stance(PetLook::Stance::STAND);
                    }

                    break;
            }

            pet.update(physics, get_position());
        }
    }

    uint16_t stancespeed = 0;

    if (speed >= 1.0f / Constants::TIMESTEP) {
        stancespeed = static_cast<uint16_t>(Constants::TIMESTEP * speed);
    }

    after_image_.update(look_.get_frame(), stancespeed);

    return look_.update(stancespeed);
}

float Char::get_stancespeed() const {
    if (attacking_) {
        return get_real_attackspeed();
    }

    switch (state_) {
        case State::WALK: return static_cast<float>(std::abs(phobj_.hspeed));
        case State::LADDER:
        case State::ROPE: return static_cast<float>(std::abs(phobj_.vspeed));
        default: return 1.0f;
    }
}

float Char::get_real_attackspeed() const {
    int8_t speed = get_integer_attackspeed();

    return 1.7f - static_cast<float>(speed) / 10;
}

uint16_t Char::get_attackdelay(size_t no) const {
    uint8_t first_frame = after_image_.get_first_frame();
    uint16_t delay = look_.get_attackdelay(no, first_frame);
    float fspeed = get_real_attackspeed();

    return static_cast<uint16_t>(delay / fspeed);
}

int8_t Char::update(const Physics &physics) {
    update(physics, 1.0f);

    return get_layer();
}

int8_t Char::get_layer() const {
    return is_climbing() ? 7 : phobj_.fhlayer;
}

void Char::show_attack_effect(const Animation &toshow, int8_t z) {
    float attackspeed = get_real_attackspeed();

    effects_.add(toshow, DrawArgument(facing_right_), z, attackspeed);
}

void Char::show_effect_id(CharEffect::Id toshow) {
    effects_.add(char_effects_[toshow]);
}

void Char::add_recurring_effect(int16_t effect_id,
                                const Animation &animation,
                                int8_t z) {
    recurring_effects_.add(animation, DrawArgument(0, -60));
}

void Char::remove_recurring_effect() {
    recurring_effects_ = RecurringEffect();
}

void Char::show_iron_body() {
    iron_body_.set_for(500);
}

void Char::show_damage(int32_t damage) {
    int16_t start_y = phobj_.get_y() - 60;
    int16_t x = phobj_.get_x() - 10;

    damage_numbers_.emplace_back(DamageNumber::Type::TOPLAYER,
                                 damage,
                                 start_y,
                                 x);

    look_.set_alerted(5000);
    invincible_.set_for(2000);
}

void Char::speak(const std::string &line) {
    chat_balloon_.change_text(line);
}

void Char::change_look(MapleStat::Id stat, int32_t id) {
    switch (stat) {
        case MapleStat::Id::SKIN: look_.set_body(id); break;
        case MapleStat::Id::FACE: look_.set_face(id); break;
        case MapleStat::Id::HAIR: look_.set_hair(id); break;
        case MapleStat::Id::LEVEL: break;
        case MapleStat::Id::JOB: break;
        case MapleStat::Id::STR: break;
        case MapleStat::Id::DEX: break;
        case MapleStat::Id::INT: break;
        case MapleStat::Id::LUK: break;
        case MapleStat::Id::HP: break;
        case MapleStat::Id::MAXHP: break;
        case MapleStat::Id::MP: break;
        case MapleStat::Id::MAXMP: break;
        case MapleStat::Id::AP: break;
        case MapleStat::Id::SP: break;
        case MapleStat::Id::EXP: break;
        case MapleStat::Id::FAME: break;
        case MapleStat::Id::MESO: break;
        case MapleStat::Id::PET: break;
        case MapleStat::Id::GACHAEXP: break;
        case MapleStat::Id::LENGTH: break;
    }
}

void Char::set_state(uint8_t statebyte) {
    if (statebyte % 2 == 1) {
        set_direction(false);

        statebyte -= 1;
    } else {
        set_direction(true);
    }

    Char::State newstate = by_value(statebyte);
    set_state(newstate);
}

void Char::set_expression(int32_t expid) {
    Expression::Id expression = Expression::byaction(expid);
    look_.set_expression(expression);
}

void Char::attack(const std::string &action) {
    look_.set_action(action);

    attacking_ = true;
    look_.set_alerted(5000);
}

void Char::attack(Stance::Id stance) {
    look_.attack(stance);

    attacking_ = true;
    look_.set_alerted(5000);
}

void Char::attack(bool degenerate) {
    look_.attack(degenerate);

    attacking_ = true;
    look_.set_alerted(5000);
}

void Char::set_afterimage(int32_t skill_id) {
    int32_t weapon_id = look_.get_equips().get_weapon();

    if (weapon_id <= 0) {
        return;
    }

    const WeaponData &weapon = WeaponData::get(weapon_id);

    std::string stance_name = Stance::names[look_.get_stance()];
    int16_t weapon_level =
        weapon.get_equipdata().get_reqstat(MapleStat::Id::LEVEL);
    const std::string &ai_name = weapon.get_afterimage();

    after_image_ = Afterimage(skill_id, ai_name, stance_name, weapon_level);
}

const Afterimage &Char::get_afterimage() const {
    return after_image_;
}

void Char::set_direction(bool f) {
    facing_right_ = f;
    look_.set_direction(f);
}

void Char::set_state(State st) {
    state_ = st;

    Stance::Id stance = Stance::by_state(state_);
    look_.set_stance(stance);
}

void Char::add_pet(uint8_t index,
                   int32_t iid,
                   const std::string &name,
                   int32_t uniqueid,
                   Point<int16_t> pos,
                   uint8_t stance,
                   int32_t fhid) {
    if (index >= pets_.size()) {
        return;
    }

    pets_[index] = PetLook(iid, name, uniqueid, pos, stance, fhid);
}

void Char::remove_pet(uint8_t index, bool hunger) {
    if (index >= pets_.size()) {
        return;
    }

    pets_[index] = PetLook();

    if (hunger) {
        // TODO: Empty
    }
}

bool Char::is_invincible() const {
    return invincible_ == true;
}

bool Char::is_sitting() const {
    return state_ == State::SIT;
}

bool Char::is_climbing() const {
    return state_ == State::LADDER || state_ == State::ROPE;
}

bool Char::is_twohanded() const {
    return look_.get_equips().is_twohanded();
}

Weapon::Type Char::get_weapontype() const {
    int32_t weapon_id = look_.get_equips().get_weapon();

    if (weapon_id <= 0) {
        return Weapon::Type::NONE;
    }

    return WeaponData::get(weapon_id).get_type();
}

bool Char::getflip() const {
    return facing_right_;
}

std::string Char::get_name() const {
    return name_label_.get_text();
}

CharLook &Char::get_look() {
    return look_;
}

const CharLook &Char::get_look() const {
    return look_;
}

PhysicsObject &Char::get_phobj() {
    return phobj_;
}

void Char::init() {
    CharLook::init();

    nl::node src = nl::nx::effect["BasicEff.img"];

    for (auto iter : CharEffect::PATHS) {
        char_effects_.emplace(iter.first, src.resolve(iter.second));
    }
}

EnumMap<CharEffect::Id, Animation> Char::char_effects_;
}  // namespace ms