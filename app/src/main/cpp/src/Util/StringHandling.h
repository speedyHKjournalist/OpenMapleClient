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

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../Graphics/Text.h"

namespace ms {
namespace string_conversion {
std::string to_utf8_string(uint32_t value);

std::vector<uint32_t> to_utf8_vector(const std::string &text);

template<typename T>
inline T or_default(const std::string &str, T def) {
    try {
        int32_t intval = std::stoi(str);
        return static_cast<T>(intval);
    } catch (const std::exception &ex) {
        std::cout << __func__ << ": " << ex.what() << std::endl;

        return def;
    }
}

template<typename T>
inline T or_zero(const std::string &str) {
    return or_default<T>(str, T(0));
}
}  // namespace string_conversion

namespace string_format {
// Format a number string so that each 3 decimal points
// are seperated by a ',' character.
inline void split_number(std::string &input) {
    for (size_t i = input.size(); i > 3; i -= 3) {
        input.insert(i - 3, 1, ',');
    }
}

// Prefix an id with zeros so that it has the minimum specified length
inline std::string extend_id(int32_t id, size_t length) {
    std::string strid = std::to_string(id);

    if (strid.size() < length) {
        strid.insert(0, length - strid.size(), '0');
    }

    return strid;
}

// Cut off a string at a specified length with an ellipsis
inline void format_with_ellipsis(Text &input, size_t length) {
    std::string text = input.get_text();

    while (input.width() > length) {
        text.pop_back();
        input.change_text(text + "..");
    }
}
}  // namespace string_format
}  // namespace ms