#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdint>
#include <cinttypes>
#include <jni.h>
#include <unordered_map>
#include "polyray_WallpaperWindow.h"

struct GLWindowData {
    HDC hdc;
    HGLRC hglrc;
};

std::unordered_map<HWND, GLWindowData> windowMap;

BOOL CALLBACK EnumWindowsProcCallback(HWND hwnd, LPARAM lParam) {
    HWND shellDllDefView = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
    if (shellDllDefView != NULL) {
        HWND workerw = FindWindowEx(NULL, hwnd, "WorkerW", NULL);
        if (workerw != NULL) {
            *(HWND*)lParam = workerw;
            return FALSE;
        }
    }
    return TRUE;
}

JNIEXPORT jlong JNICALL Java_polyray_WallpaperWindow_createWindow (JNIEnv* env, jobject) {
    HWND progman = FindWindowA("Progman", NULL);
    if (!progman) {
        return 0;
    }
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);
    HWND workerw = NULL;
    EnumWindows(EnumWindowsProcCallback, (LPARAM)&workerw);
    if (!workerw) {
        return 0;
    }
    HDC hdc = GetDC(workerw);
    if (!hdc) {
        return 0;
    }

    // Pixel format for OpenGL
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0) {
        return 0;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        return 0;
    }

    HGLRC hglrc = wglCreateContext(hdc);
    if (!hglrc) {
        return 0;
    }
    if (!wglMakeCurrent(hdc, hglrc)) {
        return 0;
    }

    windowMap[workerw] = {hdc, hglrc};
    return reinterpret_cast<jlong>(workerw);
}

JNIEXPORT void JNICALL Java_polyray_WallpaperWindow_swapBuffers (JNIEnv* env, jobject, jlong hwndLong) {
    HWND hwnd = reinterpret_cast<HWND>(hwndLong);
    auto it = windowMap.find(hwnd);
    if (it != windowMap.end()) {
        SwapBuffers(it->second.hdc);
    }
}

JNIEXPORT void JNICALL Java_polyray_WallpaperWindow_deleteWindow (JNIEnv* env, jobject, jlong hwndLong) {
    HWND hwnd = reinterpret_cast<HWND>(hwndLong);
    auto it = windowMap.find(hwnd);
    if (it != windowMap.end()) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(it->second.hglrc);
        ReleaseDC(hwnd, it->second.hdc);
        windowMap.erase(it);
    }
}