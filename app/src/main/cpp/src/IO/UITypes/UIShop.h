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
#pragma once

#include "../Character/Look/CharLook.h"
#include "../Components/Slider.h"
#include "../Graphics/Text.h"
#include "../UIDragElement.h"

namespace ms {
class UIShop : public UIDragElement<PosSHOP> {
public:
    static constexpr Type TYPE = UIElement::Type::SHOP;
    static constexpr bool FOCUSED = true;
    static constexpr bool TOGGLED = true;

    UIShop(const CharLook &charlook, const Inventory &inventory);

    void draw(float alpha) const override;

    void update() override;

    void remove_cursor() override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> position) override;

    void send_scroll(double yoffset) override;

    void rightclick(Point<int16_t> cursorpos) override;

    void send_key(int32_t keycode, bool pressed, bool escape) override;

    UIElement::Type get_type() const override;

    void reset(int32_t npcid);

    void add_item(int32_t id,
                  int32_t price,
                  int32_t pitch,
                  int32_t time,
                  int16_t buyable);

    void add_rechargable(int32_t id,
                         int32_t price,
                         int32_t pitch,
                         int32_t time,
                         int16_t chargeprice,
                         int16_t buyable);

    void modify(InventoryType::Id type);

protected:
    Button::State button_pressed(uint16_t buttonid) override;

private:
    void clear_tooltip();

    void show_item(int16_t slot, bool sale);

    void changeselltab(InventoryType::Id tab);

    int16_t slot_by_position(int16_t y);

    uint16_t tabbyinventory(InventoryType::Id type);

    void exit_shop();

    enum Buttons : int16_t {
        BUY_ITEM,
        SELL_ITEM,
        EXIT,
        CHECKBOX,
        OVERALL,
        EQUIP,
        USE,
        ETC,
        SETUP,
        CASH,
        BUY0,
        BUY1,
        BUY2,
        BUY3,
        BUY4,
        BUY5,
        BUY6,
        BUY7,
        BUY8,
        SELL0,
        SELL1,
        SELL2,
        SELL3,
        SELL4,
        SELL5,
        SELL6,
        SELL7,
        SELL8,
        NUM_BUTTONS
    };

    const CharLook &charlook_;
    const Inventory &inventory_;

    Texture npc_;
    Texture buy_selection_;
    Texture sell_selection_;
    Texture meso_;
    Text meso_label_;

    Slider buy_slider_;
    Slider sell_slider_;

    int16_t buy_x_;
    int16_t buy_width_;
    int16_t sell_x_;
    int16_t sell_width_;

    BoolPair<Texture> checkbox_;

    bool right_click_sell_;

    Point<int16_t> last_cursor_pos_;

    class BuyItem {
    public:
        BuyItem(Texture cur,
                int32_t i,
                int32_t p,
                int32_t pt,
                int32_t t,
                int16_t cp,
                int16_t b);

        void draw(Point<int16_t> position) const;

        int32_t get_id() const;

        int16_t get_buyable() const;

    private:
        Texture icon_;
        Texture currency_;
        int32_t id_;
        int32_t price_;
        int32_t pitch_;
        int32_t time;
        int16_t charge_price_;
        int16_t buyable_;
        Text name_label_;
        Text price_label_;
    };

    class SellItem {
    public:
        SellItem(int32_t item_id,
                 int16_t count,
                 int16_t slot,
                 bool showcount,
                 Texture cur);

        void draw(Point<int16_t> position) const;

        int32_t get_id() const;

        int16_t get_slot() const;

        int16_t get_sellable() const;

    private:
        Texture icon_;
        int32_t id_;
        int16_t slot_;
        int16_t sellable_;
        bool show_count_;
        Text name_label_;
        Text price_label_;
    };

    struct BuyState {
        std::vector<BuyItem> items;
        int16_t offset;
        int16_t last_slot;
        int16_t selection;

        void reset();

        void draw(Point<int16_t> position, const Texture &selected) const;

        void show_item(int16_t slot);

        void add(const BuyItem &item);

        void buy() const;

        void select(int16_t selected);
    };

    BuyState buy_state_;

    struct SellState {
        std::vector<SellItem> items;
        int16_t offset;
        InventoryType::Id tab;
        int16_t last_slot;
        int16_t selection;

        void reset();

        void change_tab(const Inventory &inventory,
                        InventoryType::Id type,
                        Texture meso);

        void draw(Point<int16_t> position, const Texture &selected) const;

        void show_item(int16_t slot);

        void sell(bool skip_confirmation) const;

        void select(int16_t selected);
    };

    SellState sell_state_;
};
}  // namespace ms