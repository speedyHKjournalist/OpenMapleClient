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
#include "UIMiniMap.h"

#include <nlnx/nx.hpp>

#include "../Components/MapleButton.h"
#include "../Gameplay/MapleMap/Npc.h"
#include "../UI.h"
#include "UIWorldMap.h"
#include "GameInfo.h"

namespace ms {
UIMiniMap::UIMiniMap(const CharStats &stats) :
    UIDragElement<PosMINIMAP>(Point<int16_t>(128, 20)),
    stats_(stats) {
    big_map_ = true;
    has_map_ = false;
    list_npc_enabled_ = false;
    list_npc_dimensions_ = Point<int16_t>(150, 170);
    list_npc_offset_ = 0;
    selected_ = -1;

    type_ = Setting<MiniMapType>::get().load();
    user_type_ = type_;
    simple_mode_ = Setting<MiniMapSimpleMode>::get().load();

    std::string node = simple_mode_ ? "MiniMapSimpleMode" : "MiniMap";
    mini_map_ = nl::nx::ui["UIWindow2.img"][node];
    list_npc_ = nl::nx::ui["UIWindow2.img"]["MiniMap"]["ListNpc"];

    buttons_[Buttons::BT_MIN] =
        std::make_unique<MapleButton>(mini_map_["BtMin"],
                                      Point<int16_t>(195, -6));
    buttons_[Buttons::BT_MAX] =
        std::make_unique<MapleButton>(mini_map_["BtMax"],
                                      Point<int16_t>(209, -6));
    buttons_[Buttons::BT_SMALL] =
        std::make_unique<MapleButton>(mini_map_["BtSmall"],
                                      Point<int16_t>(223, -6));
    buttons_[Buttons::BT_BIG] =
        std::make_unique<MapleButton>(mini_map_["BtBig"],
                                      Point<int16_t>(223, -6));
    buttons_[Buttons::BT_MAP] =
        std::make_unique<MapleButton>(mini_map_["BtMap"],
                                      Point<int16_t>(237, -6));
    buttons_[Buttons::BT_NPC] =
        std::make_unique<MapleButton>(mini_map_["BtNpc"],
                                      Point<int16_t>(276, -6));

    region_text_ =
        Text(Text::Font::A12B, Text::Alignment::LEFT, Color::Name::WHITE);
    town_text_ =
        Text(Text::Font::A12B, Text::Alignment::LEFT, Color::Name::WHITE);
    combined_text_ =
        Text(Text::Font::A12M, Text::Alignment::LEFT, Color::Name::WHITE);

    marker_ =
        Setting<MiniMapDefaultHelpers>::get().load()
            ? nl::nx::ui["UIWindow2.img"]["MiniMapSimpleMode"]["DefaultHelper"]
            : nl::nx::mapLatest["MapHelper.img"]["minimap"];

    player_marker_ = Animation(marker_["user"]);
    selected_marker_ = Animation(mini_map_["iconNpc"]);
}

void UIMiniMap::draw(float alpha) const {
    if (type_ == Type::MIN) {
        for (const auto &sprite : min_sprites_) {
            sprite.draw(position_, alpha);
        }

        combined_text_.draw(position_ + Point<int16_t>(7, -3));
    } else if (type_ == Type::NORMAL) {
        for (const auto &sprite : normal_sprites_) {
            sprite.draw(position_, alpha);
        }

        if (has_map_) {
            Animation portal_marker = Animation(marker_["portal"]);

            for (const auto &sprite : static_marker_info_) {
                portal_marker.draw(position_ + sprite.second, alpha);
            }

            draw_movable_markers(position_, alpha);

            if (list_npc_enabled_) {
                draw_npclist(normal_dimensions_, alpha);
            }
        }
    } else {
        for (const auto &sprite : max_sprites_) {
            sprite.draw(position_, alpha);
        }

        region_text_.draw(position_ + Point<int16_t>(48, 14));
        town_text_.draw(position_ + Point<int16_t>(48, 28));

        if (has_map_) {
            Animation portal_marker(marker_["portal"]);

            for (const auto &sprite : static_marker_info_) {
                portal_marker.draw(
                    position_ + sprite.second + Point<int16_t>(0, MAX_ADJ_),
                    alpha);
            }

            draw_movable_markers(position_ + Point<int16_t>(0, MAX_ADJ_),
                                 alpha);

            if (list_npc_enabled_) {
                draw_npclist(max_dimensions_, alpha);
            }
        }
    }

    UIElement::draw(alpha);
}

void UIMiniMap::update() {
    int32_t mid = Stage::get().get_mapid();

    if (mid != mapid_) {
        mapid_ = mid;
        map_ = NxHelper::Map::get_map_node_name(mapid_);

        nl::node town = map_["info"]["town"];
        nl::node miniMap = map_["miniMap"];

        if (!miniMap) {
            has_map_ = false;
            type_ = Type::MIN;
        } else {
            has_map_ = true;

            if (town && town.get_bool()) {
                type_ = Type::MAX;
            } else {
                type_ = user_type_;
            }
        }

        scale_ = std::pow(2, (int)miniMap["mag"]);
        center_offset_ = Point<int16_t>(miniMap["centerX"], miniMap["centerY"]);

        update_text();
        update_buttons();
        update_canvas();
        update_static_markers();
        toggle_buttons();
        update_npclist();
    }

    if (type_ == Type::MIN) {
        for (auto sprite : min_sprites_) {
            sprite.update();
        }
    } else if (type_ == Type::NORMAL) {
        for (auto sprite : normal_sprites_) {
            sprite.update();
        }
    } else {
        for (auto sprite : max_sprites_) {
            sprite.update();
        }
    }

    if (list_npc_enabled_) {
        for (Sprite sprite : list_npc_sprites_) {
            sprite.update();
        }
    }

    if (selected_ >= 0) {
        selected_marker_.update();
    }

    UIElement::update();
}

void UIMiniMap::remove_cursor() {
    UIDragElement::remove_cursor();

    list_npc_slider_.remove_cursor();

    UI::get().clear_tooltip(Tooltip::Parent::MINI_MAP);
}

Cursor::State UIMiniMap::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Cursor::State dstate = UIDragElement::send_cursor(clicked, cursorpos);

