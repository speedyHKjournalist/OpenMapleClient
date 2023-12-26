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
#include <memory>
#include <string_view>

#include "Forwarder.h"
#include "PacketHandler.h"

namespace ms {
// Class which contains the array of handler classes to use
class PacketSwitch : public Forwarder {
public:
    // Register all handlers
    PacketSwitch();

    // Forward a packet to the correct handler
    void forward(int8_t *bytes, size_t length) const override;

private:
    // Print a warning
    void warn(std::string_view message, size_t opcode) const;

    // Opcodes for which handlers can be registered
    enum Opcode : uint16_t;

    // Message when an unhandled packet is received
    static constexpr std::string_view MSG_UNHANDLED =
        "Unhandled packet detected";
    // Message when a packet with a larger opcode than the array size is
    // received
    static constexpr std::string_view MSG_OUT_OF_BOUNDS =
        "Large opcode detected";
    // Message when a packet with a larger opcode than the array size is
    // received
    static constexpr std::string_view MSG_REREGISTER =
        "Handler was registered twice";
    // Maximum number of handlers needed
    static constexpr size_t NUM_HANDLERS = 500;

    std::array<std::unique_ptr<PacketHandler>, NUM_HANDLERS> handlers_;

    /**
     * @brief Register a handler for the specified opcode
     *
     * @tparam O opcode
     * @tparam T packet handler
     * @tparam Args types
     * @param args parameter pack
     */
    template<size_t O, typename T, typename... Args>
    void emplace(Args &&... args) {
        static_assert(O < NUM_HANDLERS,
                      "PacketSwitch::emplace - Opcode out of array bounds");
        static_assert(std::is_base_of<PacketHandler, T>::value,
                      "Error: Packet handlers must derive from PacketHandler");

        if (handlers_[O]) {
            warn(MSG_REREGISTER, O);
        }

        handlers_[O] = std::make_unique<T>(std::forward<Args>(args)...);
    }
};
}  // namespace ms