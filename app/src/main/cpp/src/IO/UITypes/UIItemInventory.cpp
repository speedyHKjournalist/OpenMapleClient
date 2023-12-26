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
#include "UIItemInventory.h"

#include <nlnx/nx.hpp>
#include <string>

#include "../../Data/EquipData.h"
#include "../../Gameplay/Stage.h"
#include "../../Net/Packets/InventoryPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "../UITypes/UIKeyConfig.h"
#include "UINotice.h"

namespace ms {
namespace {
auto fn_gather_items = []<typename... T>(T && ...args) {
    GatherItemsPacket(std::forward<T>(args)...).dispatch();
};
auto fn_sort_items = []<typename... T>(T && ...args) {
    SortItemsPacket(std::forward<T>(args)...).dispatch();
};
auto fn_equip_item = []<typename... T>(T && ...args) {
    EquipItemPacket(std::forward<T>(args)...).dispatch();
};
auto fn_use_item = []<typename... T>(T && ...args) {
    UseItemPacket(std::forward<T>(args)...).dispatch();
};
auto fn_move_item = []<typename... T>(T && ...args) {
    MoveItemPacket(std::forward<T>(args)...).dispatch();
};
auto fn_scroll_equip = []<typename... T>(T && ...args) {
    ScrollEquipPacket(std::forward<T>(args)...).dispatch();
};
auto fn_drop_mesos = []<typename... T>(T && ...args) {
    DropMesosPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIItemInventory::UIItemInventory(const Inventory &invent) :
    UIDragElement<PosINV>(),
    inventory_(invent),
    tab_(InventoryType::Id::EQUIP),
    ignore_tooltip_(false),
    sort_enabled_(false) {
    nl::node Item = nl::nx::ui["UIWindow2.img"]["Item"];

    // TODO: Change these to production
    backgrnd_ = Item["backgrnd"];
    backgrnd2_ = Item["productionBackgrnd2"];
    backgrnd3_ = Item["backgrnd3"];

    full_backgrnd_ = Item["FullBackgrnd"];
    full_backgrnd2_ = Item["FullBackgrnd2"];
    full_backgrnd3_ = Item["FullBackgrnd3"];

    bg_dimensions_ = backgrnd_.get_dimensions();
    bg_full_dimensions_ = full_backgrnd_.get_dimensions();

    nl::node New = Item["New"];
    new_item_slot_ = New["inventory"];
    new_item_tab_ = New["Tab0"];

    projectile_ = Item["activeIcon"];
    disabled_ = Item["disabled"];

    nl::node Tab = Item["Tab"];
    nl::node taben = Tab["enabled"];
    nl::node tabdis = Tab["disabled"];

    nl::node close = nl::nx::ui["Basic.img"]["BtClose3"];
    buttons_[Buttons::BT_CLOSE] = std::make_unique<MapleButton>(close);

    buttons_[Buttons::BT_TAB_EQUIP] =
        std::make_unique<TwoSpriteButton>(tabdis["0"], taben["0"]);
    buttons_[Buttons::BT_TAB_USE] =
        std::make_unique<TwoSpriteButton>(tabdis["1"], taben["1"]);
    buttons_[Buttons::BT_TAB_ETC] =
        std::make_unique<TwoSpriteButton>(tabdis["2"], taben["2"]);
    buttons_[Buttons::BT_TAB_SETUP] =
        std::make_unique<TwoSpriteButton>(tabdis["3"], taben["3"]);
    buttons_[Buttons::BT_TAB_CASH] =
        std::make_unique<TwoSpriteButton>(tabdis["4"], taben["4"]);

    buttons_[Buttons::BT_COIN] = std::make_unique<MapleButton>(Item["BtCoin3"]);
    buttons_[Buttons::BT_POINT] =
        std::make_unique<MapleButton>(Item["BtPoint0"]);
    buttons_[Buttons::BT_GATHER] =
        std::make_unique<MapleButton>(Item["BtGather3"]);
    buttons_[Buttons::BT_SORT] = std::make_unique<MapleButton>(Item["BtSort3"]);
    buttons_[Buttons::BT_FULL] = std::make_unique<MapleButton>(Item["BtFull3"]);
    buttons_[Buttons::BT_SMALL] =
        std::make_unique<MapleButton>(Item["BtSmall3"]);
    buttons_[Buttons::BT_POT] = std::make_unique<MapleButton>(Item["BtPot3"]);
    buttons_[Buttons::BT_UPGRADE] =
        std::make_unique<MapleButton>(Item["BtUpgrade3"]);
    buttons_[Buttons::BT_APPRAISE] =
        std::make_unique<MapleButton>(Item["BtAppraise3"]);
    buttons_[Buttons::BT_EXTRACT] =
        std::make_unique<MapleButton>(Item["BtExtract3"]);
    buttons_[Buttons::BT_DISASSEMBLE] =
        std::make_unique<MapleButton>(Item["BtDisassemble3"]);
    buttons_[Buttons::BT_TOAD] = std::make_unique<MapleButton>(Item["BtToad3"]);

    buttons_[Buttons::BT_COIN_SM] =
        std::make_unique<MapleButton>(Item["BtCoin4"]);
    buttons_[Buttons::BT_POINT_SM] =
        std::make_unique<MapleButton>(Item["BtPoint1"]);
    buttons_[Buttons::BT_GATHER_SM] =
        std::make_unique<MapleButton>(Item["BtGather4"]);
    buttons_[Buttons::BT_SORT_SM] =
        std::make_unique<MapleButton>(Item["BtSort4"]);
    buttons_[Buttons::BT_FULL_SM] =
        std::make_unique<MapleButton>(Item["BtFull4"]);
    buttons_[Buttons::BT_SMALL_SM] =
        std::make_unique<MapleButton>(Item["BtSmall4"]);
    buttons_[Buttons::BT_POT_SM] =
        std::make_unique<MapleButton>(Item["BtPot4"]);
    buttons_[Buttons::BT_UPGRADE_SM] =
        std::make_unique<MapleButton>(Item["BtUpgrade4"]);
    buttons_[Buttons::BT_APPRAISE_SM] =
        std::make_unique<MapleButton>(Item["BtAppraise4"]);
    buttons_[Buttons::BT_EXTRACT_SM] =
        std::make_unique<MapleButton>(Item["BtExtract4"]);
    buttons_[Buttons::BT_DISASSEMBLE_SM] =
        std::make_unique<MapleButton>(Item["BtDisassemble4"]);
    buttons_[Buttons::BT_TOAD_SM] =
        std::make_unique<MapleButton>(Item["BtToad4"]);
    buttons_[Buttons::BT_CASHSHOP] =
        std::make_unique<MapleButton>(Item["BtCashshop"]);

    buttons_[Buttons::BT_POT]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_POT_SM]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_EXTRACT]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_EXTRACT_SM]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_DISASSEMBLE]->set_state(Button::State::DISABLED);
    buttons_[Buttons::BT_DISASSEMBLE_SM]->set_state(Button::State::DISABLED);
    buttons_[button_by_tab(tab_)]->set_state(Button::State::PRESSED);

    meso_label_ =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::BLACK);
    maple_points_label_ =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::BLACK);
    maple_points_label_.change_text("0");  // TODO: Implement

    slot_range_[InventoryType::Id::EQUIPPED] = { 1, 24 };
    slot_range_[InventoryType::Id::EQUIP] = { 1, 24 };
    slot_range_[InventoryType::Id::USE] = { 1, 24 };
    slot_range_[InventoryType::Id::SETUP] = { 1, 24 };
    slot_range_[InventoryType::Id::ETC] = { 1, 24 };
    slot_range_[InventoryType::Id::CASH] = { 1, 24 };

    slider_ = Slider(Slider::Type::DEFAULT_SILVER,
                     Range<int16_t>(50, 245),
                     152,
                     6,
                     1 + inventory_.get_slotmax(tab_) / COLUMNS_,
                     [&](bool upwards) {
                         int16_t shift = upwards ? -COLUMNS_ : COLUMNS_;
                         bool above = slot_range_[tab_].first + shift > 0;
                         bool below =
                             slot_range_[tab_].second + shift
                             < inventory_.get_slotmax(tab_) + 1 + COLUMNS_;

                         if (above && below) {
                             slot_range_[tab_].first += shift;
                             slot_range_[tab_].second += shift;
                         }
                     });

    set_full(false);
    clear_new();
    load_icons();
}

