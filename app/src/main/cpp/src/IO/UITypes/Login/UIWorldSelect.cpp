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
#include "UIWorldSelect.h"

#include <nlnx/nx.hpp>

#include "../../Audio/Audio.h"
#include "../../Net/Packets/LoginPackets.h"
#include "../../Util/Randomizer.h"
#include "../Components/MapleButton.h"
#include "../Components/TwoSpriteButton.h"
#include "../UI.h"
#include "UILoginNotice.h"
#include "UILoginWait.h"
#include "UIRegion.h"

namespace ms {
namespace {
auto fn_charlist_req = [](auto&&... args) {
    CharlistRequestPacket(std::forward<decltype(args)>(args)...).dispatch();
};

auto fn_server_status_req = [](auto&&... args) {
    ServerStatusRequestPacket(std::forward<decltype(args)>(args)...).dispatch();
};
}  // namespace

UIWorldSelect::UIWorldSelect() :
    UIElement(Point<int16_t>(0, 0), Point<int16_t>(800, 600)) {
    world_count_ = 0;
    recommended_world_count_ = 0;
    recommended_worldid_ = 0;
    world_selected_ = false;
    use_recommended_ = false;
    show_recommended_ = false;
    draw_chatballoon_ = true;

    std::string version_text = Configuration::get().get_version();
    version_ = Text(Text::Font::A11M,
                    Text::Alignment::LEFT,
                    Color::Name::LEMONGRASS,
                    "Ver. " + version_text);

    recommended_message_ = Text(Text::Font::A11M,
                                Text::Alignment::CENTER,
                                Color::Name::JAMBALAYA,
                                "",
                                100,
                                true,
                                5);

    Point<int16_t> background_pos = Point<int16_t>(400, 301);
    channel_src_pos_ = Point<int16_t>(203, 164);

    worldid_ = Setting<DefaultWorld>::get().load();
    channelid_ = Setting<DefaultChannel>::get().load();
    uint8_t regionid = Setting<DefaultRegion>::get().load();

    nl::node obj = nl::nx::mapLatest["Obj"]["login.img"];
    nl::node login = nl::nx::ui["Login.img"];
    world_select_ = login["WorldSelect"];
    world_src_ = world_select_["BtWorld"]["release"];
    channel_src_ = world_select_["BtChannel"];
    nl::node common = login["Common"];

    set_region(regionid);

    sprites_.emplace_back(obj["WorldSelect"]["default"][0], background_pos);

    std::vector<std::string> backgrounds = { "cernium" };
    auto backgrounds_size = backgrounds.size();

    if (backgrounds_size > 0) {
        if (backgrounds_size > 1) {
            auto randomizer = Randomizer();
            int32_t index = randomizer.next_int(backgrounds_size);

            sprites_.emplace_back(obj["WorldSelect"][backgrounds[index]][0],
                                  background_pos);
        } else {
            sprites_.emplace_back(obj["WorldSelect"][backgrounds[0]][0],
                                  background_pos);
        }
    }

    sprites_.emplace_back(common["frame"], Point<int16_t>(400, 300));
    sprites_.emplace_back(common["step"]["1"], Point<int16_t>(40, 0));

    buttons_[Buttons::BT_VIEW_ALL] =
        std::make_unique<MapleButton>(world_select_["BtViewAll"],
                                      Point<int16_t>(0, 53));
    buttons_[Buttons::BT_VIEW_RECOMMENDED] =
        std::make_unique<MapleButton>(world_select_["BtViewChoice"],
                                      Point<int16_t>(0, 53));
    buttons_[Buttons::BT_VIEW_RECOMMENDED_SELECT] =
        std::make_unique<MapleButton>(world_select_["alert"]["BtChoice"],
                                      Point<int16_t>(349, 327));
    buttons_[Buttons::BT_VIEW_RECOMMENDED_CANCEL] =
        std::make_unique<MapleButton>(world_select_["alert"]["BtClose"],
                                      Point<int16_t>(407, 327));
    buttons_[Buttons::BT_VIEW_RECOMMENDED_PREV] =
        std::make_unique<MapleButton>(world_select_["alert"]["BtArrowL"],
                                      Point<int16_t>(338, 244));
    buttons_[Buttons::BT_VIEW_RECOMMENDED_NEXT] =
        std::make_unique<MapleButton>(world_select_["alert"]["BtArrowR"],
                                      Point<int16_t>(439, 244));

    buttons_[Buttons::BT_VIEW_ALL]->set_active(false);
    buttons_[Buttons::BT_VIEW_RECOMMENDED]->set_active(use_recommended_);
    buttons_[Buttons::BT_VIEW_RECOMMENDED_SELECT]->set_active(false);
    buttons_[Buttons::BT_VIEW_RECOMMENDED_CANCEL]->set_active(false);
    buttons_[Buttons::BT_VIEW_RECOMMENDED_PREV]->set_active(false);
    buttons_[Buttons::BT_VIEW_RECOMMENDED_NEXT]->set_active(false);

    buttons_[Buttons::BT_VIEW_RECOMMENDED]->set_state(Button::State::DISABLED);

    recommended_textures_.emplace_back(world_select_["alert"]["backgrd"]);

    buttons_[Buttons::BT_CHANGE_REGION] =
        std::make_unique<MapleButton>(world_select_["BtRegion"],
                                      Point<int16_t>(3, 127));
    buttons_[Buttons::BT_QUIT_GAME] =
        std::make_unique<MapleButton>(common["BtExit"], Point<int16_t>(0, 515));

    for (size_t i = 0; i < Buttons::BT_ENTER_WORLD - Buttons::BT_CHANNEL0;
         i++) {
        std::string ch = std::to_string(i);

        buttons_[Buttons::BT_CHANNEL0 + i] = std::make_unique<TwoSpriteButton>(
            channel_src_["button:" + ch]["normal"]["0"],
            channel_src_["button:" + ch]["keyFocused"]["0"],
            channel_src_pos_);
        buttons_[Buttons::BT_CHANNEL0 + i]->set_active(false);
    }

    channels_background_ = channel_src_["layer:bg"];

    buttons_[Buttons::BT_ENTER_WORLD] =
        std::make_unique<MapleButton>(channel_src_["button:GoWorld"],
                                      channel_src_pos_);
    buttons_[Buttons::BT_ENTER_WORLD]->set_active(false);

    chatballoon_.change_text(
        "Please select the World you would like to play in.");

    if (Configuration::get().get_auto_login()) {
        auto world = Configuration::get().get_auto_world();
        auto channel = Configuration::get().get_auto_channel();

        Configuration::get().set_worldid(world);
        Configuration::get().set_channelid(channel);

        UI::get().emplace<UILoginWait>();
        auto loginwait = UI::get().get_element<UILoginWait>();

        if (loginwait && loginwait->get().is_active()) {
            fn_charlist_req(world, channel);
        }
    }
}

void UIWorldSelect::draw(float alpha) const {
    UIElement::draw_sprites(alpha);

    worlds_background_.draw(position_ + world_src_pos_);

    if (show_recommended_) {
        recommended_textures_[0].draw(position_ + Point<int16_t>(302, 152));
        recommended_world_textures_[recommended_worldid_].draw(
            position_ + Point<int16_t>(336, 187));
        recommended_message_.draw(position_ + Point<int16_t>(401, 259));
    }

    if (world_selected_) {
        channels_background_.draw(position_ + channel_src_pos_);
        world_textures_[worldid_].draw(position_ + channel_src_pos_);
    }

    UIElement::draw_buttons(alpha);

    version_.draw(position_ + Point<int16_t>(707, 1));

    if (draw_chatballoon_) {
        chatballoon_.draw(position_ + Point<int16_t>(500, 80));
    }
}

Cursor::State UIWorldSelect::send_cursor(bool clicked,
                                         Point<int16_t> cursorpos) {
    Rectangle<int16_t> channels_bounds = Rectangle<int16_t>(
        position_ + channel_src_pos_,
        position_ + channel_src_pos_ + channels_background_.get_dimensions());

    Rectangle<int16_t> worlds_bounds = Rectangle<int16_t>(
        position_ + world_src_pos_,
        position_ + world_src_pos_ + worlds_background_.get_dimensions());

    if (world_selected_ && !channels_bounds.contains(cursorpos)
        && !worlds_bounds.contains(cursorpos)) {
        if (clicked) {
            world_selected_ = false;
            clear_selected_world();
        }
    }

    Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

    for (auto &[btnid, button] : buttons_) {
        if (button->is_active()
            && button->bounds(position_).contains(cursorpos)) {
            if (button->get_state() == Button::State::NORMAL) {
                Sound(Sound::Name::BUTTON_OVER).play();

                button->set_state(Button::State::MOUSEOVER);
                ret = Cursor::State::CAN_CLICK;
            } else if (button->get_state() == Button::State::PRESSED) {
                if (clicked) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

                    button->set_state(button_pressed(btnid));

                    ret = Cursor::State::IDLE;
                } else {
                    ret = Cursor::State::CAN_CLICK;
                }
            } else if (button->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

                    button->set_state(button_pressed(btnid));

                    ret = Cursor::State::IDLE;
                } else {
                    ret = Cursor::State::CAN_CLICK;
                }
            }
        } else if (button->get_state() == Button::State::MOUSEOVER) {
            button->set_state(Button::State::NORMAL);
        }
    }

    return ret;
}

