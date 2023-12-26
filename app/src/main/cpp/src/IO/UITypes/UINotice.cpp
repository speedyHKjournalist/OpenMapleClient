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
#include "UINotice.h"

#include <nlnx/nx.hpp>
#include <utility>

#include "../Audio/Audio.h"
#include "../Components/MapleButton.h"
#include "../UI.h"

namespace ms {
UINotice::UINotice(const std::string &message,
                   NoticeType t,
                   Text::Alignment a) :
    UIDragElement<PosNOTICE>(),
    type_(t),
    alignment_(a) {
    nl::node src = nl::nx::ui["Basic.img"]["Notice6"];

    top_ = src["t"];
    center_ = src["c"];
    centerbox_ = src["c_box"];
    box_ = src["box"];
    box2_ = src["box2"];
    bottom_ = src["s"];
    bottombox_ = src["s_box"];

    if (type_ == NoticeType::YESNO) {
        position_.shift_y(-8);
        question_ = Text(Text::Font::A11M,
                         alignment_,
                         Color::Name::WHITE,
                         message,
                         200);
    } else if (type_ == NoticeType::ENTER_NUMBER) {
        position_.shift_y(-16);
        question_ = Text(Text::Font::A12M,
                         Text::Alignment::LEFT,
                         Color::Name::WHITE,
                         message,
                         200);
    } else if (type_ == NoticeType::OK) {
        uint16_t maxwidth = top_.width() - 6;

        position_.shift_y(-8);
        question_ = Text(Text::Font::A11M,
                         Text::Alignment::CENTER,
                         Color::Name::WHITE,
                         message,
                         maxwidth);
    }

    height_ = question_.height();
    dimension_ = Point<int16_t>(top_.width(),
                                top_.height() + height_ + bottom_.height());
    position_ = Point<int16_t>(position_.x() - dimension_.x() / 2,
                               position_.y() - dimension_.y() / 2);
    drag_area_ = Point<int16_t>(dimension_.x(), 20);

    if (type_ != NoticeType::ENTER_NUMBER) {
        Sound(Sound::Name::DLG_NOTICE).play();
    }
}

UINotice::UINotice(std::string message, NoticeType t) :
    UINotice(std::move(message), t, Text::Alignment::CENTER) {}

void UINotice::draw(bool textfield) const {
    Point<int16_t> start = position_;

    top_.draw(start);
    start.shift_y(top_.height());

    if (textfield) {
        center_.draw(start);
        start.shift_y(center_.height());
        centerbox_.draw(start);
        start.shift_y(centerbox_.height() - 1);
        box2_.draw(start);
        start.shift_y(box2_.height());
        box_.draw(DrawArgument(start, Point<int16_t>(0, 29)));
        start.shift_y(29);

        question_.draw(position_ + Point<int16_t>(13, 13));
    } else {
        int16_t pos_y = height_ >= 32 ? height_ : 32;

        center_.draw(DrawArgument(start, Point<int16_t>(0, pos_y)));
        start.shift_y(pos_y);
        centerbox_.draw(start);
        start.shift_y(centerbox_.height());
        box_.draw(start);
        start.shift_y(box_.height());

        if (type_ == NoticeType::YESNO && alignment_ == Text::Alignment::LEFT) {
            question_.draw(position_ + Point<int16_t>(31, 14));
        } else {
            question_.draw(position_ + Point<int16_t>(131, 14));
        }
    }

    bottombox_.draw(start);
}

int16_t UINotice::box2offset(bool textfield) const {
    int16_t offset = top_.height() + centerbox_.height() + box_.height()
                     + height_ - (textfield ? 0 : 16);

    if (type_ == NoticeType::OK) {
        if (height_ < 34) {
            offset += 15;
        }
    }

    return offset;
}

UIYesNo::UIYesNo(std::string message,
                 std::function<void(bool yes)> yh,
                 Text::Alignment alignment) :
    UINotice(std::move(message), NoticeType::YESNO, alignment) {
    yesno_handler_ = std::move(yh);

    int16_t belowtext = box2offset(false);

    nl::node src = nl::nx::ui["Basic.img"];

    buttons_[Buttons::YES] =
        std::make_unique<MapleButton>(src["BtOK4"],
                                      Point<int16_t>(156, belowtext));
    buttons_[Buttons::NO] =
        std::make_unique<MapleButton>(src["BtCancel4"],
                                      Point<int16_t>(198, belowtext));
}

UIYesNo::UIYesNo(std::string message,
                 std::function<void(bool yes)> yesnohandler) :
    UIYesNo(std::move(message),
            std::move(yesnohandler),
            Text::Alignment::CENTER) {}

void UIYesNo::draw(float alpha) const {
    UINotice::draw(false);
    UIElement::draw(alpha);
}

void UIYesNo::send_key(int32_t keycode, bool pressed, bool escape) {
    if (keycode == KeyAction::Id::RETURN) {
        yesno_handler_(true);
        deactivate();
    } else if (escape) {
        yesno_handler_(false);
        deactivate();
    }
}

UIElement::Type UIYesNo::get_type() const {
    return TYPE;
}

Button::State UIYesNo::button_pressed(uint16_t buttonid) {
    deactivate();

    switch (buttonid) {
        case Buttons::YES: yesno_handler_(true); break;
        case Buttons::NO: yesno_handler_(false); break;
    }

    return Button::State::PRESSED;
}

UIEnterNumber::UIEnterNumber(std::string message,
                             std::function<void(int32_t)> nh,
                             int32_t m,
                             int32_t quantity) :
    UINotice(std::move(message), NoticeType::ENTER_NUMBER) {
    numhandler = std::move(nh);
    max = m;

    int16_t belowtext = box2offset(true) - 21;
    int16_t pos_y = belowtext + 35;

    nl::node src = nl::nx::ui["Basic.img"];

    buttons_[Buttons::OK] =
        std::make_unique<MapleButton>(src["BtOK4"], 156, pos_y);
    buttons_[Buttons::CANCEL] =
        std::make_unique<MapleButton>(src["BtCancel4"], 198, pos_y);

    numfield = Textfield(Text::Font::A11M,
                         Text::Alignment::LEFT,
                         Color::Name::LIGHTGREY,
                         Rectangle<int16_t>(24, 232, belowtext, belowtext + 20),
                         10);
    numfield.change_text(std::to_string(quantity));

    numfield.set_enter_callback(
        [&](std::string numstr) { handlestring(std::move(numstr)); });

    numfield.set_key_callback(KeyAction::Id::ESCAPE, [&]() { deactivate(); });

    numfield.set_state(Textfield::State::FOCUSED);
}

void UIEnterNumber::draw(float alpha) const {
    UINotice::draw(true);
    UIElement::draw(alpha);

    numfield.draw(position_);
}

void UIEnterNumber::update() {
    UIElement::update();

    numfield.update(position_);
}

Cursor::State UIEnterNumber::send_cursor(bool clicked,
                                         Point<int16_t> cursorpos) {
    if (numfield.get_state() == Textfield::State::NORMAL) {
        Cursor::State nstate = numfield.send_cursor(cursorpos, clicked);

        if (nstate != Cursor::State::IDLE) {
            return nstate;
        }
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

void UIEnterNumber::send_key(int32_t keycode, bool pressed, bool escape) {
    if (keycode == KeyAction::Id::RETURN) {
        handlestring(numfield.get_text());
        deactivate();
    } else if (escape) {
        deactivate();
    }
}

UIElement::Type UIEnterNumber::get_type() const {
    return TYPE;
}

Button::State UIEnterNumber::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::OK: handlestring(numfield.get_text()); break;
        case Buttons::CANCEL: deactivate(); break;
    }

    return Button::State::NORMAL;
}

void UIEnterNumber::handlestring(const std::string &numstr) {
    int num = -1;
    bool has_only_digits =
        (numstr.find_first_not_of("0123456789") == std::string::npos);

    auto okhandler = [&](bool) {
        numfield.set_state(Textfield::State::FOCUSED);
        buttons_[Buttons::OK]->set_state(Button::State::NORMAL);
    };

    if (!has_only_digits) {
        numfield.set_state(Textfield::State::DISABLED);
        UI::get().emplace<UIOk>("Only numbers are allowed.", okhandler);
        return;
    }
    num = std::stoi(numstr);

    if (num < 1) {
        numfield.set_state(Textfield::State::DISABLED);
        UI::get().emplace<UIOk>(
            "You may only enter a number equal to or higher than 1.",
            okhandler);
        return;
    }
    if (num > max) {
        numfield.set_state(Textfield::State::DISABLED);
        UI::get().emplace<UIOk>(
            "You may only enter a number equal to or lower than "
                + std::to_string(max) + ".",
            okhandler);
        return;
    }
    numhandler(num);
    deactivate();

    buttons_[Buttons::OK]->set_state(Button::State::NORMAL);
}

UIOk::UIOk(std::string message, std::function<void(bool ok)> oh) :
    UINotice(std::move(message), NoticeType::OK) {
    okhandler_ = std::move(oh);

    nl::node src = nl::nx::ui["Basic.img"];

    buttons_[Buttons::OK] =
        std::make_unique<MapleButton>(src["BtOK4"], 197, box2offset(false));
}

void UIOk::draw(float alpha) const {
    UINotice::draw(false);
    UIElement::draw(alpha);
}

void UIOk::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (keycode == KeyAction::Id::RETURN) {
            okhandler_(true);
            deactivate();
        } else if (escape) {
            okhandler_(false);
            deactivate();
        }
    }
}

UIElement::Type UIOk::get_type() const {
    return TYPE;
}

Button::State UIOk::button_pressed(uint16_t buttonid) {
    deactivate();

    switch (buttonid) {
        case Buttons::OK: okhandler_(true); break;
    }

    return Button::State::NORMAL;
}
}  // namespace ms