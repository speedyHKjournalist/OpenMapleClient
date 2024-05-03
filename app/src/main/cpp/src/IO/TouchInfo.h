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
        TouchInfo(GLFMTouchPhase phase, Point<float_t> position, long timestamp, int16_t touch_id);
        GLFMTouchPhase get_phase();
        int16_t get_touch_id() const;
        Point<float_t>& get_position();
        long get_timestamp() const;
    private:
        GLFMTouchPhase phase_;
        Point<float_t> position_;
        long timestamp_;
        int16_t touch_id_;
    };
}