void UIWorldSelect::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (world_selected_) {
            World selectedWorld = worlds_[worldid_];

            uint8_t selected_channel = channelid_;
            uint8_t channel_total = selectedWorld.channelcount;

            uint8_t COLUMNS = 5;
            uint8_t columns = std::min(channel_total, COLUMNS);

            uint8_t rows = std::floor((channel_total - 1) / COLUMNS) + 1;

            div_t div = std::div(selected_channel, columns);
            auto current_col = div.rem;
            // auto current_row = div.quot;

            if (keycode == KeyAction::Id::UP) {
                auto next_channel =
                    (selected_channel - COLUMNS < 0
                         ? (selected_channel - COLUMNS) + rows * COLUMNS
                         : selected_channel - COLUMNS);

                if (next_channel == channelid_) {
                    return;
                }

                if (next_channel > channel_total) {
                    button_pressed(next_channel - COLUMNS
                                   + Buttons::BT_CHANNEL0);
                } else {
                    button_pressed(next_channel + Buttons::BT_CHANNEL0);
                }
            } else if (keycode == KeyAction::Id::DOWN) {
                auto next_channel = (selected_channel + COLUMNS >= channel_total
                                         ? current_col
                                         : selected_channel + COLUMNS);

                if (next_channel == channelid_) {
                    return;
                }

                if (next_channel > channel_total) {
                    button_pressed(next_channel + COLUMNS
                                   + Buttons::BT_CHANNEL0);
                } else {
                    button_pressed(next_channel + Buttons::BT_CHANNEL0);
                }
            } else if (keycode == KeyAction::Id::LEFT
                       || keycode == KeyAction::Id::TAB) {
                if (selected_channel != 0) {
                    selected_channel--;
                } else {
                    selected_channel = channel_total - 1;
                }

                button_pressed(selected_channel + Buttons::BT_CHANNEL0);
            } else if (keycode == KeyAction::Id::RIGHT) {
                if (selected_channel != channel_total - 1) {
                    selected_channel++;
                } else {
                    selected_channel = 0;
                }

                button_pressed(selected_channel + Buttons::BT_CHANNEL0);
            } else if (escape) {
                world_selected_ = false;

                clear_selected_world();
            } else if (keycode == KeyAction::Id::RETURN) {
                button_pressed(Buttons::BT_ENTER_WORLD);
            }
        } else if (show_recommended_) {
            if (escape || keycode == KeyAction::Id::RETURN) {
                toggle_recommended(false);
            }
        } else {
            auto selected_world = worldid_;
//            auto world_count = world_count_ - 1;

            if (keycode == KeyAction::Id::LEFT
                || keycode == KeyAction::Id::RIGHT
                || keycode == KeyAction::Id::UP
                || keycode == KeyAction::Id::DOWN
                || keycode == KeyAction::Id::TAB) {
                bool world_found = false;
                bool forward = keycode == KeyAction::Id::LEFT
                               || keycode == KeyAction::Id::UP;

                while (!world_found) {
                    selected_world = get_next_world(selected_world, forward);

                    for (const auto &world : worlds_) {
                        if (world.wid == selected_world) {
                            world_found = true;
                            break;
                        }
                    }
                }

                buttons_[Buttons::BT_WORLD0 + worldid_]->set_state(
                    Button::State::NORMAL);

                worldid_ = static_cast<uint8_t>(selected_world);

                buttons_[Buttons::BT_WORLD0 + worldid_]->set_state(
                    Button::State::PRESSED);
            } else if (escape) {
                auto quitconfirm = UI::get().get_element<UIQuitConfirm>();

                if (quitconfirm && quitconfirm->get().is_active()) {
                    return UI::get().send_key(keycode, pressed);
                }

                button_pressed(Buttons::BT_QUIT_GAME);
            } else if (keycode == KeyAction::Id::RETURN) {
                auto quitconfirm = UI::get().get_element<UIQuitConfirm>();

                if (quitconfirm && quitconfirm->get().is_active()) {
                    return UI::get().send_key(keycode, pressed);
                }

                bool found = false;

                for (size_t i = Buttons::BT_WORLD0; i < Buttons::BT_CHANNEL0;
                     i++) {
                    auto state = buttons_[Buttons::BT_WORLD0 + i]->get_state();

                    if (state == Button::State::PRESSED) {
                        found = true;
                        break;
                    }
                }

                if (found) {
                    button_pressed(selected_world + Buttons::BT_WORLD0);
                } else {
                    buttons_[Buttons::BT_WORLD0 + selected_world]->set_state(
                        Button::State::PRESSED);
                }
            }
        }
    }
}

