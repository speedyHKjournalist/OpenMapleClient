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
#include "UILoginNotice.h"

#include <nlnx/nx.hpp>
#include <utility>

#include "../Audio/Audio.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "KeyAction.h"

namespace ms {
UILoginNotice::UILoginNotice(uint16_t message,
                             std::function<void()> okhandler,
                             std::function<void()> cancelhandler) :
    saveid_(false),
    multiple_(false),
    okhandler_(std::move(okhandler)),
    cancel_handler_(std::move(cancelhandler)) {
    nl::node Notice = nl::nx::ui["Login.img"]["Notice"];
    nl::node backgrnd;

    switch (message) {
        case Message::NAME_IN_USE:
        case Message::ILLEGAL_NAME:
        case Message::BLOCKED_ID:
        case Message::INCORRECT_PIC: backgrnd = Notice["backgrnd"]["1"]; break;
        default: backgrnd = Notice["backgrnd"]["0"]; break;
    }

    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(Notice["text"][message], Point<int16_t>(17, 13));

    if (message == Message::DELETE_CONFIRMATION) {
        multiple_ = true;

        buttons_[Buttons::YES] =
            std::make_unique<MapleButton>(Notice["BtYes"],
                                          Point<int16_t>(70, 106));
        buttons_[Buttons::NO] =
            std::make_unique<MapleButton>(Notice["BtNo"],
                                          Point<int16_t>(130, 106));
    } else {
        buttons_[Buttons::YES] =
            std::make_unique<MapleButton>(Notice["BtYes"],
                                          Point<int16_t>(100, 106));
    }

    position_ = Point<int16_t>(275, 209);
    dimension_ = Texture(backgrnd).get_dimensions();
}

UILoginNotice::UILoginNotice(uint16_t message,
                             std::function<void()> okhandler) :
    UILoginNotice(message, std::move(okhandler), []() {}) {}

UILoginNotice::UILoginNotice(uint16_t message) :
    UILoginNotice(message, []() {}) {}

void UILoginNotice::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            if (!multiple_) {
                okhandler_();
            } else {
                cancel_handler_();
            }

            deactivate();
        } else if (keycode == KeyAction::RETURN) {
            okhandler_();
            deactivate();
        }
    }
}

UIElement::Type UILoginNotice::get_type() const {
    return TYPE;
}

Button::State UILoginNotice::button_pressed(uint16_t buttonid) {
    if (buttonid == Buttons::YES) {
        okhandler_();
    } else if (buttonid == Buttons::NO) {
        cancel_handler_();
    }

    deactivate();

    return Button::State::NORMAL;
}

UIQuitConfirm::UIQuitConfirm() {
    nl::node notice = nl::nx::ui["Login.img"]["Notice"];
    nl::node backgrnd = notice["backgrnd"]["0"];

    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(notice["text"][UILoginNotice::Message::CONFIRM_EXIT],
                          Point<int16_t>(17, 13));

    buttons_[Buttons::BT_OK] =
        std::make_unique<MapleButton>(notice["BtYes"], Point<int16_t>(70, 106));
    buttons_[Buttons::BT_CANCEL] =
        std::make_unique<MapleButton>(notice["BtNo"], Point<int16_t>(130, 106));

    position_ = Point<int16_t>(275, 209);
    dimension_ = Texture(backgrnd).get_dimensions();
}

void UIQuitConfirm::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            deactivate();
        } else if (keycode == KeyAction::RETURN) {
            UI::get().quit();
            deactivate();
        }
    }
}

UIElement::Type UIQuitConfirm::get_type() const {
    return TYPE;
}

Button::State UIQuitConfirm::button_pressed(uint16_t buttonid) {
    if (buttonid == BT_OK) {
        UI::get().quit();
    }

    deactivate();

    return Button::PRESSED;
}

UIClassConfirm::UIClassConfirm(uint8_t selected_class,
                               bool unavailable,
                               std::function<void()> okhandler) :
    okhandler_(std::move(okhandler)) {
    nl::node RaceSelect = nl::nx::ui["Login.img"]["RaceSelect_new"];
    nl::node type = unavailable ? RaceSelect["deny"] : RaceSelect["confirm"];
    nl::node backgrnd = type["backgrnd"];
    nl::node race = type["race"][selected_class];

    Point<int16_t> race_pos = Point<int16_t>(0, 0);

    // TODO: Calculate position based on width of race?
    switch (selected_class) {
        case Classes::RESISTANCE: race_pos = Point<int16_t>(57, 10); break;
        case Classes::EXPLORER: race_pos = Point<int16_t>(71, 10); break;
        case Classes::CYGNUSKNIGHTS: race_pos = Point<int16_t>(32, 10); break;
        case Classes::ARAN: race_pos = Point<int16_t>(95, 10); break;
        case Classes::EVAN: race_pos = Point<int16_t>(93, 10); break;
        case Classes::MERCEDES: race_pos = Point<int16_t>(65, 10); break;
        case Classes::DEMON: race_pos = Point<int16_t>(85, 10); break;
        case Classes::PHANTOM: race_pos = Point<int16_t>(71, 10); break;
        case Classes::DUALBLADE: race_pos = Point<int16_t>(54, 10); break;
        case Classes::MIHILE: race_pos = Point<int16_t>(85, 10); break;
        case Classes::LUMINOUS: race_pos = Point<int16_t>(66, 10); break;
        case Classes::KAISER: race_pos = Point<int16_t>(87, 10); break;
        case Classes::ANGELICBUSTER: race_pos = Point<int16_t>(41, 10); break;
        case Classes::CANNONEER: race_pos = Point<int16_t>(57, 10); break;
        case Classes::XENON: race_pos = Point<int16_t>(88, 10); break;
        case Classes::ZERO: break;
        case Classes::SHADE: race_pos = Point<int16_t>(86, 10); break;
        case Classes::JETT: race_pos = Point<int16_t>(101, 10); break;
        case Classes::HAYATO: race_pos = Point<int16_t>(81, 10); break;
        case Classes::KANNA: race_pos = Point<int16_t>(86, 10); break;
        case Classes::CHASE:
        case Classes::PINKBEAN: break;
        case Classes::KINESIS: race_pos = Point<int16_t>(84, 10); break;
        case Classes::CADENA: race_pos = Point<int16_t>(77, 10); break;
        case Classes::ILLIUM: race_pos = Point<int16_t>(92, 10); break;
        case Classes::ARK: race_pos = Point<int16_t>(100, 10); break;
        default: break;
    }

    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(race, race_pos + (Point<int16_t>)race["origin"]);

    if (unavailable) {
        buttons_[Buttons::OK] = std::make_unique<MapleButton>(type["BtOK"]);
    } else {
        buttons_[Buttons::OK] =
            std::make_unique<MapleButton>(type["BtOK"],
                                          Point<int16_t>(62, 107));
        buttons_[Buttons::CANCEL] =
            std::make_unique<MapleButton>(type["BtCancel"],
                                          Point<int16_t>(137, 107));
    }

    position_ = Point<int16_t>(286, 189);
    dimension_ = Texture(backgrnd).get_dimensions();
}

