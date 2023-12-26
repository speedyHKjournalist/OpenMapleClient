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

#include <array>
#include <memory>

#include "../Graphics/Geometry.h"
#include "Button.h"

namespace ms {
// A standard maplestory-combobox with 4 states and three textures for each
// state.
class MapleComboBox : public Button {
public:
    enum Type : uint8_t {
        DEFAULT = 1,
        BROWN = 3,
        BLUENEG,
        DEFAULT2,
        BLACKM,
        BLACKL,
        BLACKS,
        BROWNNEG,
        BLACKL2,
        GREENNEG
    };

    MapleComboBox(Type type,
                  std::vector<std::string> options,
                  uint16_t default_option,
                  Point<int16_t> parentpos,
                  Point<int16_t> position,
                  int64_t width);

    void draw(Point<int16_t> parentpos) const override;

    void update() override {}

    Rectangle<int16_t> bounds(Point<int16_t> parentpos) const override;

    int16_t width() const override;

    Point<int16_t> origin() const override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    bool in_combobox(Point<int16_t> cursorpos) override;

    uint16_t get_selected() const override;

protected:
    Button::State button_pressed(uint16_t buttonid);

private:
    enum Buttons : uint16_t {
        OPTION1,
        OPTION2,
        OPTION3,
        OPTION4,
        OPTION5,
        OPTION6,
        OPTION7,
        OPTION8,
        OPTION9,
        OPTION10
    };

    std::array<std::array<Texture, 3>, Button::State::NUM_STATES> textures_;
    std::vector<std::string> options_;
    std::vector<Text> option_text_;
    Text selected_;
    ColorBox background_;
    ColorBox rect_;
    ColorBox current_rect_;
    uint16_t rwidth_;
    static constexpr uint16_t HEIGHT = 16;
    std::map<uint16_t, std::unique_ptr<Button>> buttons_;
    uint16_t current_pos_;
    bool current_shown_;
    uint16_t last_shown_;
    uint16_t selected_index_;
    Point<int16_t> selected_adj_;
    Point<int16_t> parent_pos_;
};
}  // namespace ms