void UIItemInventory::draw(float alpha) const {
    UIElement::draw_sprites(alpha);

    Point<int16_t> mesolabel_pos = position_ + Point<int16_t>(127, 262);
    Point<int16_t> maplepointslabel_pos = position_ + Point<int16_t>(159, 279);

    if (full_enabled_) {
        full_backgrnd_.draw(position_);
        full_backgrnd2_.draw(position_);
        full_backgrnd3_.draw(position_);

        meso_label_.draw(mesolabel_pos + Point<int16_t>(3, 70));
        maple_points_label_.draw(maplepointslabel_pos
                                 + Point<int16_t>(181, 53));
    } else {
        backgrnd_.draw(position_);
        backgrnd2_.draw(position_);
        backgrnd3_.draw(position_);

        slider_.draw(position_ + Point<int16_t>(0, 1));

        meso_label_.draw(mesolabel_pos);
        maple_points_label_.draw(maplepointslabel_pos);
    }

    auto range = slot_range_.at(tab_);

    size_t numslots = inventory_.get_slotmax(tab_);
    size_t firstslot = full_enabled_ ? 1 : range.first;
    size_t lastslot = full_enabled_ ? MAX_FULL_SLOTS_ : range.second;

    for (size_t i = 0; i <= MAX_FULL_SLOTS_; i++) {
        Point<int16_t> slotpos =
            full_enabled_ ? get_full_slotpos(i) : get_slotpos(i);

        if (icons_.find(i) != icons_.end()) {
            const auto &icon = icons_.at(i);

            if (icon && i >= firstslot && i <= lastslot) {
                icon->draw(position_ + slotpos);
            }
        } else {
            if (i > numslots && i <= lastslot) {
                disabled_.draw(position_ + slotpos);
            }
        }
    }

    int16_t bulletslot = inventory_.get_bulletslot();

    if (tab_ == InventoryType::Id::USE && is_visible(bulletslot)) {
        projectile_.draw(position_ + get_slotpos(bulletslot));
    }

    if (tab_ == new_tab_) {
        new_item_tab_.draw(position_ + get_tabpos(new_tab_), alpha);

        if (is_visible(new_slot_)) {
            new_item_slot_.draw(
                position_ + get_slotpos(new_slot_) + Point<int16_t>(1, 1),
                alpha);
        }
    }

    UIElement::draw_buttons(alpha);
}

