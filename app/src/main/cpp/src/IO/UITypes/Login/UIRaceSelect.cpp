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
#include "UIRaceSelect.h"

#include <cctype>
#include <nlnx/nx.hpp>

#include "../Audio/Audio.h"
#include "../Components/AreaButton.h"
#include "../Components/MapleButton.h"
#include "../Configuration.h"
#include "../UI.h"
#include "UIAranCreation.h"
#include "UICharSelect.h"
#include "UICygnusCreation.h"
#include "UIExplorerCreation.h"
#include "UILoginNotice.h"

namespace ms {
UIRaceSelect::UIRaceSelect() :
    UIElement(Point<int16_t>(0, 0), Point<int16_t>(800, 600)) {
    std::string version_text = Configuration::get().get_version();
    version_ = Text(Text::Font::A11M,
                    Text::Alignment::LEFT,
                    Color::Name::LEMONGRASS,
                    "Ver. " + version_text);

    nl::node Login = nl::nx::ui["Login.img"];
    nl::node Common = Login["Common"];
    nl::node RaceSelect = Login["RaceSelect_new"];

    Point<int16_t> make_pos = RaceSelect["make"]["pos"];
    Point<int16_t> make_posZero = RaceSelect["make"]["posZero"];

    pos_ = Point<int16_t>(std::abs(make_pos.x()), std::abs(make_pos.y()));
    pos_zero_ =
        Point<int16_t>(std::abs(make_posZero.x()), std::abs(make_posZero.y()));

    uint8_t selected_list = 1;

    order_ = RaceSelect["order"][selected_list];
    hotlist_ = RaceSelect["hotList"][selected_list];
    newlist_ = RaceSelect["newList"][selected_list];
    bgm_ = RaceSelect["bgm"];

    hotlabel_ = RaceSelect["hotLabel"];
    hotlabel_zero_ = RaceSelect["hotLabel2"];
    newlabel_ = RaceSelect["newLabel"];
    hotbtn_ = RaceSelect["hot"];
    newbtn_ = RaceSelect["new"];

    class_index_[0] = order_[0];
    class_index_[1] = order_[1];
    class_index_[2] = order_[2];
    class_index_[3] = order_[3];
    class_index_[4] = order_[4];
    mouseover_[0] = true;
    mouseover_[1] = false;
    mouseover_[2] = false;
    mouseover_[3] = false;
    mouseover_[4] = false;

    class_isdisabled_[Classes::RESISTANCE] = true;
    class_isdisabled_[Classes::EXPLORER] = false;
    class_isdisabled_[Classes::CYGNUSKNIGHTS] = false;
    class_isdisabled_[Classes::ARAN] = false;
    class_isdisabled_[Classes::EVAN] = true;
    class_isdisabled_[Classes::MERCEDES] = true;
    class_isdisabled_[Classes::DEMON] = true;
    class_isdisabled_[Classes::PHANTOM] = true;
    class_isdisabled_[Classes::DUALBLADE] = true;
    class_isdisabled_[Classes::MIHILE] = true;
    class_isdisabled_[Classes::LUMINOUS] = true;
    class_isdisabled_[Classes::KAISER] = true;
    class_isdisabled_[Classes::ANGELICBUSTER] = true;
    class_isdisabled_[Classes::CANNONEER] = true;
    class_isdisabled_[Classes::XENON] = true;
    class_isdisabled_[Classes::ZERO] = true;
    class_isdisabled_[Classes::SHADE] = true;
    class_isdisabled_[Classes::JETT] = true;
    class_isdisabled_[Classes::HAYATO] = true;
    class_isdisabled_[Classes::KANNA] = true;
    class_isdisabled_[Classes::CHASE] = true;
    class_isdisabled_[Classes::PINKBEAN] = true;
    class_isdisabled_[Classes::KINESIS] = true;
    class_isdisabled_[Classes::CADENA] = true;
    class_isdisabled_[Classes::ILLIUM] = true;
    class_isdisabled_[Classes::ARK] = true;

    sprites_.emplace_back(Common["frame"], Point<int16_t>(400, 300));

    back_ = RaceSelect["Back"]["1"]["0"];
    back_zero_ = RaceSelect["Back"]["2"]["0"];
    back_ani_ = RaceSelect["BackAni"];
    class_details_background_ = RaceSelect["Back1"]["0"]["0"];
    class_details_background_zero_ = RaceSelect["Back1"]["1"]["0"];

    for (size_t i = 0; i < CLASS_COUNT_; i++) {
        class_normal_[i][false] = RaceSelect["button"][i]["normal"]["0"];
        class_normal_[i][true] = RaceSelect["button"][i]["mouseOver"]["0"];

        class_disabled_[i][false] =
            RaceSelect["buttonDisabled"][i]["normal"]["0"];
        class_disabled_[i][true] =
            RaceSelect["buttonDisabled"][i]["mouseOver"]["0"];

        class_background_[i] = RaceSelect["Back0"][i]["0"];
        class_details_[i] = RaceSelect["Back2"][i]["0"];
        class_title_[i] = RaceSelect["Back3"][i]["0"];
    }

    buttons_[Buttons::BACK] =
        std::make_unique<MapleButton>(Common["BtStart"],
                                      Point<int16_t>(0, 515));
    buttons_[Buttons::MAKE] = std::make_unique<MapleButton>(RaceSelect["make"]);
    buttons_[Buttons::LEFT] =
        std::make_unique<MapleButton>(RaceSelect["leftArrow"],
                                      Point<int16_t>(41, 458));
    buttons_[Buttons::RIGHT] =
        std::make_unique<MapleButton>(RaceSelect["rightArrow"],
                                      Point<int16_t>(718, 458));

    for (size_t i = 0; i <= Buttons::CLASS0; i++) {
        buttons_[Buttons::CLASS0 + i] = std::make_unique<AreaButton>(
            get_class_pos(i),
            class_normal_[0][true].get_dimensions());
    }

    index_shift_ = 0;
    selected_index_ = 0;
    selected_class_ = class_index_[selected_index_];

    buttons_[Buttons::LEFT]->set_state(Button::State::DISABLED);

    Sound(Sound::Name::RACE_SELECT).play();
}

void UIRaceSelect::draw(float inter) const {
    if (selected_class_ == Classes::ZERO) {
        back_zero_.draw(position_);
    } else {
        back_.draw(position_);
    }

    UIElement::draw_sprites(inter);

    version_.draw(position_ + Point<int16_t>(707, 1));

    if (selected_class_ == Classes::KANNA
        || selected_class_ == Classes::CHASE) {
        if (selected_class_ == Classes::ZERO) {
            class_details_background_zero_.draw(position_);
        } else {
            class_details_background_.draw(position_);
        }

        class_background_[selected_class_].draw(position_);
    } else {
        class_background_[selected_class_].draw(position_);

        if (selected_class_ == Classes::ZERO) {
            class_details_background_zero_.draw(position_);
        } else {
            class_details_background_.draw(position_);
        }
    }

    class_details_[selected_class_].draw(position_);
    class_title_[selected_class_].draw(position_);

    for (const auto &node : hotlist_) {
        if (node.get_integer() == selected_class_) {
            if (selected_class_ == Classes::ZERO) {
                hotlabel_zero_.draw(position_, inter);
            } else {
                hotlabel_.draw(position_, inter);
            }

            break;
        }
    }

    for (const auto &node : newlist_) {
        if (node.get_integer() == selected_class_) {
            newlabel_.draw(position_, inter);
            break;
        }
    }

    for (size_t i = 0; i < INDEX_COUNT_; i++) {
        Point<int16_t> button_pos = get_class_pos(i);

        class_isdisabled_[class_index_[i]]
            ? class_disabled_[class_index_[i]][mouseover_[i]].draw(position_
                                                                   + button_pos)
            : class_normal_[class_index_[i]][mouseover_[i]].draw(position_
                                                                 + button_pos);

        for (const auto &node : hotlist_) {
            if (node.get_integer() == class_index_[i]) {
                hotbtn_.draw(position_ + button_pos, inter);
                break;
            }
        }

        for (const auto &node : newlist_) {
            if (node.get_integer() == selected_class_) {
                newbtn_.draw(position_ + button_pos, inter);
                break;
            }
        }
    }

    UIElement::draw_buttons(inter);

    back_ani_.draw(position_, inter);
}

void UIRaceSelect::update() {
    UIElement::update();

    hotlabel_.update();
    hotlabel_zero_.update();
    newlabel_.update();
    hotbtn_.update();
    newbtn_.update();

    if (selected_class_ == Classes::ZERO) {
        buttons_[Buttons::MAKE]->set_position(position_ + pos_zero_);
    } else {
        buttons_[Buttons::MAKE]->set_position(position_ + pos_);
    }

    back_ani_.update();

    for (const auto &node : bgm_) {
        uint8_t name = std::stoi(node.name());

        if (name == selected_class_) {
            std::string found_bgm = node["bgm"];
            std::size_t found_img = found_bgm.find(".img");

            if (found_img == std::string::npos) {
                std::size_t found_slash = found_bgm.find('/');

                if (found_slash != std::string::npos) {
                    found_bgm.insert(found_slash, ".img");

                    Music(found_bgm).play();
                }
            } else {
                Music(found_bgm).play();
            }

            break;
        }

        Music("BgmUI.img/Title").play();
        break;
    }
}

Cursor::State UIRaceSelect::send_cursor(bool clicked,
                                        Point<int16_t> cursorpos) {
    for (auto &[btnid, button] : buttons_) {
        if (button->is_active()
            && button->bounds(position_).contains(cursorpos)) {
            if (button->get_state() == Button::State::NORMAL) {
                Sound(Sound::Name::BUTTON_OVER).play();

                if (btnid >= Buttons::CLASS0) {
                    mouseover_[btnid - Buttons::CLASS0] = true;
                }

                button->set_state(Button::State::MOUSEOVER);
            } else if (button->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

                    button->set_state(button_pressed(btnid));
                } else {
                    if (btnid >= Buttons::CLASS0) {
                        mouseover_[btnid - Buttons::CLASS0] = true;
                    }
                }
            }
        } else if (button->get_state() == Button::State::MOUSEOVER) {
            if (btnid >= Buttons::CLASS0) {
                mouseover_[btnid - Buttons::CLASS0] = false;
            }

            button->set_state(Button::State::NORMAL);
        }
    }

