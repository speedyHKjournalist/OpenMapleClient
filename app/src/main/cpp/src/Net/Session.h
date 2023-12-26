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

#include <memory>

#include "../Error.h"
#include "../MSClient.h"
#include "Cryptography.h"
#include "Forwarder.h"

#ifdef USE_ASIO
#include "SocketAsio.h"
#else
#include "SocketWinsock.h"
#endif

namespace ms {
class Session {
public:
    Session(std::unique_ptr<Forwarder> packet_forwarder);
    ~Session();

    // Connect using host and port from the configuration file
    Error init();
    // Send a packet to the server
    void write(int8_t *bytes, size_t length);
    // Check for incoming packets and handle them
    void read();
    // Closes the current connection and opens a new one with default connection
    // settings
    void reconnect();
    // Closes the current connection and opens a new one
    void reconnect(const char *address, const char *port);
    // Check if the connection is alive
    bool is_connected() const;

private:
    bool init(const char *host, const char *port);
    void process(const int8_t *bytes, size_t available);

    Cryptography cryptography_;
    std::unique_ptr<Forwarder> packet_forwarder_;

    int8_t buffer_[MAX_PACKET_LENGTH];
    size_t length_;
    size_t pos_;
    bool is_connected_;

#ifdef USE_ASIO
    SocketAsio socket_;
#else
    SocketWinsock socket_;
#endif
};
}  // namespace ms