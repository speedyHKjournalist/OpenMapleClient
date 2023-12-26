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

#include "../Components/AreaButton.h"
#include "../Components/Textfield.h"
#include "../UIDragElement.h"

namespace ms {
class UIWorldMap : public UIDragElement<PosMAP> {
public:
    static constexpr Type TYPE = UIElement::Type::WORLD_MAP;
    static constexpr bool FOCUSED = false;
    static constexpr bool TOGGLED = true;

    UIWorldMap();

    void draw(float inter) const override;

    void update() override;

    void toggle_active() override;

    void remove_cursor() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursor_pos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    static constexpr uint8_t MAPSPOT_TYPE_MAX_ = 4u;

    void set_search(bool enable);

    void update_world(const std::string &parent_map);

    enum Buttons {
        BT_CLOSE,
        BT_SEARCH,
        BT_AUTOFLY,
        BT_NAVIREG,
        BT_ALL_SEARCH,
        BT_SEARCH_CLOSE,
        BT_LINK0,
        BT_LINK1,
        BT_LINK2,
        BT_LINK3,
        BT_LINK4,
        BT_LINK5,
        BT_LINK6,
        BT_LINK7,
        BT_LINK8,
        BT_LINK9
    };

    struct MapSpot {
        std::string description;
        Texture path;
        std::string title;
        uint8_t type;
        Texture marker;
        bool bolded;
        std::vector<int32_t> map_ids;
    };

    bool search_;
    bool show_path_img_;

    int32_t mapid_;

    std::string parent_map_;
    std::string user_map_;

    Texture search_background_;
    Texture search_notice_;
    Texture base_img_;
    Texture path_img_;

    Animation cur_pos_;
    std::array<Animation, MAPSPOT_TYPE_MAX_> npc_pos_;

    Textfield search_text_;

    std::map<uint16_t, Texture> link_images_;
    std::map<uint16_t, std::string> link_maps_;

    std::vector<std::pair<Point<int16_t>, MapSpot>> map_spots_;

    Point<int16_t> bg_dimensions_;
    Point<int16_t> bg_search_dimensions_;
    Point<int16_t> background_dimensions_;
    Point<int16_t> base_position_;
};
}  // namespace ms