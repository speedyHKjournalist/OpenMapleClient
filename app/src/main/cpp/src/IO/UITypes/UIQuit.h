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

#include "../Components/Charset.h"
#include "../Graphics/Geometry.h"
#include "../Graphics/Text.h"
#include "../UIElement.h"
#include "../Character/CharStats.h"

namespace ms {
class UIQuit : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::QUIT;
    static constexpr bool FOCUSED = true;
    static constexpr bool TOGGLED = false;

    UIQuit(const CharStats &stats);

    void draw(float inter) const override;

    void update() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    const CharStats &stats_;

    std::string pad_time(int64_t time);

    float getexppercent(uint16_t level, int64_t exp) const;
    void close();

    enum Buttons : uint16_t { NO, YES };

    Point<int16_t> screen_adj_;
    ColorBox background_;

    /// Time
    int64_t minutes_;
    int64_t hours_;

    Charset time_minutes_;
    Point<int16_t> time_minutes_pos_;
    std::string time_minutes_text_;

    Charset time_hours_;
    Point<int16_t> time_hours_pos_;
    std::string time_hours_text_;

    int64_t time_number_width_;

    Point<int16_t> time_lt_;
    Point<int16_t> time_rb_;

    /// Level
    Sprite levelup_effect_;

    uint16_t uplevel_;

    Charset level_before_;
    Point<int16_t> level_before_pos_;
    std::string level_before_text_;

    uint16_t cur_level_;

    Charset level_after_;
    Point<int16_t> level_after_pos_;
    std::string level_after_text_;

    int64_t level_number_width_;
    Point<int16_t> level_adj_;

    /// Experience
    Text exp_before_;
    Point<int16_t> exp_before_pos_;

    Text exp_after_;
    Point<int16_t> exp_after_pos_;

    Point<int16_t> exp_adj_;
};
}  // namespace ms