    if (dragged_) {
        return dstate;
    }

    Point<int16_t> cursor_relative = cursorpos - position_;

    if (list_npc_slider_.isenabled()) {
        if (Cursor::State new_state =
                list_npc_slider_.send_cursor(cursor_relative, clicked)) {
            return new_state;
        }
    }

    if (list_npc_enabled_) {
        Point<int16_t> relative_point =
            cursor_relative
            - Point<int16_t>(10
                                 + (type_ == Type::MAX ? max_dimensions_
                                                       : normal_dimensions_)
                                       .x(),
                             23);
        Rectangle<int16_t> list_bounds =
            Rectangle<int16_t>(0,
                               LISTNPC_ITEM_WIDTH_,
                               0,
                               LISTNPC_ITEM_HEIGHT_ * 8);

        if (list_bounds.contains(relative_point)) {
            int16_t list_index =
                list_npc_offset_ + relative_point.y() / LISTNPC_ITEM_HEIGHT_;
            bool in_list = list_index < list_npc_names_.size();

            if (clicked) {
                select_npclist(in_list ? list_index : -1);
            } else if (in_list) {
                UI::get().show_text(Tooltip::Parent::MINI_MAP,
                                    list_npc_full_names_[list_index]);
            }

            return Cursor::State::IDLE;
        }
    }

    bool found = false;
    auto *npcs = Stage::get().get_npcs().get_npcs();

    for (auto npc = npcs->begin(); npc != npcs->end(); npc++) {
        Point<int16_t> npc_pos =
            (npc->second->get_position() + center_offset_) / scale_
            + Point<int16_t>(map_draw_origin_x_, map_draw_origin_y_);
        Rectangle<int16_t> marker_spot =
            Rectangle<int16_t>(npc_pos - Point<int16_t>(4, 8), npc_pos);

        if (type_ == Type::MAX) {
            marker_spot.shift(Point<int16_t>(0, MAX_ADJ_));
        }

        if (marker_spot.contains(cursor_relative)) {
            found = true;

            auto *n = static_cast<Npc *>(npc->second.get());
            std::string name = n->get_name();
            std::string func = n->get_func();

            UI::get().show_map(Tooltip::Parent::MINI_MAP,
                               name,
                               func,
                               {},
                               false);
            break;
        }
    }