    return Cursor::State::LEAF;
}

void UIRaceSelect::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            show_charselect();
        } else if (keycode == KeyAction::Id::LEFT
                   || keycode == KeyAction::Id::DOWN) {
            if (buttons_[Buttons::LEFT]->get_state() == Button::State::NORMAL) {
                button_pressed(Buttons::LEFT);
            }
        } else if (keycode == KeyAction::Id::RIGHT
                   || keycode == KeyAction::Id::UP) {
            if (buttons_[Buttons::RIGHT]->get_state()
                == Button::State::NORMAL) {
                button_pressed(Buttons::RIGHT);
            }
        } else if (keycode == KeyAction::Id::RETURN) {
            button_pressed(Buttons::MAKE);
        }
    }
}

UIElement::Type UIRaceSelect::get_type() const {
    return TYPE;
}

bool UIRaceSelect::check_name(const std::string &name) const {
    nl::node ForbiddenName = nl::nx::etc["ForbiddenName.img"];

    for (const auto &forbiddenName : ForbiddenName) {
        std::string lName = to_lower(name);
        std::string fName = to_lower(forbiddenName);

        if (lName.find(fName) != std::string::npos) {
            return false;
        }
    }

    return true;
}

void UIRaceSelect::send_naming_result(bool nameused) {
    if (selected_class_ == Classes::EXPLORER) {
        if (auto explorercreation =
                UI::get().get_element<UIExplorerCreation>()) {
            explorercreation->get().send_naming_result(nameused);
        }
    } else if (selected_class_ == Classes::CYGNUSKNIGHTS) {
        if (auto cygnuscreation = UI::get().get_element<UICygnusCreation>()) {
            cygnuscreation->get().send_naming_result(nameused);
        }
    } else if (selected_class_ == Classes::ARAN) {
        if (auto arancreation = UI::get().get_element<UIAranCreation>()) {
            arancreation->get().send_naming_result(nameused);
        }
    }
}

