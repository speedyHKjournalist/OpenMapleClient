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
#include "UIChannel.h"

#include <nlnx/nx.hpp>

#include "../../Gameplay/Stage.h"
#include "../Audio/Audio.h"
#include "../Components/AreaButton.h"
#include "../Components/MapleButton.h"
#include "../KeyAction.h"

namespace ms {
UIChannel::UIChannel(uint8_t wid, uint8_t ch, uint8_t ch_count) :
    UIDragElement<PosCHANNEL>(),
    current_channel_(ch),
    selected_channel_(ch),
    channel_count_(ch_count) {
    uint8_t selected_world = wid;

    nl::node Channel = nl::nx::ui["UIWindow2.img"]["Channel"];

    nl::node backgrnd = Channel["backgrnd"];
    Texture bg = backgrnd;

    sprites_.emplace_back(backgrnd, Point<int16_t>(1, 0));
    sprites_.emplace_back(Channel["backgrnd2"]);
    sprites_.emplace_back(Channel["backgrnd3"]);
    sprites_.emplace_back(Channel["world"][selected_world],
                          Point<int16_t>(16, 30));

    buttons_[Buttons::CANCEL] =
        std::make_unique<MapleButton>(Channel["BtCancel"]);
    buttons_[Buttons::CHANGE] =
        std::make_unique<MapleButton>(Channel["BtChange"],
                                      Point<int16_t>(-20, 0));

    channel_[true] = Channel["channel1"];
    channel_[false] = Channel["channel0"];

    size_t x = 0;
    size_t y = 0;

    for (size_t i = 0; i < channel_count_; i++) {
        if (x >= 5) {
            x = 0;
            y++;
        }

        ch_.emplace_back(Channel["ch"][i],
                         Point<int16_t>(19 + 70 * x, 60 + 20 * y));
        buttons_[Buttons::CH + i] = std::make_unique<AreaButton>(
            Point<int16_t>(11 + 70 * x, 55 + 20 * y),
            channel_[true].get_dimensions());

        if (i == selected_channel_) {
            current_channel_x_ = 11 + 70 * x;
            current_channel_y_ = 55 + 20 * y;
            selected_channel_x_ = current_channel_x_;
            selected_channel_y_ = current_channel_y_;
        }

        x++;
    }

    old_state_ = { current_channel_, current_channel_x_, current_channel_y_ };
    dimension_ = bg.get_dimensions();
    drag_area_ = Point<int16_t>(dimension_.x(), 20);
}

void UIChannel::draw(float inter) const {
    UIElement::draw(inter);

    if (current_channel_ == selected_channel_) {
        channel_[true].draw(DrawArgument(position_.x() + selected_channel_x_,
                                         position_.y() + selected_channel_y_));
    } else {
        channel_[true].draw(DrawArgument(position_.x() + selected_channel_x_,
                                         position_.y() + selected_channel_y_));
        channel_[false].draw(DrawArgument(position_.x() + current_channel_x_,
                                          position_.y() + current_channel_y_));
    }

    for (const auto &sprite : ch_) {
        sprite.draw(position_, inter);
    }
}

void UIChannel::update() {
    UIElement::update();

    for (auto &sprite : ch_) {
        sprite.update();
    }
}

void UIChannel::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            cancel();
        } else if (keycode == KeyAction::Id::RETURN) {
            change_channel();
        } else if (keycode == KeyAction::Id::UP) {
            if (selected_channel_ > 4) {
                selected_channel_ -= 5;
            } else {
                for (size_t i = 0; i < 3; i++) {
                    selected_channel_ += 5;
                }
            }

            if (selected_channel_ == current_channel_) {
                if (selected_channel_ > 4) {
                    selected_channel_ -= 5;
                } else {
                    for (size_t i = 0; i < 3; i++) {
                        selected_channel_ += 5;
                    }
                }
            }

            update_selected_channel_position();
        } else if (keycode == KeyAction::Id::DOWN) {
            if (selected_channel_ < 15) {
                selected_channel_ += 5;
            } else {
                for (size_t i = 0; i < 3; i++) {
                    selected_channel_ -= 5;
                }
            }

            if (selected_channel_ == current_channel_) {
                if (selected_channel_ < 15) {
                    selected_channel_ += 5;
                } else {
                    for (size_t i = 0; i < 3; i++) {
                        selected_channel_ -= 5;
                    }
                }
            }

            update_selected_channel_position();
        } else if (keycode == KeyAction::Id::LEFT) {
            if (selected_channel_ != 0) {
                selected_channel_--;
            } else {
                selected_channel_ = channel_count_ - 1;
            }

            if (selected_channel_ == current_channel_) {
                if (selected_channel_ != 0) {
                    selected_channel_--;
                } else {
                    selected_channel_ = channel_count_ - 1;
                }
            }

            update_selected_channel_position();
        } else if (keycode == KeyAction::Id::RIGHT) {
            if (selected_channel_ != channel_count_ - 1) {
                selected_channel_++;
            } else {
                selected_channel_ = 0;
            }

            if (selected_channel_ == current_channel_) {
                if (selected_channel_ != channel_count_ - 1) {
                    selected_channel_++;
                } else {
                    selected_channel_ = 0;
                }
            }

            update_selected_channel_position();
        }
    }
}

