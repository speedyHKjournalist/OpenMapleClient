//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2017 Curi0 <curi0minecraft@gmail.com>
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <android/log.h>
#include "internal.h"
#include <unistd.h>

#define LOGV(...) (__android_log_print(ANDROID_LOG_VERBOSE, "GLFW3", __VA_ARGS__))
#define LOGI(...) (__android_log_print(ANDROID_LOG_INFO,    "GLFW3", __VA_ARGS__))
#define LOGW(...) (__android_log_print(ANDROID_LOG_WARN,    "GLFW3", __VA_ARGS__))
#define LOGE(...) (__android_log_print(ANDROID_LOG_ERROR,   "GLFW3", __VA_ARGS__))
#define LOGF(...) (__android_log_print(ANDROID_LOG_FATAL,   "GLFW3", __VA_ARGS__))

struct android_app* _globalApp;

extern int entrance(struct android_app* app);
void handle_cmd(struct android_app* _app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_DESTROY: // onDestroy
            _glfw.android.lifecycle_state &= ~_GLFW_ANDROID_LIFECYCLE_STATE_CREATED;
            LOGV("[App Destroyed]");
            break;

        case APP_CMD_START: // onStart
            LOGV("[App Started]");
            _glfw.android.lifecycle_state |= _GLFW_ANDROID_LIFECYCLE_STATE_STARTED;
            break;
        case APP_CMD_STOP: // onStop
            _glfw.android.lifecycle_state &= ~_GLFW_ANDROID_LIFECYCLE_STATE_STARTED;
            LOGV("[App Stopped]");
            break;

        case APP_CMD_RESUME: // onResume
            LOGV("[App Resumed]");
            _glfw.android.lifecycle_state |= _GLFW_ANDROID_LIFECYCLE_STATE_RESUMED;
            break;
        case APP_CMD_PAUSE: // onPause
            LOGV("[App Paused]");
            _glfw.android.lifecycle_state &= ~_GLFW_ANDROID_LIFECYCLE_STATE_RESUMED;
            break;

        case APP_CMD_GAINED_FOCUS: { // onWindowFocusChanged
            LOGV("[Window Gained Focus]");
            _glfw.android.lifecycle_state |= _GLFW_ANDROID_LIFECYCLE_STATE_FOCUSED;
            break;
        }
        case APP_CMD_LOST_FOCUS: { // onWindowFocusChanged
            LOGV("[Window Lost Focus]");
            _glfw.android.lifecycle_state &= ~_GLFW_ANDROID_LIFECYCLE_STATE_FOCUSED;
            break;
        }

        case APP_CMD_INIT_WINDOW: { // onNativeWindowCreated (surfaceCreated)
            _glfw.android.lifecycle_state |= _GLFW_ANDROID_LIFECYCLE_STATE_SURFACE;
            LOGV("[Surface Created]");
            break;
        }
        case APP_CMD_WINDOW_RESIZED: { // onNativeWindowResized (surfaceChanged)
            LOGV("[Surface Resized]");
            break;
        }
        case APP_CMD_TERM_WINDOW: { // onNativeWindowDestroyed (surfaceDestroyed)
            LOGV("[Surface Destroyed]");
            _glfw.android.lifecycle_state &= ~_GLFW_ANDROID_LIFECYCLE_STATE_SURFACE;
            _glfwInputWindowCloseRequest(_glfw.windowListHead);
            break;
        }

        case APP_CMD_CONFIG_CHANGED: { // onConfigurationChanged
            LOGV("[Configuration Changed] New orientation: %d", AConfiguration_getOrientation(_globalApp->config));
            break;
        }


    }
}

// Android Entry Point
void android_main(struct android_app *app) { // ANativeActivity_onCreate (onCreate)
    LOGV("[App Created]");
    _glfw.android.lifecycle_state |= _GLFW_ANDROID_LIFECYCLE_STATE_CREATED;

    app->onAppCmd = handle_cmd;

    // hmmm...global....eek
    _globalApp = app;

    struct android_poll_source* source;
    do {
        if ((_glfw.android.lifecycle_state & _GLFW_ANDROID_LIFECYCLE_CHECK) == _GLFW_ANDROID_LIFECYCLE_CHECK) {
            LOGV("[Starting Main]");
            entrance(app);
            LOGV("[Main Finished] destroyRequested: %d", app->destroyRequested);
        } else {
            usleep(10);
        }

        ALooper_pollAll(-1, NULL, NULL,(void**)&source);

        if (source != NULL) {
            source->process(app, source);
        }
    } while (!app->destroyRequested); // Continue with the event loop until android_app->destroyRequested

    LOGV("[Completely Done] destroyRequested: %d", app->destroyRequested);
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformInit(void) {
    _glfw.android.lifecycle_state = 0;
    _glfw.android.app = _globalApp;
    _glfw.android.source = 0;
    _glfw.android.window_created = 0;
    _glfw.android.last_cursor_x = 0;
    _glfw.android.last_cursor_y = 0;

    _glfwInitTimerPOSIX();
    return GLFW_TRUE;
}

void _glfwPlatformTerminate(void) {
    _glfwTerminateOSMesa();
}

const char* _glfwPlatformGetVersionString(void) {
    return _GLFW_VERSION_NUMBER " Android EGL";
}