Button::State UIRaceSelect::button_pressed(uint16_t buttonid) {
    if (buttonid == Buttons::BACK) {
        show_charselect();

        return Button::State::NORMAL;
    }

    if (buttonid == Buttons::MAKE) {
        auto okhandler = [&]() {
            if (!class_isdisabled_[selected_class_]) {
                Sound(Sound::Name::SCROLL_UP).play();

                deactivate();

                if (selected_class_ == Classes::EXPLORER) {
                    UI::get().emplace<UIExplorerCreation>();
                } else if (selected_class_ == Classes::CYGNUSKNIGHTS) {
                    UI::get().emplace<UICygnusCreation>();
                } else if (selected_class_ == Classes::ARAN) {
                    UI::get().emplace<UIAranCreation>();
                }
            }
        };

        UI::get().emplace<UIClassConfirm>(selected_class_,
                                          class_isdisabled_[selected_class_],
                                          okhandler);

        return Button::State::NORMAL;
    }

    if (buttonid == Buttons::LEFT) {
        uint8_t new_index = selected_index_ - 1;

        int size = sizeof(class_index_) / sizeof(class_index_[0]);

        int selected = class_index_[selected_index_ - index_shift_];
        auto *selected_itr =
            std::find(class_index_, class_index_ + size, selected);

        if (selected_itr != std::end(class_index_)) {
            auto button_index = std::distance(class_index_, selected_itr);

            if (button_index == 0) {
                index_shift_--;

                class_index_[0] = order_[new_index + 4 - Buttons::CLASS0];
                class_index_[1] = order_[new_index + 5 - Buttons::CLASS0];
                class_index_[2] = order_[new_index + 6 - Buttons::CLASS0];
                class_index_[3] = order_[new_index + 7 - Buttons::CLASS0];
                class_index_[4] = order_[new_index + 8 - Buttons::CLASS0];
            }
        }

        select_class(new_index);

        return Button::State::IDENTITY;
    }

    if (buttonid == Buttons::RIGHT) {
        uint8_t new_index = selected_index_ + 1;

        int size = sizeof(class_index_) / sizeof(class_index_[0]);

        int selected = class_index_[selected_index_ - index_shift_];
        auto *selected_itr =
            std::find(class_index_, class_index_ + size, selected);

        if (selected_itr != std::end(class_index_)) {
            auto button_index = std::distance(class_index_, selected_itr);

            if (button_index == 4) {
                index_shift_++;

                class_index_[0] = order_[new_index + 0 - Buttons::CLASS0];
                class_index_[1] = order_[new_index + 1 - Buttons::CLASS0];
                class_index_[2] = order_[new_index + 2 - Buttons::CLASS0];
                class_index_[3] = order_[new_index + 3 - Buttons::CLASS0];
                class_index_[4] = order_[new_index + 4 - Buttons::CLASS0];
            }
        }

        select_class(new_index);

        return Button::State::IDENTITY;
    }

    if (buttonid >= Buttons::CLASS0) {
        auto index = buttonid - Buttons::CLASS0 + index_shift_;

        select_class(index);

        return Button::State::IDENTITY;
    }

    return Button::State::NORMAL;
}

