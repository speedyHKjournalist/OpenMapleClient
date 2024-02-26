#include <gl.h>
#include <android/native_activity.h>

extern "C" {

int entrance(struct android_app *app) {
    keep_screen_always_on(app);
    ms::Game game;
    game.start(app);
}
}

void keep_screen_always_on(android_app *app) {
    ANativeActivity* activity = app->activity;
    // Get the native activity's JNIEnv
    JNIEnv* env;
    activity->vm->AttachCurrentThread(&env, NULL);

    // Get the WindowManager.LayoutParams class and its constants
    jclass layoutParamsClass = env->FindClass("android/view/WindowManager$LayoutParams");
    jfieldID flagKeepScreenOn = env->GetStaticFieldID(layoutParamsClass, "FLAG_KEEP_SCREEN_ON", "I");
    jint keepScreenOnFlag = env->GetStaticIntField(layoutParamsClass, flagKeepScreenOn);

    // Set the FLAG_KEEP_SCREEN_ON flag
    ANativeActivity_setWindowFlags(activity, keepScreenOnFlag, 0);

    // Detach the thread
    activity->vm->DetachCurrentThread();
}
