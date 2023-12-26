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

#include "../Template/Singleton.h"
#include "PacketSwitch.h"
#include "Session.h"

namespace ms {
/**
 * @brief For performing actions over network.
 *
 */
class PacketProcessor : public Singleton<PacketProcessor> {
public:
    PacketProcessor() : session_(std::make_unique<PacketSwitch>()) {}

    auto init() { return session_.init(); }

    /**
     * @brief Process outgoing data.
     *
     * @param data to process
     * @param size data size
     */
    void process(int8_t *data, size_t size) { session_.write(data, size); }

    /**
     * @brief Process incoming data.
     *
     */
    void process() { session_.read(); }

    void reconnect() { session_.reconnect(); }

    void reconnect(const std::string &ip, const std::string &port) {
        session_.reconnect(ip.c_str(), port.c_str());
    }

    bool is_connected() { return session_.is_connected(); }

private:
    Session session_;
};
}  // namespace ms