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
#include "Npc.h"

#include <nlnx/nx.hpp>

namespace ms {
Npc::Npc(int32_t id,
         int32_t o,
         bool fl,
         uint16_t f,
         bool cnt,
         Point<int16_t> position) :
    MapObject(o) {
    std::string strid = std::to_string(id);
    strid.insert(0, 7 - strid.size(), '0');
    strid.append(".img");

    nl::node src = nl::nx::npc[strid];
    nl::node strsrc = nl::nx::string["Npc.img"][std::to_string(id)];

    std::string link = src["info"]["link"];

    if (!link.empty()) {
        link.append(".img");
        src = nl::nx::npc[link];
    }

    nl::node info = src["info"];

    hide_name_ = info["hideName"].get_bool();
    mouse_only_ = info["talkMouseOnly"].get_bool();
    scripted_ = info["script"].size() > 0 || info["shop"].get_bool();

    for (int i = 0; strsrc['d' + std::to_string(i)]; ++i) {
        std::string key = 'd' + std::to_string(i);
        dialogue_[key].push_back(strsrc[key].get_string());
    }

    for (const auto &npcnode : src) {
        std::string state = npcnode.name();

        if (state != "info") {
            animations_[state] = npcnode;
            states_.push_back(state);
        }

        for (const auto &speaknode : npcnode["speak"]) {
            lines_[state].push_back(strsrc[speaknode.get_string()]);
        }
    }

    name_ = std::string(strsrc["name"]);
    func_ = std::string(strsrc["func"]);

    name_label_ = Text(Text::Font::A13B,
                       Text::Alignment::CENTER,
                       Color::Name::YELLOW,
                       Text::Background::NAMETAG,
                       name_);
    func_label_ = Text(Text::Font::A13B,
                       Text::Alignment::CENTER,
                       Color::Name::YELLOW,
                       Text::Background::NAMETAG,
                       func_);

    npc_id_ = id;
    flip_ = !fl;
    control_ = cnt;
    stance_ = "stand";

    phobj_.fhid = f;
    set_position(position);
}

void Npc::draw(double viewx, double viewy, float alpha) const {
    Point<int16_t> absp = phobj_.get_absolute(viewx, viewy, alpha);

    if (animations_.count(stance_)) {
        animations_.at(stance_).draw(DrawArgument(absp, flip_), alpha);
    }

    if (!hide_name_) {
        // If ever changing code for namelabel confirm placements with map 10000
        name_label_.draw(absp + Point<int16_t>(0, -4));
        func_label_.draw(absp + Point<int16_t>(0, 18));
    }
}

int8_t Npc::update(const Physics &physics) {
    if (!active_) {
        return phobj_.fhlayer;
    }

    physics.move_object(phobj_);

    if (animations_.count(stance_)) {
        bool aniend = animations_.at(stance_).update();

        if (aniend && !states_.empty()) {
            size_t next_stance = random_.next_int(states_.size());
            std::string new_stance = states_[next_stance];
            set_stance(new_stance);
        }
    }

    return phobj_.fhlayer;
}

void Npc::set_stance(const std::string &st) {
    if (stance_ != st) {
        stance_ = st;

        auto iter = animations_.find(stance_);

        if (iter == animations_.end()) {
            return;
        }

        iter->second.reset();
    }
}

bool Npc::isscripted() const {
    return scripted_;
}

bool Npc::inrange(Point<int16_t> cursorpos, Point<int16_t> viewpos) const {
    if (!active_) {
        return false;
    }

    Point<int16_t> absp = get_position() + viewpos;

    Point<int16_t> dim = animations_.count(stance_)
                             ? animations_.at(stance_).get_dimensions()
                             : Point<int16_t>();

    return Rectangle<int16_t>(absp.x() - dim.x() / 2,
                              absp.x() + dim.x() / 2,
                              absp.y() - dim.y(),
                              absp.y())
        .contains(cursorpos);
}

std::string Npc::get_name() {
    return name_;
}

std::string Npc::get_func() {
    return func_;
}
}  // namespace ms