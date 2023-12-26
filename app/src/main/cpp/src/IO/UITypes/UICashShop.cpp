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
#include "UICashShop.h"

#include "../../Gameplay/Stage.h"
#include "../../Net/Packets/GameplayPackets.h"
#include "../../Net/Packets/LoginPackets.h"
#include "../Components/MapleButton.h"
#include "../UI.h"
#include "../Window.h"
#include "../../../Timer.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <nlnx/nx.hpp>

namespace ms {
namespace {
auto fn_change_map = []<typename... T>(T && ...args) {
    ChangeMapPacket(std::forward<T>(args)...).dispatch();
};
}  // namespace

UICashShop::UICashShop() :
    preview_index_(0),
    menu_index_(1),
    promotion_index_(0),
    mvp_grade_(1),
    mvp_exp_(0.07f),
    list_offset_(0) {
    nl::node CashShop = nl::nx::ui["CashShop.img"];
    nl::node Base = CashShop["Base"];
    nl::node backgrnd = Base["backgrnd"];
    nl::node BestNew = Base["BestNew"];
    nl::node Preview = Base["Preview"];
    nl::node CSTab = CashShop["CSTab"];
    nl::node CSGLChargeNX = CSTab["CSGLChargeNX"];
    nl::node CSStatus = CashShop["CSStatus"];
    nl::node CSPromotionBanner = CashShop["CSPromotionBanner"];
    nl::node CSMVPBanner = CashShop["CSMVPBanner"];
    nl::node CSItemSearch = CashShop["CSItemSearch"];
    nl::node CSChar = CashShop["CSChar"];
    nl::node CSList = CashShop["CSList"];
    nl::node CSEffect = CashShop["CSEffect"];

    sprites_.emplace_back(backgrnd);
    sprites_.emplace_back(BestNew, Point<int16_t>(139, 346));

    best_new_dim_ = Texture(BestNew).get_dimensions();

    for (size_t i = 0; i < preview_sprites_.size(); i++) {
        preview_sprites_[i] = Preview[i];
    }

    for (size_t i = 0; i < 3; i++) {
        buttons_[Buttons::BT_PREVIEW1 + i] = std::make_unique<TwoSpriteButton>(
            Base["Tab"]["Disable"][i],
            Base["Tab"]["Enable"][i],
            Point<int16_t>(957 + (i * 17), 46));
    }

    buttons_[Buttons::BT_PREVIEW1]->set_state(Button::State::PRESSED);

    buttons_[Buttons::BT_EXIT] =
        std::make_unique<MapleButton>(CSTab["BtExit"], Point<int16_t>(5, 728));
    buttons_[Buttons::BT_CHARGE_NX] =
        std::make_unique<MapleButton>(CSGLChargeNX["BtChargeNX"],
                                      Point<int16_t>(5, 554));
    buttons_[Buttons::BT_CHARGE_REFRESH] =
        std::make_unique<MapleButton>(CSGLChargeNX["BtChargeRefresh"],
                                      Point<int16_t>(92, 554));

    for (size_t i = 0; i < menu_tabs_.size(); i++) {
        menu_tabs_[i] = CSTab["Tab"][i];
    }

    buttons_[Buttons::BT_CHARGE_REFRESH] =
        std::make_unique<MapleButton>(CSGLChargeNX["BtChargeRefresh"],
                                      Point<int16_t>(92, 554));
    buttons_[Buttons::BT_WISH] =
        std::make_unique<MapleButton>(CSStatus["BtWish"],
                                      Point<int16_t>(226, 6));
    buttons_[Buttons::BT_MILEAGE] =
        std::make_unique<MapleButton>(CSStatus["BtMileage"],
                                      Point<int16_t>(869, 4));
    buttons_[Buttons::BT_HELP] =
        std::make_unique<MapleButton>(CSStatus["BtHelp"],
                                      Point<int16_t>(997, 4));
    buttons_[Buttons::BT_COUPON] =
        std::make_unique<MapleButton>(CSStatus["BtCoupon"],
                                      Point<int16_t>(950, 4));

    Charset tab;

    job_label_ = Text(Text::Font::A11B,
                      Text::Alignment::LEFT,
                      Color::Name::SUPERNOVA,
                      "Illium");
    name_label_ = Text(Text::Font::A11B,
                       Text::Alignment::LEFT,
                       Color::Name::WHITE,
                       "ShomeiZekkou");

    promotion_pos_ = Point<int16_t>(138, 40);
    sprites_.emplace_back(CSPromotionBanner["shadow"], promotion_pos_);

    promotion_sprites_.emplace_back(CSPromotionBanner["basic"]);

    buttons_[Buttons::BT_NEXT] =
        std::make_unique<MapleButton>(CSPromotionBanner["BtNext"],
                                      promotion_pos_);
    buttons_[Buttons::BT_PREV] =
        std::make_unique<MapleButton>(CSPromotionBanner["BtPrev"],
                                      promotion_pos_);

    for (size_t i = 0; i < mvp_sprites_.size(); i++) {
        mvp_sprites_[i] = CSMVPBanner["grade"][i];
    }

    mvp_pos_ = Point<int16_t>(63, 681);
    buttons_[Buttons::BT_DETAIL_PACKAGE] =
        std::make_unique<MapleButton>(CSMVPBanner["BtDetailPackage"], mvp_pos_);
    buttons_[Buttons::BT_NON_GRADE] =
        std::make_unique<MapleButton>(CSMVPBanner["BtNonGrade"], mvp_pos_);

    buttons_[Buttons::BT_DETAIL_PACKAGE]->set_active(mvp_grade_);
    buttons_[Buttons::BT_NON_GRADE]->set_active(!mvp_grade_);

    mvp_gauge_ = Gauge(Gauge::Type::CASHSHOP,
                       CSMVPBanner["gage"][0],
                       CSMVPBanner["gage"][2],
                       CSMVPBanner["gage"][1],
                       84,
                       0.0f);

    Point<int16_t> search_pos = Point<int16_t>(0, 36);
    sprites_.emplace_back(CSItemSearch["backgrnd"], search_pos);
    sprites_.emplace_back(CSItemSearch["search"],
                          search_pos + Point<int16_t>(35, 8));

    buttons_[Buttons::BT_BUY_AVATAR] =
        std::make_unique<MapleButton>(CSChar["BtBuyAvatar"],
                                      Point<int16_t>(642, 305));
    buttons_[Buttons::BT_DEFAULT_AVATAR] =
        std::make_unique<MapleButton>(CSChar["BtDefaultAvatar"],
                                      Point<int16_t>(716, 305));
    buttons_[Buttons::BT_INVENTORY] =
        std::make_unique<MapleButton>(CSChar["BtInventory"],
                                      Point<int16_t>(938, 305));
    buttons_[Buttons::BT_SAVE_AVATAR] =
        std::make_unique<MapleButton>(CSChar["BtSaveAvatar"],
                                      Point<int16_t>(864, 305));
    buttons_[Buttons::BT_TAKE_OFF_AVATAR] =
        std::make_unique<MapleButton>(CSChar["BtTakeoffAvatar"],
                                      Point<int16_t>(790, 305));

    charge_charset_ =
        Charset(CSGLChargeNX["Number"], Charset::Alignment::RIGHT);

    item_base_ = CSList["Base"];
    item_line_ = Base["line"];
    item_none_ = Base["noItem"];

    for (const nl::node &item_label : CSEffect) {
        item_labels_.emplace_back(item_label);
    }

    items_.emplace_back(5220000, Item::Label::HOT, 34000, 11);
    items_.emplace_back(5220000, Item::Label::HOT, 34000, 11);
    items_.emplace_back(5220000, Item::Label::HOT, 0, 0);
    items_.emplace_back(5220000, Item::Label::HOT, 0, 0);
    items_.emplace_back(5220000, Item::Label::HOT, 10000, 11);
    items_.emplace_back(5220000, Item::Label::NEW, 0, 0);
    items_.emplace_back(5220000, Item::Label::SALE, 7000, 0);
    items_.emplace_back(5220000, Item::Label::NEW, 13440, 0);
    items_.emplace_back(5220000, Item::Label::NEW, 7480, 0);
    items_.emplace_back(5220000, Item::Label::NEW, 7480, 0);
    items_.emplace_back(5220000, Item::Label::NEW, 7480, 0);
    items_.emplace_back(5220000, Item::Label::NONE, 12000, 11);
    items_.emplace_back(5220000, Item::Label::NONE, 22000, 11);
    items_.emplace_back(5220000, Item::Label::NONE, 0, 0);
    items_.emplace_back(5220000, Item::Label::NONE, 0, 0);
    items_.emplace_back(5220000, Item::Label::MASTER, 0, 15);

    for (size_t i = 0; i < MAX_ITEMS; i++) {
        div_t div = std::div(i, 7);

        buttons_[Buttons::BT_BUY + i] = std::make_unique<MapleButton>(
            CSList["BtBuy"],
            Point<int16_t>(146, 523)
                + Point<int16_t>(124 * div.rem, 205 * div.quot));

        item_name_[i] = Text(Text::Font::A11B,
                             Text::Alignment::CENTER,
                             Color::Name::MINESHAFT);
        item_price_[i] =
            Text(Text::Font::A11M, Text::Alignment::CENTER, Color::Name::GRAY);
        item_discount_[i] = Text(Text::Font::A11M,
                                 Text::Alignment::CENTER,
                                 Color::Name::SILVERCHALICE);
        item_percent_[i] = Text(Text::Font::A11M,
                                Text::Alignment::CENTER,
                                Color::Name::TORCHRED);
    }

    Point<int16_t> slider_pos = Point<int16_t>(1007, 372);

    list_slider_ = Slider(Slider::Type::THIN_MINESHAFT,
                          Range<int16_t>(slider_pos.y(), slider_pos.y() + 381),
                          slider_pos.x(),
                          2,
                          7,
                          [&](bool upwards) {
                              int16_t shift = upwards ? -7 : 7;
                              bool above = list_offset_ >= 0;
                              bool below = list_offset_ + shift < items_.size();

                              if (above && below) {
                                  list_offset_ += shift;

                                  update_items();
                              }
                          });

    update_items();

    dimension_ = Texture(backgrnd).get_dimensions();
}

void UICashShop::draw(float inter) const {
    preview_sprites_[preview_index_].draw(position_ + Point<int16_t>(644, 65),
                                          inter);

    UIElement::draw_sprites(inter);

    menu_tabs_[menu_index_].draw(position_ + Point<int16_t>(0, 63), inter);

    Point<int16_t> label_pos = position_ + Point<int16_t>(4, 3);
    job_label_.draw(label_pos);

    size_t length = job_label_.width();
    name_label_.draw(label_pos + Point<int16_t>(length + 10, 0));

    promotion_sprites_[promotion_index_].draw(position_ + promotion_pos_,
                                              inter);

    mvp_sprites_[mvp_grade_].draw(position_ + mvp_pos_, inter);
    mvp_gauge_.draw(position_ + mvp_pos_);

    Point<int16_t> charge_pos = position_ + Point<int16_t>(107, 388);

    charge_charset_.draw("0", charge_pos + Point<int16_t>(0, 30 * 1));
    charge_charset_.draw("3,300", charge_pos + Point<int16_t>(0, 30 * 2));
    charge_charset_.draw("0", charge_pos + Point<int16_t>(0, 30 * 3));
    charge_charset_.draw("8,698,565", charge_pos + Point<int16_t>(0, 30 * 4));
    charge_charset_.draw("0", charge_pos + Point<int16_t>(0, 30 * 5));

    if (!items_.empty()) {
        item_line_.draw(position_ + Point<int16_t>(139, 566), inter);
    } else {
        item_none_.draw(
            position_ + Point<int16_t>(137, 372)
                + Point<int16_t>(best_new_dim_.x() / 2,
                                 list_slider_.getvertical().length() / 2)
                - item_none_.get_dimensions() / 2,
            inter);
    }

    for (size_t i = 0; i < MAX_ITEMS; i++) {
        int16_t index = i + list_offset_;

        if (index < items_.size()) {
            div_t div = std::div(i, 7);
            Item item = items_[index];

            item_base_.draw(position_ + Point<int16_t>(137, 372)
                                + Point<int16_t>(124 * div.rem, 205 * div.quot),
                            inter);
            item.draw(DrawArgument(
                position_ + Point<int16_t>(164, 473)
                    + Point<int16_t>(124 * div.rem, 205 * div.quot),
                2.0f,
                2.0f));

            if (item.label != Item::Label::NONE) {
                item_labels_[item.label + 1].draw(
                    position_ + Point<int16_t>(152, 372)
                        + Point<int16_t>(124 * div.rem, 205 * div.quot),
                    inter);
            }

            item_name_[i].draw(position_ + Point<int16_t>(192, 480)
                               + Point<int16_t>(124 * div.rem, 205 * div.quot));

            if (item_discount_[i].get_text().empty()) {
                item_price_[i].draw(
                    position_ + Point<int16_t>(195, 499)
                    + Point<int16_t>(124 * div.rem, 205 * div.quot));
            } else {
                item_price_[i].draw(
                    position_ + Point<int16_t>(196, 506)
                    + Point<int16_t>(124 * div.rem, 205 * div.quot));

                item_discount_[i].draw(
                    position_ + Point<int16_t>(185, 495)
                    + Point<int16_t>(124 * div.rem, 205 * div.quot));
                item_percent_[i].draw(
                    position_
                    + Point<int16_t>(198 + (item_discount_[i].width() / 2), 495)
                    + Point<int16_t>(124 * div.rem, 205 * div.quot));
            }
        }
    }

    list_slider_.draw(position_);

    UIElement::draw_buttons(inter);
}

void UICashShop::update() {
    UIElement::update();

    mvp_gauge_.update(mvp_exp_);
}

Button::State UICashShop::button_pressed(uint16_t buttonid) {
    switch (buttonid) {
        case Buttons::BT_PREVIEW1:
        case Buttons::BT_PREVIEW2:
        case Buttons::BT_PREVIEW3:
            buttons_[preview_index_]->set_state(Button::State::NORMAL);

            preview_index_ = buttonid;
            return Button::State::PRESSED;
        case Buttons::BT_EXIT: {
            uint16_t width = Setting<Width>::get().load();
            uint16_t height = Setting<Height>::get().load();

            Constants::Constants::get().set_viewwidth(width);
            Constants::Constants::get().set_viewheight(height);

            float fadestep = 0.025f;

            Window::get().fadeout(fadestep, []() {
                GraphicsGL::get().clear();
                fn_change_map();
            });

            GraphicsGL::get().lock();
            Stage::get().clear();
            Timer::get().start();

            return Button::State::NORMAL;
        }
        case Buttons::BT_NEXT: {
            size_t size = promotion_sprites_.size() - 1;

            promotion_index_++;

            if (promotion_index_ > size) {
                promotion_index_ = 0;
            }

            return Button::State::NORMAL;
        }
        case Buttons::BT_PREV: {
            size_t size = promotion_sprites_.size() - 1;

            promotion_index_--;

            if (promotion_index_ < 0) {
                promotion_index_ = size;
            }

            return Button::State::NORMAL;
        }
        case Buttons::BT_CHARGE_NX: {
            std::string url = Configuration::get().get_chargenx();

// todo: open url on linux
#ifdef WIN32
            ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif

            return Button::State::NORMAL;
        }
        default: break;
    }

    if (buttonid >= Buttons::BT_BUY) {
        int16_t index = buttonid - Buttons::BT_BUY + list_offset_;

        Item item = items_[index];

        // TODO: Purchase item

        return Button::State::NORMAL;
    }

    return Button::State::DISABLED;
}

Cursor::State UICashShop::send_cursor(bool clicked, Point<int16_t> cursorpos) {
    Point<int16_t> cursor_relative = cursorpos - position_;

    if (list_slider_.isenabled()) {
        Cursor::State state =
            list_slider_.send_cursor(cursor_relative, clicked);

        if (state != Cursor::State::IDLE) {
            return state;
        }
    }

    return UIElement::send_cursor(clicked, cursorpos);
}

UIElement::Type UICashShop::get_type() const {
    return TYPE;
}

void UICashShop::update_items() {
    for (size_t i = 0; i < MAX_ITEMS; i++) {
        int16_t index = i + list_offset_;
        bool found_item = index < items_.size();

        buttons_[Buttons::BT_BUY + i]->set_active(found_item);

        std::string name = "";
        std::string price_text = "";
        std::string discount_text = "";
        std::string percent_text = "";

        if (found_item) {
            Item item = items_[index];

            name = item.get_name();

            int32_t price = item.get_price();
            price_text = std::to_string(price);

            if (item.discount_price > 0 && price > 0) {
                discount_text = price_text;

                uint32_t discount = item.discount_price;
                price_text = std::to_string(discount);

                float_t percent = (float)discount / price;
                std::string percent_str = std::to_string(percent);
                percent_text = "(" + percent_str.substr(2, 1) + "%)";
            }

            string_format::split_number(price_text);
            string_format::split_number(discount_text);

            price_text += " NX";

            if (!discount_text.empty()) {
                discount_text += " NX";
            }

            if (item.count > 0) {
                price_text += "(" + std::to_string(item.count) + ")";
            }
        }

        item_name_[i].change_text(name);
        item_price_[i].change_text(price_text);
        item_discount_[i].change_text(discount_text);
        item_percent_[i].change_text(percent_text);

        string_format::format_with_ellipsis(item_name_[i], 92);
    }
}
}  // namespace ms