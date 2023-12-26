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
#include "UILogin.h"

#include "../../Audio/Audio.h"
#include "../../Configuration.h"
#include "../../Net/Packets/LoginPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "UILoginNotice.h"
#include "UILoginWait.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <nlnx/nx.hpp>

namespace ms {
namespace {
auto fn_login_start = []() { LoginStartPacket().dispatch(); };

auto fn_login = []<typename... T>(T && ...args) {
    LoginPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UILogin::UILogin() : UIElement(Point<int16_t>(0, 0), Point<int16_t>(800, 600)) {
    fn_login_start();

    Music("BgmUI.img/Title").play();

    std::string version_text = Configuration::get().get_version();
    version_ = Text(Text::Font::A11M,
                    Text::Alignment::LEFT,
                    Color::Name::LEMONGRASS,
                    "Ver. " + version_text);

    nl::node map = nl::nx::map001["Back"]["login.img"];
    nl::node back = map["back"];
    nl::node ani = map["ani"];

    nl::node obj = nl::nx::map["Obj"]["login.img"];
    nl::node title = nl::nx::ui["Login.img"]["Title"];
    nl::node common = nl::nx::ui["Login.img"]["Common"];

    nl::node prettyLogo = nl::nx::mapPretty["Back"]["login.img"]["ani"]["16"];

    sprites_.emplace_back(back["11"], Point<int16_t>(400, 300));
    sprites_.emplace_back(ani["17"], Point<int16_t>(129, 283));
    sprites_.emplace_back(ani["18"], Point<int16_t>(306, 252));
    sprites_.emplace_back(ani["19"], Point<int16_t>(379, 207));
    sprites_.emplace_back(back["35"], Point<int16_t>(399, 260));
    sprites_.emplace_back(prettyLogo, Point<int16_t>(394, 173));
    sprites_.emplace_back(title["signboard"], Point<int16_t>(391, 330));
    sprites_.emplace_back(common["frame"], Point<int16_t>(400, 300));

    buttons_[Buttons::BT_LOGIN] =
        std::make_unique<MapleButton>(title["BtLogin"],
                                      Point<int16_t>(454, 279));
    buttons_[Buttons::BT_SAVEID] =
        std::make_unique<MapleButton>(title["BtLoginIDSave"],
                                      Point<int16_t>(303, 332));
    buttons_[Buttons::BT_IDLOST] =
        std::make_unique<MapleButton>(title["BtLoginIDLost"],
                                      Point<int16_t>(375, 332));
    buttons_[Buttons::BT_PASSLOST] =
        std::make_unique<MapleButton>(title["BtPasswdLost"],
                                      Point<int16_t>(447, 332));
    buttons_[Buttons::BT_REGISTER] =
        std::make_unique<MapleButton>(title["BtNew"], Point<int16_t>(291, 352));
    buttons_[Buttons::BT_HOMEPAGE] =
        std::make_unique<MapleButton>(title["BtHomePage"],
                                      Point<int16_t>(363, 352));
    buttons_[Buttons::BT_QUIT] =
        std::make_unique<MapleButton>(title["BtQuit"],
                                      Point<int16_t>(435, 352));

    checkbox_[false] = title["check"]["0"];
    checkbox_[true] = title["check"]["1"];

    account_ = Textfield(
        Text::Font::A13M,
        Text::Alignment::LEFT,
        Color::Name::WHITE,
        Rectangle<int16_t>(Point<int16_t>(296, 279), Point<int16_t>(446, 303)),
        12);

    account_.set_key_callback(KeyAction::Id::TAB, [&] {
        account_.set_state(Textfield::State::NORMAL);
        password_.set_state(Textfield::State::FOCUSED);
    });

    account_.set_enter_callback(
        [&](const std::string & /*unused*/) { login(); });

    accountbg_ = title["ID"];

    password_ = Textfield(
        Text::Font::A13M,
        Text::Alignment::LEFT,
        Color::Name::WHITE,
        Rectangle<int16_t>(Point<int16_t>(296, 305), Point<int16_t>(446, 329)),
        12);

    password_.set_key_callback(KeyAction::Id::TAB, [&] {
        account_.set_state(Textfield::State::FOCUSED);
        password_.set_state(Textfield::State::NORMAL);
    });

    password_.set_enter_callback(
        [&](const std::string & /*unused*/) { login(); });

    password_.set_cryptchar('*');
    passwordbg_ = title["PW"];

    saveid_ = Setting<SaveLogin>::get().load();

    if (saveid_) {
        account_.change_text(Setting<DefaultAccount>::get().load());
        password_.set_state(Textfield::State::FOCUSED);
    } else {
        account_.set_state(Textfield::State::FOCUSED);
    }

    if (Configuration::get().get_auto_login()) {
        UI::get().emplace<UILoginWait>([]() {});

        auto loginwait = UI::get().get_element<UILoginWait>();

        if (loginwait && loginwait->get().is_active()) {
            fn_login(Configuration::get().get_auto_acc(),
                     Configuration::get().get_auto_pass());
        }
    }
}

void UILogin::draw(float alpha) const {
    UIElement::draw(alpha);

    version_.draw(position_ + Point<int16_t>(707, 1));
    account_.draw(position_);
    password_.draw(position_);

    if (account_.get_state() == Textfield::State::NORMAL && account_.empty()) {
        accountbg_.draw(DrawArgument(position_ + Point<int16_t>(291, 279)));
    }

    if (password_.get_state() == Textfield::State::NORMAL
        && password_.empty()) {
        passwordbg_.draw(DrawArgument(position_ + Point<int16_t>(291, 305)));
    }

    checkbox_[saveid_].draw(DrawArgument(position_ + Point<int16_t>(291, 335)));
}

void UILogin::update() {
    UIElement::update();

    account_.update(position_);
    password_.update(position_);
}

void UILogin::login() {
    account_.set_state(Textfield::State::DISABLED);
    password_.set_state(Textfield::State::DISABLED);

    std::string account_text = account_.get_text();
    std::string password_text = password_.get_text();

    std::function<void()> okhandler = [&, password_text]() {
        account_.set_state(Textfield::State::NORMAL);
        password_.set_state(Textfield::State::NORMAL);

        if (!password_text.empty()) {
            password_.set_state(Textfield::State::FOCUSED);
        } else {
            account_.set_state(Textfield::State::FOCUSED);
        }
    };

    if (account_text.empty()) {
        UI::get().emplace<UILoginNotice>(UILoginNotice::Message::NOT_REGISTERED,
                                         okhandler);
        return;
    }

    if (password_text.length() <= 4) {
        UI::get().emplace<UILoginNotice>(UILoginNotice::Message::WRONG_PASSWORD,
                                         okhandler);
        return;
    }

    UI::get().emplace<UILoginWait>(okhandler);

    auto loginwait = UI::get().get_element<UILoginWait>();

    if (loginwait && loginwait->get().is_active()) {
        fn_login(account_text, password_text);
    }
}

void UILogin::open_url(uint16_t id) {
    std::string url;

    switch (id) {
        case Buttons::BT_REGISTER:
            url = Configuration::get().get_joinlink();
            break;
        case Buttons::BT_HOMEPAGE:
            url = Configuration::get().get_website();
            break;
        case Buttons::BT_PASSLOST:
            url = Configuration::get().get_findpass();
            break;
        case Buttons::BT_IDLOST: url = Configuration::get().get_findid(); break;
        default: return;
    }

    // TODO: (rich) fix
    // ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

Button::State UILogin::button_pressed(uint16_t id) {
    switch (id) {
        case Buttons::BT_LOGIN: login(); return Button::State::NORMAL;
        case Buttons::BT_REGISTER:
        case Buttons::BT_HOMEPAGE:
        case Buttons::BT_PASSLOST:
        case Buttons::BT_IDLOST: open_url(id); return Button::State::NORMAL;
        case Buttons::BT_SAVEID:
            saveid_ = !saveid_;
            Setting<SaveLogin>::get().save(saveid_);

            return Button::State::MOUSEOVER;
        case Buttons::BT_QUIT: UI::get().quit(); return Button::State::PRESSED;
        default: return Button::State::NORMAL;
    }
}

Cursor::State UILogin::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    if (Cursor::State new_state = account_.send_cursor(cursorpos, clicked)) {
        return new_state;
    }

    if (Cursor::State new_state = password_.send_cursor(cursorpos, clicked)) {
        return new_state;
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

UIElement::Type UILogin::get_type() const {
    return TYPE;
}
}  // namespace ms