//	This file is part of the continued Journey MMORPG client
//	Copyright (C) 2015-2020  Daniel Allendorf, Ryan Payton
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

#include <functional>
#include <optional>

template<typename D,
         typename B,
         typename = std::enable_if_t<std::is_base_of<B, D>::value
                                     || std::is_base_of<D, B>::value>>
constexpr std::optional<std::reference_wrapper<D>> create_optional(B *ptr) {
    if (ptr == nullptr) {
        return {};
    }

    auto *elem = static_cast<D *>(ptr);
    return std::optional<std::reference_wrapper<D>>(*elem);
}

template<typename D,
         typename B,
         typename = std::enable_if_t<std::is_base_of<B, D>::value
                                     || std::is_base_of<D, B>::value>>
constexpr std::optional<std::reference_wrapper<D>> create_optional(
    std::optional<std::reference_wrapper<B>> opt) {
    if (!opt) {
        return {};
    }

    return create_optional<D>(&opt->get());
}