void UIItemInventory::update() {
    UIElement::update();

    new_item_tab_.update(6);
    new_item_slot_.update(6);

    std::string meso_str = std::to_string(inventory_.get_meso());
    string_format::split_number(meso_str);

    meso_label_.change_text(meso_str);
}

void UIItemInventory::update_slot(int16_t slot) {
    if (int32_t item_id = inventory_.get_item_id(tab_, slot)) {
        int16_t count;

        if (tab_ == InventoryType::Id::EQUIP) {
            count = -1;
        } else {
            count = inventory_.get_item_count(tab_, slot);
        }

        const bool untradable = ItemData::get(item_id).is_untradable();
        const bool cashitem = ItemData::get(item_id).is_cashitem();
        const Texture &texture = ItemData::get(item_id).get_icon(false);
        EquipSlot::Id eqslot = inventory_.find_equipslot(item_id);

        icons_[slot] =
            std::make_unique<Icon>(std::make_unique<ItemIcon>(*this,
                                                              tab_,
                                                              eqslot,
                                                              slot,
                                                              item_id,
                                                              count,
                                                              untradable,
                                                              cashitem),
                                   texture,
                                   count);
    } else if (icons_.count(slot)) {
        icons_.erase(slot);
    }
}

void UIItemInventory::load_icons() {
    icons_.clear();

    uint8_t numslots = inventory_.get_slotmax(tab_);

    for (size_t i = 0; i <= MAX_FULL_SLOTS_; i++) {
        if (i <= numslots) {
            update_slot(i);
        }
    }
}

