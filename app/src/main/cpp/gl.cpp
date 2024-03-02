#include <gl.h>
#include <android/native_activity.h>
#include "glfm.h"

void glfmMain(GLFMDisplay *display) {
    ms::game.start(display);
}