Cursor::State UIClassConfirm::send_cursor(bool clicked,
                                          Point<int16_t> cursorpos) {
    for (auto &[btnid, button] : buttons_) {
        if (button->is_active()
            && button->bounds(position_).contains(cursorpos)) {
            if (button->get_state() == Button::State::NORMAL) {
                Sound(Sound::Name::BUTTON_OVER).play();

                button->set_state(Button::State::MOUSEOVER);
            } else if (button->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    Sound(Sound::Name::BUTTON_CLICK).play();

                    button->set_state(button_pressed(btnid));
                }
            }
        } else if (button->get_state() == Button::State::MOUSEOVER) {
            button->set_state(Button::State::NORMAL);
        }
    }

    return Cursor::State::LEAF;
}

void UIClassConfirm::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            deactivate();
        } else if (keycode == KeyAction::Id::RETURN) {
            button_pressed(Buttons::OK);
        }
    }
}

UIElement::Type UIClassConfirm::get_type() const {
    return TYPE;
}

Button::State UIClassConfirm::button_pressed(uint16_t buttonid) {
    deactivate();

    if (buttonid == Buttons::OK) {
        okhandler_();
    }

    return Button::State::NORMAL;
}

UIKeySelect::UIKeySelect(std::function<void(bool)> oh, bool l) :
    okhandler_(std::move(oh)),
    login_(l) {
    nl::node KeyType = nl::nx::ui["UIWindow2.img"]["KeyConfig"]["KeyType"];
    nl::node backgrnd = KeyType["backgrnd"];

    sprites_.emplace_back(backgrnd);

    buttons_[Buttons::CLOSE] =
        std::make_unique<MapleButton>(KeyType["btClose"]);
    buttons_[Buttons::TYPEA] =
        std::make_unique<MapleButton>(KeyType["btTypeA"]);
    buttons_[Buttons::TYPEB] =
        std::make_unique<MapleButton>(KeyType["btTypeB"], Point<int16_t>(1, 1));

    if (login_) {
        buttons_[Buttons::CLOSE]->set_active(false);
    }

    position_ = Point<int16_t>(181, 145);
    dimension_ = Texture(backgrnd).get_dimensions();
}

void UIKeySelect::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && !login_) {
        if (escape || keycode == KeyAction::Id::RETURN) {
            deactivate();
        }
    }
}

UIElement::Type UIKeySelect::get_type() const {
    return TYPE;
}

Button::State UIKeySelect::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        default:
        case Buttons::CLOSE: deactivate(); break;
        case Buttons::TYPEA:
        case Buttons::TYPEB: {
            bool alternate = buttonid != Buttons::TYPEA;

            if (alternate) {
                buttons_[Buttons::TYPEA]->set_state(Button::State::DISABLED);
            } else {
                buttons_[Buttons::TYPEB]->set_state(Button::State::DISABLED);
            }

            auto onok = [&, alternate]() {
                okhandler_(alternate);
                deactivate();
            };

            UI::get().emplace<UIKeyConfirm>(alternate, onok, login_);
            break;
        }
    }

    return Button::State::DISABLED;
}

UIKeyConfirm::UIKeyConfirm(bool alternate, std::function<void()> oh, bool l) :
    okhandler_(std::move(oh)),
    login_(l) {
    nl::node alert =
        nl::nx::ui["UIWindow2.img"]["KeyConfig"]["KeyType"]["alert"];
    nl::node background = alternate ? alert["alternate"] : alert["default"];

    sprites_.emplace_back(background);

    buttons_[Buttons::OK] = std::make_unique<MapleButton>(alert["btOk"]);

    position_ = Point<int16_t>(276, 229);
    dimension_ = Texture(background).get_dimensions();
}

void UIKeyConfirm::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (keycode == KeyAction::Id::RETURN) {
            confirm();
        } else if (!login_ && escape) {
            deactivate();

            UI::get().remove(UIElement::Type::LOGIN_NOTICE);
        }
    }
}

UIElement::Type UIKeyConfirm::get_type() const {
    return TYPE;
}

Button::State UIKeyConfirm::button_pressed(uint16_t buttonid) {
    confirm();

    return Button::State::NORMAL;
}

void UIKeyConfirm::confirm() {
    okhandler_();
    deactivate();

    UI::get().remove(UIElement::Type::LOGIN_NOTICE);
}
}  // namespace ms