Button::State UIItemInventory::button_pressed(uint16_t buttonid) {
    InventoryType::Id oldtab = tab_;

    switch (buttonid) {
        case Buttons::BT_CLOSE: toggle_active(); return Button::State::NORMAL;
        case Buttons::BT_TAB_EQUIP: tab_ = InventoryType::Id::EQUIP; break;
        case Buttons::BT_TAB_USE: tab_ = InventoryType::Id::USE; break;
        case Buttons::BT_TAB_SETUP: tab_ = InventoryType::Id::SETUP; break;
        case Buttons::BT_TAB_ETC: tab_ = InventoryType::Id::ETC; break;
        case Buttons::BT_TAB_CASH: tab_ = InventoryType::Id::CASH; break;
        case Buttons::BT_GATHER:
        case Buttons::BT_GATHER_SM: fn_gather_items(tab_); break;
        case Buttons::BT_SORT:
        case Buttons::BT_SORT_SM: fn_sort_items(tab_); break;
        case Buttons::BT_FULL:
        case Buttons::BT_FULL_SM: set_full(true); return Button::State::NORMAL;
        case Buttons::BT_SMALL:
        case Buttons::BT_SMALL_SM:
            set_full(false);
            return Button::State::NORMAL;
        case Buttons::BT_COIN: {
            const std::string question = "How many do you want to drop?";
            auto onenter = [](auto amount) { fn_drop_mesos(amount); };

            UI::get().emplace<UIEnterNumber>(question, onenter, 50000, 10);
        } break;
        case Buttons::BT_COIN_SM:
        case Buttons::BT_POINT:
        case Buttons::BT_POINT_SM:
        case Buttons::BT_POT:
        case Buttons::BT_POT_SM:
        case Buttons::BT_UPGRADE:
        case Buttons::BT_UPGRADE_SM:
        case Buttons::BT_APPRAISE:
        case Buttons::BT_APPRAISE_SM:
        case Buttons::BT_EXTRACT:
        case Buttons::BT_EXTRACT_SM:
        case Buttons::BT_DISASSEMBLE:
        case Buttons::BT_DISASSEMBLE_SM:
        case Buttons::BT_TOAD:
        case Buttons::BT_TOAD_SM:
        case Buttons::BT_CASHSHOP: return Button::State::NORMAL;
    }

    if (tab_ != oldtab) {
        uint16_t row = slot_range_.at(tab_).first / COLUMNS_;
        slider_.setrows(row, 6, 1 + inventory_.get_slotmax(tab_) / COLUMNS_);

        buttons_[button_by_tab(oldtab)]->set_state(Button::State::NORMAL);
        buttons_[button_by_tab(tab_)]->set_state(Button::State::PRESSED);

        load_icons();
        set_sort(false);
    }

    return Button::State::IDENTITY;
}

void UIItemInventory::doubleclick(Point<int16_t> cursorpos) {
    int16_t slot = slot_by_position(cursorpos - position_);

    if (icons_.count(slot) && is_visible(slot)) {
        if (int32_t item_id = inventory_.get_item_id(tab_, slot)) {
            switch (tab_) {
                case InventoryType::Id::EQUIP:
                    if (can_wear_equip(slot)) {
                        fn_equip_item(slot, inventory_.find_equipslot(item_id));
                    }
                    break;
                case InventoryType::Id::USE: fn_use_item(slot, item_id); break;
            }
        }
    }
}

bool UIItemInventory::send_icon(const Icon &icon, Point<int16_t> cursorpos) {
    int16_t slot = slot_by_position(cursorpos - position_);

    if (slot > 0) {
        int32_t item_id = inventory_.get_item_id(tab_, slot);
        EquipSlot::Id eqslot;
        bool equip;

        if (item_id && tab_ == InventoryType::Id::EQUIP) {
            eqslot = inventory_.find_equipslot(item_id);
            equip = true;
        } else {
            eqslot = EquipSlot::Id::NONE;
            equip = false;
        }

        ignore_tooltip_ = true;

        return icon.drop_on_items(tab_, eqslot, slot, equip);
    }

    return true;
}

