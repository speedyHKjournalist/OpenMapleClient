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

#include "../../Character/Look/CharLook.h"
#include "../../Template/BoolPair.h"
#include "../Components/Textfield.h"
#include "../UIElement.h"

namespace ms {
class UIAranCreation : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::CLASS_CREATION;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = false;

    UIAranCreation();

    void draw(float inter) const override;

    void update() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void send_naming_result(bool nameused);

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    void randomize_look();

    const std::string &get_equipname(EquipSlot::Id slot) const;

    enum Buttons : uint16_t {
        BT_BACK,
        BT_CHARC_OK,
        BT_CHARC_CANCEL,
        BT_CHARC_SKINL,
        BT_CHARC_SKINR,
        BT_CHARC_GENDER_M,
        BT_CHARC_GEMDER_F
    };

    enum GenderButtons : uint8_t {
        GENDER_BACKGROUND,
        GENDER_HEAD,
        GENDER_TOP,
        GENDER_MID,
        GENDER_BOTTOM
    };

    std::vector<Sprite> sprites_lookboard_;
    std::vector<Sprite> sprites_gender_select_;
    std::vector<Sprite> sprites_keytype_;
    Texture sky_;
    Texture cloud_;
    float cloudfx_;
    Texture nameboard_;
    Textfield namechar_;
    CharLook newchar_;
    Randomizer randomizer_;

    BoolPair<std::vector<uint8_t>> skins_;
    BoolPair<std::vector<uint8_t>> haircolors_;
    BoolPair<std::vector<int32_t>> faces_;
    BoolPair<std::vector<int32_t>> hairs_;
    BoolPair<std::vector<int32_t>> tops_;
    BoolPair<std::vector<int32_t>> bots_;
    BoolPair<std::vector<int32_t>> shoes_;
    BoolPair<std::vector<int32_t>> weapons_;

    bool gender_;
    bool char_set_;
    bool named_;
    bool female_;
    size_t skin_;
    size_t haircolor_;
    size_t face;
    size_t hair_;
    size_t top_;
    size_t bot_;
    size_t shoe_;
    size_t weapon_;
    Text facename_;
    Text hairname_;
    Text bodyname_;
    Text topname_;
    Text botname_;
    Text shoename_;
    Text wepname_;
    Text version_;
};
}  // namespace ms