    if (!found) {
        for (const auto &[sprite_name, sprite_pos] : static_marker_info_) {
            Rectangle<int16_t> marker_spot =
                Rectangle<int16_t>(sprite_pos, sprite_pos + 8);

            if (type_ == Type::MAX) {
                marker_spot.shift(Point<int16_t>(0, MAX_ADJ_));
            }

            if (marker_spot.contains(cursor_relative)) {
                nl::node portal_tm = map_["portal"][sprite_name]["tm"];
                std::string portal_cat =
                    NxHelper::Map::get_map_category(portal_tm);
                nl::node portal_name =
                    nl::nx::string["Map.img"][portal_cat][portal_tm]["mapName"];

                if (portal_name) {
                    found = true;

                    UI::get().show_map(Tooltip::Parent::MINI_MAP,
                                       portal_name,
                                       "",
                                       portal_tm,
                                       false);
                    break;
                }
            }
        }
    }

    return Cursor::State::IDLE;
}

void UIMiniMap::send_scroll(double yoffset) {
    if (list_npc_enabled_ && list_npc_slider_.isenabled()) {
        list_npc_slider_.send_scroll(yoffset);
    }
}

void UIMiniMap::send_key(int32_t keycode, bool pressed, bool escape) {
    if (has_map_) {
        if (type_ < Type::MAX) {
            type_++;
        } else {
            type_ = Type::MIN;
        }

        user_type_ = type_;

        toggle_buttons();
    }
}

Button::State UIMiniMap::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case BT_MIN:
            type_ -= 1;
            toggle_buttons();
            return type_ == Type::MIN ? Button::State::DISABLED
                                      : Button::State::NORMAL;
        case BT_MAX:
            type_ += 1;
            toggle_buttons();
            return type_ == Type::MAX ? Button::State::DISABLED
                                      : Button::State::NORMAL;
        case BT_SMALL:
        case BT_BIG:
            big_map_ = !big_map_;
            // TODO: Toggle scrolling map
            toggle_buttons();
            break;
        case BT_MAP: UI::get().emplace<UIWorldMap>(); break;
        case BT_NPC: set_npclist_active(!list_npc_enabled_); break;
    }

    return Button::State::NORMAL;
}

UIElement::Type UIMiniMap::get_type() const {
    return TYPE;
}

void UIMiniMap::update_buttons() {
    // Add one pixel for a space to the right of each button
    bt_min_width_ = buttons_[Buttons::BT_MIN]->width() + 1;
    bt_max_width_ = buttons_[Buttons::BT_MAX]->width() + 1;
    bt_map_width_ = buttons_[Buttons::BT_MAP]->width() + 1;

    combined_text_width_ = combined_text_.width();
}

