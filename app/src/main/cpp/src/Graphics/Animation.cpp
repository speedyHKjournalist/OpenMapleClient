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
#include "Animation.h"

#include <set>

#include "../Constants.h"
#include "StringHandling.h"

namespace ms {
Frame::Frame(const nl::node &src) {
    texture_ = src;
    bounds_ = src;
    head_ = src["head"];
    delay_ = src["delay"];

    if (delay_ == 0) {
        delay_ = 100;
    }

    bool hasa0 = src["a0"].data_type() == nl::node::type::integer;
    bool hasa1 = src["a1"].data_type() == nl::node::type::integer;

    if (hasa0 && hasa1) {
        opacities_ = { src["a0"], src["a1"] };
    } else if (hasa0) {
        uint8_t a0 = src["a0"];
        opacities_ = { a0, 255 - a0 };
    } else if (hasa1) {
        uint8_t a1 = src["a1"];
        opacities_ = { 255 - a1, a1 };
    } else {
        opacities_ = { 255, 255 };
    }

    bool hasz0 = src["z0"].data_type() == nl::node::type::integer;
    bool hasz1 = src["z1"].data_type() == nl::node::type::integer;

    if (hasz0 && hasz1) {
        scales_ = { src["z0"], src["z1"] };
    } else if (hasz0) {
        scales_ = { src["z0"], 0 };
    } else if (hasz1) {
        scales_ = { 100, src["z1"] };
    } else {
        scales_ = { 100, 100 };
    }
}

Frame::Frame() : delay_(0), opacities_({ 0, 0 }), scales_({ 0, 0 }) {}

void Frame::draw(const DrawArgument &args) const {
    texture_.draw(args);
}

uint8_t Frame::start_opacity() const {
    return opacities_.first;
}

uint16_t Frame::start_scale() const {
    return scales_.first;
}

uint16_t Frame::get_delay() const {
    return delay_;
}

Point<int16_t> Frame::get_origin() const {
    return texture_.get_origin();
}

Point<int16_t> Frame::get_dimensions() const {
    return texture_.get_dimensions();
}

Point<int16_t> Frame::get_head() const {
    return head_;
}

Rectangle<int16_t> Frame::get_bounds() const {
    return bounds_;
}

float Frame::opcstep(uint16_t timestep) const {
    return timestep * static_cast<float>(opacities_.second - opacities_.first)
           / delay_;
}

float Frame::scalestep(uint16_t timestep) const {
    return timestep * static_cast<float>(scales_.second - scales_.first)
           / delay_;
}

Animation::Animation(const nl::node &src) {
    bool istexture = src.data_type() == nl::node::type::bitmap;

    if (istexture) {
        frames_.emplace_back(src);
    } else {
        std::set<int16_t> frameids;

        for (const auto &sub : src) {
            if (sub.data_type() == nl::node::type::bitmap) {
                auto fid =
                    string_conversion::or_default<int16_t>(sub.name(), -1);

                if (fid >= 0) {
                    frameids.insert(fid);
                }
            }
        }

        for (const auto &fid : frameids) {
            auto sub = src[std::to_string(fid)];
            frames_.emplace_back(sub);
        }

        if (frames_.empty()) {
            frames_.emplace_back();
        }
    }

    animated_ = frames_.size() > 1;
    zigzag_ = src["zigzag"].get_bool();

    reset();
}

Animation::Animation() : animated_(false), zigzag_(false) {
    frames_.emplace_back();
    reset();
}

void Animation::reset() {
    frame_.set(0);
    opacity_.set(frames_[0].start_opacity());
    xyscale_.set(frames_[0].start_scale());
    delay_ = frames_[0].get_delay();
    frame_step_ = 1;
}

void Animation::stop() {
    auto lastframe = static_cast<int16_t>(frames_.size() - 1);
    frame_.set(lastframe);
    frame_step_ = -1;
}

void Animation::draw(const DrawArgument &args, float alpha) const {
    int16_t interframe = frame_.get(alpha);
    float interopc = opacity_.get(alpha) / 255;
    float interscale = xyscale_.get(alpha) / 100;

    bool modifyopc = interopc != 1.0f;
    bool modifyscale = interscale != 1.0f;

    if (modifyopc || modifyscale) {
        frames_[interframe].draw(
            args + DrawArgument(interscale, interscale, interopc));
    } else {
        frames_[interframe].draw(args);
    }
}

bool Animation::update() {
    return update(Constants::TIMESTEP);
}

bool Animation::update(uint16_t timestep) {
    const Frame &framedata = get_frame();

    opacity_ += framedata.opcstep(timestep);

    if (opacity_.last() < 0.0f) {
        opacity_.set(0.0f);
    } else if (opacity_.last() > 255.0f) {
        opacity_.set(255.0f);
    }

    xyscale_ += framedata.scalestep(timestep);

    if (xyscale_.last() < 0.0f) {
        opacity_.set(0.0f);
    }

    if (timestep >= delay_) {
        int16_t lastframe = static_cast<int16_t>(frames_.size() - 1);
        int16_t nextframe = 0;
        bool ended = false;

        if (zigzag_ && lastframe > 0) {
            if (frame_step_ == 1 && frame_ == lastframe) {
                frame_step_ = -frame_step_;
                ended = false;
            } else if (frame_step_ == -1 && frame_ == 0) {
                frame_step_ = -frame_step_;
                ended = true;
            }

            nextframe = frame_ + frame_step_;
        } else {
            if (frame_ == lastframe) {
                nextframe = 0;
                ended = true;
            } else {
                nextframe = frame_ + 1;
                ended = false;
            }
        }

        uint16_t delta = timestep - delay_;
        float threshold = static_cast<float>(delta) / timestep;
        frame_.next(nextframe, threshold);

        delay_ = frames_[nextframe].get_delay();

        if (delay_ >= delta) {
            delay_ -= delta;
        }

        opacity_.set(frames_[nextframe].start_opacity());
        xyscale_.set(frames_[nextframe].start_scale());

        return ended;
    }

    frame_.normalize();
    delay_ -= timestep;

    return false;
}

uint16_t Animation::get_delay(int16_t frame_id) const {
    return frame_id < frames_.size() ? frames_[frame_id].get_delay() : 0;
}

uint16_t Animation::getdelayuntil(int16_t frame_id) const {
    uint16_t total = 0;

    for (int i = 0; i < frame_id; i++) {
        if (i >= frames_.size()) {
            break;
        }

        total += frames_[frame_id].get_delay();
    }

    return total;
}

Point<int16_t> Animation::get_origin() const {
    return get_frame().get_origin();
}

Point<int16_t> Animation::get_dimensions() const {
    return get_frame().get_dimensions();
}

Point<int16_t> Animation::get_head() const {
    return get_frame().get_head();
}

Rectangle<int16_t> Animation::get_bounds() const {
    return get_frame().get_bounds();
}

const Frame &Animation::get_frame() const {
    return frames_[frame_.get()];
}
}  // namespace ms