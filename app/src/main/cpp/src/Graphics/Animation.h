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
#pragma once

#include <vector>

#include "../Template/Interpolated.h"
#include "../Template/Rectangle.h"
#include "Texture.h"

namespace ms {
// A single frame within an animation.
class Frame {
public:
    Frame(const nl::node &src);

    Frame();

    void draw(const DrawArgument &args) const;

    uint8_t start_opacity() const;

    uint16_t start_scale() const;

    uint16_t get_delay() const;

    Point<int16_t> get_origin() const;

    Point<int16_t> get_dimensions() const;

    Point<int16_t> get_head() const;

    Rectangle<int16_t> get_bounds() const;

    float opcstep(uint16_t timestep) const;

    float scalestep(uint16_t timestep) const;

private:
    Texture texture_;
    uint16_t delay_;
    std::pair<uint8_t, uint8_t> opacities_;
    std::pair<int16_t, int16_t> scales_;
    Rectangle<int16_t> bounds_;
    Point<int16_t> head_;
};

// Class which consists of multiple textures to make an Animation.
class Animation {
public:
    Animation(const nl::node &source);

    Animation();

    bool update();

    bool update(uint16_t timestep);

    void reset();

    void stop();

    void draw(const DrawArgument &arguments, float alpha) const;

    uint16_t get_delay(int16_t frame) const;

    uint16_t getdelayuntil(int16_t frame) const;

    Point<int16_t> get_origin() const;

    Point<int16_t> get_dimensions() const;

    Point<int16_t> get_head() const;

    Rectangle<int16_t> get_bounds() const;

private:
    const Frame &get_frame() const;

    std::vector<Frame> frames_;
    bool animated_;
    bool zigzag_;

    Nominal<int16_t> frame_;
    Linear<float> opacity_;
    Linear<float> xyscale_;

    uint16_t delay_;
    int16_t frame_step_;
    float opc_step_;
};
}  // namespace ms