UIElement::Type UIWorldSelect::get_type() const {
    return TYPE;
}

void UIWorldSelect::draw_world() {
    if (world_count_ <= 0) {
        return;  // TODO: Send the user back to the login screen? Otherwise, I
                 // think the screen will be blank with no worlds, or throw a
                 // UILoginNotice up with failed to communite to server?
    }

    for (const auto &world : worlds_) {
        if (world.channelcount < 2) {
            return;  // I remove the world if there is only one channel because
                     // the graphic for the channel selection is defaulted to at
                     // least 2
        }

        buttons_[Buttons::BT_WORLD0 + world.wid]->set_active(true);

        if (channelid_ >= world.channelcount) {
            channelid_ = 0;
        }
    }
}

void UIWorldSelect::add_world(World world) {
    worlds_.emplace_back(std::move(world));
    world_count_++;
}

void UIWorldSelect::add_recommended_world(RecommendedWorld world) {
    if (use_recommended_) {
        recommended_worlds_.emplace_back(std::move(world));
        recommended_world_count_++;

        buttons_[Buttons::BT_VIEW_RECOMMENDED]->set_state(
            Button::State::NORMAL);
    }
}

void UIWorldSelect::change_world(const World &selectedWorld) {
    buttons_[Buttons::BT_WORLD0 + selectedWorld.wid]->set_state(
        Button::State::PRESSED);

    for (size_t i = 0; i < selectedWorld.channelcount; ++i) {
        buttons_[Buttons::BT_CHANNEL0 + i]->set_active(true);

        if (i == channelid_) {
            buttons_[Buttons::BT_CHANNEL0 + i]->set_state(
                Button::State::PRESSED);
        }
    }

    buttons_[Buttons::BT_ENTER_WORLD]->set_active(true);
}

