//
// Created by 任睿思 on 2024/3/15.
//
#pragma once

#include "../Template/Point.h"
#include "../../thirdparty/glfm/include/glfm.h"
#include <cmath>

namespace ms {
    class TouchInfo {
    public:
        TouchInfo() = default;

        TouchInfo(const Point <double_t> &pos, GLFMTouchPhase p)
                : relative_pos(pos), phase(p) {}

        Point <double_t> relative_pos;
        GLFMTouchPhase phase;
    };
}
