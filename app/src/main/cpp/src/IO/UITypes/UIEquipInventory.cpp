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
#include "UIEquipInventory.h"

#include <nlnx/nx.hpp>

#include "../../Audio/Audio.h"
#include "../../Data/ItemData.h"
#include "../../Net/Packets/InventoryPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "../UITypes/UIItemInventory.h"

namespace ms {
namespace {
auto fn_equip_item = []<typename... T>(T && ...args) {
    EquipItemPacket(std::forward<T>(args)...).dispatch();
};
auto fn_unequip_item = []<typename... T>(T && ...args) {
    UnequipItemPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIEquipInventory::UIEquipInventory(const Inventory &invent) :
    UIDragElement<PosEQINV>(),
    inventory_(invent),
    tab_(Buttons::BT_TAB1),
    has_pendant_slot_(false),
    has_pocket_slot_(false) {
    // Column 1
    icon_positions_[EquipSlot::Id::RING1] = Point<int16_t>(14, 50);
    icon_positions_[EquipSlot::Id::RING2] = Point<int16_t>(14, 91);
    icon_positions_[EquipSlot::Id::RING3] = Point<int16_t>(14, 132);
    icon_positions_[EquipSlot::Id::RING4] = Point<int16_t>(14, 173);
    icon_positions_[EquipSlot::Id::POCKET] = Point<int16_t>(14, 214);
    icon_positions_[EquipSlot::Id::BOOK] = Point<int16_t>(14, 255);

    // Column 2
    // icon_positions_[EquipSlot::Id::NONE] = Point<int16_t>(55, 50);
    icon_positions_[EquipSlot::Id::PENDANT2] = Point<int16_t>(55, 91);
    icon_positions_[EquipSlot::Id::PENDANT1] = Point<int16_t>(55, 132);
    icon_positions_[EquipSlot::Id::WEAPON] = Point<int16_t>(55, 173);
    icon_positions_[EquipSlot::Id::BELT] = Point<int16_t>(55, 214);
    // icon_positions_[EquipSlot::Id::NONE] = Point<int16_t>(55, 255);

    // Column 3
    icon_positions_[EquipSlot::Id::HAT] = Point<int16_t>(96, 50);
    icon_positions_[EquipSlot::Id::FACE] = Point<int16_t>(96, 91);
    icon_positions_[EquipSlot::Id::EYE_ACC] = Point<int16_t>(96, 132);
    icon_positions_[EquipSlot::Id::TOP] = Point<int16_t>(96, 173);
    icon_positions_[EquipSlot::Id::BOTTOM] = Point<int16_t>(96, 214);
    icon_positions_[EquipSlot::Id::SHOES] = Point<int16_t>(96, 255);

    // Column 4
    // icon_positions_[EquipSlot::Id::NONE] = Point<int16_t>(137, 50);
    // icon_positions_[EquipSlot::Id::NONE] = Point<int16_t>(137, 91);
    icon_positions_[EquipSlot::Id::EARACC] = Point<int16_t>(137, 132);
    icon_positions_[EquipSlot::Id::SHOULDER] = Point<int16_t>(137, 173);
    icon_positions_[EquipSlot::Id::GLOVES] = Point<int16_t>(137, 214);
    icon_positions_[EquipSlot::Id::ANDROID_] = Point<int16_t>(137, 255);

    // Column 5
    icon_positions_[EquipSlot::Id::EMBLEM] = Point<int16_t>(178, 50);
    icon_positions_[EquipSlot::Id::BADGE] = Point<int16_t>(178, 91);
    icon_positions_[EquipSlot::Id::MEDAL] = Point<int16_t>(178, 132);
    icon_positions_[EquipSlot::Id::SUBWEAPON] = Point<int16_t>(178, 173);
    icon_positions_[EquipSlot::Id::CAPE] = Point<int16_t>(178, 214);
    icon_positions_[EquipSlot::Id::HEART] = Point<int16_t>(178, 255);

    // icon_positions_[EquipSlot::Id::SHIELD] = Point<int16_t>(142, 124);
    // icon_positions_[EquipSlot::Id::TAMEDMOB] = Point<int16_t>(142, 91);
    // icon_positions_[EquipSlot::Id::SADDLE] = Point<int16_t>(76, 124);

    tab_source_[Buttons::BT_TAB0] = "Equip";
    tab_source_[Buttons::BT_TAB1] = "Cash";
    tab_source_[Buttons::BT_TAB2] = "Pet";
    tab_source_[Buttons::BT_TAB3] = "Android";

    nl::node close = nl::nx::ui["Basic.img"]["BtClose3"];
    nl::node Equip = nl::nx::ui["UIWindow4.img"]["Equip"];

    background_[Buttons::BT_TAB0] =
        Equip[tab_source_[Buttons::BT_TAB0]]["backgrnd"];
    background_[Buttons::BT_TAB1] =
        Equip[tab_source_[Buttons::BT_TAB1]]["backgrnd"];
    background_[Buttons::BT_TAB2] =
        Equip[tab_source_[Buttons::BT_TAB2]]["backgrnd"];
    background_[Buttons::BT_TAB3] =
        Equip[tab_source_[Buttons::BT_TAB3]]["backgrnd"];

    for (int i = Buttons::BT_TAB0; i < Buttons::BT_TABE; i++) {
        for (const auto &slot : Equip[tab_source_[i]]["Slots"]) {
            if (slot.name().find('_') == std::string::npos) {
                slots_[i].emplace_back(slot);
            }
        }
    }

    nl::node EquipGL = nl::nx::ui["UIWindowGL.img"]["Equip"];
    nl::node backgrnd = Equip["backgrnd"];
    nl::node totem_backgrnd = EquipGL["Totem"]["backgrnd"];

    Point<int16_t> bg_dimensions = Texture(backgrnd).get_dimensions();
    totem_dimensions_ = Texture(totem_backgrnd).get_dimensions();
    totem_adj_ = Point<int16_t>(-totem_dimensions_.x() + 4, 0);

    sprites_.emplace_back(totem_backgrnd, totem_adj_);
    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(Equip["backgrnd2"]);

    tab_bar_ = Equip["tabbar"];
    disabled_ = Equip[tab_source_[Buttons::BT_TAB0]]["disabled"];
    disabled2_ = Equip[tab_source_[Buttons::BT_TAB0]]["disabled2"];

    buttons_[Buttons::BT_CLOSE] = std::make_unique<MapleButton>(
        close,
        Point<int16_t>(bg_dimensions.x() - 19, 5));
    buttons_[Buttons::BT_SLOT] = std::make_unique<MapleButton>(
        Equip[tab_source_[Buttons::BT_TAB0]]["BtSlot"]);
    buttons_[Buttons::BT_EFFECT] =
        std::make_unique<MapleButton>(EquipGL["Equip"]["btEffect"]);
    buttons_[Buttons::BT_SALON] =
        std::make_unique<MapleButton>(EquipGL["Equip"]["btSalon"]);
    buttons_[Buttons::BT_CONSUMESETTING] = std::make_unique<MapleButton>(
        Equip[tab_source_[Buttons::BT_TAB2]]["BtConsumeSetting"]);
    buttons_[Buttons::BT_EXCEPTION] = std::make_unique<MapleButton>(
        Equip[tab_source_[Buttons::BT_TAB2]]["BtException"]);
    buttons_[Buttons::BT_SHOP] = std::make_unique<MapleButton>(
        Equip[tab_source_[Buttons::BT_TAB3]]["BtShop"]);

    buttons_[Buttons::BT_CONSUMESETTING]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_EXCEPTION]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_SHOP]->set_state(Button::State::DISABLED);