void UIWorldSelect::remove_selected() {
    deactivate();

    Sound(Sound::Name::SCROLL_UP).play();

    world_selected_ = false;

    clear_selected_world();

    draw_chatballoon_ = false;
}

void UIWorldSelect::set_region(uint8_t regionid) {
    world_map_[Buttons::BT_WORLD0] =
        regionid == 5 ? Worlds::SCANIA : Worlds::LUNA;
    world_map_[Buttons::BT_WORLD1] = Worlds::BERA;
    world_map_[Buttons::BT_WORLD2] = Worlds::AURORA;
    world_map_[Buttons::BT_WORLD3] = Worlds::ELYSIUM1;
    world_map_[Buttons::BT_WORLD4] = Worlds::REBOOT1;

    nl::node region = world_src_["index"][regionid];

    worlds_background_ = region["layer:bg"];

    world_src_pos_ = region["pos"];

    for (size_t i = Buttons::BT_WORLD0; i <= Buttons::BT_WORLD4; i++) {
        std::string world = std::to_string(world_map_[i]);
        world_textures_.emplace_back(channel_src_["release"]["layer:" + world]);
        recommended_world_textures_.emplace_back(world_select_["world"][world]);

        nl::node worldbtn = world_src_["button:" + world];

        buttons_[Buttons::BT_WORLD0 + i] = std::make_unique<TwoSpriteButton>(
            worldbtn["normal"]["0"],
            worldbtn["keyFocused"]["0"],
            world_src_pos_ + Point<int16_t>(region["origin"][i + 1]));
        buttons_[Buttons::BT_WORLD0 + i]->set_active(false);
    }
}