void UIMiniMap::toggle_buttons() {
    int16_t bt_min_x;

    if (type_ == Type::MIN) {
        buttons_[Buttons::BT_MAP]->set_active(true);
        buttons_[Buttons::BT_MAX]->set_active(true);
        buttons_[Buttons::BT_MIN]->set_active(true);
        buttons_[Buttons::BT_NPC]->set_active(false);
        buttons_[Buttons::BT_SMALL]->set_active(false);
        buttons_[Buttons::BT_BIG]->set_active(false);

        buttons_[Buttons::BT_MIN]->set_state(Button::State::DISABLED);

        if (has_map_) {
            buttons_[Buttons::BT_MAX]->set_state(Button::State::NORMAL);
        } else {
            buttons_[Buttons::BT_MAX]->set_state(Button::State::DISABLED);
        }

        bt_min_x = combined_text_width_ + 11;

        buttons_[Buttons::BT_MIN]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        bt_min_x += bt_min_width_;

        buttons_[Buttons::BT_MAX]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        bt_min_x += bt_max_width_;

        buttons_[Buttons::BT_MAP]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        min_dimensions_ = Point<int16_t>(bt_min_x + bt_map_width_ + 7, 20);

        update_dimensions();

        drag_area_ = dimension_;

        set_npclist_active(false);
    } else {
        bool has_npcs = Stage::get().get_npcs().get_npcs()->size() > 0;

        buttons_[Buttons::BT_MAP]->set_active(true);
        buttons_[Buttons::BT_MAX]->set_active(true);
        buttons_[Buttons::BT_MIN]->set_active(true);
        buttons_[Buttons::BT_NPC]->set_active(has_npcs);

        if (big_map_) {
            buttons_[Buttons::BT_BIG]->set_active(false);
            buttons_[Buttons::BT_SMALL]->set_active(true);
        } else {
            buttons_[Buttons::BT_BIG]->set_active(true);
            buttons_[Buttons::BT_SMALL]->set_active(false);
        }

        buttons_[Buttons::BT_MIN]->set_state(Button::State::NORMAL);

        bt_min_x = middle_right_x_
                   - (bt_min_width_ + buttons_[Buttons::BT_SMALL]->width() + 1
                      + bt_max_width_ + bt_map_width_
                      + (has_npcs ? buttons_[Buttons::BT_NPC]->width() : 0));

        buttons_[Buttons::BT_MIN]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        bt_min_x += bt_max_width_;

        buttons_[Buttons::BT_MAX]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        bt_min_x += bt_max_width_;

        buttons_[Buttons::BT_SMALL]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));
        buttons_[Buttons::BT_BIG]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        bt_min_x += bt_max_width_;

        buttons_[Buttons::BT_MAP]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        bt_min_x += bt_map_width_;

        buttons_[Buttons::BT_NPC]->set_position(
            Point<int16_t>(bt_min_x, BTN_MIN_Y_));

        if (type_ == Type::MAX) {
            buttons_[Buttons::BT_MAX]->set_state(Button::State::DISABLED);
        } else {
            buttons_[Buttons::BT_MAX]->set_state(Button::State::NORMAL);
        }

        set_npclist_active(list_npc_enabled_ && has_npcs);

        drag_area_ = Point<int16_t>(dimension_.x(), 20);
    }
}

void UIMiniMap::update_text() {
    NxHelper::Map::MapInfo map_info = NxHelper::Map::get_map_info_by_id(mapid_);
    combined_text_.change_text(map_info.full_name);
    region_text_.change_text(map_info.name);
    town_text_.change_text(map_info.street_name);
}

