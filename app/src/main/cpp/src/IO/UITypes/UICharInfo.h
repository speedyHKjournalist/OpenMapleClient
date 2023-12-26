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

#include "../../Character/Char.h"
#include "../../Graphics/Text.h"
#include "../../Template/BoolPair.h"
#include "../Components/Charset.h"
#include "../UIDragElement.h"

namespace ms {
class UICharInfo : public UIDragElement<PosCHARINFO> {
public:
    static constexpr Type TYPE = UIElement::Type::CHAR_INFO;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UICharInfo(int32_t cid);

    void draw(float inter) const override;
    void update() override;

    Button::State button_pressed(uint16_t buttonid) override;

    bool is_in_range(Point<int16_t> cursorpos) const override;
    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void update_stats(int32_t character_id,
                      int16_t job_id,
                      uint8_t level,
                      int16_t fame,
                      const std::string &guild,
                      const std::string &alliance);

private:
    void show_bottom_window(uint16_t buttonid);
    void show_right_window(uint16_t buttonid);

    enum Buttons : uint16_t {
        BT_CLOSE,
        BT_COLLECT,
        BT_DAMAGE,
        BT_FAMILY,
        BT_ITEM,
        BT_PARTY,
        BT_PERSONALITY,
        BT_PET,
        BT_POP_DOWN,
        BT_POP_UP,
        BT_RIDE,
        BT_TRADE,
        BT_FRIEND,
        BT_VISIT,
        BT_ARRAY_GET,
        BT_ARRAY_NAME,
        BT_FAQ,
        BT_REGIST
    };

    /// Main Window
    Text name_;
    Text job_;
    Text level_;
    Text fame_;
    Text guild_;
    Text alliance_;

    Char *target_character_;
    int32_t cid_;

    /// Sub Windows
    Point<int16_t> bottom_window_adj_;
    Point<int16_t> right_window_adj_;

    /// Farm
    Text farm_name_;
    Sprite loading_;
    bool is_loading_;
    uint16_t timestep_;
    Charset farm_level_;
    Point<int16_t> farm_dim_;
    Point<int16_t> farm_adj_;
    std::string farm_level_text_;

    /// Personality
    bool personality_enabled_;
    std::vector<Sprite> personality_sprites_;
    BoolPair<std::vector<Sprite>> personality_sprites_enabled_;
    Point<int16_t> personality_dimensions_;

    /// Collect
    bool collect_enabled_;
    std::vector<Sprite> collect_sprites_;
    Point<int16_t> collect_dimensions_;
    Point<int16_t> collect_adj_;
    Sprite default_medal_;
    Text medal_text_;
    Text medal_total_;

    /// Damage
    bool damage_enabled_;
    std::vector<Sprite> damage_sprites_;
    Point<int16_t> damage_dimensions_;

    /// Item
    bool item_enabled_;
    std::vector<Sprite> item_sprites_;
    Point<int16_t> item_dimensions_;
};
}  // namespace ms