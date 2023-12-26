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
#include "Slider.h"

#include <nlnx/nx.hpp>
#include <utility>

namespace ms {
Slider::Slider(int32_t t,
               Range<int16_t> ver,
               int16_t xp,
               int16_t ur,
               int16_t rm,
               std::function<void(bool)> om) :
    onmoved_(std::move(om)),
    vertical_(ver),
    type_(t),
    x_(xp) {
    start_ = Point<int16_t>(x_, vertical_.first());
    end_ = Point<int16_t>(x_, vertical_.second());

    nl::node src;
    std::string base_str = "base";

    if (type_ == Type::CHAT_BAR) {
        src = nl::nx::ui["StatusBar3.img"]["chat"]["common"]["scroll"];
        base_str += "_c";
    } else {
        std::string VScr = "VScr";

        if (type_ != Type::LINE_CYAN) {
            VScr += std::to_string(type_);
        }

        src = nl::nx::ui["Basic.img"][VScr];
    }

    nl::node dsrc = src["disabled"];

    dbase_ = dsrc[base_str];

    dprev_ = dsrc["prev"];
    dnext_ = dsrc["next"];

    nl::node esrc = src["enabled"];

    base_ = esrc[base_str];

    prev_ = TwoSpriteButton(esrc["prev0"], esrc["prev1"], start_);
    next_ = TwoSpriteButton(esrc["next0"], esrc["next1"], end_);

    thumb_ = TwoSpriteButton(esrc["thumb0"], esrc["thumb1"]);

    button_height_ = dnext_.get_dimensions().y();

    setrows(ur, rm);

    enabled_ = true;
    scrolling_ = false;
}

Slider::Slider() : Slider(0, {}, 0, 0, 0, {}) {}

bool Slider::isenabled() const {
    return enabled_;
}

void Slider::setenabled(bool en) {
    enabled_ = en;
}

void Slider::setrows(int16_t nr, int16_t ur, int16_t rm) {
    row_max_ = rm - ur;

    if (row_max_ > 0) {
        row_height_ = (vertical_.length() - button_height_ * 2) / row_max_;
    } else {
        row_height_ = 0;
    }

    row_ = nr;
}

void Slider::setrows(int16_t ur, int16_t rm) {
    setrows(0, ur, rm);
}

void Slider::setvertical(Range<int16_t> ver) {
    vertical_ = ver;
    start_ = Point<int16_t>(x_, vertical_.first());
    end_ = Point<int16_t>(x_, vertical_.second());
    prev_.set_position(start_);
    next_.set_position(end_);

    if (row_max_ > 0) {
        row_height_ = (vertical_.length() - button_height_ * 2) / row_max_;
    } else {
        row_height_ = 0;
    }
}

Range<int16_t> Slider::getvertical() const {
    return vertical_;
}

void Slider::draw(Point<int16_t> position) const {
    Point<int16_t> base_pos = position + start_;
    Point<int16_t> fill =
        Point<int16_t>(0, vertical_.length() + button_height_ - 2);
    DrawArgument base_arg =
        DrawArgument(Point<int16_t>(base_pos.x(), base_pos.y() + 1), fill);

    int16_t height = dbase_.height();
    int16_t maxheight = vertical_.first() + height;

    while (maxheight < vertical_.second()) {
        dbase_.draw(position + Point<int16_t>(start_.x(), maxheight));

        maxheight += height;
    }

    if (enabled_) {
        if (row_height_ > 0) {
            prev_.draw(position);
            next_.draw(position);
            thumb_.draw(position + getthumbpos());
        } else {
            dprev_.draw(position + start_);
            dnext_.draw(position + end_);
        }
    } else {
        dprev_.draw(position + start_);
        dnext_.draw(position + end_);
    }
}

void Slider::remove_cursor() {
    scrolling_ = false;

    thumb_.set_state(Button::State::NORMAL);
    next_.set_state(Button::State::NORMAL);
    prev_.set_state(Button::State::NORMAL);
}

Point<int16_t> Slider::getthumbpos() const {
    int16_t y = row_ < row_max_
                    ? vertical_.first() + row_ * row_height_ + button_height_
                    : vertical_.second() - button_height_ * 2 - 2;

    return Point<int16_t>(x_, y);
}

Cursor::State Slider::send_cursor(Point<int16_t> cursor, bool pressed) {
    Point<int16_t> relative = cursor - start_;

    if (scrolling_) {
        if (pressed) {
            int16_t thumby = row_ * row_height_ + button_height_ * 2;
            int16_t delta = relative.y() - thumby;

            if (delta > row_height_ / 2 && row_ < row_max_) {
                row_++;
                onmoved_(false);
            } else if (delta < -row_height_ / 2 && row_ > 0) {
                row_--;
                onmoved_(true);
            }

            return Cursor::State::VSCROLL_IDLE;
        }

        scrolling_ = false;

    } else if (relative.x() < 0 || relative.y() < 0 || relative.x() > 8
               || relative.y() > vertical_.second()) {
        thumb_.set_state(Button::State::NORMAL);
        next_.set_state(Button::State::NORMAL);
        prev_.set_state(Button::State::NORMAL);

        return Cursor::State::IDLE;
    }

    Point<int16_t> thumbpos = getthumbpos();

    if (thumb_.bounds(thumbpos).contains(cursor)) {
        if (pressed) {
            scrolling_ = true;
            thumb_.set_state(Button::State::PRESSED);

            return Cursor::State::VSCROLL_IDLE;
        }

        thumb_.set_state(Button::State::NORMAL);

        return Cursor::State::VSCROLL;
    }

    thumb_.set_state(Button::State::NORMAL);

    if (prev_.bounds(Point<int16_t>()).contains(cursor)) {
        if (pressed) {
            if (row_ > 0) {
                row_--;
                onmoved_(true);
            }

            prev_.set_state(Button::State::PRESSED);

            return Cursor::State::VSCROLL_IDLE;
        }

        prev_.set_state(Button::State::MOUSEOVER);

        return Cursor::State::VSCROLL;
    }

    prev_.set_state(Button::State::NORMAL);

    if (next_.bounds(Point<int16_t>()).contains(cursor)) {
        if (pressed) {
            if (row_ < row_max_) {
                row_++;
                onmoved_(false);
            }

            next_.set_state(Button::State::PRESSED);

            return Cursor::State::VSCROLL_IDLE;
        }

        next_.set_state(Button::State::MOUSEOVER);

        return Cursor::State::VSCROLL;
    }

    next_.set_state(Button::State::NORMAL);

    if (cursor.y() < vertical_.second()) {
        if (pressed) {
            auto yoffset =
                static_cast<double>(relative.y() - button_height_ * 2);
            auto cursorrow =
                static_cast<int16_t>(std::round(yoffset / row_height_));

            if (cursorrow < 0) {
                cursorrow = 0;
            } else if (cursorrow > row_max_) {
                cursorrow = row_max_;
            }

            int16_t delta = row_ - cursorrow;

            for (size_t i = 0; i < 2; i++) {
                if (delta > 0) {
                    row_--;
                    delta--;
                    onmoved_(true);
                }

                if (delta < 0) {
                    row_++;
                    delta++;
                    onmoved_(false);
                }
            }

            return Cursor::State::VSCROLL_IDLE;
        }
    }

    return Cursor::State::VSCROLL;
}

void Slider::send_scroll(double yoffset) {
    if (yoffset < 0 && row_ < row_max_) {
        row_++;
        onmoved_(false);
    }

    if (yoffset > 0 && row_ > 0) {
        row_--;
        onmoved_(true);
    }
}
}  // namespace ms