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

#include "file_impl.hpp"
#include "node_impl.hpp"
#ifdef _WIN32
#    ifdef __MINGW32__
#        include <windows.h>
#    else
#        include <Windows.h>
#    endif // __MINGW32__
#else
#    include <cerrno>
#    include <cstring>
#    include <sys/fcntl.h>
#    include <sys/mman.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif
#include <stdexcept>

#ifndef NDEBUG
#    include <iostream>
#endif

namespace nl
{
file::file(std::string name)
{
    open(name);
}

file::~file()
{
    close();
}

void file::open(std::string name)
{
    close();
    m_data = new data();

#ifdef _WIN32
    m_data->file_handle = ::CreateFileA(name.c_str(),
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        nullptr,
                                        OPEN_EXISTING,
                                        FILE_FLAG_RANDOM_ACCESS,
                                        nullptr);
    if (m_data->file_handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open file " + name);
    m_data->map = ::CreateFileMappingA(
        m_data->file_handle, 0, PAGE_READONLY, 0, 0, nullptr);
    if (!m_data->map)
        throw std::runtime_error("Failed to create file mapping of file " +
                                 name);
    m_data->base = ::MapViewOfFile(m_data->map, FILE_MAP_READ, 0, 0, 0);
    if (!m_data->base)
        throw std::runtime_error("Failed to map view of file " + name);
#else
    m_data->file_handle = ::open(name.c_str(), O_RDONLY);
    if (m_data->file_handle == -1) {
        throw std::runtime_error("Failed to open file " + name +
                                 ", errno: " + std::strerror(errno));
    }

    struct stat finfo;
    if (::fstat(m_data->file_handle, &finfo) == -1) {
        throw std::runtime_error("Failed to obtain file information of file " +
                                 name + ", errno: " + std::strerror(errno));
    }

    m_data->size = finfo.st_size;
    m_data->base = ::mmap(
        nullptr, m_data->size, PROT_READ, MAP_SHARED, m_data->file_handle, 0);
    if (m_data->base == MAP_FAILED) {
        throw std::runtime_error("Failed to create memory mapping of file " +
                                 name + ", errno: " + std::strerror(errno));
    }

#    ifndef NDEBUG
    std::cout << "=== File " << name << " mapped to address " << m_data->base
              << " ===\n";
#    endif

#endif

    m_data->header = reinterpret_cast<header const*>(m_data->base);
    if (m_data->header->magic != 0x34474B50) {
        throw std::runtime_error(name + " is not a PKG4 NX file (incorrect "
                                        "magic)");
    }

    m_data->node_table = reinterpret_cast<node::data const*>(
        reinterpret_cast<char const*>(m_data->base) +
        m_data->header->node_offset);
    m_data->string_table = reinterpret_cast<uint64_t const*>(
        reinterpret_cast<char const*>(m_data->base) +
        m_data->header->string_offset);
    m_data->bitmap_table = reinterpret_cast<uint64_t const*>(
        reinterpret_cast<char const*>(m_data->base) +
        m_data->header->bitmap_offset);
    m_data->audio_table = reinterpret_cast<uint64_t const*>(
        reinterpret_cast<char const*>(m_data->base) +
        m_data->header->audio_offset);
}

void file::close()
{
    if (!m_data) {
        return;
    }

#ifdef _WIN32
    ::UnmapViewOfFile(m_data->base);
    ::CloseHandle(m_data->map);
    ::CloseHandle(m_data->file_handle);
#else
    ::munmap(const_cast<void*>(m_data->base), m_data->size);
    ::close(m_data->file_handle);
#endif

    delete m_data;
    m_data = nullptr;
}

node file::root() const
{
    return {m_data->node_table, m_data};
}

file::operator node() const
{
    return root();
}

std::uint32_t file::string_count() const
{
    return m_data->header->string_count;
}

std::uint32_t file::bitmap_count() const
{
    return m_data->header->bitmap_count;
}

std::uint32_t file::audio_count() const
{
    return m_data->header->audio_count;
}

std::uint32_t file::node_count() const
{
    return m_data->header->node_count;
}

std::string file::get_string(std::uint32_t i) const
{
    auto s =
        reinterpret_cast<char const*>(m_data->base) + m_data->string_table[i];
    return {s + 2, *reinterpret_cast<std::uint16_t const*>(s)};
}
} // namespace nl
