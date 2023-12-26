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
#include "UIChat.h"

#include <nlnx/nx.hpp>

#include "../Components/MapleButton.h"
#include "../UI.h"

namespace ms {
UIChat::UIChat() : UIDragElement<PosMAPLECHAT>() {
    show_weekly_ = Configuration::get().get_show_weekly();

    nl::node socialChatEnter = nl::nx::ui["UIWindow2.img"]["socialChatEnter"];

    nl::node backgrnd = socialChatEnter["backgrnd"];
    nl::node backgrnd4 = socialChatEnter["backgrnd4"];
    nl::node backgrnd5 = socialChatEnter["backgrnd5"];

    rank_shift_ = Point<int16_t>(86, 130);
    name_shift_ = Point<int16_t>(50, 5);

    origin_left_ = Texture(backgrnd4).get_origin();
    origin_right_ = Texture(backgrnd5).get_origin();

    origin_left_ =
        Point<int16_t>(std::abs(origin_left_.x()), std::abs(origin_left_.y()));
    origin_right_ = Point<int16_t>(std::abs(origin_right_.x()),
                                   std::abs(origin_right_.y()));

    sprites_.emplace_back(socialChatEnter["ribbon"]);
    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(socialChatEnter["backgrnd2"]);
    sprites_.emplace_back(socialChatEnter["backgrnd3"]);
    sprites_.emplace_back(backgrnd4);
    sprites_.emplace_back(backgrnd5);

    buttons_[Buttons::CLOSE] =
        std::make_unique<MapleButton>(socialChatEnter["btX"]);
    buttons_[Buttons::CHAT_DUO] =
        std::make_unique<MapleButton>(socialChatEnter["duoChat"]);
    buttons_[Buttons::CHAT_FRIEND] =
        std::make_unique<MapleButton>(socialChatEnter["groupChatFrd"]);
    buttons_[Buttons::CHAT_RANDOM] =
        std::make_unique<MapleButton>(socialChatEnter["groupChatRnd"]);

    charset_ = Charset(socialChatEnter["number"], Charset::Alignment::RIGHT);

    name_left_ =
        Text(Text::Font::A12B, Text::Alignment::CENTER, Color::Name::WHITE);
    name_right_ =
        Text(Text::Font::A12B, Text::Alignment::CENTER, Color::Name::WHITE);

    dimension_ = Texture(backgrnd).get_dimensions();

    if (show_weekly_) {
        UI::get().emplace<UIRank>();
    }
}

void UIChat::draw(float inter) const {
    UIElement::draw(inter);

    charset_.draw("0", position_ + origin_left_ + rank_shift_);
    charset_.draw("0", position_ + origin_right_ + rank_shift_);

    name_left_.draw(position_ + origin_left_ + name_shift_);
    name_right_.draw(position_ + origin_right_ + name_shift_);
}

void UIChat::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        close();
    }
}

UIElement::Type UIChat::get_type() const {
    return TYPE;
}

Button::State UIChat::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::CLOSE: close(); break;
        case Buttons::CHAT_DUO: break;
        case Buttons::CHAT_FRIEND: break;
        case Buttons::CHAT_RANDOM: break;
        default: break;
    }

    return Button::State::NORMAL;
}

void UIChat::close() {
    deactivate();
}

UIRank::UIRank() : UIDragElement<PosMAPLECHAT>() {
    Configuration::get().set_show_weekly(false);

    nl::node socialRank = nl::nx::ui["UIWindow2.img"]["socialRank"];

    nl::node backgrnd = socialRank["backgrnd"];
    nl::node backgrnd4 = socialRank["backgrnd4"];
    nl::node backgrnd5 = socialRank["backgrnd5"];

    rank_shift_ = Point<int16_t>(86, 130);
    name_shift_ = Point<int16_t>(52, 4);

    origin_left_ = Texture(backgrnd4).get_origin();
    origin_right_ = Texture(backgrnd5).get_origin();

    origin_left_ = Point<int16_t>(std::abs(origin_left_.x()) - 1,
                                  std::abs(origin_left_.y()));
    origin_right_ = Point<int16_t>(std::abs(origin_right_.x()),
                                   std::abs(origin_right_.y()));

    sprites_.emplace_back(socialRank["ribbon"]);
    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(socialRank["backgrnd2"]);
    sprites_.emplace_back(socialRank["backgrnd3"]);
    sprites_.emplace_back(backgrnd4);
    sprites_.emplace_back(backgrnd5);

    buttons_[Buttons::CLOSE] = std::make_unique<MapleButton>(socialRank["btX"]);

    charset_ = Charset(socialRank["number"], Charset::Alignment::RIGHT);

    name_left_ =
        Text(Text::Font::A12B, Text::Alignment::CENTER, Color::Name::WHITE);
    name_right_ =
        Text(Text::Font::A12B, Text::Alignment::CENTER, Color::Name::WHITE);

    dimension_ = Texture(backgrnd).get_dimensions();
    position_ = position_ + Point<int16_t>(211, 124);
}

void UIRank::draw(float inter) const {
    UIElement::draw(inter);

    charset_.draw("0", position_ + origin_left_ + rank_shift_);
    charset_.draw("0", position_ + origin_right_ + rank_shift_);

    name_left_.draw(position_ + origin_left_ + name_shift_);
    name_right_.draw(position_ + origin_right_ + name_shift_);
}

void UIRank::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        close();
    }
}

UIElement::Type UIRank::get_type() const {
    return TYPE;
}

Button::State UIRank::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::CLOSE: close(); break;
        default: break;
    }

    return Button::State::NORMAL;
}

void UIRank::close() {
    deactivate();
}
}  // namespace ms