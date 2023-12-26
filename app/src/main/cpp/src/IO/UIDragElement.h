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

#include "../Configuration.h"
#include "UIElement.h"

namespace ms {
template<typename T>
// Base class for UI Windows which can be moved with the mouse cursor.
class UIDragElement : public UIElement {
public:
    void remove_cursor() override {
        UIElement::remove_cursor();

        if (dragged_) {
            dragged_ = false;

            Setting<T>::get().save(position_);
        }
    }

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override {
        if (clicked) {
            if (dragged_) {
                position_ = cursorpos - cursor_offset_;

                return Cursor::State::CLICKING;
            }
            if (is_in_drag_range(cursorpos)) {
                cursor_offset_ = cursorpos - position_;
                dragged_ = true;

                return UIElement::send_cursor(clicked, cursorpos);
            }
        } else {
            if (dragged_) {
                dragged_ = false;

                Setting<T>::get().save(position_);
            }
        }

        return UIElement::send_cursor(clicked, cursorpos);
    }

protected:
    UIDragElement() : UIDragElement(Point<int16_t>(0, 0)) {}

    UIDragElement(Point<int16_t> d) : drag_area_(d) {
        position_ = Setting<T>::get().load();
    }

    bool dragged_ = false;
    Point<int16_t> drag_area_;
    Point<int16_t> cursor_offset_;

private:
    virtual bool is_in_drag_range(Point<int16_t> cursorpos) const {
        auto bounds = Rectangle<int16_t>(position_, position_ + drag_area_);

        return bounds.contains(cursorpos);
    }
};
}  // namespace ms