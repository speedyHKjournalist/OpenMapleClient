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
#pragma once

#include <string>

#include "../Components/Textfield.h"
#include "../UIElement.h"

namespace ms {
class UINotification : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::NOTIFICATION;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UINotification(std::string message);

    void draw(float alpha) const override;

    Cursor::State send_cursor(bool pressed, Point<int16_t> cursorpos) override;

    UIElement::Type get_type() const override { return TYPE; };

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    enum Buttons : int16_t { YES, NO };

    Text message_;
};
}  // namespace ms