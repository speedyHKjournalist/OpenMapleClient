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
#include "UIStateLogin.h"

#include "../Configuration.h"
#include "UITypes/Login/UICharSelect.h"
#include "UITypes/Login/UILogin.h"
#include "UITypes/Login/UILoginNotice.h"
#include "UITypes/Login/UILogo.h"
#include "UITypes/Login/UIRegion.h"

namespace ms {
UIStateLogin::UIStateLogin() {
    focused_ = UIElement::Type::NONE;

    bool start_shown = Configuration::get().get_start_shown();

    if (!start_shown) {
        emplace<UILogo>();
    } else {
        emplace<UILogin>();
    }
}

void UIStateLogin::draw(float inter, Point<int16_t> cursor) const {
    for (auto iter : elements_) {
        UIElement *element = iter.second.get();

        if (element && element->is_active()) {
            element->draw(inter);
        }
    }

    if (tooltip_) {
        tooltip_->get().draw(cursor + Point<int16_t>(0, 22));
    }
}

void UIStateLogin::update() {
    for (auto &elem : uis_to_remove_) {
        elem.reset(nullptr);
    }

    for (auto iter : elements_) {
        UIElement *element = iter.second.get();

        if (element && element->is_active()) {
            element->update();
        }
    }
}

void UIStateLogin::doubleclick(Point<int16_t> pos) {
    if (auto charselect = UI::get().get_element<UICharSelect>()) {
        charselect->get().doubleclick(pos);
    }
}

void UIStateLogin::send_key(KeyType::Id type,
                            int32_t action,
                            bool pressed,
                            bool escape) {
    if (UIElement *focusedelement = get(focused_)) {
        if (focusedelement->is_active()) {
            return focusedelement->send_key(action, pressed, escape);
        }

        focused_ = UIElement::Type::NONE;
        return;
    }
}

Cursor::State UIStateLogin::send_cursor(Cursor::State cursorstate,
                                        Point<int16_t> cursorpos) {
    bool clicked = cursorstate == Cursor::State::CLICKING
                   || cursorstate == Cursor::State::VSCROLL_IDLE;

    if (auto *focusedelement = get(focused_)) {
        if (focusedelement->is_active()) {
            remove_cursor(focusedelement->get_type());
            return focusedelement->send_cursor(clicked, cursorpos);
        }

        focused_ = UIElement::Type::NONE;
        return cursorstate;
    }

    if (auto *front = get_front()) {
        remove_cursor(front->get_type());
        return front->send_cursor(clicked, cursorpos);
    }

    return Cursor::State::IDLE;
}

void UIStateLogin::send_close() {
    auto logo = UI::get().get_element<UILogo>();
    auto login = UI::get().get_element<UILogin>();
    auto region = UI::get().get_element<UIRegion>();

    if ((logo && logo->get().is_active()) || (login && login->get().is_active())
        || (region && region->get().is_active())) {
        UI::get().quit();
    } else {
        UI::get().emplace<UIQuitConfirm>();
    }
}

void UIStateLogin::clear_tooltip(Tooltip::Parent parent) {
    if (parent == tooltip_parent_) {
        te_tooltip_.set_text("");
        tooltip_ = {};
        tooltip_parent_ = Tooltip::Parent::NONE;
    }
}

void UIStateLogin::show_text(Tooltip::Parent parent, std::string text) {
    te_tooltip_.set_text(text);

    if (!text.empty()) {
        tooltip_ = te_tooltip_;
        tooltip_parent_ = parent;
    }
}

template<class T, typename... Args>
void UIStateLogin::emplace(Args &&... args) {
    if (auto iter = pre_add(T::TYPE, T::TOGGLED, T::FOCUSED)) {
        (*iter).second = std::make_unique<T>(std::forward<Args>(args)...);
    }
}

UIState::Iterator UIStateLogin::pre_add(UIElement::Type type,
                                        bool,
                                        bool is_focused) {
    remove(type);

    if (is_focused) {
        focused_ = type;
    }

    return elements_.find(type);
}

void UIStateLogin::remove(UIElement::Type type) {
    if (focused_ == type) {
        focused_ = UIElement::Type::NONE;
    }

    if (auto &element = elements_[type]) {
        element->deactivate();
        uis_to_remove_.emplace_back(std::move(element));
    }
}

UIElement *UIStateLogin::get(UIElement::Type type) {
    return elements_[type].get();
}

UIElement *UIStateLogin::get_front() {
    UIElement *front = nullptr;

    for (auto iter : elements_) {
        auto &element = iter.second;

        if (element && element->is_active()) {
            front = element.get();
        }
    }

    return front;
}

UIElement *UIStateLogin::get_front(std::list<UIElement::Type> types) {
    auto begin = types.rbegin();
    auto end = types.rend();

    for (auto iter = begin; iter != end; ++iter) {
        auto &element = elements_[*iter];

        if (element && element->is_active()) {
            return element.get();
        }
    }

    return nullptr;
}
// TODO: (rich) fix

UIElement *UIStateLogin::get_front(Point<int16_t> pos) {
    auto begin = elements_.values().rbegin();
    auto end = elements_.values().rend();

    for (auto iter = begin; iter != end; ++iter) {
        auto &element = *iter;

        if (element && element->is_active() && element->is_in_range(pos)) {
            return element.get();
        }
    }

    return nullptr;
}

void UIStateLogin::remove_cursor(UIElement::Type type) {
    for (auto iter : elements_) {
        auto &element = iter.second;

        if (element && element->is_active() && element->get_type() != type) {
            element->remove_cursor();
        }
    }
}
}  // namespace ms