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

#include "../../Gameplay/Stage.h"
#include "../Components/Slider.h"
#include "../UIDragElement.h"

namespace ms {
class UIMiniMap : public UIDragElement<PosMINIMAP> {
public:
    static constexpr Type TYPE = UIElement::Type::MINI_MAP;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UIMiniMap(const CharStats &stats);

    void draw(float alpha) const override;

    void update() override;

    void remove_cursor() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> pos) override;
    void send_scroll(double yoffset) override;
    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    static constexpr int16_t CENTER_START_X_ = 64;
    static constexpr int16_t BTN_MIN_Y_ = 4;
    static constexpr int16_t ML_MR_Y_ = 17;
    static constexpr int16_t MAX_ADJ_ = 40;
    static constexpr int16_t M_START_ = 36;
    static constexpr int16_t LISTNPC_ITEM_HEIGHT_ = 17;
    static constexpr int16_t LISTNPC_ITEM_WIDTH_ = 140;
    static constexpr int16_t LISTNPC_TEXT_WIDTH_ = 114;
    static constexpr Point<int16_t> WINDOW_UL_POS_ = Point<int16_t>(0, 0);

    void update_buttons();

    void toggle_buttons();

    void update_text();

    void update_canvas();

    void draw_movable_markers(Point<int16_t> init_pos, float alpha) const;

    void update_static_markers();

    void set_npclist_active(bool active);

    void update_dimensions();

    void update_npclist();

    void draw_npclist(Point<int16_t> minimap_dims, float alpha) const;

    void select_npclist(int16_t choice);

    enum Buttons { BT_MIN, BT_MAX, BT_SMALL, BT_BIG, BT_MAP, BT_NPC };

    enum Type { MIN, NORMAL, MAX };

    /// Constants
    int32_t mapid_;
    int8_t type_;
    int8_t user_type_;
    bool simple_mode_;
    bool big_map_;
    bool has_map_;
    int16_t scale_;
    nl::node map_;
    nl::node mini_map_;
    nl::node marker_;
    Texture map_sprite_;
    Animation player_marker_;
    int16_t combined_text_width_;
    int16_t middle_right_x_;
    int16_t bt_min_width_;
    int16_t bt_max_width_;
    int16_t bt_map_width_;
    std::vector<Sprite> min_sprites_;
    std::vector<Sprite> normal_sprites_;
    std::vector<Sprite> max_sprites_;
    std::vector<std::pair<std::string, Point<int16_t>>> static_marker_info_;
    int16_t map_draw_origin_x_, map_draw_origin_y_;
    Point<int16_t> center_offset_;
    Point<int16_t> min_dimensions_;
    Point<int16_t> normal_dimensions_;
    Point<int16_t> max_dimensions_;
    Text combined_text_;
    Text region_text_;
    Text town_text_;

    bool list_npc_enabled_;
    nl::node list_npc_;
    std::vector<Sprite> list_npc_sprites_;
    std::vector<MapObject *> list_npc_list_;
    std::vector<Text> list_npc_names_;
    std::vector<std::string> list_npc_full_names_;

    Point<int16_t> list_npc_dimensions_;

    Slider list_npc_slider_;
    int16_t list_npc_offset_;
    int16_t selected_;
    Animation selected_marker_;

    const CharStats &stats_;
};
}  // namespace ms