uint16_t UIWorldSelect::get_worldbyid(uint16_t worldid) {
    return world_map_.find(worldid)->second;
}

Button::State UIWorldSelect::button_pressed(uint16_t id) {
    if (id == Buttons::BT_ENTER_WORLD) {
        enter_world();

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_QUIT_GAME) {
        UI::get().emplace<UIQuitConfirm>();

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_VIEW_RECOMMENDED) {
        world_selected_ = false;
        clear_selected_world();
        toggle_recommended(true);

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_VIEW_ALL) {
        toggle_recommended(false);

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_VIEW_RECOMMENDED_SELECT) {
        buttons_[Buttons::BT_WORLD0 + worldid_]->set_state(
            Button::State::NORMAL);

        worldid_ = recommended_worldid_;

        buttons_[Buttons::BT_WORLD0 + worldid_]->set_state(
            Button::State::PRESSED);

        toggle_recommended(false);

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_VIEW_RECOMMENDED_CANCEL) {
        toggle_recommended(false);

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_VIEW_RECOMMENDED_PREV) {
        if (recommended_worldid_ > 0) {
            recommended_worldid_--;
        } else {
            recommended_worldid_ = recommended_world_count_ - 1;
        }

        recommended_message_.change_text(
            recommended_worlds_[recommended_worldid_].message);

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_VIEW_RECOMMENDED_NEXT) {
        if (recommended_worldid_ < recommended_world_count_ - 1) {
            recommended_worldid_++;
        } else {
            recommended_worldid_ = 0;
        }

        recommended_message_.change_text(
            recommended_worlds_[recommended_worldid_].message);

        return Button::State::NORMAL;
    }

    if (id == Buttons::BT_CHANGE_REGION) {
        UI::get().emplace<UIRegion>();

        deactivate();

        return Button::State::NORMAL;
    }

    if (id >= Buttons::BT_WORLD0 && id < Buttons::BT_CHANNEL0) {
        toggle_recommended(false);

        buttons_[Buttons::BT_WORLD0 + worldid_]->set_state(
            Button::State::NORMAL);

        worldid_ = id - Buttons::BT_WORLD0;

        fn_server_status_req(worldid_);

        world_selected_ = true;
        clear_selected_world();
        change_world(worlds_[worldid_]);

        return Button::State::PRESSED;
    }

    if (id >= Buttons::BT_CHANNEL0 && id < Buttons::BT_ENTER_WORLD) {
        auto selectedch = static_cast<uint8_t>(id - Buttons::BT_CHANNEL0);

        if (selectedch != channelid_) {
            buttons_[Buttons::BT_CHANNEL0 + channelid_]->set_state(
                Button::State::NORMAL);
            channelid_ = static_cast<uint8_t>(id - Buttons::BT_CHANNEL0);
            buttons_[Buttons::BT_CHANNEL0 + channelid_]->set_state(
                Button::State::PRESSED);
            Sound(Sound::Name::WORLD_SELECT).play();
        } else {
            enter_world();
        }

        return Button::State::PRESSED;
    }

    return Button::State::NORMAL;
}

uint8_t UIWorldSelect::get_channel_count(uint8_t wid) const {
    for (const auto &world : worlds_) {
        if (world.wid == wid) {
            return world.channelcount;
        }
    }

    return 0;
}