Cursor::State UIItemInventory::send_cursor(bool pressed,
                                           Point<int16_t> cursorpos) {
    Cursor::State dstate = UIDragElement::send_cursor(pressed, cursorpos);

    if (dragged_) {
        clear_tooltip();

        return dstate;
    }

    Point<int16_t> cursor_relative = cursorpos - position_;

    if (!full_enabled_ && slider_.isenabled()) {
        Cursor::State sstate = slider_.send_cursor(cursor_relative, pressed);

        if (sstate != Cursor::State::IDLE) {
            clear_tooltip();

            return sstate;
        }
    }

    int16_t slot = slot_by_position(cursor_relative);
    Icon *icon = get_icon(slot);
    bool is_icon = icon && is_visible(slot);

    if (is_icon) {
        if (pressed) {
            Point<int16_t> slotpos = get_slotpos(slot);
            icon->start_drag(cursor_relative - slotpos);
            UI::get().drag_icon(icon);

            clear_tooltip();

            return Cursor::State::GRABBING;
        }
        if (!ignore_tooltip_) {
            show_item(slot);

            return Cursor::State::CAN_GRAB;
        }
        ignore_tooltip_ = false;

        return Cursor::State::CAN_GRAB;
    }
    clear_tooltip();

    return UIElement::send_cursor(pressed, cursorpos);
}

void UIItemInventory::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed) {
        if (escape) {
            toggle_active();
        } else if (keycode == KeyAction::Id::TAB) {
            clear_tooltip();

            InventoryType::Id newtab;

            switch (tab_) {
                case InventoryType::Id::EQUIP:
                    newtab = InventoryType::Id::USE;
                    break;
                case InventoryType::Id::USE:
                    newtab = InventoryType::Id::ETC;
                    break;
                case InventoryType::Id::ETC:
                    newtab = InventoryType::Id::SETUP;
                    break;
                case InventoryType::Id::SETUP:
                    newtab = InventoryType::Id::CASH;
                    break;
                case InventoryType::Id::CASH:
                    newtab = InventoryType::Id::EQUIP;
                    break;
            }

            button_pressed(button_by_tab(newtab));
        }
    }
}

UIElement::Type UIItemInventory::get_type() const {
    return TYPE;
}

void UIItemInventory::modify(InventoryType::Id type,
                             int16_t slot,
                             int8_t mode,
                             int16_t arg) {
    if (slot <= 0) {
        return;
    }

    if (type == tab_) {
        switch (mode) {
            case Inventory::Modification::ADD:
                update_slot(slot);
                new_tab_ = type;
                new_slot_ = slot;
                break;
            case Inventory::Modification::CHANGE_COUNT:
            case Inventory::Modification::ADD_COUNT:
                if (auto *icon = get_icon(slot)) {
                    icon->set_count(arg);
                }

                break;
            case Inventory::Modification::SWAP:
                if (arg != slot) {
                    update_slot(slot);
                    update_slot(arg);
                }

                break;
            case Inventory::Modification::REMOVE: update_slot(slot); break;
        }
    }

    switch (mode) {
        case Inventory::Modification::ADD:
        case Inventory::Modification::ADD_COUNT:
            new_tab_ = type;
            new_slot_ = slot;
            break;
        case Inventory::Modification::CHANGE_COUNT:
        case Inventory::Modification::SWAP:
        case Inventory::Modification::REMOVE:
            if (new_slot_ == slot && new_tab_ == type) {
                clear_new();
            }

            break;
    }
}

void UIItemInventory::set_sort(bool enabled) {
    sort_enabled_ = enabled;

    if (full_enabled_) {
        if (sort_enabled_) {
            buttons_[Buttons::BT_SORT]->set_active(false);
            buttons_[Buttons::BT_SORT_SM]->set_active(true);
            buttons_[Buttons::BT_GATHER]->set_active(false);
            buttons_[Buttons::BT_GATHER_SM]->set_active(false);
        } else {
            buttons_[Buttons::BT_SORT]->set_active(false);
            buttons_[Buttons::BT_SORT_SM]->set_active(false);
            buttons_[Buttons::BT_GATHER]->set_active(false);
            buttons_[Buttons::BT_GATHER_SM]->set_active(true);
        }
    } else {
        if (sort_enabled_) {
            buttons_[Buttons::BT_SORT]->set_active(true);
            buttons_[Buttons::BT_SORT_SM]->set_active(false);
            buttons_[Buttons::BT_GATHER]->set_active(false);
            buttons_[Buttons::BT_GATHER_SM]->set_active(false);
        } else {
            buttons_[Buttons::BT_SORT]->set_active(false);
            buttons_[Buttons::BT_SORT_SM]->set_active(false);
            buttons_[Buttons::BT_GATHER]->set_active(true);
            buttons_[Buttons::BT_GATHER_SM]->set_active(false);
        }
    }
}

