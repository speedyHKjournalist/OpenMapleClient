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

#include <array>
#include <unordered_map>
#include <vector>

#include "../../Template/Point.h"
#include "Stance.h"

namespace ms {
// A frame of animation for a skill or similar 'meta-stance'
// This simply redirects to a different stance and frame to use
class BodyAction {
public:
    BodyAction(const nl::node &src) {
        stance_ = Stance::by_string(src["action"]);
        frame_ = src["frame"];
        move_ = src["move"];

        int16_t sgndelay = src["delay"];

        if (sgndelay == 0) {
            sgndelay = 100;
        }

        if (sgndelay > 0) {
            delay_ = sgndelay;
            attack_frame_ = true;
        } else if (sgndelay < 0) {
            delay_ = -sgndelay;
            attack_frame_ = false;
        }
    }

    BodyAction() = default;

    bool is_attack_frame() const { return attack_frame_; }

    uint8_t get_frame() const { return frame_; }

    uint16_t get_delay() const { return delay_; }

    Point<int16_t> get_move() const { return move_; }

    Stance::Id get_stance() const { return stance_; }

private:
    Stance::Id stance_;
    uint8_t frame_;
    uint16_t delay_;
    Point<int16_t> move_;
    bool attack_frame_;
};

class BodyDrawInfo {
public:
    void init();

    Point<int16_t> get_body_position(Stance::Id stance, uint8_t frame) const;

    Point<int16_t> get_arm_position(Stance::Id stance, uint8_t frame) const;

    Point<int16_t> get_hand_position(Stance::Id stance, uint8_t frame) const;

    Point<int16_t> get_head_position(Stance::Id stance, uint8_t frame) const;

    Point<int16_t> get_hair_pos(Stance::Id stance, uint8_t frame) const;

    Point<int16_t> get_face_pos(Stance::Id stance, uint8_t frame) const;

    uint8_t next_frame(Stance::Id stance, uint8_t frame) const;

    uint16_t get_delay(Stance::Id stance, uint8_t frame) const;

    uint16_t get_attack_delay(const std::string &action, size_t no) const;

    uint8_t next_action_frame(const std::string &action, uint8_t frame) const;

    const BodyAction *get_action(const std::string &action,
                                 uint8_t frame) const;

private:
    using MapPoint = std::unordered_map<uint8_t, Point<int16_t>>;

    std::array<MapPoint, Stance::Id::LENGTH> body_positions_;
    std::array<MapPoint, Stance::Id::LENGTH> arm_positions_;
    std::array<MapPoint, Stance::Id::LENGTH> hand_positions_;
    std::array<MapPoint, Stance::Id::LENGTH> head_positions_;
    std::array<MapPoint, Stance::Id::LENGTH> hair_positions_;
    std::array<MapPoint, Stance::Id::LENGTH> face_positions_;

    std::array<std::unordered_map<uint8_t, uint16_t>, Stance::Id::LENGTH>
        stance_delays_;

    std::unordered_map<std::string, std::unordered_map<uint8_t, BodyAction>>
        body_actions_;
    std::unordered_map<std::string, std::vector<uint16_t>> attack_delays_;
};
}  // namespace ms