void UIMiniMap::update_canvas() {
    min_sprites_.clear();
    normal_sprites_.clear();
    max_sprites_.clear();

    nl::node Min, Normal, Max;

    if (simple_mode_) {
        Min = mini_map_["Window"]["Min"];
        Normal = mini_map_["Window"]["Normal"];
        Max = mini_map_["Window"]["Max"];
    } else {
        Min = mini_map_["Min"];
        Normal = mini_map_["MinMap"];
        Max = mini_map_["MaxMap"];
    }

    map_sprite_ = Texture(map_["miniMap"]["canvas"]);
    Point<int16_t> map_dimensions = map_sprite_.get_dimensions();

    // 48 (offset for text) + longer text's width + 10 (space for right side
    // border)
    int16_t mark_text_width =
        48 + std::max(region_text_.width(), town_text_.width()) + 10;
    int16_t c_stretch, ur_x_offset, m_stretch, down_y_offset;
    int16_t window_width =
        std::max(178, std::max((int)mark_text_width, map_dimensions.x() + 20));

    c_stretch = std::max(0, window_width - 128);
    ur_x_offset = CENTER_START_X_ + c_stretch;
    map_draw_origin_x_ =
        std::max(10, window_width / 2 - map_dimensions.x() / 2);

    if (map_dimensions.y() <= 20) {
        m_stretch = 5;
        down_y_offset = 17 + m_stretch;
        map_draw_origin_y_ = 10 + m_stretch - map_dimensions.y();
    } else {
        m_stretch = map_dimensions.y() - 17;
        down_y_offset = 17 + m_stretch;
        map_draw_origin_y_ = 20;
    }

    middle_right_x_ = ur_x_offset + 55;

    std::string Left = simple_mode_ ? "Left" : "w";
    std::string Center = simple_mode_ ? "Center" : "c";
    std::string Right = simple_mode_ ? "Right" : "e";

    std::string DownCenter = simple_mode_ ? "DownCenter" : "s";
    std::string DownLeft = simple_mode_ ? "DownLeft" : "sw";
    std::string DownRight = simple_mode_ ? "DownRight" : "se";
    std::string MiddleLeft = simple_mode_ ? "MiddleLeft" : "w";
    std::string MiddleRight = simple_mode_ ? "MiddleRight" : "e";
    std::string UpCenter = simple_mode_ ? "UpCenter" : "n";
    std::string UpLeft = simple_mode_ ? "UpLeft" : "nw";
    std::string UpRight = simple_mode_ ? "UpRight" : "ne";

    // SimpleMode's backdrop is opaque, the other is transparent but lightly
    // colored UI.wz v208 has normal center sprite in-linked to bottom right
    // window frame, not sure why.
    nl::node MiddleCenter = simple_mode_
                                ? mini_map_["Window"]["Max"]["MiddleCenter"]
                                : mini_map_["MaxMap"]["c"];

    int16_t dl_dr_y = std::max(map_dimensions.y(), (int16_t)10);

    // combined_text_width + 14 (7px buffer on both sides) + 4 (buffer between
    // name and buttons) + 3 buttons' widths - 128 (length of left and right
    // window borders)
    int16_t min_c_stretch = combined_text_width_ + 18 + bt_min_width_
                            + bt_max_width_ + bt_map_width_ - 128;

    // Min sprites queue
    min_sprites_.emplace_back(
        Min[Center],
        DrawArgument(WINDOW_UL_POS_ + Point<int16_t>(CENTER_START_X_, 0),
                     Point<int16_t>(min_c_stretch, 0)));
    min_sprites_.emplace_back(Min[Left], DrawArgument(WINDOW_UL_POS_));
    min_sprites_.emplace_back(
        Min[Right],
        DrawArgument(WINDOW_UL_POS_
                     + Point<int16_t>(min_c_stretch + CENTER_START_X_, 0)));

    // Normal sprites queue
    // (7, 10) is the top left corner of the inner window
    // 114 = 128 (width of left and right borders) - 14 (width of middle borders
    // * 2). 27 = height of inner frame drawn on up and down borders
    normal_sprites_.emplace_back(
        MiddleCenter,
        DrawArgument(Point<int16_t>(7, 10),
                     Point<int16_t>(c_stretch + 114, m_stretch + 27)));

    if (has_map_) {
        normal_sprites_.emplace_back(
            map_["miniMap"]["canvas"],
            DrawArgument(
                Point<int16_t>(map_draw_origin_x_, map_draw_origin_y_)));
    }

    normal_sprites_.emplace_back(Normal[MiddleLeft],
                                 DrawArgument(Point<int16_t>(0, ML_MR_Y_),
                                              Point<int16_t>(0, m_stretch)));
    normal_sprites_.emplace_back(
        Normal[MiddleRight],
        DrawArgument(Point<int16_t>(middle_right_x_, ML_MR_Y_),
                     Point<int16_t>(0, m_stretch)));
    normal_sprites_.emplace_back(
        Normal[UpCenter],
        DrawArgument(Point<int16_t>(CENTER_START_X_, 0) + WINDOW_UL_POS_,
                     Point<int16_t>(c_stretch, 0)));
    normal_sprites_.emplace_back(Normal[UpLeft], WINDOW_UL_POS_);
    normal_sprites_.emplace_back(
        Normal[UpRight],
        DrawArgument(Point<int16_t>(ur_x_offset, 0) + WINDOW_UL_POS_));
    normal_sprites_.emplace_back(
        Normal[DownCenter],
        DrawArgument(Point<int16_t>(CENTER_START_X_, down_y_offset + 18),
                     Point<int16_t>(c_stretch, 0)));
    normal_sprites_.emplace_back(Normal[DownLeft],
                                 Point<int16_t>(0, down_y_offset));
    normal_sprites_.emplace_back(Normal[DownRight],
                                 Point<int16_t>(ur_x_offset, down_y_offset));

    normal_dimensions_ = Point<int16_t>(ur_x_offset + 64, down_y_offset + 27);

    // Max sprites queue
    max_sprites_.emplace_back(
        MiddleCenter,
        DrawArgument(Point<int16_t>(7, 50),
                     Point<int16_t>(c_stretch + 114, m_stretch + 27)));

    if (has_map_) {
        max_sprites_.emplace_back(
            map_["miniMap"]["canvas"],
            DrawArgument(Point<int16_t>(map_draw_origin_x_,
                                        map_draw_origin_y_ + MAX_ADJ_)));
    }

    max_sprites_.emplace_back(
        Max[MiddleLeft],
        DrawArgument(Point<int16_t>(0, ML_MR_Y_ + MAX_ADJ_),
                     Point<int16_t>(0, m_stretch)));
    max_sprites_.emplace_back(
        Max[MiddleRight],
        DrawArgument(Point<int16_t>(middle_right_x_, ML_MR_Y_ + MAX_ADJ_),
                     Point<int16_t>(0, m_stretch)));
    max_sprites_.emplace_back(
        Max[UpCenter],
        DrawArgument(Point<int16_t>(CENTER_START_X_, 0) + WINDOW_UL_POS_,
                     Point<int16_t>(c_stretch, 0)));
    max_sprites_.emplace_back(Max[UpLeft], WINDOW_UL_POS_);
    max_sprites_.emplace_back(
        Max[UpRight],
        DrawArgument(Point<int16_t>(ur_x_offset, 0) + WINDOW_UL_POS_));
    max_sprites_.emplace_back(
        Max[DownCenter],
        DrawArgument(
            Point<int16_t>(CENTER_START_X_, down_y_offset + MAX_ADJ_ + 18),
            Point<int16_t>(c_stretch, 0)));
    max_sprites_.emplace_back(Max[DownLeft],
                              Point<int16_t>(0, down_y_offset + MAX_ADJ_));
    max_sprites_.emplace_back(
        Max[DownRight],
        Point<int16_t>(ur_x_offset, down_y_offset + MAX_ADJ_));
    max_sprites_.emplace_back(
        nl::nx::mapLatest["MapHelper.img"]["mark"][map_["info"]["mapMark"]],
        DrawArgument(Point<int16_t>(7, 17)));

    max_dimensions_ = normal_dimensions_ + Point<int16_t>(0, MAX_ADJ_);
}