void UIItemInventory::change_tab(InventoryType::Id type) {
    button_pressed(button_by_tab(type));
}

void UIItemInventory::clear_new() {
    new_tab_ = InventoryType::Id::NONE;
    new_slot_ = 0;
}

void UIItemInventory::toggle_active() {
    UIElement::toggle_active();

    if (!active_) {
        clear_new();
        clear_tooltip();
    }
}

void UIItemInventory::remove_cursor() {
    UIDragElement::remove_cursor();

    slider_.remove_cursor();
}

void UIItemInventory::show_item(int16_t slot) {
    if (tab_ == InventoryType::Id::EQUIP) {
        UI::get().show_equip(Tooltip::Parent::ITEM_INVENTORY, slot);
    } else {
        int32_t item_id = inventory_.get_item_id(tab_, slot);
        UI::get().show_item(Tooltip::Parent::ITEM_INVENTORY, item_id);
    }
}

void UIItemInventory::clear_tooltip() {
    UI::get().clear_tooltip(Tooltip::Parent::ITEM_INVENTORY);
}

bool UIItemInventory::is_visible(int16_t slot) const {
    return !is_not_visible(slot);
}

bool UIItemInventory::is_not_visible(int16_t slot) const {
    auto range = slot_range_.at(tab_);

    if (full_enabled_) {
        return slot < 1 || slot > 24;
    }
    return slot < range.first || slot > range.second;
}

bool UIItemInventory::can_wear_equip(int16_t slot) const {
    const Player &player = Stage::get().get_player();
    const CharStats &stats = player.get_stats();
    const CharLook &look = player.get_look();
    const bool alerted = look.get_alerted();

    if (alerted) {
        UI::get().emplace<UIOk>("You cannot complete this action right "
                                "now.\\nEvade the attack and try again.",
                                [](bool) {});
        return false;
    }

    const int32_t item_id =
        inventory_.get_item_id(InventoryType::Id::EQUIP, slot);
    const EquipData &equipdata = EquipData::get(item_id);
    const ItemData &itemdata = equipdata.get_itemdata();

    const int8_t reqGender = itemdata.get_gender();
    const bool female = stats.get_female();

    switch (reqGender) {
        case 0:  // Male
            if (female) {
                return false;
            }

            break;
        case 1:  // Female
            if (!female) {
                return false;
            }

            break;
        case 2:  // Unisex
        default: break;
    }

    const std::string jobname = stats.get_jobname();

    if (jobname == "GM" || jobname == "SuperGM") {
        return true;
    }

    int16_t reqJOB = equipdata.get_reqstat(MapleStat::Id::JOB);

    if (!stats.get_job().is_sub_job(reqJOB)) {
        UI::get().emplace<UIOk>(
            "Your current job\\ncannot equip the selected item.",
            [](bool) {});
        return false;
    }

    int16_t reqLevel = equipdata.get_reqstat(MapleStat::Id::LEVEL);
    int16_t reqDEX = equipdata.get_reqstat(MapleStat::Id::DEX);
    int16_t reqSTR = equipdata.get_reqstat(MapleStat::Id::STR);
    int16_t reqLUK = equipdata.get_reqstat(MapleStat::Id::LUK);
    int16_t reqINT = equipdata.get_reqstat(MapleStat::Id::INT);
    int16_t reqFAME = equipdata.get_reqstat(MapleStat::Id::FAME);

    int8_t i = 0;

    if (reqLevel > stats.get_stat(MapleStat::Id::LEVEL)) {
        i++;
    } else if (reqDEX > stats.get_total(EquipStat::Id::DEX)) {
        i++;
    } else if (reqSTR > stats.get_total(EquipStat::Id::STR)) {
        i++;
    } else if (reqLUK > stats.get_total(EquipStat::Id::LUK)) {
        i++;
    } else if (reqINT > stats.get_total(EquipStat::Id::INT)) {
        i++;
    } else if (reqFAME > stats.get_honor()) {
        i++;
    }

    if (i > 0) {
        UI::get().emplace<UIOk>(
            "Your stats are too low to equip this item\\nor you do not meet "
            "the job requirement.",
            [](bool) {});
        return false;
    }

    return true;
}

