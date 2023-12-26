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
#include "MapleComboBox.h"

#include <nlnx/nx.hpp>
#include <utility>

#include "../Audio/Audio.h"
#include "AreaButton.h"

namespace ms {
MapleComboBox::MapleComboBox(Type type,
                             std::vector<std::string> o,
                             uint16_t default_option,
                             Point<int16_t> ppos,
                             Point<int16_t> pos,
                             int64_t w) :
    options_(std::move(o)),
    rwidth_(w),
    selected_index_(default_option),
    parent_pos_(ppos) {
    std::string combobox = "ComboBox";

    if (type != Type::DEFAULT) {
        combobox += std::to_string(type);
    }

    nl::node src = nl::nx::ui["Basic.img"][combobox];

    textures_[Button::State::PRESSED][0] = src["pressed"][0];
    textures_[Button::State::PRESSED][1] = src["pressed"][1];
    textures_[Button::State::PRESSED][2] = src["pressed"][2];

    textures_[Button::State::MOUSEOVER][0] = src["mouseOver"][0];
    textures_[Button::State::MOUSEOVER][1] = src["mouseOver"][1];
    textures_[Button::State::MOUSEOVER][2] = src["mouseOver"][2];

    textures_[Button::State::NORMAL][0] = src["normal"][0];
    textures_[Button::State::NORMAL][1] = src["normal"][1];
    textures_[Button::State::NORMAL][2] = src["normal"][2];

    textures_[Button::State::DISABLED][0] = src["disabled"][0];
    textures_[Button::State::DISABLED][1] = src["disabled"][1];
    textures_[Button::State::DISABLED][2] = src["disabled"][2];

    for (auto &option : options_) {
        option_text_.push_back(Text(Text::Font::A12M,
                                    Text::Alignment::LEFT,
                                    Color::Name::BLACK,
                                    option));
    }

    Text::Font selected_font = Text::Font::A12M;
    Color::Name selected_color = Color::Name::BLACK;
    selected_adj_ = Point<int16_t>(2, -3);

    if (type == Type::BLACKL) {
        selected_font = Text::Font::A11M;
        selected_color = Color::Name::WHITE;
        selected_adj_ = Point<int16_t>(11, 2);
    }

    selected_ = Text(selected_font,
                     Text::Alignment::LEFT,
                     selected_color,
                     options_[selected_index_]);

    state_ = Button::State::NORMAL;
    background_ = ColorBox(width(),
                           options_.size() * HEIGHT,
                           Color::Name::DUSTYGRAY,
                           1.0f);
    rect_ = ColorBox(width() - 2,
                     options_.size() * HEIGHT - 2,
                     Color::Name::GALLERY,
                     1.0f);
    current_rect_ =
        ColorBox(width() - 2, HEIGHT - 2, Color::Name::GRAYOLIVE, 1.0f);

    Point<int16_t> option_pos =
        Point<int16_t>(
            position_.x(),
            position_.y() + textures_[state_][0].get_dimensions().y())
        + parent_pos_;

    for (size_t i = 0; i < option_text_.size(); i++) {
        buttons_[i] = std::make_unique<AreaButton>(
            Point<int16_t>(option_pos.x() + 1,
                           option_pos.y() + (i * HEIGHT) + 1),
            Point<int16_t>(width() - 2, HEIGHT - 2));
    }

    current_pos_ = 0;
    current_shown_ = false;
    last_shown_ = 0;

    position_ = pos;
    active_ = true;
    pressed_ = false;
}

void MapleComboBox::draw(Point<int16_t>) const {
    if (active_) {
        auto lpos = Point<int16_t>(position_ + parent_pos_);

        textures_[state_][0].draw(lpos);
        lpos.shift_x(textures_[state_][0].width());

        int16_t middle_width = textures_[state_][1].width();
        int16_t current_width = middle_width;

        while (current_width < rwidth_) {
            textures_[state_][1].draw(lpos);
            lpos.shift_x(middle_width);
            current_width += middle_width;
        }

        textures_[state_][2].draw(lpos);

        selected_.draw(Point<int16_t>(position_ + parent_pos_) + selected_adj_);

        if (pressed_) {
            Point<int16_t> pos =
                Point<int16_t>(
                    position_.x(),
                    position_.y() + textures_[state_][0].get_dimensions().y())
                + parent_pos_;

            background_.draw(pos + Point<int16_t>(0, 2));
            rect_.draw(pos + Point<int16_t>(1, 3));

            if (current_shown_) {
                current_rect_.draw(
                    DrawArgument(pos.x() + 1, pos.y() + current_pos_ + 3));
            }

            for (size_t i = 0; i < option_text_.size(); i++) {
                option_text_[i].draw(
                    DrawArgument(pos.x() + 6, pos.y() + (i * HEIGHT) - 4));
            }
        }
    }
}

Rectangle<int16_t> MapleComboBox::bounds(Point<int16_t>) const {
    auto lt = parent_pos_ + position_ - origin();
    auto rb = lt + textures_[state_][0].get_dimensions();

    auto end = textures_[state_][2].get_dimensions();

    rb = Point<int16_t>(rb.x() + end.x() + rwidth_, rb.y());

    return Rectangle<int16_t>(lt, rb);
}

int16_t MapleComboBox::width() const {
    return textures_[state_][0].width() + textures_[state_][2].width()
           + rwidth_;
}

Point<int16_t> MapleComboBox::origin() const {
    return textures_[state_][0].get_origin();
}

Cursor::State MapleComboBox::send_cursor(bool clicked,
                                         Point<int16_t> cursorpos) {
    Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

    current_shown_ = false;
    option_text_[last_shown_].change_color(Color::Name::BLACK);

    for (auto &[btnid, button] : buttons_) {
        if (button->is_active()
            && button->bounds(position_).contains(cursorpos)) {
            if (button->get_state() == Button::State::NORMAL) {
                Sound(Sound::Name::BUTTON_OVER).play();

                button->set_state(Button::State::MOUSEOVER);
                ret = Cursor::State::CAN_CLICK;
            } else if (button->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

                    button->set_state(button_pressed(btnid));

                    ret = Cursor::State::IDLE;
                } else {
                    ret = Cursor::State::CAN_CLICK;
                    current_pos_ = btnid * HEIGHT;
                    current_shown_ = true;
                    last_shown_ = btnid;
                    option_text_[btnid].change_color(Color::Name::WHITE);
                }
            }
        } else if (button->get_state() == Button::State::MOUSEOVER) {
            button->set_state(Button::State::NORMAL);
        }
    }

    return ret;
}

bool MapleComboBox::in_combobox(Point<int16_t> cursorpos) {
    Point<int16_t> lt =
        Point<int16_t>(
            position_.x() + 1,
            position_.y() + textures_[state_][0].get_dimensions().y() + 1)
        + parent_pos_;
    Point<int16_t> rb =
        lt + Point<int16_t>(width() - 2, options_.size() * HEIGHT - 2);

    return Rectangle<int16_t>(lt, rb).contains(cursorpos);
}

uint16_t MapleComboBox::get_selected() const {
    return selected_index_;
}

Button::State MapleComboBox::button_pressed(uint16_t buttonid) {
    selected_index_ = buttonid;

    selected_.change_text(options_[selected_index_]);

    toggle_pressed();

    return Button::State::NORMAL;
}
}  // namespace ms