void UIMiniMap::draw_movable_markers(Point<int16_t> init_pos,
                                     float alpha) const {
    if (!has_map_) {
        return;
    }

    Animation marker_sprite;
    Point<int16_t> sprite_offset;
    constexpr Point<int16_t> char_pos_offset = { 60, 150 };
    constexpr Point<int16_t> npc_pos_offset = { 0, 65 };

    /// NPCs
    MapObjects *npcs = Stage::get().get_npcs().get_npcs();
    marker_sprite = Animation(marker_["npc"]);
    sprite_offset = marker_sprite.get_dimensions() / Point<int16_t>(2, 0);

    for (auto npc = npcs->begin(); npc != npcs->end(); ++npc) {
        Point<int16_t> npc_pos = npc->second->get_position();
        marker_sprite.draw(
            (npc_pos + center_offset_ + npc_pos_offset) / scale_ - sprite_offset
                + Point<int16_t>(map_draw_origin_x_, map_draw_origin_y_)
                + init_pos,
            alpha);
    }

    /// Other characters
    MapObjects *chars = Stage::get().get_chars().get_chars();
    marker_sprite = Animation(marker_["another"]);
    sprite_offset = marker_sprite.get_dimensions() / Point<int16_t>(2, 0);

    for (auto chr = chars->begin(); chr != chars->end(); ++chr) {
        Point<int16_t> chr_pos = chr->second->get_position();
        marker_sprite.draw(
            (chr_pos + char_pos_offset + center_offset_) / scale_
                - sprite_offset
                + Point<int16_t>(map_draw_origin_x_, map_draw_origin_y_)
                + init_pos,
            alpha);
    }

    /// Player
    Point<int16_t> player_pos = Stage::get().get_player().get_position();
    sprite_offset = player_marker_.get_dimensions() / Point<int16_t>(2, 0);
    player_marker_.draw(
        (player_pos + char_pos_offset + center_offset_) / scale_ - sprite_offset
            + Point<int16_t>(map_draw_origin_x_, map_draw_origin_y_) + init_pos,
        alpha);
}

