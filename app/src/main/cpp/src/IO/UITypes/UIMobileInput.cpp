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
#include <nlnx/node.hpp>
#include <nlnx/nx.hpp>

#include "../Data/ItemData.h"
#include "StringHandling.h"
#include "UIMobileInput.h"
#include "UI.h"
#include "TouchButton.h"

namespace ms {
    UIMobileInput::UIMobileInput() {
        touch_buttons_[MobileButtons::ButtonJump] = std::make_unique<TouchButton>(Point<int16_t>(1050, 550),
                                                                          TouchButton::ActionType::Jump, "JUMP");
        touch_buttons_[MobileButtons::ButtonNormal0] = std::make_unique<TouchButton>(Point<int16_t>(1050, 400),
                                                                                  TouchButton::ActionType::Default,GLFMKeyCode0, "0");
        touch_buttons_[MobileButtons::ButtonNormal1] = std::make_unique<TouchButton>(Point<int16_t>(1050, 250),
                                                                                    TouchButton::ActionType::Default,GLFMKeyCode1, "1");
        touch_buttons_[MobileButtons::ButtonNormal2] = std::make_unique<TouchButton>(Point<int16_t>(1050, 100),
                                                                                     TouchButton::ActionType::Default,GLFMKeyCode2, "2");
        touch_buttons_[MobileButtons::ButtonNormal3] = std::make_unique<TouchButton>(Point<int16_t>(1200, 100),
                                                                                     TouchButton::ActionType::Default,GLFMKeyCode3, "3");
        touch_buttons_[MobileButtons::ButtonNormal4] = std::make_unique<TouchButton>(Point<int16_t>(1200, 250),
                                                                                     TouchButton::ActionType::Default,GLFMKeyCode4, "4");
        touch_buttons_[MobileButtons::ButtonNormal5] = std::make_unique<TouchButton>(Point<int16_t>(1200, 400),
                                                                                     TouchButton::ActionType::Default,GLFMKeyCode5, "5");
        touch_buttons_[MobileButtons::ButtonNormal6] = std::make_unique<TouchButton>(Point<int16_t>(1200, 550),
                                                                                     TouchButton::ActionType::Default,GLFMKeyCode6, "6");
    }

    void UIMobileInput::draw() const {
        for (const auto &iter : touch_buttons_) {
            if (const TouchButton *button = iter.second.get()) {
                button->draw();
            }
        }
    }

    void UIMobileInput::update() {
        for (const auto &iter : touch_buttons_) {
            if (TouchButton *button = iter.second.get()) {
                button->update();
            }
        }
    }

    std::map<uint16_t, std::unique_ptr<TouchButton>>& UIMobileInput::getTouchButtons() {
        return touch_buttons_;
    }
}  // namespace ms