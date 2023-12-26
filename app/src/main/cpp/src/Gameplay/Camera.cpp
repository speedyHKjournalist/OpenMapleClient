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
#include "Camera.h"

#include "../Constants.h"

namespace ms {
Camera::Camera() {
    x_.set(0.0);
    y_.set(0.0 + Y_OFFSET);

    VWIDTH = Constants::Constants::get().get_viewwidth();
    VHEIGHT = Constants::Constants::get().get_viewheight();
}

void Camera::update(Point<int16_t> position) {
    int32_t new_width = Constants::Constants::get().get_viewwidth();
    int32_t new_height = Constants::Constants::get().get_viewheight();

    if (VWIDTH != new_width || VHEIGHT != new_height) {
        VWIDTH = new_width;
        VHEIGHT = new_height;
    }

    double next_x = x_.get();
    double hdelta = VWIDTH / 2 - position.x() - next_x;

    if (std::abs(hdelta) >= 5.0) {
        next_x += hdelta * (12.0 / VWIDTH);
    }

    double next_y = y_.get();
    double vdelta = VHEIGHT / 2 - position.y() + Y_OFFSET - next_y;

    if (std::abs(vdelta) >= 5.0) {
        next_y += vdelta * (12.0 / VHEIGHT);
    }

    if (next_x > hbounds_.first() || hbounds_.length() < VWIDTH) {
        next_x = hbounds_.first();
    } else if (next_x < hbounds_.second() + VWIDTH) {
        next_x = hbounds_.second() + VWIDTH;
    }

    if (next_y > vbounds_.first() || vbounds_.length() < VHEIGHT) {
        next_y = vbounds_.first();
    } else if (next_y < vbounds_.second() + VHEIGHT) {
        next_y = vbounds_.second() + VHEIGHT;
    }

    x_ = next_x;
    y_ = next_y;
}

void Camera::set_position(Point<int16_t> position) {
    int32_t new_width = Constants::Constants::get().get_viewwidth();
    int32_t new_height = Constants::Constants::get().get_viewheight();

    if (VWIDTH != new_width || VHEIGHT != new_height) {
        VWIDTH = new_width;
        VHEIGHT = new_height;
    }

    x_.set(VWIDTH / 2 - position.x());
    y_.set(VHEIGHT / 2 - position.y() + Y_OFFSET);
}

void Camera::set_view(Range<int16_t> mapwalls, Range<int16_t> mapborders) {
    hbounds_ = -mapwalls;
    vbounds_ = -mapborders;
}

Point<int16_t> Camera::position() const {
    auto shortx = static_cast<int16_t>(std::round(x_.get()));
    auto shorty = static_cast<int16_t>(std::round(y_.get()));

    return { shortx, shorty };
}

Point<int16_t> Camera::position(float alpha) const {
    auto interx = static_cast<int16_t>(std::round(x_.get(alpha)));
    auto intery = static_cast<int16_t>(std::round(y_.get(alpha)));

    return { interx, intery };
}

Point<double> Camera::real_position(float alpha) const {
    return { x_.get(alpha), y_.get(alpha) };
}
}  // namespace ms