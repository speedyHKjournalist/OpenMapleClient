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

#include "../Graphics/Texture.h"
#include "../UIElement.h"

namespace ms {
class UIGaugeBoss : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::GAUGE_BOSS;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = false;

    UIGaugeBoss(int16_t screen_width,
                int8_t tag_color,
                int8_t tag_bg_color,
                int32_t mob_id,
                float percent);

    void draw(float alpha) const override;
    void update(float p);
    Type get_type() const override { return TYPE; };

private:
    Texture tag_front_;
    Texture tag_bg_;

    bool should_use_tag_bg_;

    int16_t screen_width_;

    float percentage_;
};
}  // namespace ms