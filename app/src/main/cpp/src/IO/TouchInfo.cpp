//
// Created by 任睿思 on 2024/4/21.
//

#include "TouchInfo.h"

namespace ms {
    TouchInfo::TouchInfo(GLFMTouchPhase phase, Point<float_t> position, long timestamp, int16_t touch_id) {
        phase_ = phase;
        position_ = position;
        timestamp_ = timestamp;
        touch_id_ = touch_id;
    }

    GLFMTouchPhase TouchInfo::get_phase() {
        return phase_;
    }

    int16_t TouchInfo::get_touch_id() const {
        return touch_id_;
    }

    Point<float_t> &TouchInfo::get_position() {
        return position_;
    }

    long TouchInfo::get_timestamp() const {
        return timestamp_;
    }
}