int16_t UIItemInventory::slot_by_position(Point<int16_t> cursorpos) const {
    int16_t xoff = cursorpos.x() - 11;
    int16_t yoff = cursorpos.y() - 51;

    if (xoff < 1 || xoff > 143 || yoff < 1) {
        return 0;
    }

    int16_t slot = (full_enabled_ ? 1 : slot_range_.at(tab_).first)
                   + (xoff / ICON_WIDTH_) + COLUMNS_ * (yoff / ICON_HEIGHT_);

    return is_visible(slot) ? slot : 0;
}

Point<int16_t> UIItemInventory::get_slotpos(int16_t slot) const {
    int16_t absslot = slot - slot_range_.at(tab_).first;

    return Point<int16_t>(10 + (absslot % COLUMNS_) * ICON_WIDTH_,
                          51 + (absslot / COLUMNS_) * ICON_HEIGHT_);
}

Point<int16_t> UIItemInventory::get_full_slotpos(int16_t slot) const {
    int16_t absslot = slot - 1;
    div_t div = std::div(absslot, MAX_SLOTS_);
    int16_t new_slot = absslot - (div.quot * MAX_SLOTS_);
    int16_t adj_x = div.quot * COLUMNS_ * ICON_WIDTH_;

    return Point<int16_t>(10 + adj_x + (new_slot % COLUMNS_) * ICON_WIDTH_,
                          51 + (new_slot / COLUMNS_) * ICON_HEIGHT_);
}

Point<int16_t> UIItemInventory::get_tabpos(InventoryType::Id tb) const {
    int8_t fixed_tab = tb;

    switch (tb) {
        case InventoryType::Id::ETC: fixed_tab = 3; break;
        case InventoryType::Id::SETUP: fixed_tab = 4; break;
    }

    return Point<int16_t>(10 + ((fixed_tab - 1) * 31), 29);
}

uint16_t UIItemInventory::button_by_tab(InventoryType::Id tb) const {
    switch (tb) {
        case InventoryType::Id::EQUIP: return Buttons::BT_TAB_EQUIP;
        case InventoryType::Id::USE: return Buttons::BT_TAB_USE;
        case InventoryType::Id::SETUP: return Buttons::BT_TAB_SETUP;
        case InventoryType::Id::ETC: return Buttons::BT_TAB_ETC;
        default: return Buttons::BT_TAB_CASH;
    }
}

Icon *UIItemInventory::get_icon(int16_t slot) {
    auto iter = icons_.find(slot);

    if (iter != icons_.end()) {
        return iter->second.get();
    }
    return nullptr;
}

void UIItemInventory::set_full(bool enabled) {
    full_enabled_ = enabled;

    if (full_enabled_) {
        dimension_ = bg_full_dimensions_;

        buttons_[Buttons::BT_FULL]->set_active(false);
        buttons_[Buttons::BT_FULL_SM]->set_active(false);
        buttons_[Buttons::BT_SMALL]->set_active(false);
        buttons_[Buttons::BT_SMALL_SM]->set_active(true);
    } else {
        dimension_ = bg_dimensions_;

        buttons_[Buttons::BT_FULL]->set_active(true);
        buttons_[Buttons::BT_FULL_SM]->set_active(false);
        buttons_[Buttons::BT_SMALL]->set_active(false);
        buttons_[Buttons::BT_SMALL_SM]->set_active(false);
    }

    drag_area_ = Point<int16_t>(dimension_.x(), 20);

    int16_t adj_x = full_enabled_ ? 20 : 22;
    buttons_[Buttons::BT_CLOSE]->set_position(
        Point<int16_t>(dimension_.x() - adj_x, 6));

    buttons_[Buttons::BT_COIN]->set_active(!enabled);
    buttons_[Buttons::BT_POINT]->set_active(!enabled);
    buttons_[Buttons::BT_POT]->set_active(!enabled);
    buttons_[Buttons::BT_UPGRADE]->set_active(!enabled);
    buttons_[Buttons::BT_APPRAISE]->set_active(!enabled);
    buttons_[Buttons::BT_EXTRACT]->set_active(!enabled);
    buttons_[Buttons::BT_DISASSEMBLE]->set_active(!enabled);
    buttons_[Buttons::BT_TOAD]->set_active(!enabled);
    buttons_[Buttons::BT_CASHSHOP]->set_active(!enabled);

    buttons_[Buttons::BT_COIN_SM]->set_active(enabled);
    buttons_[Buttons::BT_POINT_SM]->set_active(enabled);
    buttons_[Buttons::BT_POT_SM]->set_active(enabled);
    buttons_[Buttons::BT_UPGRADE_SM]->set_active(enabled);
    buttons_[Buttons::BT_APPRAISE_SM]->set_active(enabled);
    buttons_[Buttons::BT_EXTRACT_SM]->set_active(enabled);
    buttons_[Buttons::BT_DISASSEMBLE_SM]->set_active(enabled);
    buttons_[Buttons::BT_TOAD_SM]->set_active(enabled);
    buttons_[Buttons::BT_CASHSHOP]->set_active(enabled);

    set_sort(sort_enabled_);
    load_icons();
}