void UIMiniMap::update_static_markers() {
    static_marker_info_.clear();

    if (!has_map_) {
        return;
    }

    Animation marker_sprite;

    /// Portals
    nl::node portals = map_["portal"];
    marker_sprite = Animation(marker_["portal"]);
    Point<int16_t> marker_offset =
        marker_sprite.get_dimensions() / Point<int16_t>(2, 0);
    constexpr Point<int16_t> marker_pos_offset = { 0, 65 };

    for (nl::node portal = portals.begin(); portal != portals.end(); ++portal) {
        int portal_type = portal["pt"];

        if (portal_type == 2) {
            Point<int16_t> marker_pos =
                (Point<int16_t>(portal["x"], portal["y"]) + marker_pos_offset
                 + center_offset_)
                    / scale_
                - marker_offset
                + Point<int16_t>(map_draw_origin_x_, map_draw_origin_y_);
            static_marker_info_.emplace_back(portal.name(), marker_pos);
        }
    }
}

void UIMiniMap::set_npclist_active(bool active) {
    list_npc_enabled_ = active;

    if (!active) {
        select_npclist(-1);
    }

    update_dimensions();
}

void UIMiniMap::update_dimensions() {
    if (type_ == Type::MIN) {
        dimension_ = min_dimensions_;
    } else {
        Point<int16_t> base_dims =
            type_ == Type::MAX ? max_dimensions_ : normal_dimensions_;
        dimension_ = base_dims;

        if (list_npc_enabled_) {
            dimension_ += list_npc_dimensions_;
            dimension_.set_y(std::max(base_dims.y(), list_npc_dimensions_.y()));
        }
    }
}

void UIMiniMap::update_npclist() {
    list_npc_sprites_.clear();
    list_npc_names_.clear();
    list_npc_full_names_.clear();
    list_npc_list_.clear();
    selected_ = -1;
    list_npc_offset_ = 0;

    if (simple_mode_) {
        return;
    }

    auto *npcs = Stage::get().get_npcs().get_npcs();

    for (auto npc = npcs->begin(); npc != npcs->end(); ++npc) {
        list_npc_list_.emplace_back(npc->second.get());

        auto *n = static_cast<Npc *>(npc->second.get());
        std::string name = n->get_name();
        std::string func = n->get_func();

        if (!func.empty()) {
            name += " (" + func + ")";
        }

        Text name_text = Text(Text::Font::A11M,
                              Text::Alignment::LEFT,
                              Color::Name::WHITE,
                              name);

        list_npc_names_.emplace_back(name_text);
        list_npc_full_names_.emplace_back(name);
    }

    for (size_t i = 0; i < list_npc_names_.size(); i++) {
        string_format::format_with_ellipsis(
            list_npc_names_[i],
            LISTNPC_TEXT_WIDTH_ - (list_npc_names_.size() > 8 ? 0 : 20));
    }

    const Point<int16_t> listNpc_pos = Point<int16_t>(
        type_ == Type::MAX ? max_dimensions_.x() : normal_dimensions_.x(),
        0);
    int16_t c_stretch = 20;
    int16_t m_stretch = 102;

    if (list_npc_names_.size() > 8) {
        list_npc_slider_ =
            Slider(Slider::DEFAULT_SILVER,
                   Range<int16_t>(23, 11 + LISTNPC_ITEM_HEIGHT_ * 8),
                   listNpc_pos.x() + LISTNPC_ITEM_WIDTH_ + 1,
                   8,
                   list_npc_names_.size(),
                   [&](bool upwards) {
                       int16_t shift = upwards ? -1 : 1;
                       bool above = list_npc_offset_ + shift >= 0;
                       bool below = list_npc_offset_ + 8 + shift
                                    <= list_npc_names_.size();

                       if (above && below) {
                           list_npc_offset_ += shift;
                       }
                   });

        c_stretch += 12;
    } else {
        list_npc_slider_.setenabled(false);
        m_stretch = LISTNPC_ITEM_HEIGHT_ * list_npc_names_.size() - 34;
        c_stretch -= 17;
    }

    list_npc_sprites_.emplace_back(
        list_npc_["c"],
        DrawArgument(listNpc_pos + Point<int16_t>(CENTER_START_X_, M_START_),
                     Point<int16_t>(c_stretch, m_stretch)));
    list_npc_sprites_.emplace_back(
        list_npc_["w"],
        DrawArgument(listNpc_pos + Point<int16_t>(0, M_START_),
                     Point<int16_t>(0, m_stretch)));
    list_npc_sprites_.emplace_back(
        list_npc_["e"],
        DrawArgument(
            listNpc_pos + Point<int16_t>(CENTER_START_X_ + c_stretch, M_START_),
            Point<int16_t>(0, m_stretch)));
    list_npc_sprites_.emplace_back(
        list_npc_["n"],
        DrawArgument(listNpc_pos + Point<int16_t>(CENTER_START_X_, 0),
                     Point<int16_t>(c_stretch, 0)));
    list_npc_sprites_.emplace_back(
        list_npc_["s"],
        DrawArgument(
            listNpc_pos + Point<int16_t>(CENTER_START_X_, M_START_ + m_stretch),
            Point<int16_t>(c_stretch, 0)));
    list_npc_sprites_.emplace_back(
        list_npc_["nw"],
        DrawArgument(listNpc_pos + Point<int16_t>(0, 0)));
    list_npc_sprites_.emplace_back(
        list_npc_["ne"],
        DrawArgument(listNpc_pos
                     + Point<int16_t>(CENTER_START_X_ + c_stretch, 0)));
    list_npc_sprites_.emplace_back(
        list_npc_["sw"],
        DrawArgument(listNpc_pos + Point<int16_t>(0, M_START_ + m_stretch)));
    list_npc_sprites_.emplace_back(
        list_npc_["se"],
        DrawArgument(listNpc_pos
                     + Point<int16_t>(CENTER_START_X_ + c_stretch,
                                      M_START_ + m_stretch)));

    list_npc_dimensions_ = Point<int16_t>(CENTER_START_X_ * 2 + c_stretch,
                                          M_START_ + m_stretch + 30);

    update_dimensions();
}

