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
#include "DamageNumber.h"

#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>

#include "../Constants.h"

namespace ms {
DamageNumber::DamageNumber(Type t, int32_t damage, int16_t starty, int16_t x) {
    type_ = t;

    if (damage > 0) {
        miss_ = false;

        std::string number = std::to_string(damage);
        first_num_ = number[0];

        if (number.size() > 1) {
            rest_num_ = number.substr(1);
            multiple_ = true;
        } else {
            rest_num_ = "";
            multiple_ = false;
        }

        int16_t total = get_advance(first_num_, true);

        for (size_t i = 0; i < rest_num_.length(); i++) {
            char c = rest_num_[i];
            int16_t advance = 0;

            if (i < rest_num_.length() - 1) {
                char n = rest_num_[i + 1];
                advance = (get_advance(c, false) + get_advance(n, false)) / 2;
            } else {
                advance = get_advance(c, false);
            }

            total += advance;
        }

        shift_ = total / 2;
    } else {
        shift_ = charsets_[type_][true].getw('M') / 2;
        miss_ = true;
    }

    move_obj_.set_x(x);
    move_obj_.set_y(starty);
    move_obj_.vspeed = -0.25;
    opacity_.set(1.5f);
}

DamageNumber::DamageNumber() = default;

void DamageNumber::draw(double viewx, double viewy, float alpha) const {
    Point<int16_t> absolute = move_obj_.get_absolute(viewx, viewy, alpha);
    Point<int16_t> position = absolute - Point<int16_t>(0, shift_);
    float interopc = opacity_.get(alpha);

    if (miss_) {
        charsets_[type_][true].draw('M', { position, interopc });
    } else {
        charsets_[type_][false].draw(first_num_, { position, interopc });

        if (multiple_) {
            int16_t first_advance = get_advance(first_num_, true);
            position.shift_x(first_advance);

            for (size_t i = 0; i < rest_num_.length(); i++) {
                char c = rest_num_[i];
                Point<int16_t> yshift = { 0, static_cast<int16_t>((i % 2) ? -2 : 2) };
                charsets_[type_][true].draw(c, { position + yshift, interopc });

                int16_t advance;

                if (i < rest_num_.length() - 1) {
                    char n = rest_num_[i + 1];
                    int16_t c_advance = get_advance(c, false);
                    int16_t n_advance = get_advance(n, false);
                    advance = (c_advance + n_advance) / 2;
                } else {
                    advance = get_advance(c, false);
                }

                position.shift_x(advance);
            }
        }
    }
}

int16_t DamageNumber::get_advance(char c, bool first) const {
    constexpr size_t LENGTH = 10;

    constexpr int16_t advances[LENGTH] = { 24, 20, 22, 22, 24,
                                           23, 24, 22, 24, 24 };

    size_t index = c - 48;

    if (index < LENGTH) {
        int16_t advance = advances[index];

        switch (type_) {
            case DamageNumber::Type::CRITICAL:
                if (first) {
                    advance += 8;
                } else {
                    advance += 4;
                }

                break;
            default:
                if (first) {
                    advance += 2;
                }

                break;
        }

        return advance;
    }

    return 0;
}

void DamageNumber::set_x(int16_t headx) {
    move_obj_.set_x(headx);
}

bool DamageNumber::update() {
    move_obj_.move();

    constexpr float FADE_STEP = Constants::TIMESTEP * 1.0f / FADE_TIME;
    opacity_ -= FADE_STEP;

    return opacity_.last() <= 0.0f;
}

int16_t DamageNumber::row_height(bool critical) {
    return critical ? 36 : 30;
}

void DamageNumber::init() {
    charsets_[DamageNumber::Type::NORMAL].set(
        false,
        nl::nx::effect["BasicEff.img"]["NoRed1"],
        Charset::Alignment::LEFT);
    charsets_[DamageNumber::Type::NORMAL].set(
        true,
        nl::nx::effect["BasicEff.img"]["NoRed0"],
        Charset::Alignment::LEFT);
    charsets_[DamageNumber::Type::CRITICAL].set(
        false,
        nl::nx::effect["BasicEff.img"]["NoCri1"],
        Charset::Alignment::LEFT);
    charsets_[DamageNumber::Type::CRITICAL].set(
        true,
        nl::nx::effect["BasicEff.img"]["NoCri0"],
        Charset::Alignment::LEFT);
    charsets_[DamageNumber::Type::TOPLAYER].set(
        false,
        nl::nx::effect["BasicEff.img"]["NoViolet1"],
        Charset::Alignment::LEFT);
    charsets_[DamageNumber::Type::TOPLAYER].set(
        true,
        nl::nx::effect["BasicEff.img"]["NoViolet0"],
        Charset::Alignment::LEFT);
}

std::array<BoolPair<Charset>, DamageNumber::NUM_TYPES> DamageNumber::charsets_;
}  // namespace ms