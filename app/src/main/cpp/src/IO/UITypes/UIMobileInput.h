//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2024  Daniel Allendorf, Ryan Payton, Bizhou Xing
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

#include <unordered_map>
#include "TouchButton.h"
#include "VirtualJoyStick.h"

namespace ms {
class UIMobileInput {
public:
    enum MobileButtons : uint16_t {
        ButtonJump,
        ButtonNormal0,
        ButtonNormal1,
        ButtonNormal2,
        ButtonNormal3,
        ButtonNormal4,
        ButtonNormal5,
        ButtonNormal6
    };

    UIMobileInput();

    void draw() const;

    void update();

    std::map<uint16_t, std::unique_ptr<TouchButton>>& getTouchButtons();

    VirtualJoyStick& getVirtualJoyStick();

private:
    std::map<uint16_t, std::unique_ptr<TouchButton>> touch_buttons_;
    VirtualJoyStick joystick_;
};
}  // namespace ms