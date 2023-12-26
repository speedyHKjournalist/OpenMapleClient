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
#include <string>
#include <string_view>

namespace ms {
// Error codes to be checked after initialization.
class Error {
public:
    enum Code {
        NONE,
        CONNECTION,
        NLNX,
        MISSING_FILE,
        WRONG_UI_FILE,
        GLFW,
        GLEW,
        FREETYPE,
        VERTEX_SHADER,
        FRAGMENT_SHADER,
        SHADER_PROGRAM_LINK,
        SHADER_PROGRAM_VALID,
        SHADER_VARS,
        WINDOW,
        AUDIO,
        MISSING_ICON,
        FONT_PATH,
        LENGTH
    };

    Error(Code c) : Error(c, "This error has no accompanying details.") {}
    Error(Code c, const char *args) : error_code_(c), msg_details_(args) {}

    operator bool() const { return error_code_ != Code::NONE; }

    bool can_retry() const {
        return error_code_ == Code::CONNECTION
               || error_code_ == Code::MISSING_FILE
               || error_code_ == Code::WRONG_UI_FILE
               || error_code_ == Code::MISSING_ICON
               || error_code_ == Code::FONT_PATH;
    }

    std::string_view get_message() const { return msgs_[error_code_]; }

    std::string_view get_details() const { return msg_details_; }

private:
    const Code error_code_;
    const std::string msg_details_;

    static constexpr const char* msgs_[Code::LENGTH] = {
        "",
        "Cannot connect to server.",
        "Could not initialize NLNX.",
        "Missing a game file: ",
        "UI.nx has wrong version.",
        "Could not initialize GLFW.",
        "Could not initialize GLEW.",
        "Could not initialize FreeType.",
        "Failed to compile vertex shader.",
        "Failed to compile fragment shader.",
        "Failed to link shader program.",
        "Failed to validate shader program.",
        "Failed to locate shader variables.",
        "Failed to create window.",
        "Failed to initialize audio.",
        "Could not load icon.",
        "Could not load fonts."
    };
};
}  // namespace ms