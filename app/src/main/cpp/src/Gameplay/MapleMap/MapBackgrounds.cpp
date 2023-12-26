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
#include "MapBackgrounds.h"

#include <nlnx/nx.hpp>

#include "../../Graphics/GraphicsGL.h"

namespace ms {
Background::Background(const nl::node &src) {
    VWIDTH = Constants::Constants::get().get_viewwidth();
    VHEIGHT = Constants::Constants::get().get_viewheight();
    WOFFSET = VWIDTH / 2;
    HOFFSET = VHEIGHT / 2;

    nl::node backsrc = nl::nx::map["Back"];

    animated_ = src["ani"].get_bool();
    animation_ =
        backsrc[src["bS"] + ".img"][animated_ ? "ani" : "back"][src["no"]];
    opacity_ = src["a"];
    flipped_ = src["f"].get_bool();
    cx_ = src["cx"];
    cy_ = src["cy"];
    rx_ = src["rx"];
    ry_ = src["ry"];

    move_obj_.set_x(src["x"]);
    move_obj_.set_y(src["y"]);

    Type type = typebyid(src["type"]);

    settype(type);
}

void Background::settype(Type type) {
    int16_t dim_x = animation_.get_dimensions().x();
    int16_t dim_y = animation_.get_dimensions().y();

    // TODO: Double check for zero. Is this a WZ reading issue?
    if (cx_ == 0) {
        cx_ = (dim_x > 0) ? dim_x : 1;
    }

    if (cy_ == 0) {
        cy_ = (dim_y > 0) ? dim_y : 1;
    }

    htile_ = 1;
    vtile_ = 1;

    switch (type) {
        case Type::HTILED:
        case Type::HMOVEA: htile_ = VWIDTH / cx_ + 3; break;
        case Type::VTILED:
        case Type::VMOVEA: vtile_ = VHEIGHT / cy_ + 3; break;
        case Type::TILED:
        case Type::HMOVEB:
        case Type::VMOVEB:
            htile_ = VWIDTH / cx_ + 3;
            vtile_ = VHEIGHT / cy_ + 3;
            break;
        default:
            break;
    }

    switch (type) {
        case Type::HMOVEA:
        case Type::HMOVEB: move_obj_.hspeed = rx_ / 16; break;
        case Type::VMOVEA:
        case Type::VMOVEB: move_obj_.vspeed = ry_ / 16; break;
        default:
            break;
    }
}

void Background::draw(double viewx, double viewy, float alpha) const {
    double x;

    if (move_obj_.hmobile()) {
        x = move_obj_.get_absolute_x(viewx, alpha);
    } else {
        double shift_x = rx_ * (WOFFSET - viewx) / 100 + WOFFSET;
        x = move_obj_.get_absolute_x(shift_x, alpha);
    }

    double y;

    if (move_obj_.vmobile()) {
        y = move_obj_.get_absolute_y(viewy, alpha);
    } else {
        double shift_y = ry_ * (HOFFSET - viewy) / 100 + HOFFSET;
        y = move_obj_.get_absolute_y(shift_y, alpha);
    }

    if (htile_ > 1) {
        while (x > 0) {
            x -= cx_;
        }

        while (x < -cx_) {
            x += cx_;
        }
    }

    if (vtile_ > 1) {
        while (y > 0) {
            y -= cy_;
        }

        while (y < -cy_) {
            y += cy_;
        }
    }

    int16_t ix = static_cast<int16_t>(std::round(x));
    int16_t iy = static_cast<int16_t>(std::round(y));

    int16_t tw = cx_ * htile_;
    int16_t th = cy_ * vtile_;

    for (int tx = 0; tx < tw; tx += cx_) {
        for (int ty = 0; ty < th; ty += cy_) {
            animation_.draw(DrawArgument(Point<int16_t>(ix + tx, iy + ty),
                                         flipped_,
                                         opacity_ / 255),
                            alpha);
        }
    }
}

void Background::update() {
    move_obj_.move();
    animation_.update();
}

MapBackgrounds::MapBackgrounds(const nl::node &src) {
    int16_t no = 0;
    nl::node back = src[std::to_string(no)];

    while (back.size() > 0) {
        bool front = back["front"].get_bool();

        if (front) {
            foregrounds_.push_back(back);
        } else {
            backgrounds_.push_back(back);
        }

        no++;
        back = src[std::to_string(no)];
    }

    black_ = src["0"]["bS"].get_string().empty();
}

MapBackgrounds::MapBackgrounds() = default;

void MapBackgrounds::drawbackgrounds(double viewx,
                                     double viewy,
                                     float alpha) const {
    if (black_) {
        GraphicsGL::get().draw_screen_fill(0.0f, 0.0f, 0.0f, 1.0f);
    }

    for (const auto &background : backgrounds_) {
        background.draw(viewx, viewy, alpha);
    }
}

void MapBackgrounds::drawforegrounds(double viewx,
                                     double viewy,
                                     float alpha) const {
    for (const auto &foreground : foregrounds_) {
        foreground.draw(viewx, viewy, alpha);
    }
}

void MapBackgrounds::update() {
    for (auto &background : backgrounds_) {
        background.update();
    }

    for (auto &foreground : foregrounds_) {
        foreground.update();
    }
}
}  // namespace ms