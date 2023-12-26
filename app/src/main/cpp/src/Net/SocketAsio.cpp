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
#include "SocketAsio.h"

#ifdef USE_ASIO
namespace ms {
SocketAsio::SocketAsio() : resolver_(ioservice_), socket_(ioservice_) {}

SocketAsio::~SocketAsio() {
    if (socket_.is_open()) {
        error_code error;
        socket_.close(error);
    }
}

bool SocketAsio::open(const char *address, const char *port) {
    tcp::resolver::query query(address, port);
    tcp::resolver::iterator endpointiter = resolver_.resolve(query);
    error_code error;
    asio::connect(socket_, endpointiter, error);

    if (!error) {
        size_t result = socket_.read_some(asio::buffer(buffer_), error);
        return !error && (result == HANDSHAKE_LEN);
    }

    return !error;
}

bool SocketAsio::close() {
    error_code error;
    socket_.shutdown(tcp::socket::shutdown_both, error);
    socket_.close(error);

    return !error;
}

size_t SocketAsio::receive(bool *recvok) {
    if (socket_.available() > 0) {
        error_code error;
        size_t result = socket_.read_some(asio::buffer(buffer_), error);
        *recvok = !error;

        return result;
    }

    return 0;
}

const int8_t *SocketAsio::get_buffer() const {
    return buffer_;
}

bool SocketAsio::dispatch(const int8_t *bytes, size_t length) {
    error_code error;
    size_t result = asio::write(socket_, asio::buffer(bytes, length), error);

    return !error && (result == length);
}
}  // namespace ms
#endif