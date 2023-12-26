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
#include "UITrade.h"

#include <nlnx/nx.hpp>

#include "../Components/MapleButton.h"
#include "../Net/Packets/PlayerInteractionPackets.h"

namespace ms {
namespace {
auto fn_player_interaction = [](auto&&... args) {
    PlayerInteractionPacket(std::forward<decltype(args)>(args)...).dispatch();
};
}  // namespace

UITrade::UITrade() {
    nl::node src = nl::nx::ui["UIWindow2.img"]["TradingRoom"];

    sprites_.emplace_back(src["backgrnd"], position_);
    sprites_.emplace_back(src["backgrnd2"], position_);
    sprites_.emplace_back(src["backgrnd3"], position_);

    buttons_[Buttons::TRADE] =
        std::make_unique<MapleButton>(src["BtTrade"], -position_);
    buttons_[Buttons::CANCEL] =
        std::make_unique<MapleButton>(src["BtReset"], -position_);

    buttons_[Buttons::COIN] =
        std::make_unique<MapleButton>(src["BtCoin"], position_);
}

void UITrade::draw(float alpha) const {
    UIElement::draw(alpha);
}

Cursor::State UITrade::send_cursor(bool pressed, Point<int16_t> cursorpos) {
    return UIElement::send_cursor(pressed, cursorpos);
}

Button::State UITrade::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::TRADE:
            fn_player_interaction(PlayerInteractionPacket::mode::CONFIRM);
            break;
        case Buttons::CANCEL:
            fn_player_interaction(PlayerInteractionPacket::mode::EXIT);
            deactivate();
            break;
    }

    return Button::State::PRESSED;
}
}  // namespace ms