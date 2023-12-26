//	This file is part of MSClient.
//	Copyright (C) 2020 filoper
//
//	MSClient is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Affero General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	MSClient is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Affero General Public License for more details.
//
//	You should have received a copy of the GNU Affero General Public License
//	along with MSClient.  If not, see <https://www.gnu.org/licenses/>.
#include "UINotification.h"

#include <nlnx/nx.hpp>

#include "../Components/MapleButton.h"
#include "../Net/Packets/PlayerInteractionPackets.h"

namespace ms {
namespace {
auto fn_player_interaction = []<typename... T>(T && ...args) {
    PlayerInteractionPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UINotification::UINotification(std::string message) :
    UIElement({ 500, 400 }, { 200, 100 }) {
    nl::node src = nl::nx::ui["UIWindow2.img"]["FadeYesNo"];

    sprites_.emplace_back(src["backgrnd"],
                          Point<int16_t>(500, 400) - position_);

    buttons_[Buttons::YES] =
        std::make_unique<MapleButton>(src["BtOK"],
                                      Point<int16_t>(685, 405) - position_);
    buttons_[Buttons::NO] =
        std::make_unique<MapleButton>(src["BtCancel"],
                                      Point<int16_t>(685, 435) - position_);

    sprites_.emplace_back(src["icon2"], Point<int16_t>(500, 415) - position_);

    message_ = Text(Text::Font::A11M,
                    Text::Alignment::CENTER,
                    Color::Name::WHITE,
                    "Trade request from '" + message + '\'',
                    200);
}

void UINotification::draw(float alpha) const {
    UIElement::draw(alpha);
    message_.draw(position_ + Point<int16_t>(80, 0));
}

Cursor::State UINotification::send_cursor(bool pressed,
                                          Point<int16_t> cursorpos) {
    return UIElement::send_cursor(pressed, cursorpos);
}

Button::State UINotification::button_pressed(uint16_t buttonid) {
    deactivate();

    switch (buttonid) {
        case Buttons::YES:
            fn_player_interaction(PlayerInteractionPacket::mode::VISIT);
            break;
        case Buttons::NO:
            fn_player_interaction(PlayerInteractionPacket::mode::DECLINE);
            break;
    }

    return Button::State::PRESSED;
}
}  // namespace ms