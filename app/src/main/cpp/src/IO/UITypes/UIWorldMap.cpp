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
#include "UIWorldMap.h"

#include <nlnx/nx.hpp>

#include "../Gameplay/Stage.h"
#include "../IO/Components/MapleButton.h"
#include "../UI.h"
#include "GameInfo.h"

namespace ms {
UIWorldMap::UIWorldMap() : UIDragElement<PosMAP>() {
    nl::node close = nl::nx::ui["Basic.img"]["BtClose3"];
    nl::node WorldMap = nl::nx::ui["UIWindow2.img"]["WorldMap"];
    nl::node WorldMapSearch = WorldMap["WorldMapSearch"];
    nl::node Border = WorldMap["Border"]["0"];
    nl::node backgrnd = WorldMapSearch["backgrnd"];
    nl::node MapHelper = nl::nx::map["MapHelper.img"]["worldMap"];

    cur_pos_ = MapHelper["curPos"];

    for (size_t i = 0; i < npc_pos_.size(); i++) {
        npc_pos_[i] = MapHelper["npcPos" + std::to_string(i)];
    }

    sprites_.emplace_back(Border);

    search_background_ = backgrnd;
    search_notice_ = WorldMapSearch["notice"];

    bg_dimensions_ = Texture(Border).get_dimensions();
    bg_search_dimensions_ = search_background_.get_dimensions();

    int16_t bg_dimension_x = bg_dimensions_.x();
    background_dimensions_ = Point<int16_t>(bg_dimension_x, 0);

    int16_t base_x = bg_dimension_x / 2;
    int16_t base_y = bg_dimensions_.y() / 2;
    base_position_ = Point<int16_t>(base_x, base_y + 15);

    Point<int16_t> close_dimensions = Point<int16_t>(bg_dimension_x - 22, 4);

    buttons_[Buttons::BT_CLOSE] =
        std::make_unique<MapleButton>(close, close_dimensions);
    buttons_[Buttons::BT_SEARCH] =
        std::make_unique<MapleButton>(WorldMap["BtSearch"]);
    buttons_[Buttons::BT_AUTOFLY] =
        std::make_unique<MapleButton>(WorldMap["BtAutoFly_1"]);
    buttons_[Buttons::BT_NAVIREG] =
        std::make_unique<MapleButton>(WorldMap["BtNaviRegister"]);
    buttons_[Buttons::BT_SEARCH_CLOSE] = std::make_unique<MapleButton>(
        close,
        close_dimensions + Point<int16_t>(bg_search_dimensions_.x(), 0));
    buttons_[Buttons::BT_ALL_SEARCH] =
        std::make_unique<MapleButton>(WorldMapSearch["BtAllsearch"],
                                      background_dimensions_);

    Point<int16_t> search_text_pos = Point<int16_t>(bg_dimension_x + 14, 25);
    Point<int16_t> search_box_dim = Point<int16_t>(83, 15);
    Rectangle<int16_t> search_text_dim =
        Rectangle<int16_t>(search_text_pos, search_text_pos + search_box_dim);

    search_text_ = Textfield(Text::Font::A11M,
                             Text::Alignment::LEFT,
                             Color::Name::BLACK,
                             search_text_dim,
                             8);

    set_search(true);

    drag_area_ = Point<int16_t>(bg_dimension_x, 20);
}

void UIWorldMap::draw(float alpha) const {
    UIElement::draw_sprites(alpha);

    if (search_) {
        search_background_.draw(position_ + background_dimensions_);
        search_notice_.draw(position_ + background_dimensions_);
        search_text_.draw(position_ + Point<int16_t>(1, -5));
    }

    base_img_.draw(position_ + base_position_);

    if (!link_images_.empty()) {
        for (const auto &[btnid, button] : buttons_) {
            if (button.get()) {
                if (btnid >= Buttons::BT_LINK0
                    && button->get_state() == Button::State::MOUSEOVER) {
                    if (link_images_.find(btnid) != link_images_.end()) {
                        link_images_.at(btnid).draw(position_ + base_position_);
                        break;
                    }
                }
            }
        }
    }

    if (show_path_img_) {
        path_img_.draw(position_ + base_position_);
    }

    for (const auto &spot : map_spots_) {
        spot.second.marker.draw(spot.first + position_ + base_position_);
    }

    bool found = false;

    if (!found) {
        for (const auto &[spot_pos, spot] : map_spots_) {
            for (auto map_id : spot.map_ids) {
                if (map_id == mapid_) {
                    found = true;
                    npc_pos_[spot.type].draw(
                        spot_pos + position_ + base_position_,
                        alpha);
                    cur_pos_.draw(spot_pos + position_ + base_position_, alpha);
                    break;
                }
            }

            if (found) {
                break;
            }
        }
    }

    UIElement::draw_buttons(alpha);
}

void UIWorldMap::update() {
    int32_t mid = Stage::get().get_mapid();

    if (mid != mapid_) {
        mapid_ = mid;
        auto prefix = mapid_ / 10000000;
        auto parent_map = "WorldMap0" + std::to_string(prefix);
        user_map_ = parent_map;

        update_world(parent_map);
    }

    if (search_) {
        search_text_.update(position_);
    }

    for (auto &npc_po : npc_pos_) {
        npc_po.update(1);
    }

    cur_pos_.update();

    UIElement::update();
}

void UIWorldMap::toggle_active() {
    UIElement::toggle_active();

    if (!active_) {
        set_search(true);
        update_world(user_map_);
    }
}

void UIWorldMap::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        if (search_) {
            set_search(false);
        } else {
            if (parent_map_.empty()) {
                toggle_active();

                update_world(user_map_);
            } else {
                Sound(Sound::Name::SELECT_MAP).play();

                update_world(parent_map_);
            }
        }
    }
}

