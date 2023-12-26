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

#include <array>

#include "../../Data/ItemData.h"
#include "../../Graphics/Text.h"
#include "../Components/Charset.h"
#include "../Components/Gauge.h"
#include "../Components/Slider.h"
#include "../UIElement.h"

namespace ms {
class UICashShop : public UIElement {
public:
    static constexpr Type TYPE = UIElement::Type::CASHSHOP;
    static constexpr bool FOCUSED = true;
    static constexpr bool TOGGLED = false;

    UICashShop();

    void draw(float inter) const override;
    void update() override;

    Button::State button_pressed(uint16_t buttonid) override;

    Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;

    UIElement::Type get_type() const override;

private:
    void update_items();

    static constexpr uint8_t MAX_ITEMS = 7u * 2u + 1u;

    class Item {
    public:
        enum Label : uint8_t {
            ACTION,
            BOMB_SALE,
            BONUS,
            EVENT = 4,
            HOT,
            LIMITED,
            LIMITED_BRONZE,
            LIMITED_GOLD,
            LIMITED_SILVER,
            LUNA_CRYSTAL,
            MASTER = 12,
            MUST,
            NEW,
            SALE = 17,
            SPEICAL,
            SPECIAL_PRICE,
            TIME,
            TODAY,
            WEEKLY,
            WONDER_BERRY,
            WORLD_SALE,
            NONE
        };

        Item(int32_t itemid, Label label, int32_t discount, uint16_t count) :
            label(label),
            discount_price(discount),
            count(count),
            data_(ItemData::get(itemid)) {}

        Label label;
        int32_t discount_price;
        uint16_t count;

        void draw(const DrawArgument &args) const {
            data_.get_icon(false).draw(args);
        }

        const std::string get_name() const { return data_.get_name(); }

        const int32_t get_price() const { return data_.get_price(); }

    private:
        const ItemData &data_;
    };

    enum Buttons : uint16_t {
        BT_PREVIEW1,
        BT_PREVIEW2,
        BT_PREVIEW3,
        BT_EXIT,
        BT_CHARGE_NX,
        BT_CHARGE_REFRESH,
        BT_WISH,
        BT_MILEAGE,
        BT_HELP,
        BT_COUPON,
        BT_NEXT,
        BT_PREV,
        BT_DETAIL_PACKAGE,
        BT_NON_GRADE,
        BT_BUY_AVATAR,
        BT_DEFAULT_AVATAR,
        BT_INVENTORY,
        BT_SAVE_AVATAR,
        BT_TAKE_OFF_AVATAR,
        BT_BUY
    };

    Point<int16_t> best_new_dim_;

    std::array<Sprite, 3> preview_sprites_;
    uint8_t preview_index_;

    std::array<Sprite, 9> menu_tabs_;
    uint8_t menu_index_;

    Text job_label_;
    Text name_label_;

    std::vector<Sprite> promotion_sprites_;
    Point<int16_t> promotion_pos_;
    int8_t promotion_index_;

    std::array<Sprite, 7> mvp_sprites_;
    Point<int16_t> mvp_pos_;
    uint8_t mvp_grade_;
    Gauge mvp_gauge_;
    float_t mvp_exp_;

    Charset charge_charset_;

    Sprite item_base_;
    Sprite item_line_;
    Sprite item_none_;
    std::vector<Sprite> item_labels_;
    std::vector<Item> items_;
    std::array<Text, MAX_ITEMS> item_name_;
    std::array<Text, MAX_ITEMS> item_price_;
    std::array<Text, MAX_ITEMS> item_discount_;
    std::array<Text, MAX_ITEMS> item_percent_;

    Slider list_slider_;
    int16_t list_offset_;
};
}  // namespace ms