void UIItemInventory::ItemIcon::set_count(int16_t c) {
    count_ = c;
}

Icon::IconType UIItemInventory::ItemIcon::get_type() {
    return Icon::IconType::ITEM;
}

UIItemInventory::ItemIcon::ItemIcon(const UIItemInventory &parent,
                                    InventoryType::Id st,
                                    EquipSlot::Id eqs,
                                    int16_t s,
                                    int32_t iid,
                                    int16_t c,
                                    bool u,
                                    bool cash) :
    parent_(parent) {
    source_tab_ = st;
    eqsource_ = eqs;
    source_ = s;
    item_id_ = iid;
    count_ = c;
    untradable_ = u;
    cash_item_ = cash;
}

void UIItemInventory::ItemIcon::drop_on_stage() const {
    const std::string dropmessage = "How many will you drop?";
    const std::string untradablemessage =
        "This item can't be taken back once thrown away.\\nWill you still drop "
        "it?";
    const std::string cashmessage = "You can't drop this item.";

    if (cash_item_) {
        UI::get().emplace<UIOk>(cashmessage, [](bool) {});
    } else {
        if (untradable_) {
            auto onok = [&, dropmessage](bool ok) {
                if (ok) {
                    if (count_ <= 1) {
                        fn_move_item(source_tab_, source_, 0, 1);
                    } else {
                        auto onenter = [&](int32_t qty) {
                            fn_move_item(source_tab_, source_, 0, qty);
                        };

                        UI::get().emplace<UIEnterNumber>(dropmessage,
                                                         onenter,
                                                         count_,
                                                         count_);
                    }
                }
            };

            UI::get().emplace<UIYesNo>(untradablemessage, onok);
        } else {
            if (count_ <= 1) {
                fn_move_item(source_tab_, source_, 0, 1);
            } else {
                auto onenter = [&](int32_t qty) {
                    fn_move_item(source_tab_, source_, 0, qty);
                };

                UI::get().emplace<UIEnterNumber>(dropmessage,
                                                 onenter,
                                                 count_,
                                                 count_);
            }
        }
    }
}

void UIItemInventory::ItemIcon::drop_on_equips(EquipSlot::Id eqslot) const {
    switch (source_tab_) {
        case InventoryType::Id::EQUIP:
            if (eqsource_ == eqslot) {
                if (parent_.can_wear_equip(source_)) {
                    fn_equip_item(source_, eqslot);
                }
            }

            Sound(Sound::Name::DRAG_END).play();

            break;
        case InventoryType::Id::USE: fn_scroll_equip(source_, eqslot); break;
    }
}

bool UIItemInventory::ItemIcon::drop_on_items(InventoryType::Id tab,
                                              EquipSlot::Id,
                                              int16_t slot,
                                              bool) const {
    if (tab != source_tab_ || slot == source_) {
        return true;
    }

    fn_move_item(tab, source_, slot, 1);

    return true;
}

void UIItemInventory::ItemIcon::drop_on_bindings(Point<int16_t> cursorposition,
                                                 bool remove) const {
    if (source_tab_ == InventoryType::Id::USE
        || source_tab_ == InventoryType::Id::SETUP) {
        auto keyconfig = UI::get().get_element<UIKeyConfig>();
        Keyboard::Mapping mapping = Keyboard::Mapping(KeyType::ITEM, item_id_);

        if (remove) {
            keyconfig->get().unstage_mapping(mapping);
        } else {
            keyconfig->get().stage_mapping(cursorposition, mapping);
        }
    }
}
}  // namespace ms