UIElement::Type UIWorldMap::get_type() const {
    return TYPE;
}

Button::State UIWorldMap::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::BT_CLOSE: deactivate(); break;
        case Buttons::BT_SEARCH: set_search(!search_); break;
        case Buttons::BT_SEARCH_CLOSE: set_search(false); break;
        default: break;
    }

    if (buttonid >= Buttons::BT_LINK0) {
        update_world(link_maps_[buttonid]);

        return Button::State::IDENTITY;
    }

    return Button::State::NORMAL;
}

void UIWorldMap::remove_cursor() {
    UIDragElement::remove_cursor();

    UI::get().clear_tooltip(Tooltip::Parent::WORLD_MAP);

    show_path_img_ = false;
}

Cursor::State UIWorldMap::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    if (Cursor::State new_state =
            search_text_.send_cursor(cursorpos, clicked)) {
        return new_state;
    }

    show_path_img_ = false;

    for (auto [path_pos, path_spot] : map_spots_) {
        Point<int16_t> p = path_pos + position_ + base_position_ - 10;
        Point<int16_t> d = p + path_spot.marker.get_dimensions();
        Rectangle<int16_t> abs_bounds = Rectangle<int16_t>(p, d);

        if (abs_bounds.contains(cursorpos)) {
            path_img_ = path_spot.path;
            show_path_img_ = path_img_.is_valid();

            UI::get().show_map(Tooltip::Parent::WORLD_MAP,
                               path_spot.title,
                               path_spot.description,
                               path_spot.map_ids[0],
                               path_spot.bolded);
            break;
        }
    }

    return UIDragElement::send_cursor(clicked, cursorpos);
}

void UIWorldMap::set_search(bool enable) {
    search_ = enable;

    buttons_[Buttons::BT_SEARCH_CLOSE]->set_active(enable);
    buttons_[Buttons::BT_ALL_SEARCH]->set_active(enable);

    if (enable) {
        search_text_.set_state(Textfield::State::NORMAL);
        dimension_ =
            bg_dimensions_ + Point<int16_t>(bg_search_dimensions_.x(), 0);
    } else {
        search_text_.set_state(Textfield::State::DISABLED);
        dimension_ = bg_dimensions_;
    }
}

void UIWorldMap::update_world(const std::string &map) {
    nl::node WorldMap = nl::nx::map["WorldMap"][map + ".img"];

    if (!WorldMap) {
        WorldMap = nl::nx::map["WorldMap"]["WorldMap.img"];
    }

    base_img_ = WorldMap["BaseImg"][0];
    parent_map_ = std::string(WorldMap["info"]["parentMap"]);

    link_images_.clear();
    link_maps_.clear();

    for (auto &[btnid, button] : buttons_) {
        if (button.get()) {
            if (btnid >= Buttons::BT_LINK0) {
                button->set_active(false);
            }
        }
    }

    size_t i = Buttons::BT_LINK0;

    for (const nl::node &link : WorldMap["MapLink"]) {
        nl::node l = link["link"];
        Texture link_image = l["linkImg"];

        link_images_[i] = link_image;
        link_maps_[i] = std::string(l["linkMap"]);

        buttons_[i] = std::make_unique<AreaButton>(
            base_position_ - link_image.get_origin(),
            link_image.get_dimensions());
        buttons_[i]->set_active(true);

        i++;
    }

    nl::node mapImage = nl::nx::map["MapHelper.img"]["worldMap"]["mapImage"];

    map_spots_.clear();

    for (const nl::node &list : WorldMap["MapList"]) {
        nl::node desc = list["desc"];
        nl::node mapNo = list["mapNo"];
        nl::node path = list["path"];
        nl::node spot = list["spot"];
        nl::node title = list["title"];
        nl::node type = list["type"];
        nl::node marker = mapImage[type];

        std::vector<int32_t> map_ids;

        for (const nl::node &map_no : mapNo) {
            map_ids.push_back(map_no);
        }

        if (!desc && !title) {
            NxHelper::Map::MapInfo map_info =
                NxHelper::Map::get_map_info_by_id(mapNo[0]);

            map_spots_.emplace_back(
                std::make_pair<Point<int16_t>, MapSpot>(spot,
                                                        { map_info.description,
                                                          path,
                                                          map_info.full_name,
                                                          type,
                                                          marker,
                                                          true,
                                                          map_ids }));
        } else {
            map_spots_.emplace_back(std::make_pair<Point<int16_t>, MapSpot>(
                spot,
                { desc, path, title, type, marker, false, map_ids }));
        }
    }
}
}  // namespace ms