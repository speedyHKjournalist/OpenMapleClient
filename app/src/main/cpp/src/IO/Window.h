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
#include "glfm.h"
#include "TapDetector.h"
#include <functional>
#include <string>
#include <unistd.h>
#include <iostream>
#include <unistd.h>

namespace ms {
    class Window : public Singleton<Window> {
    public:
        Window();

        ~Window() override;

        Error init(GLFMDisplay *pApp);

        Error init_window();

        bool not_closed() const;

        void update();

        void begin() const;

        void end() const;

        void fadeout(float step, std::function<void()> fadeprocedure);

        void check_events();

        void set_clipboard(const std::string &text) const;

        std::string get_clipboard() const;

        void toggle_fullscreen();

        void set_ratio(float ratio_x, float ratio_y);

        float get_ratio_x();

        float get_ratio_y();

        GLFMDisplay* get_display();

    private:
        void update_opc();

        GLFMDisplay *display;
        bool fullscreen_;
        float opacity_;
        float opc_step_;
        float ratio_x;
        float ratio_y;
        std::function<void()> fade_procedure_;
        int16_t width_;
        int16_t height_;
    };
}  // namespace ms