void UIRaceSelect::select_class(uint8_t index) {
    uint8_t previous_index = selected_index_;
    selected_index_ = index;

    if (previous_index != selected_index_) {
        Sound(Sound::Name::RACE_SELECT).play();

        int size = sizeof(class_index_) / sizeof(class_index_[0]);

        int previous = class_index_[previous_index - index_shift_];
        auto *previous_itr =
            std::find(class_index_, class_index_ + size, previous);

        if (previous_itr != std::end(class_index_)) {
            auto button_index = std::distance(class_index_, previous_itr);

            mouseover_[previous_index - index_shift_] = false;
            buttons_[button_index + Buttons::CLASS0]->set_state(
                Button::State::NORMAL);
        }

        int selected = class_index_[selected_index_ - index_shift_];
        auto *selected_itr =
            std::find(class_index_, class_index_ + size, selected);

        if (selected_itr != std::end(class_index_)) {
            auto button_index = std::distance(class_index_, selected_itr);

            selected_class_ = class_index_[button_index];
            mouseover_[selected_index_ - index_shift_] = true;
        }
    } else {
        button_pressed(Buttons::MAKE);
    }

    if (selected_index_ > 0) {
        buttons_[Buttons::LEFT]->set_state(Button::State::NORMAL);
    } else {
        buttons_[Buttons::LEFT]->set_state(Button::State::DISABLED);
    }

    if (selected_index_ < CLASS_COUNT_ - 2) {
        buttons_[Buttons::RIGHT]->set_state(Button::State::NORMAL);
    } else {
        buttons_[Buttons::RIGHT]->set_state(Button::State::DISABLED);
    }
}

void UIRaceSelect::show_charselect() {
    Sound(Sound::Name::SCROLL_UP).play();

    deactivate();

    if (auto charselect = UI::get().get_element<UICharSelect>()) {
        charselect->get().makeactive();
    }
}

Point<int16_t> UIRaceSelect::get_class_pos(size_t index) const {
    uint16_t x_adj = index * 126;

    return Point<int16_t>(95 + x_adj, 430);
}

std::string UIRaceSelect::to_lower(std::string value) const {
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);

    return value;
}
}  // namespace ms