void UIWorldSelect::enter_world() {
    Configuration::get().set_worldid(worldid_);
    Configuration::get().set_channelid(channelid_);

    UI::get().emplace<UILoginWait>();
    auto loginwait = UI::get().get_element<UILoginWait>();

    if (loginwait && loginwait->get().is_active()) {
        fn_charlist_req(worldid_, channelid_);
    }
}

void UIWorldSelect::toggle_recommended(bool active) {
    if (recommended_world_count_ > 0) {
        recommended_worldid_ = 0;
        show_recommended_ = active;

        buttons_[Buttons::BT_VIEW_ALL]->set_active(active);
        buttons_[Buttons::BT_VIEW_RECOMMENDED]->set_active(!active);
        buttons_[Buttons::BT_VIEW_RECOMMENDED_SELECT]->set_active(active);
        buttons_[Buttons::BT_VIEW_RECOMMENDED_CANCEL]->set_active(active);
        buttons_[Buttons::BT_VIEW_RECOMMENDED_PREV]->set_active(active);
        buttons_[Buttons::BT_VIEW_RECOMMENDED_NEXT]->set_active(active);

        if (recommended_world_count_ <= 1) {
            buttons_[Buttons::BT_VIEW_RECOMMENDED_PREV]->set_state(
                Button::State::DISABLED);
            buttons_[Buttons::BT_VIEW_RECOMMENDED_NEXT]->set_state(
                Button::State::DISABLED);
        } else {
            buttons_[Buttons::BT_VIEW_RECOMMENDED_PREV]->set_state(
                Button::State::NORMAL);
            buttons_[Buttons::BT_VIEW_RECOMMENDED_NEXT]->set_state(
                Button::State::NORMAL);
        }

        if (!active) {
            recommended_message_.change_text("");
        } else {
            recommended_message_.change_text(
                recommended_worlds_[recommended_worldid_].message);
        }
    }
}

void UIWorldSelect::clear_selected_world() {
    channelid_ = 0;

    for (size_t i = Buttons::BT_CHANNEL0; i < Buttons::BT_ENTER_WORLD; i++) {
        buttons_[i]->set_state(Button::State::NORMAL);
    }

    buttons_[Buttons::BT_CHANNEL0]->set_state(Button::State::PRESSED);

    for (size_t i = 0; i < Buttons::BT_ENTER_WORLD - Buttons::BT_CHANNEL0;
         i++) {
        buttons_[Buttons::BT_CHANNEL0 + i]->set_active(false);
    }

    buttons_[Buttons::BT_ENTER_WORLD]->set_active(false);
}

uint16_t UIWorldSelect::get_next_world(uint16_t id, bool upward) {
    uint16_t next_world;

    if (world_map_[Buttons::BT_WORLD0] == Worlds::SCANIA) {
        switch (id) {
            case Buttons::BT_WORLD0:
                next_world = (upward) ? Worlds::REBOOT1 : Worlds::BERA;
                break;
            case Buttons::BT_WORLD1:
                next_world = (upward) ? Worlds::SCANIA : Worlds::AURORA;
                break;
            case Buttons::BT_WORLD2:
                next_world = (upward) ? Worlds::BERA : Worlds::ELYSIUM1;
                break;
            case Buttons::BT_WORLD3:
                next_world = (upward) ? Worlds::AURORA : Worlds::REBOOT1;
                break;
            case Buttons::BT_WORLD4:
                next_world = (upward) ? Worlds::ELYSIUM1 : Worlds::SCANIA;
                break;
            default: break;
        }
    } else {
        switch (id) {
            case Buttons::BT_WORLD0:
                next_world = (upward) ? Worlds::REBOOT1 : Worlds::REBOOT1;
                break;
            case Buttons::BT_WORLD4:
                next_world = (upward) ? Worlds::SCANIA : Worlds::SCANIA;
                break;
            default: break;
        }
    }

    auto world = world_map_.begin();

    while (world != world_map_.end()) {
        if (world->second == next_world) {
            return world->first;
        }

        world++;
    }

    return Worlds::SCANIA;
}
}  // namespace ms