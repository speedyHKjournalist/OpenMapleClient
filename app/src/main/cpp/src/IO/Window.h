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

#include "../Error.h"
#include "../Template/Singleton.h"
#include "glfw/deps/glad/gl.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <unistd.h>
#include <iostream>
#include <android_native_app_glue.h>
#include <unistd.h>
#define GetCurrentDir getcwd

namespace ms {
    class Window : public Singleton<Window> {
    public:
        Window();

        ~Window() override;

        Error init(android_app *pApp);

        Error init_window(android_app *pApp);

        bool not_closed() const;

        void update();

        void begin() const;

        void end() const;

        void fadeout(float step, std::function<void()> fadeprocedure);

        void check_events();

        void set_clipboard(const std::string &text) const;

        std::string get_clipboard() const;

        void toggle_fullscreen();

    private:
        void update_opc();

        GLFWwindow *glwnd_;
        GLFWwindow *context_;
        bool fullscreen_;
        float opacity_;
        float opc_step_;
        std::function<void()> fade_procedure_;
        int16_t width_;
        int16_t height_;

        std::string get_current_working_dir();
    };
}  // namespace ms