void UIMiniMap::draw_npclist(Point<int16_t> minimap_dims, float alpha) const {
    Animation npc_marker = Animation(marker_["npc"]);

    for (const Sprite &sprite : list_npc_sprites_) {
        sprite.draw(position_, alpha);
    }

    Point<int16_t> listNpc_pos =
        position_ + Point<int16_t>(minimap_dims.x() + 10, 23);

    for (int i = 0; i + list_npc_offset_ < list_npc_list_.size() && i < 8;
         i++) {
        if (selected_ - list_npc_offset_ == i) {
            ColorBox highlight = ColorBox(
                LISTNPC_ITEM_WIDTH_ - (list_npc_slider_.isenabled() ? 0 : 30),
                LISTNPC_ITEM_HEIGHT_,
                Color::Name::YELLOW,
                1.0f);
            highlight.draw(listNpc_pos);
        }

        npc_marker.draw(DrawArgument(listNpc_pos + Point<int16_t>(0, 2),
                                     false,
                                     npc_marker.get_dimensions() / 2),
                        alpha);
        list_npc_names_[list_npc_offset_ + i].draw(
            DrawArgument(listNpc_pos + Point<int16_t>(14, -2)));

        listNpc_pos.shift_y(LISTNPC_ITEM_HEIGHT_);
    }

    if (list_npc_slider_.isenabled()) {
        list_npc_slider_.draw(position_);
    }

    if (selected_ >= 0) {
        Point<int16_t> npc_pos =
            (list_npc_list_[selected_]->get_position() + center_offset_)
                / scale_
            + Point<int16_t>(map_draw_origin_x_,
                             map_draw_origin_y_
                                 - npc_marker.get_dimensions().y()
                                 + (type_ == Type::MAX ? MAX_ADJ_ : 0));

        selected_marker_.draw(position_ + npc_pos, 0.5f);
    }
}

void UIMiniMap::select_npclist(int16_t choice) {
    if (selected_ == choice) {
        return;
    }

    if (selected_ >= 0 && selected_ < list_npc_names_.size()) {
        list_npc_names_[selected_].change_color(Color::Name::WHITE);
    }

    if (choice > list_npc_names_.size() || choice < 0) {
        selected_ = -1;
    } else {
        selected_ = choice != selected_ ? choice : -1;

        if (selected_ >= 0) {
            list_npc_names_[selected_].change_color(Color::Name::BLACK);
        }
    }
}
}  // namespace ms