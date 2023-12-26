#include <gl.h>

extern "C" {

int entrance(struct android_app *app) {
    ms::Game game;
    game.start(app);
//    glfwInit();
//
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
//    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
//    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Window Title", NULL, NULL);
//    glfwMakeContextCurrent(window);
//
//    while (1) {
//        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//        glfwSwapBuffers(window);
//    }
}
}