    nl::node Tab = Equip["Tab"];

    for (int i = Buttons::BT_TAB0; i < Buttons::BT_TABE; i++) {
        buttons_[Buttons::BT_TAB0 + i] =
            std::make_unique<TwoSpriteButton>(Tab["disabled"][i],
                                              Tab["enabled"][i],
                                              Point<int16_t>(0, 3));
    }

    dimension_ = bg_dimensions;
    drag_area_ = Point<int16_t>(bg_dimensions.x(), 20);

    load_icons();
    change_tab(Buttons::BT_TAB0);
}

void UIEquipInventory::draw(float alpha) const {
    UIElement::draw(alpha);

    background_[tab_].draw(position_);
    tab_bar_.draw(position_);

    for (auto slot : slots_[tab_]) {
        slot.draw(position_);
    }

    if (tab_ == Buttons::BT_TAB0) {
        if (!has_pendant_slot_) {
            disabled_.draw(position_
                           + icon_positions_[EquipSlot::Id::PENDANT2]);
        }

        if (!has_pocket_slot_) {
            disabled_.draw(position_ + icon_positions_[EquipSlot::Id::POCKET]);
        }

        for (auto [eq_slot, icon] : icons_) {
            if (icon) {
                icon->draw(position_ + icon_positions_[eq_slot]
                           + Point<int16_t>(4, 4));
            }
        }
    } else if (tab_ == Buttons::BT_TAB2) {
        disabled2_.draw(position_ + Point<int16_t>(113, 57));
        disabled2_.draw(position_ + Point<int16_t>(113, 106));
        disabled2_.draw(position_ + Point<int16_t>(113, 155));
    }
}

Button::State UIEquipInventory::button_pressed(uint16_t id) {
    switch (id) {
        case Buttons::BT_CLOSE: toggle_active(); break;
        case Buttons::BT_TAB0:
        case Buttons::BT_TAB1:
        case Buttons::BT_TAB2:
        case Buttons::BT_TAB3: change_tab(id); return Button::State::IDENTITY;
        default: break;
    }

    return Button::State::NORMAL;
}

void UIEquipInventory::update_slot(EquipSlot::Id slot) {
    if (int32_t item_id =
            inventory_.get_item_id(InventoryType::Id::EQUIPPED, slot)) {
        const Texture &texture = ItemData::get(item_id).get_icon(false);

        icons_[slot] = std::make_unique<Icon>(std::make_unique<EquipIcon>(slot),
                                              texture,
                                              -1);
    } else if (icons_[slot]) {
        icons_[slot].release();
    }

    clear_tooltip();
}

void UIEquipInventory::load_icons() {
    icons_.clear();

    for (auto iter : EquipSlot::values) {
        update_slot(iter);
    }
}

Cursor::State UIEquipInventory::send_cursor(bool pressed,
                                            Point<int16_t> cursorpos) {
    Cursor::State dstate = UIDragElement::send_cursor(pressed, cursorpos);

    if (dragged_) {
        clear_tooltip();

        return dstate;
    }

    EquipSlot::Id slot = slot_by_position(cursorpos);

    if (auto *icon = icons_[slot].get()) {
        if (pressed) {
            icon->start_drag(cursorpos - position_ - icon_positions_[slot]);

            UI::get().drag_icon(icon);

            clear_tooltip();

            return Cursor::State::GRABBING;
        }
        show_equip(slot);

        return Cursor::State::CAN_GRAB;
    }
    clear_tooltip();

    return Cursor::State::IDLE;
}

void UIEquipInventory::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            toggle_active();
        } else if (keycode == KeyAction::Id::TAB) {
            uint16_t newtab = tab_ + 1;

            if (newtab >= Buttons::BT_TABE) {
                newtab = Buttons::BT_TAB0;
            }

            change_tab(newtab);
        }
    }
}

