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
#include "Session.h"

#include "../Configuration.h"
#include "PacketError.h"

namespace ms {
Session::Session(std::unique_ptr<Forwarder> packet_forwarder) :
    packet_forwarder_(std::move(packet_forwarder)),
    length_(0),
    pos_(0),
    is_connected_(false) {}

Session::~Session() {
    if (is_connected_) {
        socket_.close();
    }
}

bool Session::init(const char *host, const char *port) {
    // Connect to the server
    is_connected_ = socket_.open(host, port);

    if (is_connected_) {
        // Read keys necessary for communicating with the server
        cryptography_ = { socket_.get_buffer() };
    }

    return is_connected_;
}

std::string Session::readHostIP(GLFMDisplay* pApp) {
    std::string filePath = std::string(((ANativeActivity*)glfmGetAndroidActivity(pApp))->externalDataPath) + "/hostip.txt";
    FILE* file = fopen(filePath.c_str(), "rb");
    std::string hostIP;

    if (file != nullptr) {
        char buffer[256]; // Buffer to store read data
        size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
        fclose(file);

        // Ensure the read operation succeeded
        if (bytesRead > 0) {
            // Convert the buffer to a string
            hostIP.assign(buffer, bytesRead);
        }
    }

    return hostIP;
}

Error Session::init(GLFMDisplay *pApp) {
//    std::string HOST = Setting<ServerIP>::get().load();
    std::string HOST = readHostIP(pApp);
    std::string PORT = Setting<ServerPort>::get().load();

    if (!init(HOST.c_str(), PORT.c_str())) {
        return Error::CONNECTION;
    }

    return Error::NONE;
}

void Session::reconnect(const char *address, const char *port) {
    // Close the current connection and open a new one
    bool success = socket_.close();

    if (success) {
        init(address, port);
    } else {
        is_connected_ = false;
    }
}

void Session::process(const int8_t *bytes, size_t available) {
    if (pos_ == 0) {
        // Position is zero, meaning this is the start of a new packet. Start by
        // determining length.
        length_ = cryptography_.check_length(bytes);
        // Reading the length means we processed the header. Move forward by the
        // header length.
        bytes = bytes + HEADER_LENGTH;
        available -= HEADER_LENGTH;
    }

    // Determine how much we can write. Write data into the buffer.
    size_t towrite = length_ - pos_;

    if (towrite > available) {
        towrite = available;
    }

    memcpy(buffer_ + pos_, bytes, towrite);
    pos_ += towrite;

    // Check if the current packet has been fully processed
    if (pos_ >= length_) {
        cryptography_.decrypt(buffer_, length_);

        try {
            packet_forwarder_->forward(buffer_, length_);
        } catch (const PacketError &err) {
            std::cout << err.what() << std::endl;
        }

        pos_ = 0;
        length_ = 0;

        // Check if there is more available
        size_t remaining = available - towrite;

        if (remaining >= MIN_PACKET_LENGTH) {
            // More packets are available, so we start over.
            process(bytes + towrite, remaining);
        }
    }
}

void Session::write(int8_t *packet_bytes, size_t packet_length) {
    if (!is_connected_) {
        return;
    }

    int8_t header[HEADER_LENGTH];
    cryptography_.create_header(header, packet_length);
    cryptography_.encrypt(packet_bytes, packet_length);

    socket_.dispatch(header, HEADER_LENGTH);
    socket_.dispatch(packet_bytes, packet_length);
}

void Session::read() {
    // Check if a packet has arrived. Handle if data is sufficient: 4 bytes
    // (header) + 2 bytes (opcode) = 6 bytes.
    size_t result = socket_.receive(&is_connected_);

    if (result >= MIN_PACKET_LENGTH || length_ > 0) {
        // Retrieve buffer from the socket and process it
        const int8_t *bytes = socket_.get_buffer();
        process(bytes, result);
    }
}

void Session::reconnect() {
    std::string HOST = Setting<ServerIP>::get().load();
    std::string PORT = Setting<ServerPort>::get().load();

    reconnect(HOST.c_str(), PORT.c_str());
}

bool Session::is_connected() const {
    return is_connected_;
}
}  // namespace ms