#include <gl.h>
#include <android/window.h>
#include "glfm.h"

void glfmMain(GLFMDisplay *display) {
    ANativeActivity_setWindowFlags((ANativeActivity *) glfmGetAndroidActivity(display),
                                   AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
    ms::game.start(display);
}