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
#include "UIStateGame.h"

#include <functional>
#include <optional>

#include "../Net/Packets/GameplayPackets.h"
#include "OptionalCreator.h"
#include "UI.h"
#include "UITypes/UIBuffList.h"
#include "UITypes/UIChannel.h"
#include "UITypes/UICharInfo.h"
#include "UITypes/UIChat.h"
#include "UITypes/UIChatBar.h"
#include "UITypes/UIEquipInventory.h"
#include "UITypes/UIEvent.h"
#include "UITypes/UIItemInventory.h"
#include "UITypes/UIKeyConfig.h"
#include "UITypes/UIMiniMap.h"
#include "UITypes/UIQuestLog.h"
#include "UITypes/UIQuit.h"
#include "UITypes/UIShop.h"
#include "UITypes/UISkillBook.h"
#include "UITypes/UIStatsInfo.h"
#include "UITypes/UIStatusBar.h"
#include "UITypes/UIStatusMessenger.h"
#include "UITypes/UIUserList.h"
#include "UITypes/UIWorldMap.h"

namespace ms {
namespace {
auto fn_enter_cashshop = []<typename... T>(T && ...args) {
    EnterCashShopPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIStateGame::UIStateGame(uint8_t channel_count) :
    stats_(Stage::get().get_player().get_stats()),
    dragged_(nullptr),
    channel_count_(channel_count) {
    focused_ = UIElement::Type::NONE;
    tooltip_parent_ = Tooltip::Parent::NONE;

    const CharLook &look = Stage::get().get_player().get_look();
    const Inventory &inventory = Stage::get().get_player().get_inventory();

    emplace<UIStatusMessenger>();
    emplace<UIStatusBar>(stats_, channel_count_);
    emplace<UIChatBar>();
    emplace<UIMiniMap>(stats_);
    emplace<UIBuffList>();
    emplace<UIShop>(look, inventory);

    VWIDTH = Constants::Constants::get().get_viewwidth();
    VHEIGHT = Constants::Constants::get().get_viewheight();
}

void UIStateGame::draw(float inter, Point<int16_t> cursor) const {
    for (const auto &type : element_order_) {
        const auto &element = elements_[type];

        if (element && element->is_active()) {
            element->draw(inter);
        }
    }

    if (tooltip_) {
        tooltip_->get().draw(cursor + Point<int16_t>(0, 22));
    }

    if (dragged_icon_) {
        dragged_icon_->get().dragdraw(cursor);
    }
}

void UIStateGame::update() {
    bool update_screen = false;
    int16_t new_width = Constants::Constants::get().get_viewwidth();
    int16_t new_height = Constants::Constants::get().get_viewheight();

    if (VWIDTH != new_width || VHEIGHT != new_height) {
        update_screen = true;
        VWIDTH = new_width;
        VHEIGHT = new_height;

        UI::get().remove(UIElement::Type::STATUS_BAR);

        const CharStats &stats = Stage::get().get_player().get_stats();
        emplace<UIStatusBar>(stats, channel_count_);
    }

    for (auto &type : element_order_) {
        auto &element = elements_[type];

        if (element && element->is_active()) {
            element->update();

            if (update_screen) {
                element->update_screen(new_width, new_height);
            }
        }
    }
}

bool UIStateGame::drop_icon(const Icon &icon, Point<int16_t> pos) {
    if (UIElement *front = get_front(pos)) {
        return front->send_icon(icon, pos);
    }
    icon.drop_on_stage();

    return true;
}

void UIStateGame::remove_icon() {
    dragged_icon_->get().reset();
    dragged_icon_ = {};
}

void UIStateGame::remove_cursors() {
    for (auto type : element_order_) {
        auto *element = elements_[type].get();

        if (element && element->is_active()) {
            element->remove_cursor();
        }
    }
}

void UIStateGame::remove_cursor(UIElement::Type t) {
    for (auto type : element_order_) {
        auto *element = elements_[type].get();

        if (element && element->is_active() && element->get_type() != t) {
            element->remove_cursor();
        }
    }
}

void UIStateGame::doubleclick(Point<int16_t> pos) {
    if (UIElement *front = get_front(pos)) {
        front->doubleclick(pos);
    } else {
        Stage::get().doubleclick(pos);
    }
}

void UIStateGame::rightclick(Point<int16_t> pos) {
    if (UIElement *front = get_front(pos)) {
        front->rightclick(pos);
    }
}

void UIStateGame::send_key(KeyType::Id type,
                           int32_t action,
                           bool pressed,
                           bool escape) {
    if (UIElement *focusedelement = get(focused_)) {
        if (focusedelement->is_active()) {
            return focusedelement->send_key(action, pressed, escape);
        }
        focused_ = UIElement::NONE;

        return;
    }
    switch (type) {
        case KeyType::Id::MENU:
            if (pressed) {
                switch (action) {
                    case KeyAction::Id::EQUIPMENT:
                        emplace<UIEquipInventory>(
                            Stage::get().get_player().get_inventory());
                        break;
                    case KeyAction::Id::ITEMS:
                        emplace<UIItemInventory>(
                            Stage::get().get_player().get_inventory());
                        break;
                    case KeyAction::Id::STATS:
                        emplace<UIStatsInfo>(
                            Stage::get().get_player().get_stats());
                        break;
                    case KeyAction::Id::SKILLS:
                        emplace<UISkillBook>(
                            Stage::get().get_player().get_stats(),
                            Stage::get().get_player().get_skills());
                        break;
                    case KeyAction::Id::FRIENDS:
                    case KeyAction::Id::PARTY:
                    case KeyAction::Id::BOSS_PARTY: {
                        UIUserList::Tab tab;

                        switch (action) {
                            case KeyAction::Id::FRIENDS:
                                tab = UIUserList::Tab::FRIEND;
                                break;
                            case KeyAction::Id::PARTY:
                                tab = UIUserList::Tab::PARTY;
                                break;
                            case KeyAction::Id::BOSS_PARTY:
                                tab = UIUserList::Tab::BOSS;
                                break;
                        }

                        auto userlist = UI::get().get_element<UIUserList>();

                        if (userlist && userlist->get().get_tab() != tab
                            && userlist->get().is_active()) {
                            userlist->get().change_tab(tab);
                        } else {
                            emplace<UIUserList>(tab);

                            if (userlist && userlist->get().get_tab() != tab
                                && userlist->get().is_active()) {
                                userlist->get().change_tab(tab);
                            }
                        }
                    } break;
                    case KeyAction::Id::WORLD_MAP: emplace<UIWorldMap>(); break;
                    case KeyAction::Id::MAPLE_CHAT: {
                        auto chat = UI::get().get_element<UIChat>();

                        if (!chat || !chat->get().is_active()) {
                            emplace<UIChat>();
                        }
                    } break;
                    case KeyAction::Id::MINI_MAP:
                        if (auto minimap = UI::get().get_element<UIMiniMap>()) {
                            minimap->get().send_key(action, pressed, escape);
                        }

                        break;
                    case KeyAction::Id::QUEST_LOG:
                        emplace<UIQuestLog>(
                            Stage::get().get_player().get_quests());
                        break;
                    case KeyAction::Id::MENU:
                        if (auto statusbar =
                                UI::get().get_element<UIStatusBar>()) {
                            statusbar->get().toggle_menu();
                        }

                        break;
                    case KeyAction::Id::QUICK_SLOTS:
                        if (auto statusbar =
                                UI::get().get_element<UIStatusBar>()) {
                            statusbar->get().toggle_qs();
                        }

                        break;
                    case KeyAction::Id::CASHSHOP: fn_enter_cashshop(); break;
                    case KeyAction::Id::TOGGLE_CHAT:
                        if (auto chatbar = UI::get().get_element<UIChatBar>()) {
                            if (!chatbar->get().is_chatfield_open()) {
                                chatbar->get().toggle_chat();
                            }
                        }

                        break;
                    case KeyAction::Id::KEY_BINDINGS: {
                        auto keyconfig = UI::get().get_element<UIKeyConfig>();

                        if (!keyconfig || !keyconfig->get().is_active()) {
                            emplace<UIKeyConfig>(
                                Stage::get().get_player().get_inventory(),
                                Stage::get().get_player().get_skills());
                        } else if (keyconfig && keyconfig->get().is_active()) {
                            keyconfig->get().close();
                        }

                        break;
                    }
                    case KeyAction::Id::MAIN_MENU:
                        if (auto statusbar =
                                UI::get().get_element<UIStatusBar>()) {
                            statusbar->get().send_key(action, pressed, escape);
                        }

                        break;
                    case KeyAction::Id::EVENT: emplace<UIEvent>(); break;
                    case KeyAction::Id::CHANGE_CHANNEL:
                        emplace<UIChannel>(
                            Stage::get().get_player().get_world_id(),
                            Stage::get().get_player().get_channel_id(),
                            channel_count_);
                        break;
                    default:
                        std::cout << "Action (" << action << ") not handled!"
                                  << std::endl;
                        break;
                }
            }
            break;
        case KeyType::Id::ACTION:
        case KeyType::Id::FACE:
        case KeyType::Id::ITEM:
        case KeyType::Id::SKILL:
            Stage::get().send_key(type, action, pressed);
            break;
    }
}

Cursor::State UIStateGame::send_cursor(Cursor::State cursorstate,
                                       Point<int16_t> cursorpos) {
    if (dragged_icon_) {
        if (cursorstate == Cursor::State::CLICKING) {
            if (drop_icon(*dragged_icon_, cursorpos)) {
                remove_icon();
                time_rel_grabbed = std::chrono::steady_clock::now();
            }
            return cursorstate;
        }
        return Cursor::State::GRABBING;
    }
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
    if (!clicked) {
        dragged_ = nullptr;

        if (auto *front = get_front(cursorpos)) {
            UIElement::Type front_type = front->get_type();

            if (tooltip_parent_ != UIElement::Type::NONE) {
                if (front_type != tooltip_parent_) {
                    clear_tooltip(tooltip_parent_);
                }
            }

            remove_cursor(front_type);
            return front->send_cursor(clicked, cursorpos);
        }
        remove_cursors();
        return Stage::get().send_cursor(clicked, cursorpos);
    }
    if (!dragged_) {
        UIElement::Type drag_element_type = UIElement::Type::NONE;

        for (auto iter = element_order_.rbegin(); iter != element_order_.rend();
             ++iter) {
            auto &element = elements_[*iter];

            if (element && element->is_active()
                && element->is_in_range(cursorpos)) {
                dragged_ = element.get();
                drag_element_type = *iter;
                break;
            }
        }

        if (drag_element_type != UIElement::Type::NONE) {
            element_order_.remove(drag_element_type);
            element_order_.push_back(drag_element_type);
        }
    }

    // fixes bug with icon in keyconfig being re-grabbed after
    // assign
    if (auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - time_rel_grabbed);
        duration < MIN_DELAY_NEXT_GRAB_) {
        return Cursor::State::IDLE;
    }

    if (dragged_) {
        return dragged_->send_cursor(clicked, cursorpos);
    }

    return Stage::get().send_cursor(clicked, cursorpos);
}

void UIStateGame::send_scroll(double yoffset) {
    for (auto &type : element_order_) {
        auto &element = elements_[type];

        if (element && element->is_active()) {
            element->send_scroll(yoffset);
        }
    }
}

void UIStateGame::send_close() {
    UI::get().emplace<UIQuit>(stats_);
}

void UIStateGame::drag_icon(Icon *drgic) {
    dragged_icon_ = create_optional<Icon>(drgic);
}

void UIStateGame::clear_tooltip(Tooltip::Parent parent) {
    if (parent == tooltip_parent_) {
        eq_tooltip_.set_equip(Tooltip::Parent::NONE, 0);
        it_tooltip_.set_item(0);
        te_tooltip_.set_text("");
        ma_tooltip_.reset();
        tooltip_ = {};
        tooltip_parent_ = Tooltip::Parent::NONE;
    }
}

void UIStateGame::show_equip(Tooltip::Parent parent, int16_t slot) {
    eq_tooltip_.set_equip(parent, slot);

    if (slot) {
        tooltip_ = eq_tooltip_;
        tooltip_parent_ = parent;
    }
}

void UIStateGame::show_item(Tooltip::Parent parent, int32_t itemid) {
    it_tooltip_.set_item(itemid);

    if (itemid) {
        tooltip_ = it_tooltip_;
        tooltip_parent_ = parent;
    }
}

void UIStateGame::show_skill(Tooltip::Parent parent,
                             int32_t skill_id,
                             int32_t level,
                             int32_t masterlevel,
                             int64_t expiration) {
    sk_tooltip_.set_skill(skill_id, level, masterlevel, expiration);

    if (skill_id) {
        tooltip_ = sk_tooltip_;
        tooltip_parent_ = parent;
    }
}

void UIStateGame::show_text(Tooltip::Parent parent, std::string text) {
    te_tooltip_.set_text(text);

    if (!text.empty()) {
        tooltip_ = te_tooltip_;
        tooltip_parent_ = parent;
    }
}

void UIStateGame::show_map(Tooltip::Parent parent,
                           std::string name,
                           std::string description,
                           int32_t mapid,
                           bool bolded) {
    ma_tooltip_.set_name(parent, name, bolded);
    ma_tooltip_.set_desc(description);
    ma_tooltip_.set_mapid(mapid);

    if (!name.empty()) {
        tooltip_ = ma_tooltip_;
        tooltip_parent_ = parent;
    }
}

template<class T, typename... Args>
void UIStateGame::emplace(Args &&...args) {
    if (auto iter = pre_add(T::TYPE, T::TOGGLED, T::FOCUSED)) {
        (*iter).second = std::make_unique<T>(std::forward<Args>(args)...);

        auto silent_types = { UIElement::Type::STATUS_MESSENGER,
                              UIElement::Type::STATUS_BAR,
                              UIElement::Type::CHAT_BAR,
                              UIElement::Type::MINI_MAP,
                              UIElement::Type::BUFF_LIST,
                              UIElement::Type::NPC_TALK,
                              UIElement::Type::SHOP };

        if (std::find(silent_types.begin(), silent_types.end(), T::TYPE)
            == silent_types.end()) {
            if (T::TYPE == UIElement::Type::WORLD_MAP) {
                Sound(Sound::Name::WORLD_MAP_OPEN).play();
            } else {
                Sound(Sound::Name::MENU_UP).play();
            }

            UI::get().send_cursor(false);
        }
    }
}

UIState::Iterator UIStateGame::pre_add(UIElement::Type type,
                                       bool is_toggled,
                                       bool is_focused) {
    auto &element = elements_[type];

    if (element && is_toggled) {
        element_order_.remove(type);
        element_order_.push_back(type);

        bool active = element->is_active();

        element->toggle_active();

        if (active != element->is_active()) {
            if (element->is_active()) {
                if (type == UIElement::Type::WORLD_MAP) {
                    Sound(Sound::Name::WORLD_MAP_OPEN).play();
                } else {
                    Sound(Sound::Name::MENU_UP).play();
                }

                UI::get().send_cursor(false);
            } else {
                if (type == UIElement::Type::WORLD_MAP) {
                    Sound(Sound::Name::WORLD_MAP_CLOSE).play();
                } else {
                    Sound(Sound::Name::MENU_DOWN).play();
                }

                element->remove_cursor();

                if (dragged_icon_) {
                    if (element->get_type()
                        == icon_map_[dragged_icon_->get().get_type()]) {
                        remove_icon();
                    }
                }

                UI::get().send_cursor(false);
            }
        }

        return elements_.end();
    }
    remove(type);
    element_order_.push_back(type);

    if (is_focused) {
        focused_ = type;
    }

    return elements_.find(type);
}

void UIStateGame::remove(UIElement::Type type) {
    if (type == focused_) {
        focused_ = UIElement::Type::NONE;
    }

    if (type == tooltip_parent_) {
        clear_tooltip(tooltip_parent_);
    }

    element_order_.remove(type);

    if (auto &element = elements_[type]) {
        element->deactivate();
        element.release();
    }
}

UIElement *UIStateGame::get(UIElement::Type type) {
    return elements_[type].get();
}

UIElement *UIStateGame::get_front(std::list<UIElement::Type> types) {
    auto begin = element_order_.rbegin();
    auto end = element_order_.rend();

    for (auto iter = begin; iter != end; ++iter) {
        if (std::find(types.begin(), types.end(), *iter) != types.end()) {
            auto &element = elements_[*iter];

            if (element && element->is_active()) {
                return element.get();
            }
        }
    }

    return nullptr;
}
// TODO: (rich) fix

UIElement *UIStateGame::get_front(Point<int16_t> pos) {
    auto begin = element_order_.rbegin();
    auto end = element_order_.rend();

    for (auto iter = begin; iter != end; ++iter) {
        auto &element = elements_[*iter];

        if (element && element->is_active() && element->is_in_range(pos)) {
            return element.get();
        }
    }

    return nullptr;
}
}  // namespace ms