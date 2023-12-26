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

#include <dlfcn.h>

#include "egl_context.h"
#include "osmesa_context.h"
#include "posix_time.h"
#include "posix_thread.h"
#include "android_joystick.h"
#include <android/native_window.h>
#include <android_native_app_glue.h>

#define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
#define _glfw_dlclose(handle) dlclose(handle)
#define _glfw_dlsym(handle, name) dlsym(handle, name)

#define _GLFW_EGL_NATIVE_WINDOW  ((EGLNativeWindowType) window->android->window)
#define _GLFW_PLATFORM_WINDOW_STATE         struct android_app* android;
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE android_gstate android;
#define _GLFW_PLATFORM_MONITOR_STATE
#define _GLFW_PLATFORM_CURSOR_STATE

#define _GLFW_PLATFORM_CONTEXT_STATE
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE
#define _GLFW_EGL_NATIVE_DISPLAY EGL_DEFAULT_DISPLAY

// The created flag is set true when we receive the onCreate event (ANativeActivity_onCreate).
// The created flag is cleared to false when we receive the onDestroy event.
#define _GLFW_ANDROID_LIFECYCLE_STATE_CREATED (1 << 0)

// The started flag is set true when we receive the onStart event.
// The started flag is cleared to false when we receive the onStop event.
#define _GLFW_ANDROID_LIFECYCLE_STATE_STARTED (1 << 1)

// The resumed flag is set true when we receive the onResume event.
// The resumed flag is cleared to false when we receive the onPause event.
#define _GLFW_ANDROID_LIFECYCLE_STATE_RESUMED (1 << 2)

// The surface flag is set true when we receive the surfaceCreate event (normally in the resumed state).
// The surface flag is cleared to false when we receive surfaceDestroyed.
#define _GLFW_ANDROID_LIFECYCLE_STATE_SURFACE (1 << 3)

// The focus flag is set true when we receive a focus-gained event
// The focus flag is cleared to false when we receive a focus-lost event
#define _GLFW_ANDROID_LIFECYCLE_STATE_FOCUSED (1 << 4)

// The App should be run when created + started + resumed + surface + focus flags are set
#define _GLFW_ANDROID_LIFECYCLE_CHECK (0x1F)

typedef struct android_gstate {
    int lifecycle_state;

    struct android_app* app;
    struct android_poll_source* source;

    int window_created;
    int last_cursor_x;
    int last_cursor_y;

} android_gstate;

typedef VkFlags VkAndroidSurfaceCreateFlagsKHR;

typedef struct VkAndroidSurfaceCreateInfoKHR {
    VkStructureType                   sType;
    const void*                       pNext;
    VkAndroidSurfaceCreateFlagsKHR    flags;
    ANativeWindow*                    window;
} VkAndroidSurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateAndroidSurfaceKHR)(VkInstance,const VkAndroidSurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);
