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
#include "Window.h"
#include "../Configuration.h"
#include "../Constants.h"
#include "../Graphics/GraphicsGL.h"
#include "../Timer.h"
#include "UI.h"
#include "stb/stb_image.h"

#include <utility>
#include <android/log.h>

namespace ms {
    Window::Window() :
            glwnd_(nullptr),
            context_(nullptr),
            fullscreen_(true),
            opacity_(1.0f),
            opc_step_(0.0f),
            width_(Constants::Constants::get().get_viewwidth()),
            height_(Constants::Constants::get().get_viewheight()) {
    }

    Window::~Window() {
        glfwTerminate();
    }

    void error_callback(int no, const char *description) {
        std::cout << "GLFW error [" << no << "]: " << description << std::endl;
    }

    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods, int32_t metastate) {
    // Found that in glfw-android GLFW_PRESS and GLFW_RELEASE are reversed
        UI::get().send_key(scancode, action != GLFW_PRESS);
        bool shiftPressed = (metastate == AMETA_SHIFT_ON || metastate == AMETA_CAPS_LOCK_ON);
        int ascii = UI::get().get_keyboard().Key2Character(scancode, shiftPressed);
        bool specialCharacter = UI::get().get_keyboard().IsSpecialCharacter();
        if(action == GLFW_PRESS && !specialCharacter) {
            // char_callback does not work, so handle the keycode manually in key_callback
            UI::get().send_key(ascii);
        }
    }

    std::chrono::time_point<std::chrono::steady_clock> start =
            ContinuousTimer::get().start();

    void mousekey_callback(GLFWwindow *, int button, int action, int) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                switch (action) {
                    case GLFW_PRESS: UI::get().send_cursor(true); break;
                    case GLFW_RELEASE: {
                        auto diff_ms = ContinuousTimer::get().stop(start) / 1000;
                        start = ContinuousTimer::get().start();

                        if (diff_ms > 10 && diff_ms < 200) {
                            UI::get().doubleclick();
                        }

                        UI::get().send_cursor(false);
                    } break;
                }

                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                switch (action) {
                    case GLFW_PRESS: UI::get().rightclick(); break;
                }

                break;
        }
    }

    void cursor_callback(GLFWwindow *, double xpos, double ypos) {
        double ratio_y = 600.0 / Constants::Constants::get().get_viewheight();
        auto x = static_cast<int16_t>(xpos * ratio_y);
        auto y = static_cast<int16_t>(ypos * ratio_y) ;
        Point<int16_t> pos = Point<int16_t>(x, y);
        UI::get().send_cursor(pos);
    }

    void focus_callback(GLFWwindow *, int focused) {
        UI::get().send_focus(focused);
    }

    void scroll_callback(GLFWwindow *, double xoffset, double yoffset) {
        UI::get().send_scroll(yoffset);
    }

    void close_callback(GLFWwindow *window) {
        UI::get().send_close();

        glfwSetWindowShouldClose(window, GL_FALSE);
    }

    Error Window::init(android_app *pApp) {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            return Error::Code::GLFW;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glwnd_ = glfwCreateWindow(Constants::Constants::get().get_viewwidth(),
                                  Constants::Constants::get().get_viewheight(),
                                  Configuration::get().get_title().c_str(),
                                  nullptr,
                                  nullptr);
        glfwMakeContextCurrent(glwnd_);

        glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        if (Error error = GraphicsGL::get().init(pApp)) {
            return error;
        }

        return init_window(pApp);
    }

    Error Window::init_window(android_app *pApp) {
        if (!glwnd_) {
            return Error::Code::WINDOW;
        }

        bool vsync = Setting<VSync>::get().load();
        glfwSwapInterval(vsync ? 1 : 0);
        glViewport(0, 0, Constants::Constants::get().get_viewwidth(), Constants::Constants::get().get_viewheight());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glfwSetInputMode(glwnd_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        double xpos = 0;
        double ypos = 0;

        glfwGetCursorPos(glwnd_, &xpos, &ypos);
        cursor_callback(glwnd_, xpos, ypos);

        glfwSetInputMode(glwnd_, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetKeyCallback(glwnd_, key_callback);
        glfwSetMouseButtonCallback(glwnd_, mousekey_callback);
        glfwSetCursorPosCallback(glwnd_, cursor_callback);
        glfwSetWindowFocusCallback(glwnd_, focus_callback);
        glfwSetScrollCallback(glwnd_, scroll_callback);
        glfwSetWindowCloseCallback(glwnd_, close_callback);

        GraphicsGL::get().reinit();

        return Error::Code::NONE;
    }

    bool Window::not_closed() const {
        return glfwWindowShouldClose(glwnd_) == 0;
    }

    void Window::update() {
        update_opc();
    }

    void Window::update_opc() {
        if (opc_step_ != 0.0f) {
            opacity_ += opc_step_;

            if (opacity_ >= 1.0f) {
                opacity_ = 1.0f;
                opc_step_ = 0.0f;
            } else if (opacity_ <= 0.0f) {
                opacity_ = 0.0f;
                opc_step_ = -opc_step_;

                fade_procedure_();
            }
        }
    }

    void Window::check_events() {
        int16_t max_width = Configuration::get().get_max_width();
        int16_t max_height = Configuration::get().get_max_height();
        int16_t new_width = Constants::Constants::get().get_viewwidth();
        int16_t new_height = Constants::Constants::get().get_viewheight();

        if (width_ != new_width || height_ != new_height) {
            width_ = new_width;
            height_ = new_height;

            if (new_width >= max_width || new_height >= max_height) {
                fullscreen_ = true;
            }

//            init_window();
        }

        glfwPollEvents();
    }

    void Window::begin() const {
        GraphicsGL::get().clear_scene();
    }

    void Window::end() const {
        GraphicsGL::get().flush(opacity_);
        glfwSwapBuffers(glwnd_);
    }

    void Window::fadeout(float step, std::function<void()> fadeproc) {
        opc_step_ = -step;
        fade_procedure_ = std::move(fadeproc);
    }

    void Window::set_clipboard(const std::string &text) const {
        glfwSetClipboardString(glwnd_, text.c_str());
    }

    std::string Window::get_clipboard() const {
        const char *text = glfwGetClipboardString(glwnd_);

        return text != nullptr ? text : "";
    }

    void Window::toggle_fullscreen() {
        int16_t max_width = Configuration::get().get_max_width();
        int16_t max_height = Configuration::get().get_max_height();

        if (width_ < max_width && height_ < max_height) {
            fullscreen_ = !fullscreen_;
            Setting<Fullscreen>::get().save(fullscreen_);

//            init_window();
            glfwPollEvents();
        }
    }
}  // namespace ms