UIElement::Type UIEquipInventory::get_type() const {
    return TYPE;
}

void UIEquipInventory::doubleclick(Point<int16_t> cursorpos) {
    EquipSlot::Id slot = slot_by_position(cursorpos);

    if (icons_[slot]) {
        if (int16_t freeslot =
                inventory_.find_free_slot(InventoryType::Id::EQUIP)) {
            fn_unequip_item(slot, freeslot);
        }
    }
}

bool UIEquipInventory::is_in_range(Point<int16_t> cursorpos) const {
    Rectangle<int16_t> bounds =
        Rectangle<int16_t>(position_, position_ + dimension_);

    Rectangle<int16_t> totem_bounds =
        Rectangle<int16_t>(position_, position_ + totem_dimensions_);
    totem_bounds.shift(totem_adj_);

    return bounds.contains(cursorpos) || totem_bounds.contains(cursorpos);
}

bool UIEquipInventory::send_icon(const Icon &icon, Point<int16_t> cursorpos) {
    if (EquipSlot::Id slot = slot_by_position(cursorpos)) {
        icon.drop_on_equips(slot);
    }

    return true;
}

void UIEquipInventory::toggle_active() {
    clear_tooltip();

    UIElement::toggle_active();
}

void UIEquipInventory::modify(int16_t pos, int8_t mode, int16_t arg) {
    EquipSlot::Id eqpos = EquipSlot::by_id(pos);
    EquipSlot::Id eqarg = EquipSlot::by_id(arg);

    switch (mode) {
        case 0:
        case 3: update_slot(eqpos); break;
        case 2:
            update_slot(eqpos);
            update_slot(eqarg);
            break;
    }
}

