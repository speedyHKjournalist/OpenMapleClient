//
// Created by 任睿思 on 2024/4/21.
//
#include "TouchInfo.h"
#include "UI.h"
#include "thread"

namespace ms {
    class TapDetector {
    public:
        TapDetector();
        void onMotionEvent(TouchInfo& event);

    private:
        void onSingleTapUp(TouchInfo& event);
        void onDoubleTap(TouchInfo& event);
        void onDown(TouchInfo& event);
        void onScroll(TouchInfo& event);

        std::unordered_map<int16_t, TouchInfo> touches_;
        int16_t last_touch_id_;
        long last_touch_timestamp_;
    };
}
