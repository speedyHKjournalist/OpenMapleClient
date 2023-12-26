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
#include "UIStatusMessenger.h"

#include "../Constants.h"

namespace ms {
StatusInfo::StatusInfo(const std::string &str, Color::Name color) {
    text_ = Text(Text::Font::A12M, Text::Alignment::RIGHT, color, str);
    shadow_ =
        Text(Text::Font::A12M, Text::Alignment::RIGHT, Color::Name::BLACK, str);

    opacity_.set(1.0f);
}

void StatusInfo::draw(Point<int16_t> position, float alpha) const {
    float interopc = opacity_.get(alpha);

    shadow_.draw(DrawArgument(position + Point<int16_t>(1, 1), interopc));
    text_.draw(DrawArgument(position, interopc));
}

bool StatusInfo::update() {
    constexpr float FADE_STEP = Constants::TIMESTEP * 1.0f / FADE_DURATION_;
    opacity_ -= FADE_STEP;

    return opacity_.last() < FADE_STEP;
}

UIStatusMessenger::UIStatusMessenger() {
    int16_t height = Constants::Constants::get().get_viewheight();
    int16_t width = Constants::Constants::get().get_viewwidth();

    update_screen(width, height);
}

void UIStatusMessenger::draw(float inter) const {
    Point<int16_t> infopos = Point<int16_t>(position_.x(), position_.y());

    for (const StatusInfo &info : status_infos_) {
        info.draw(infopos, inter);
        infopos.shift_y(-14);
    }
}

void UIStatusMessenger::update() {
    for (StatusInfo &info : status_infos_) {
        info.update();
    }
}

void UIStatusMessenger::update_screen(int16_t new_width, int16_t new_height) {
    int16_t y_adj = (new_width > 800) ? 37 : 0;

    position_ = Point<int16_t>(new_width - 6, new_height - 145 + y_adj);
}

UIElement::Type UIStatusMessenger::get_type() const {
    return TYPE;
}

void UIStatusMessenger::show_status(Color::Name color,
                                    const std::string &message) {
    status_infos_.push_front(StatusInfo(message, color));

    if (status_infos_.size() > MAX_MESSAGES_) {
        status_infos_.pop_back();
    }
}
}  // namespace ms