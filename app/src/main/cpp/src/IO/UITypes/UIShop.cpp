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
#include "UIShop.h"

#include <nlnx/nx.hpp>
#include <string>

#include "../Audio/Audio.h"
#include "../Components/AreaButton.h"
#include "../Components/Charset.h"
#include "../Components/MapleButton.h"
#include "../Data/ItemData.h"
#include "../Net/Packets/NpcInteractionPackets.h"
#include "../UI.h"
#include "UINotice.h"

namespace ms {
namespace {
auto fn_npc_shop_action = []<typename... T>(T && ...args) {
    NpcShopActionPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UIShop::UIShop(const CharLook &in_charlook, const Inventory &in_inventory) :
    UIDragElement<PosSHOP>(),
    charlook_(in_charlook),
    inventory_(in_inventory) {
    nl::node src = nl::nx::ui["UIWindow2.img"]["Shop2"];

    nl::node background = src["backgrnd"];
    Texture bg = background;

    auto bg_dimensions = bg.get_dimensions();

    sprites_.emplace_back(background);
    sprites_.emplace_back(src["backgrnd2"]);
    sprites_.emplace_back(src["backgrnd3"]);
    sprites_.emplace_back(src["backgrnd4"]);

    buttons_[Buttons::BUY_ITEM] = std::make_unique<MapleButton>(src["BtBuy"]);
    buttons_[Buttons::SELL_ITEM] = std::make_unique<MapleButton>(src["BtSell"]);
    buttons_[Buttons::EXIT] = std::make_unique<MapleButton>(src["BtExit"]);

    Texture cben = src["checkBox"][0];
    Texture cbdis = src["checkBox"][1];

    Point<int16_t> cb_origin = cben.get_origin();
    int16_t cb_x = cb_origin.x();
    int16_t cb_y = cb_origin.y();

    checkbox_[0] = cbdis;
    checkbox_[1] = cben;

    buttons_[Buttons::CHECKBOX] = std::make_unique<AreaButton>(
        Point<int16_t>(std::abs(cb_x), std::abs(cb_y)),
        cben.get_dimensions());

    nl::node buyen = src["TabBuy"]["enabled"];
    nl::node buydis = src["TabBuy"]["disabled"];

    buttons_[Buttons::OVERALL] =
        std::make_unique<TwoSpriteButton>(buydis[0], buyen[0]);

    nl::node sellen = src["TabSell"]["enabled"];
    nl::node selldis = src["TabSell"]["disabled"];

    for (int i = Buttons::EQUIP; i <= Buttons::CASH; i++) {
        std::string tabnum = std::to_string(i - Buttons::EQUIP);
        buttons_[i] =
            std::make_unique<TwoSpriteButton>(selldis[tabnum], sellen[tabnum]);
    }

    int16_t item_y = 124;
    int16_t item_height = 36;

    buy_x_ = 8;
    buy_width_ = 257;

    for (int i = Buttons::BUY0; i <= Buttons::BUY8; i++) {
        Point<int16_t> pos(buy_x_, item_y + 42 * (i - Buttons::BUY0));
        Point<int16_t> dim(buy_width_, item_height);
        buttons_[i] = std::make_unique<AreaButton>(pos, dim);
    }

    sell_x_ = 284;
    sell_width_ = 200;

    for (int i = Buttons::SELL0; i <= Buttons::SELL8; i++) {
        Point<int16_t> pos(sell_x_, item_y + 42 * (i - Buttons::SELL0));
        Point<int16_t> dim(sell_width_, item_height);
        buttons_[i] = std::make_unique<AreaButton>(pos, dim);
    }

    buy_selection_ = src["select"];
    sell_selection_ = src["select2"];
    meso_ = src["meso"];

    meso_label_ =
        Text(Text::Font::A11M, Text::Alignment::RIGHT, Color::Name::MINESHAFT);

    buy_slider_ = Slider(Slider::Type::DEFAULT_SILVER,
                         Range<int16_t>(123, 484),
                         257,
                         5,
                         1,
                         [&](bool upwards) {
                             int16_t shift = upwards ? -1 : 1;
                             bool above = buy_state_.offset + shift >= 0;
                             bool below = buy_state_.offset + shift
                                          <= buy_state_.last_slot - 5;

                             if (above && below) {
                                 buy_state_.offset += shift;
                             }
                         });

    sell_slider_ = Slider(Slider::Type::DEFAULT_SILVER,
                          Range<int16_t>(123, 484),
                          488,
                          5,
                          1,
                          [&](bool upwards) {
                              int16_t shift = upwards ? -1 : 1;
                              bool above = sell_state_.offset + shift >= 0;
                              bool below = sell_state_.offset + shift
                                           <= sell_state_.last_slot - 5;

                              if (above && below) {
                                  sell_state_.offset += shift;
                              }
                          });

    active_ = false;
    dimension_ = bg_dimensions;
    drag_area_ = Point<int16_t>(bg_dimensions.x(), 10);
}

void UIShop::draw(float alpha) const {
    UIElement::draw(alpha);

    npc_.draw(DrawArgument(position_ + Point<int16_t>(58, 85), true));
    charlook_.draw(position_ + Point<int16_t>(338, 85),
                   false,
                   Stance::Id::STAND1,
                   Expression::Id::DEFAULT);

    meso_label_.draw(position_ + Point<int16_t>(493, 51));

    buy_state_.draw(position_, buy_selection_);
    sell_state_.draw(position_, sell_selection_);

    buy_slider_.draw(position_);
    sell_slider_.draw(position_);

    checkbox_[right_click_sell_].draw(position_);
}

void UIShop::update() {
    int64_t num_mesos = inventory_.get_meso();
    std::string mesostr = std::to_string(num_mesos);
    string_format::split_number(mesostr);
    meso_label_.change_text(mesostr);
}

Button::State UIShop::button_pressed(uint16_t buttonid) {
    clear_tooltip();

    constexpr Range<uint16_t> buy(Buttons::BUY0, Buttons::BUY8);
    constexpr Range<uint16_t> sell(Buttons::SELL0, Buttons::SELL8);

    if (buy.contains(buttonid)) {
        int16_t selected = buttonid - Buttons::BUY0;
        buy_state_.select(selected);
        sell_state_.selection = -1;

        return Button::State::NORMAL;
    }
    if (sell.contains(buttonid)) {
        int16_t selected = buttonid - Buttons::SELL0;
        sell_state_.select(selected);
        buy_state_.selection = -1;

        return Button::State::NORMAL;
    }
    switch (buttonid) {
        case Buttons::BUY_ITEM: buy_state_.buy(); return Button::State::NORMAL;
        case Buttons::SELL_ITEM:
            sell_state_.sell(false);

            return Button::State::NORMAL;
        case Buttons::EXIT: exit_shop(); return Button::State::PRESSED;
        case Buttons::CHECKBOX:
            right_click_sell_ = !right_click_sell_;
            Configuration::get().set_rightclicksell(right_click_sell_);

            return Button::State::NORMAL;
        case Buttons::EQUIP:
            changeselltab(InventoryType::Id::EQUIP);

            return Button::State::IDENTITY;
        case Buttons::USE:
            changeselltab(InventoryType::Id::USE);

            return Button::State::IDENTITY;
        case Buttons::ETC:
            changeselltab(InventoryType::Id::ETC);

            return Button::State::IDENTITY;
        case Buttons::SETUP:
            changeselltab(InventoryType::Id::SETUP);

            return Button::State::IDENTITY;
        case Buttons::CASH:
            changeselltab(InventoryType::Id::CASH);

            return Button::State::IDENTITY;
    }

    return Button::State::PRESSED;
}

void UIShop::remove_cursor() {
    UIDragElement::remove_cursor();

    buy_slider_.remove_cursor();
    sell_slider_.remove_cursor();
}

Cursor::State UIShop::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Point<int16_t> cursoroffset = cursorpos - position_;
    last_cursor_pos_ = cursoroffset;

    if (buy_slider_.isenabled()) {
        Cursor::State bstate = buy_slider_.send_cursor(cursoroffset, clicked);

        if (bstate != Cursor::State::IDLE) {
            clear_tooltip();

            return bstate;
        }
    }

    if (sell_slider_.isenabled()) {
        Cursor::State sstate = sell_slider_.send_cursor(cursoroffset, clicked);

        if (sstate != Cursor::State::IDLE) {
            clear_tooltip();

            return sstate;
        }
    }

    int16_t xoff = cursoroffset.x();
    int16_t yoff = cursoroffset.y();
    int16_t slot = slot_by_position(yoff);

    if (slot >= 0 && slot <= 8) {
        if (xoff >= buy_x_ && xoff <= buy_width_) {
            show_item(slot, true);
        } else if (xoff >= sell_x_ && xoff <= sell_x_ + sell_width_) {
            show_item(slot, false);
        } else {
            clear_tooltip();
        }
    } else {
        clear_tooltip();
    }

    Cursor::State ret = clicked ? Cursor::State::CLICKING : Cursor::State::IDLE;

    for (size_t i = 0; i < Buttons::NUM_BUTTONS; i++) {
        if (buttons_[i]->is_active()
            && buttons_[i]->bounds(position_).contains(cursorpos)) {
            if (buttons_[i]->get_state() == Button::State::NORMAL) {
                if (i >= Buttons::BUY_ITEM && i <= Buttons::EXIT) {
                    Sound(Sound::Name::BUTTON_OVER).play();

                    buttons_[i]->set_state(Button::State::MOUSEOVER);
                    ret = Cursor::State::CAN_CLICK;
                } else {
                    buttons_[i]->set_state(Button::State::MOUSEOVER);
                    ret = Cursor::State::IDLE;
                }
            } else if (buttons_[i]->get_state() == Button::State::MOUSEOVER) {
                if (clicked) {
                    if (i >= Buttons::BUY_ITEM && i <= Buttons::CASH) {
                        if (i >= Buttons::OVERALL && i <= Buttons::CASH) {
                            Sound(Sound::Name::TAB).play();
                        } else {
                            if (i != Buttons::CHECKBOX) {
                                Sound(Sound::Name::BUTTON_CLICK).play();
                            }
                        }

                        buttons_[i]->set_state(button_pressed(i));

                        ret = Cursor::State::IDLE;
                    } else {
                        buttons_[i]->set_state(button_pressed(i));

                        ret = Cursor::State::IDLE;
                    }
                } else {
                    if (i >= Buttons::BUY_ITEM && i <= Buttons::EXIT) {
                        ret = Cursor::State::CAN_CLICK;
                    } else {
                        ret = Cursor::State::IDLE;
                    }
                }
            } else if (buttons_[i]->get_state() == Button::State::PRESSED) {
                if (clicked) {
                    if (i >= Buttons::OVERALL && i <= Buttons::CASH) {
                        Sound(Sound::Name::TAB).play();

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

void UIShop::send_scroll(double yoffset) {
    int16_t xoff = last_cursor_pos_.x();
    int16_t slider_width = 10;

    if (buy_slider_.isenabled()) {
        if (xoff >= buy_x_ && xoff <= buy_width_ + slider_width) {
            buy_slider_.send_scroll(yoffset);
        }
    }

    if (sell_slider_.isenabled()) {
        if (xoff >= sell_x_ && xoff <= sell_x_ + sell_width_ + slider_width) {
            sell_slider_.send_scroll(yoffset);
        }
    }
}

void UIShop::rightclick(Point<int16_t> cursorpos) {
    if (right_click_sell_) {
        Point<int16_t> cursoroffset = cursorpos - position_;

        int16_t xoff = cursoroffset.x();
        int16_t yoff = cursoroffset.y();
        int16_t slot = slot_by_position(yoff);

        if (slot >= 0 && slot <= 8) {
            if (xoff >= sell_x_ && xoff <= sell_x_ + sell_width_) {
                clear_tooltip();

                sell_state_.selection = slot;
                sell_state_.sell(true);
                buy_state_.selection = -1;
            }
        }
    }
}

void UIShop::send_key(int32_t keycode, bool pressed, bool escape) {
    if (pressed && escape) {
        exit_shop();
    }
}

UIElement::Type UIShop::get_type() const {
    return TYPE;
}

void UIShop::clear_tooltip() {
    UI::get().clear_tooltip(Tooltip::Parent::SHOP);
}

void UIShop::show_item(int16_t slot, bool buy) {
    if (buy) {
        buy_state_.show_item(slot);
    } else {
        sell_state_.show_item(slot);
    }
}

void UIShop::changeselltab(InventoryType::Id type) {
    uint16_t oldtab = tabbyinventory(sell_state_.tab);

    if (oldtab > 0) {
        buttons_[oldtab]->set_state(Button::State::NORMAL);
    }

    uint16_t newtab = tabbyinventory(type);

    if (newtab > 0) {
        buttons_[newtab]->set_state(Button::State::PRESSED);
    }

    sell_state_.change_tab(inventory_, type, meso_);

    sell_slider_.setrows(5, sell_state_.last_slot);

    for (size_t i = Buttons::SELL0; i < Buttons::SELL8; i++) {
        if (i - Buttons::SELL0 < sell_state_.last_slot) {
            buttons_[i]->set_state(Button::State::NORMAL);
        } else {
            buttons_[i]->set_state(Button::State::DISABLED);
        }
    }
}

void UIShop::reset(int32_t npcid) {
    std::string strid = string_format::extend_id(npcid, 7);
    npc_ = nl::nx::npc[strid + ".img"]["stand"]["0"];

    for (auto &button : buttons_) {
        button.second->set_state(Button::State::NORMAL);
    }

    buttons_[Buttons::OVERALL]->set_state(Button::State::PRESSED);
    buttons_[Buttons::EQUIP]->set_state(Button::State::PRESSED);

    buy_state_.reset();
    sell_state_.reset();

    changeselltab(InventoryType::Id::EQUIP);

    makeactive();
    right_click_sell_ = Configuration::get().get_rightclicksell();
}

void UIShop::modify(InventoryType::Id type) {
    if (type == sell_state_.tab) {
        changeselltab(type);
    }
}

void UIShop::add_item(int32_t id,
                      int32_t price,
                      int32_t pitch,
                      int32_t time,
                      int16_t buyable) {
    add_rechargable(id, price, pitch, time, 0, buyable);
}

void UIShop::add_rechargable(int32_t id,
                             int32_t price,
                             int32_t pitch,
                             int32_t time,
                             int16_t chargeprice,
                             int16_t buyable) {
    auto buyitem = BuyItem(meso_, id, price, pitch, time, chargeprice, buyable);
    buy_state_.add(buyitem);

    buy_slider_.setrows(5, buy_state_.last_slot);
}

int16_t UIShop::slot_by_position(int16_t y) {
    int16_t yoff = y - 123;

    if (yoff > 0 && yoff < 38) {
        return 0;
    }
    if (yoff > 42 && yoff < 80) {
        return 1;
    }
    if (yoff > 84 && yoff < 122) {
        return 2;
    }
    if (yoff > 126 && yoff < 164) {
        return 3;
    }
    if (yoff > 168 && yoff < 206) {
        return 4;
    }
    if (yoff > 210 && yoff < 248) {
        return 5;
    }
    if (yoff > 252 && yoff < 290) {
        return 6;
    }
    if (yoff > 294 && yoff < 332) {
        return 7;
    }
    if (yoff > 336 && yoff < 374) {
        return 8;
    }
    return -1;
}

uint16_t UIShop::tabbyinventory(InventoryType::Id type) {
    switch (type) {
        case InventoryType::Id::EQUIP: return Buttons::EQUIP;
        case InventoryType::Id::USE: return Buttons::USE;
        case InventoryType::Id::ETC: return Buttons::ETC;
        case InventoryType::Id::SETUP: return Buttons::SETUP;
        case InventoryType::Id::CASH: return Buttons::CASH;
        default: return 0;
    }
}

void UIShop::exit_shop() {
    clear_tooltip();

    deactivate();
    fn_npc_shop_action();
}

UIShop::BuyItem::BuyItem(Texture cur,
                         int32_t i,
                         int32_t p,
                         int32_t pt,
                         int32_t t,
                         int16_t cp,
                         int16_t b) :
    currency_(cur),
    id_(i),
    price_(p),
    pitch_(pt),
    time(t),
    charge_price_(cp),
    buyable_(b) {
    name_label_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::MINESHAFT);
    price_label_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::MINESHAFT);

    const ItemData &item = ItemData::get(id_);

    if (item.is_valid()) {
        icon_ = item.get_icon(false);
        name_label_.change_text(item.get_name());
    }

    std::string mesostr = std::to_string(price_);
    string_format::split_number(mesostr);
    price_label_.change_text(mesostr + "meso");
}

void UIShop::BuyItem::draw(Point<int16_t> pos) const {
    icon_.draw(pos + Point<int16_t>(0, 42));
    name_label_.draw(pos + Point<int16_t>(40, 6));
    currency_.draw(pos + Point<int16_t>(38, 29));
    price_label_.draw(pos + Point<int16_t>(55, 24));
}

int32_t UIShop::BuyItem::get_id() const {
    return id_;
}

int16_t UIShop::BuyItem::get_buyable() const {
    return buyable_;
}

UIShop::SellItem::SellItem(int32_t item_id,
                           int16_t count,
                           int16_t s,
                           bool sc,
                           Texture cur) {
    const ItemData &idata = ItemData::get(item_id);

    icon_ = idata.get_icon(false);
    id_ = item_id;
    sellable_ = count;
    slot_ = s;
    show_count_ = sc;

    name_label_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::MINESHAFT);
    price_label_ =
        Text(Text::Font::A11M, Text::Alignment::LEFT, Color::Name::MINESHAFT);

    std::string name = idata.get_name();

    if (name.length() >= 28) {
        name = name.substr(0, 28) + "..";
    }

    name_label_.change_text(name);

    int32_t price = idata.get_price();
    std::string mesostr = std::to_string(price);
    string_format::split_number(mesostr);
    price_label_.change_text(mesostr + "meso");
}

void UIShop::SellItem::draw(Point<int16_t> pos) const {
    icon_.draw(pos + Point<int16_t>(43, 42));

    if (show_count_) {
        static const Charset countset =
            Charset(nl::nx::ui["Basic.img"]["ItemNo"],
                    Charset::Alignment::LEFT);
        countset.draw(std::to_string(sellable_), pos + Point<int16_t>(41, 28));
    }

    name_label_.draw(pos + Point<int16_t>(84, 6));
    price_label_.draw(pos + Point<int16_t>(84, 24));
}

int32_t UIShop::SellItem::get_id() const {
    return id_;
}

int16_t UIShop::SellItem::get_slot() const {
    return slot_;
}

int16_t UIShop::SellItem::get_sellable() const {
    return sellable_;
}

void UIShop::BuyState::reset() {
    items.clear();

    offset = 0;
    last_slot = 0;
    selection = -1;
}

void UIShop::BuyState::draw(Point<int16_t> parentpos,
                            const Texture &selected) const {
    for (int i = 0; i < 9; i++) {
        auto slot = i + offset;

        if (slot >= last_slot) {
            break;
        }

        auto itempos = Point<int16_t>(12, 116 + 42 * i);

        if (slot == selection) {
            selected.draw(parentpos + itempos + Point<int16_t>(35, 8));
        }

        items[slot].draw(parentpos + itempos);
    }
}

void UIShop::BuyState::show_item(int16_t slot) {
    int16_t absslot = slot + offset;

    if (absslot < 0 || absslot >= last_slot) {
        return;
    }

    int32_t itemid = items[absslot].get_id();
    UI::get().show_item(Tooltip::Parent::SHOP, itemid);
}

void UIShop::BuyState::add(const BuyItem &item) {
    items.push_back(item);

    last_slot++;
}

void UIShop::BuyState::buy() const {
    if (selection < 0 || selection >= last_slot) {
        return;
    }

    const BuyItem &item = items[selection];
    int16_t buyable = item.get_buyable();
    int16_t slot = selection;
    int32_t itemid = item.get_id();

    if (buyable > 1) {
        const std::string question = "How many are you willing to buy?";

        auto onenter = [slot, itemid](int32_t qty) {
            auto shortqty = static_cast<int16_t>(qty);

            fn_npc_shop_action(slot, itemid, shortqty, true);
        };

        UI::get().emplace<UIEnterNumber>(question, onenter, buyable, 1);
    } else if (buyable > 0) {
        const std::string question = "Are you sure you want to buy it?";

        auto ondecide = [slot, itemid](bool yes) {
            if (yes) {
                fn_npc_shop_action(slot, itemid, 1, true);
            }
        };

        UI::get().emplace<UIYesNo>(question, ondecide);
    }
}

void UIShop::BuyState::select(int16_t selected) {
    int16_t slot = selected + offset;

    if (slot == selection) {
        buy();
    } else {
        selection = slot;
    }
}

void UIShop::SellState::reset() {
    items.clear();

    offset = 0;
    last_slot = 0;
    selection = -1;
    tab = InventoryType::Id::NONE;
}

void UIShop::SellState::change_tab(const Inventory &inventory,
                                   InventoryType::Id newtab,
                                   Texture meso) {
    tab = newtab;

    offset = 0;

    items.clear();

    int16_t slots = inventory.get_slotmax(tab);

    for (int i = 1; i <= slots; i++) {
        if (int32_t item_id = inventory.get_item_id(tab, i)) {
            int16_t count = inventory.get_item_count(tab, i);
            items.emplace_back(item_id,
                               count,
                               i,
                               tab != InventoryType::Id::EQUIP,
                               meso);
        }
    }

    last_slot = static_cast<int16_t>(items.size());
}

void UIShop::SellState::draw(Point<int16_t> parentpos,
                             const Texture &selected) const {
    for (int i = 0; i <= 8; i++) {
        auto slot = i + offset;

        if (slot >= last_slot) {
            break;
        }

        Point<int16_t> itempos(243, 116 + 42 * i);

        if (slot == selection) {
            selected.draw(parentpos + itempos + Point<int16_t>(78, 8));
        }

        items[slot].draw(parentpos + itempos);
    }
}

void UIShop::SellState::show_item(int16_t slot) {
    int16_t absslot = slot + offset;

    if (absslot < 0 || absslot >= last_slot) {
        return;
    }

    if (tab == InventoryType::Id::EQUIP) {
        int16_t realslot = items[absslot].get_slot();
        UI::get().show_equip(Tooltip::Parent::SHOP, realslot);
    } else {
        int32_t itemid = items[absslot].get_id();
        UI::get().show_item(Tooltip::Parent::SHOP, itemid);
    }
}

void UIShop::SellState::sell(bool skip_confirmation) const {
    if (selection < 0 || selection >= last_slot) {
        return;
    }

    const SellItem &item = items[selection];
    int32_t itemid = item.get_id();
    int16_t sellable = item.get_sellable();
    int16_t slot = item.get_slot();

    if (sellable > 1) {
        const std::string question = "How many are you willing to sell?";

        auto onenter = [itemid, slot](int32_t qty) {
            auto shortqty = static_cast<int16_t>(qty);

            fn_npc_shop_action(slot, itemid, shortqty, false);
        };

        UI::get().emplace<UIEnterNumber>(question, onenter, sellable, 1);
    } else if (sellable > 0) {
        if (skip_confirmation) {
            fn_npc_shop_action(slot, itemid, 1, false);
            return;
        }

        const std::string question = "Are you sure you want to sell it?";

        auto ondecide = [itemid, slot](bool yes) {
            if (yes) {
                fn_npc_shop_action(slot, itemid, 1, false);
            }
        };

        UI::get().emplace<UIYesNo>(question, ondecide);
    }
}

void UIShop::SellState::select(int16_t selected) {
    int16_t slot = selected + offset;

    if (slot == selection) {
        sell(false);
    } else {
        selection = slot;
    }
}
}  // namespace ms