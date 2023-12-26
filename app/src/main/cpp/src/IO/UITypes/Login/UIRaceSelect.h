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

#include "../Components/Textfield.h"
#include "../Template/BoolPair.h"
#include "../UIElement.h"

namespace ms {
// Race selection screen.
class UIRaceSelect : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::RACE_SELECT;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = false;

    UIRaceSelect();

    void draw(float inter) const override;

    void update() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    bool check_name(const std::string &name) const;

    void send_naming_result(bool nameused);

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    void select_class(uint8_t index);

    void show_charselect();

    Point<int16_t> get_class_pos(size_t index) const;

    std::string to_lower(std::string value) const;

    static constexpr uint8_t INDEX_COUNT_ = 5;
    static constexpr uint8_t CLASS_COUNT_ = 26;

    enum Buttons : uint16_t {
        BACK,
        MAKE,
        LEFT,
        RIGHT,
        CLASS0,
        CLASS1,
        CLASS2,
        CLASS3,
        CLASS4
    };

    // TODO: Merge with UILoginNotice.h
    enum Classes : uint8_t {
        RESISTANCE,
        EXPLORER,
        CYGNUSKNIGHTS,
        ARAN,
        EVAN,
        MERCEDES,
        DEMON,
        PHANTOM,
        DUALBLADE,
        MIHILE,
        LUMINOUS,
        KAISER,
        ANGELICBUSTER,
        CANNONEER,
        XENON,
        ZERO,
        SHADE,
        JETT,
        HAYATO,
        KANNA,
        CHASE,
        PINKBEAN,
        KINESIS,
        CADENA,
        ILLIUM,
        ARK,
    };

    Text version_;
    Point<int16_t> pos_;
    Point<int16_t> pos_zero_;
    nl::node order_;
    nl::node hotlist_;
    nl::node newlist_;
    nl::node bgm_;
    Sprite hotlabel_;
    Sprite hotlabel_zero_;
    Sprite newlabel_;
    Sprite hotbtn_;
    Sprite newbtn_;
    uint8_t class_index_[INDEX_COUNT_];
    bool mouseover_[INDEX_COUNT_];
    uint8_t selected_class_;
    uint8_t index_shift_;
    uint16_t selected_index_;
    bool class_isdisabled_[CLASS_COUNT_];
    BoolPair<Texture> class_disabled_[CLASS_COUNT_];
    BoolPair<Texture> class_normal_[CLASS_COUNT_];
    Texture class_background_[CLASS_COUNT_];
    Texture class_details_[CLASS_COUNT_];
    Texture class_title_[CLASS_COUNT_];
    Texture back_;
    Texture back_zero_;
    Sprite back_ani_;
    Texture class_details_background_;
    Texture class_details_background_zero_;
};
}  // namespace ms