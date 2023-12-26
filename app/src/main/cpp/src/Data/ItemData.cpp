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
#include "ItemData.h"

#include <array>
#include <nlnx/nx.hpp>
#include <string>

namespace ms {
ItemData::ItemData(int32_t id) : item_id_(id) {
    untradable = false;
    unique_ = false;
    unsellable_ = false;
    cash_item_ = false;
    gender_ = 0;

    nl::node src;
    nl::node strsrc;

    std::string strprefix = "0" + std::to_string(get_item_prefix(item_id_));
    std::string strid = "0" + std::to_string(item_id_);
    int32_t prefix = get_prefix(item_id_);

    switch (prefix) {
        case 1:
            category_ = get_eqcategory(item_id_);
            src = nl::nx::character[category_][strid + ".img"]["info"];
            strsrc = nl::nx::string["Eqp.img"]["Eqp"][category_]
                                   [std::to_string(item_id_)];
            break;
        case 2:
            category_ = "Consume";
            src = nl::nx::item["Consume"][strprefix + ".img"][strid]["info"];
            strsrc = nl::nx::string["Consume.img"][std::to_string(item_id_)];
            break;
        case 3:
            category_ = "Install";
            src = nl::nx::item["Install"][strprefix + ".img"][strid]["info"];
            strsrc = nl::nx::string["Ins.img"][std::to_string(item_id_)];
            break;
        case 4:
            category_ = "Etc";
            src = nl::nx::item["Etc"][strprefix + ".img"][strid]["info"];
            strsrc = nl::nx::string["Etc.img"]["Etc"][std::to_string(item_id_)];
            break;
        case 5:
            category_ = "Cash";
            src = nl::nx::item["Cash"][strprefix + ".img"][strid]["info"];
            strsrc = nl::nx::string["Cash.img"][std::to_string(item_id_)];
            break;
    }

    if (src) {
        icons_[false] = src["icon"];
        icons_[true] = src["iconRaw"];
        price_ = src["price"];
        untradable = src["tradeBlock"].get_bool();
        unique_ = src["only"].get_bool();
        unsellable_ = src["notSale"].get_bool();
        cash_item_ = src["cash"].get_bool();
        gender_ = get_item_gender(item_id_);

        name_ = std::string(strsrc["name"]);
        desc_ = std::string(strsrc["desc"]);

        valid_ = true;
    } else {
        valid_ = false;
    }
}

std::string ItemData::get_eqcategory(int32_t id) const {
    const std::array<std::string, 15> categorynames = {
        "Cap",      "Accessory", "Accessory", "Accessory", "Coat",
        "Longcoat", "Pants",     "Shoes",     "Glove",     "Shield",
        "Cape",     "Ring",      "Accessory", "Accessory", "Accessory"
    };

    int32_t index = get_item_prefix(id) - 100;

    if (index < categorynames.size()) {
        return categorynames[index];
    }

    if (index >= 30 && index <= 70) {
        return "Weapon";
    }

    return "";
}

int32_t ItemData::get_prefix(int32_t id) const {
    return id / 1000000;
}

int32_t ItemData::get_item_prefix(int32_t id) const {
    return id / 10000;
}

int8_t ItemData::get_item_gender(int32_t id) const {
    const int32_t item_prefix = get_item_prefix(id);

    if ((get_prefix(id) != 1 && item_prefix != 254) || item_prefix == 119
        || item_prefix == 168) {
        return 2;
    }

    const int32_t gender_digit = id / 1000 % 10;

    return (gender_digit > 1) ? 2 : gender_digit;
}

bool ItemData::is_valid() const {
    return valid_;
}

bool ItemData::is_untradable() const {
    return untradable;
}

bool ItemData::is_unique() const {
    return unique_;
}

bool ItemData::is_unsellable() const {
    return unsellable_;
}

bool ItemData::is_cashitem() const {
    return cash_item_;
}

ItemData::operator bool() const {
    return is_valid();
}

int32_t ItemData::get_id() const {
    return item_id_;
}

int32_t ItemData::get_price() const {
    return price_;
}

int8_t ItemData::get_gender() const {
    return gender_;
}

const std::string &ItemData::get_name() const {
    return name_;
}

const std::string &ItemData::get_desc() const {
    return desc_;
}

const std::string &ItemData::get_category() const {
    return category_;
}

const Texture &ItemData::get_icon(bool raw) const {
    return icons_[raw];
}
}  // namespace ms