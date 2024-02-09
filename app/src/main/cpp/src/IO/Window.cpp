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

namespace ms {
    Window::Window() :
            glwnd_(nullptr),
            context_(nullptr),
            fullscreen_(false),
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

    void key_callback(GLFWwindow *, int key, int, int action, int) {
        UI::get().send_key(key, action != GLFW_RELEASE);
    }

    void char_callback(GLFWwindow *, unsigned int key) {
        UI::get().send_key(key);
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
        auto x = static_cast<int16_t>(xpos);
        auto y = static_cast<int16_t>(ypos);
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
        fullscreen_ = Setting<Fullscreen>::get().load();
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            return Error::Code::GLFW;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
//        context_ = glfwCreateWindow(1, 1, "", nullptr, nullptr);
//        glfwMakeContextCurrent(context_);
        glwnd_ = glfwCreateWindow(width_,
                                  height_,
                                  Configuration::get().get_title().c_str(),
                                  fullscreen_ ? glfwGetPrimaryMonitor() : nullptr,
                                  nullptr);
        glfwMakeContextCurrent(glwnd_);

        glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        if (Error error = GraphicsGL::get().init(pApp)) {
            return error;
        }

        return init_window();
    }

    Error Window::init_window() {
//        if (glwnd_) {
//            glfwDestroyWindow(glwnd_);
//        }

//        glwnd_ = glfwCreateWindow(width_,
//                                  height_,
//                                  Configuration::get().get_title().c_str(),
//                                  fullscreen_ ? glfwGetPrimaryMonitor() : nullptr,
//                                  context_);

        if (!glwnd_) {
            return Error::Code::WINDOW;
        }

//        glfwMakeContextCurrent(glwnd_);

        bool vsync = Setting<VSync>::get().load();
        glfwSwapInterval(vsync ? 1 : 0);

        glViewport(0, 0, width_, height_);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glfwSetInputMode(glwnd_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        double xpos = 0;
        double ypos = 0;

        glfwGetCursorPos(glwnd_, &xpos, &ypos);
        cursor_callback(glwnd_, xpos, ypos);

        glfwSetInputMode(glwnd_, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetKeyCallback(glwnd_, key_callback);
        glfwSetCharCallback(glwnd_, char_callback);
        glfwSetMouseButtonCallback(glwnd_, mousekey_callback);
        glfwSetCursorPosCallback(glwnd_, cursor_callback);
        glfwSetWindowFocusCallback(glwnd_, focus_callback);
        glfwSetScrollCallback(glwnd_, scroll_callback);
        glfwSetWindowCloseCallback(glwnd_, close_callback);

//        std::string icon_path = get_current_working_dir() + "/Icon.png";
//        GLFWimage images[1];
//
//        auto *stbi = stbi_load(icon_path.c_str(),
//                               &images[0].width,
//                               &images[0].height,
//                               nullptr,
//                               4);
//
//        if (stbi == nullptr) {
//            return Error(Error::Code::MISSING_ICON, stbi_failure_reason());
//        }
//
//        images[0].pixels = stbi;
//
//        glfwSetWindowIcon(glwnd_, 1, images);
//        stbi_image_free(images[0].pixels);

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

    std::string Window::get_current_working_dir(void) {
        char buff[FILENAME_MAX];
        GetCurrentDir(buff, FILENAME_MAX);
        std::string current_working_dir(buff);
        return current_working_dir;
    }
}  // namespace ms