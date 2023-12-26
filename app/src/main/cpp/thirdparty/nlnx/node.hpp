//////////////////////////////////////////////////////////////////////////////
// NoLifeNx - Part of the NoLifeStory project                               //
// Copyright © 2013 Peter Atashian                                          //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "nxfwd.hpp"

#include <cstdint>
#include <cstdio>
#include <string>

namespace nl
{
struct _file_data;
typedef std::pair<std::int32_t, std::int32_t> vector;
class node
{
public:
    struct data;
    //! Type of node data.
    enum class type : std::uint16_t {
        none = 0,
        integer = 1,
        real = 2,
        string = 3,
        vector = 4,
        bitmap = 5,
        audio = 6,
    };

    // Constructors.
    node() = default;
    node(node const&); // Only reason this isn't defaulted is because MSVC has
                       // issues.
    node& operator=(node const&) = default;
    // These methods are primarily so nodes can be used as iterators and
    // iterated over.
    node begin() const;
    node end() const;
    node operator*() const;
    node& operator++();
    node operator++(int);
    bool operator==(node const&) const;
    bool operator!=(node const&) const;
    //! This checks whether or not the node points to an actual node.
    //! Even if the node ends up being a null node, you can still use it
    //! safely; it'll just fall back to returning default values and more null
    //! nodes. However, if the file this node was obtained from was deleted
    //! then the node becomes invalid and this operator **cannot** tell you
    //! that.
    explicit operator bool() const;
    // Methods to access the children of the node by name.
    // Note that the versions taking integers convert the integer to a string.
    // They do not access the children by their integer index.
    // If you wish to do that, use `some_node.begin() + integer_index`.
    node operator[](std::uint16_t) const;
    node operator[](std::int16_t) const;
    node operator[](std::uint32_t) const;
    node operator[](std::int32_t) const;
    node operator[](std::uint64_t) const;
    node operator[](std::int64_t) const;
    node operator[](std::string_view) const;
    node operator[](const char*) const;
    //! This method uses the string value of the node, not the node's name
    node operator[](node const&) const;
    // Operators to easily cast a node to get the data.
    // Allows things like `std::string s = some_node;`.
    // Will automatically cast between data types as needed.
    // For example if the node has an integer value but you want a string,
    // then the operator will automatically convert the integer to a string.
    operator unsigned char() const;
    operator signed char() const;
    operator unsigned short() const;
    operator signed short() const;
    operator unsigned int() const;
    operator signed int() const;
    operator unsigned long() const;
    operator signed long() const;
    operator unsigned long long() const;
    operator signed long long() const;
    operator float() const;
    operator double() const;
    operator long double() const;
    operator std::string() const;
    operator vector() const;
    operator bitmap() const;
    operator audio() const;
    // Explicitly called versions of all the operators.
    // When it takes a parameter, that is used as the default value
    // in the case that a suitable data value cannot be found in the node.
    std::int64_t get_integer(std::int64_t = 0) const;
    double get_real(double = 0) const;
    std::string get_string(std::string = "") const;
    vector get_vector(vector = {0, 0}) const;
    bitmap get_bitmap() const;
    audio get_audio() const;
    bool get_bool() const;
    bool get_bool(bool) const;
    //! Returns the x coordinate of the vector data value.
    std::int32_t x() const;
    //! Returns the y coordinate of the vector data value.
    std::int32_t y() const;
    //! The name of the node.
    std::string name() const;
    //! The number of children in the node.
    std::size_t size() const;
    //! Gets the type of data contained within the node.
    type data_type() const;
    //! Returns the root node of the file this node was derived from.
    node root() const;
    //! Takes a '/' separated string view, and resolves the given path.
    node resolve(std::string_view) const noexcept;

private:
    node(data const*, _file_data const*);
    node get_child(std::string_view o) const;
    std::int64_t to_integer() const;
    double to_real() const;
    std::string to_string() const;
    vector to_vector() const;
    bitmap to_bitmap() const;
    audio to_audio() const;
    // Internal variables
    data const* m_data = nullptr;
    _file_data const* m_file = nullptr;
    friend file;
};
// More convenience string concatenation operators
std::string operator+(std::string, node);
std::string operator+(char const*, node);
std::string operator+(node, std::string);
std::string operator+(node, char const*);
} // namespace nl
