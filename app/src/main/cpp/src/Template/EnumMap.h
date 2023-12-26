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
#include <type_traits>
#include <utility>

#ifdef _WIN32
#include <stdexcept>
#endif

namespace ms {
template<typename K, typename V, K LENGTH = K::LENGTH>
// Wraps an array so that it is addressable by enum values.
class EnumMap {
public:
    template<typename... Args>
    // Initialize with an initializer list.
    EnumMap(Args &&...args) : values_ { { std::forward<Args>(args)... } } {
        static_assert(std::is_enum<K>::value,
                      "Template parameter 'K' for EnumMap must be an enum.");

        for (size_t i = 0; i < LENGTH; i++) {
            keys_[i] = static_cast<K>(i);
        }
    }

    void clear() {
        for (size_t i = 0; i < LENGTH; i++) {
            values_[i] = V();
        }
    }

    void erase(K key) {
        if (key >= 0 && key < LENGTH) {
            values_[key] = V();
        }
    }

    template<typename... Args>
    void emplace(K key, Args &&...args) {
        values_[key] = { std::forward<Args>(args)... };
    }

    V &operator[](K key) { return values_[key]; }

    const V &operator[](K key) const { return values_[key]; }

    template<typename T>
    class base_iterator {
    public:
        using index_type = typename std::underlying_type<K>::type;
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        base_iterator(T *p, index_type i) : value_(p), index_(i) {}

        struct node {
            K first;
            T &second;

            node(K f, T &s) : first(f), second(s) {}

            node &operator=(const node &) = delete;

            void set(const T &t) { second = t; }
        };

        node operator*() { return node { first(), second() }; }

        explicit operator bool() const {
            return index_ >= 0 && index_ < LENGTH;
        }

        K first() const { return static_cast<K>(index_); }

        T &second() { return *(value_ + index_); }

        base_iterator &operator++() {
            index_++;
            return *this;
        }

        bool operator!=(const base_iterator &other) const {
            return index_ != other.index_;
        }

        bool operator==(const base_iterator &other) const {
            return index_ == other.index_;
        }

    private:
        T *value_;
        index_type index_;
    };

    using iterator = base_iterator<V>;
    using const_iterator = base_iterator<const V>;

    iterator find(K key) { return { values_.data(), key }; }

    const_iterator find(K key) const { return { values_.data(), key }; }

    iterator begin() { return { values_.data(), 0 }; }

    iterator end() { return { values_.data(), LENGTH }; }

    const_iterator begin() const { return { values_.data(), 0 }; }

    const_iterator end() const { return { values_.data(), LENGTH }; }

    const std::array<K, LENGTH> &keys() const { return keys_; }

    std::array<V, LENGTH> &values() { return values_; }

    const std::array<V, LENGTH> &values() const { return values_; }

private:
    std::array<K, LENGTH> keys_;
    std::array<V, LENGTH> values_;
};
}  // namespace ms