Cursor::State UIChannel::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Cursor::State dstate = UIDragElement::send_cursor(clicked, cursorpos);

    if (dragged_) {
        return dstate;
    }

    Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

    for (size_t i = 0; i < channel_count_ + Buttons::CH; i++) {
        if (buttons_[i]->is_active()
            && buttons_[i]->bounds(position_).contains(cursorpos)) {
            if (buttons_[i]->get_state() == Button::State::NORMAL) {
                if (i < Buttons::CH) {
                    Sound(Sound::Name::BUTTON_OVER).play();

                    buttons_[i]->set_state(Button::State::MOUSEOVER);
                    ret = Cursor::State::CAN_CLICK;
                } else {
                    buttons_[i]->set_state(Button::State::MOUSEOVER);
                    ret = Cursor::State::IDLE;
                }
            } else if (buttons_[i]->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    if (i < Buttons::CH) {
                        Sound(Sound::Name::BUTTON_CLICK).play();
                    }

                    buttons_[i]->set_state(button_pressed(i));

                    ret = Cursor::State::IDLE;
                } else {
                    if (i < Buttons::CH) {
                        ret = Cursor::State::CAN_CLICK;
                    } else {
                        ret = Cursor::State::IDLE;
                    }
                }
            }
        } else if (buttons_[i]->get_state() == Button::State::MOUSEOVER) {
            buttons_[i]->set_state(Button::State::NORMAL);
        }
    }

    return ret;
}

UIElement::Type UIChannel::get_type() const {
    return TYPE;
}

Button::State UIChannel::button_pressed(uint16_t buttonid) {
    if (buttonid < Buttons::CH) {
        switch (buttonid) {
            case Buttons::CANCEL: cancel(); break;
            case Buttons::CHANGE: change_channel(); break;
            default: break;
        }
    } else {
        if (buttonid - Buttons::CH == current_channel_) {
            return Button::State::NORMAL;
        }

        selected_channel_ = buttonid - Buttons::CH;
        update_selected_channel_position();
    }

    return Button::State::NORMAL;
}

void UIChannel::cancel() {
    deactivate();

    current_channel_ = old_state_.current_channel;
    selected_channel_ = old_state_.current_channel;
    selected_channel_x_ = old_state_.current_channel_x;
    selected_channel_y_ = old_state_.current_channel_y;
}

void UIChannel::change_channel() {
    deactivate();
    Stage::get().change_channel(selected_channel_);

    current_channel_ = selected_channel_;

    update_selected_channel_position();

    current_channel_x_ = selected_channel_x_;
    current_channel_y_ = selected_channel_y_;

    old_state_.current_channel = selected_channel_;
    old_state_.current_channel = selected_channel_;
    old_state_.current_channel_x = selected_channel_x_;
    old_state_.current_channel_y = selected_channel_y_;
}

void UIChannel::update_selected_channel_position() {
    size_t x = 0;
    size_t y = 0;

    for (size_t i = 0; i < channel_count_; i++) {
        if (x >= 5) {
            x = 0;
            y++;
        }

        if (i == selected_channel_) {
            selected_channel_x_ = 11 + 70 * x;
            selected_channel_y_ = 55 + 20 * y;
            break;
        }

        x++;
    }
}
}  // namespace ms