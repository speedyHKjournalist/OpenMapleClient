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
#include "Foothold.h"

namespace ms {
Foothold::Foothold(const nl::node &src, uint16_t id, uint8_t ly) :
    m_id_(id),
    m_prev_(src["prev"]),
    m_next_(src["next"]),
    m_layer_(ly),
    m_horizontal_(src["x1"], src["x2"]),
    m_vertical_(src["y1"], src["y2"]) {}

Foothold::Foothold() : m_id_(0), m_prev_(0), m_next_(0), m_layer_(0) {}

uint16_t Foothold::id() const {
    return m_id_;
}

uint16_t Foothold::prev() const {
    return m_prev_;
}

uint16_t Foothold::next() const {
    return m_next_;
}

uint8_t Foothold::layer() const {
    return m_layer_;
}

const Range<int16_t> &Foothold::horizontal() const {
    return m_horizontal_;
}

const Range<int16_t> &Foothold::vertical() const {
    return m_vertical_;
}

int16_t Foothold::l() const {
    return m_horizontal_.smaller();
}

int16_t Foothold::r() const {
    return m_horizontal_.greater();
}

int16_t Foothold::t() const {
    return m_vertical_.smaller();
}

int16_t Foothold::b() const {
    return m_vertical_.greater();
}

int16_t Foothold::x1() const {
    return m_horizontal_.first();
}

int16_t Foothold::x2() const {
    return m_horizontal_.second();
}

int16_t Foothold::y1() const {
    return m_vertical_.first();
}

int16_t Foothold::y2() const {
    return m_vertical_.second();
}

bool Foothold::is_wall() const {
    return m_id_ && m_horizontal_.empty();
}

bool Foothold::is_floor() const {
    return m_id_ && m_vertical_.empty();
}

bool Foothold::is_left_edge() const {
    return m_id_ && m_prev_ == 0;
}

bool Foothold::is_right_edge() const {
    return m_id_ && m_next_ == 0;
}

bool Foothold::hcontains(int16_t x) const {
    return m_id_ && m_horizontal_.contains(x);
}

bool Foothold::vcontains(int16_t y) const {
    return m_id_ && m_vertical_.contains(y);
}

bool Foothold::is_blocking(const Range<int16_t> &vertical) const {
    return is_wall() && m_vertical_.overlaps(vertical);
}

int16_t Foothold::hdelta() const {
    return m_horizontal_.delta();
}

int16_t Foothold::vdelta() const {
    return m_vertical_.delta();
}

double Foothold::slope() const {
    return is_wall() ? 0.0f : static_cast<double>(vdelta()) / hdelta();
}

double Foothold::ground_below(double x) const {
    return is_floor() ? y1() : slope() * (x - x1()) + y1();
}
}  // namespace ms