void UIEquipInventory::show_equip(EquipSlot::Id slot) {
    UI::get().show_equip(Tooltip::Parent::EQUIP_INVENTORY, slot);
}

void UIEquipInventory::clear_tooltip() {
    UI::get().clear_tooltip(Tooltip::Parent::EQUIP_INVENTORY);
}

EquipSlot::Id UIEquipInventory::slot_by_position(
    Point<int16_t> cursorpos) const {
    if (tab_ != Buttons::BT_TAB0) {
        return EquipSlot::Id::NONE;
    }

    for (auto [eq_slot, icon_pos] : icon_positions_) {
        Rectangle<int16_t> iconrect =
            Rectangle<int16_t>(position_ + icon_pos,
                               position_ + icon_pos + Point<int16_t>(32, 32));

        if (iconrect.contains(cursorpos)) {
            return eq_slot;
        }
    }

    return EquipSlot::Id::NONE;
}

void UIEquipInventory::change_tab(uint16_t tabid) {
    uint8_t oldtab = tab_;
    tab_ = tabid;

    if (oldtab != tab_) {
        clear_tooltip();

        buttons_[oldtab]->set_state(Button::State::NORMAL);
        buttons_[tab_]->set_state(Button::State::PRESSED);

        if (tab_ == Buttons::BT_TAB0) {
            buttons_[Buttons::BT_SLOT]->set_active(true);
        } else {
            buttons_[Buttons::BT_SLOT]->set_active(false);
        }

        if (tab_ == Buttons::BT_TAB2) {
            buttons_[Buttons::BT_CONSUMESETTING]->set_active(true);
            buttons_[Buttons::BT_EXCEPTION]->set_active(true);
        } else {
            buttons_[Buttons::BT_CONSUMESETTING]->set_active(false);
            buttons_[Buttons::BT_EXCEPTION]->set_active(false);
        }

        if (tab_ == Buttons::BT_TAB3) {
            buttons_[Buttons::BT_SHOP]->set_active(true);
        } else {
            buttons_[Buttons::BT_SHOP]->set_active(false);
        }
    }
}

UIEquipInventory::EquipIcon::EquipIcon(int16_t s) {
    source_ = s;
}

void UIEquipInventory::EquipIcon::drop_on_stage() const {
    Sound(Sound::Name::DRAG_END).play();
}

void UIEquipInventory::EquipIcon::drop_on_equips(EquipSlot::Id slot) const {
    if (source_ == slot) {
        Sound(Sound::Name::DRAG_END).play();
    }
}

bool UIEquipInventory::EquipIcon::drop_on_items(InventoryType::Id tab,
                                                EquipSlot::Id eqslot,
                                                int16_t slot,
                                                bool equip) const {
    if (tab != InventoryType::Id::EQUIP) {
        if (auto iteminventory = UI::get().get_element<UIItemInventory>()) {
            if (iteminventory->get().is_active()) {
                iteminventory->get().change_tab(InventoryType::Id::EQUIP);
                return false;
            }
        }
    }

    if (equip) {
        if (eqslot == source_) {
            fn_equip_item(slot, eqslot);
        }
    } else {
        fn_unequip_item(source_, slot);
    }

    return true;
}

Icon::IconType UIEquipInventory::EquipIcon::get_type() {
    return Icon::IconType::EQUIP;
}
}  // namespace ms