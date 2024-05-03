//
// Created by 任睿思 on 2024/4/21.
//

#include "TapDetector.h"

#define DOUBLE_TAP_TIMEOUT_MIN 10
#define DOUBLE_TAP_TIMEOUT_MAX 200

namespace ms {
    TapDetector::TapDetector() {
        last_touch_id_ = -1;
        last_touch_timestamp_ = 0;
    }

    void TapDetector::onMotionEvent(TouchInfo &event) {
        if (event.get_phase() == GLFMTouchPhase::GLFMTouchPhaseBegan) {
            int16_t touch_id = event.get_touch_id();
            onDown(event);
            touches_[touch_id] = event;
        } else if (event.get_phase() == GLFMTouchPhase::GLFMTouchPhaseMoved) {
            onScroll(event);
            touches_[event.get_touch_id()] = event;
        } else if (event.get_phase() == GLFMTouchPhase::GLFMTouchPhaseEnded) {
            if (event.get_touch_id() == last_touch_id_ &&
                (event.get_timestamp() - last_touch_timestamp_ < DOUBLE_TAP_TIMEOUT_MAX) &&
                (event.get_timestamp() - last_touch_timestamp_ > DOUBLE_TAP_TIMEOUT_MIN)) {
                onDoubleTap(event);
            } else {
                onSingleTapUp(event);
            }
            last_touch_timestamp_ = event.get_timestamp();
            last_touch_id_ = event.get_touch_id();
            onDoubleTap(event);
            touches_.erase(event.get_touch_id());
        } else if (event.get_phase() == GLFMTouchPhase::GLFMTouchPhaseCancelled) {
            touches_.erase(event.get_touch_id());
        }
    }

    void TapDetector::onSingleTapUp(TouchInfo &event) {
        ms::UI::get().send_cursor(false);
    }

    void TapDetector::onDoubleTap(TouchInfo &event) {
        ms::UI::get().send_cursor(event.get_position().x(), event.get_position().y());
        ms::UI::get().doubleclick();
    }

    void TapDetector::onDown(TouchInfo &event) {
        ms::UI::get().send_cursor(event.get_position().x(), event.get_position().y());
        ms::UI::get().send_cursor(true);
    }

    void TapDetector::onScroll(TouchInfo &event) {
        ms::UI::get().send_cursor(event.get_position().x(), event.get_position().y());
        ms::UI::get().send_cursor(true);
    }
}