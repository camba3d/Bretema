#pragma once

#if __APPLE__
#    define GLFW_EXPOSE_NATIVE_COCOA
#elif _WIN64 || _WIN32
#    define GLFW_EXPOSE_NATIVE_WIN32
#elif __linux || __unix || __posix
#    define GLFW_EXPOSE_NATIVE_X11
// #  define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#define GLFW_INCLUDE_NONE
#include "btm_base.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace btm
{

class App;

class Window
{
public:
    Window(int32_t w, int32_t h, std::string const &title, App *app);

    void *handle() const;

    std::vector<char const *> extensions() const;

    inline std::string title() const { return mTitle; }
    inline void        title(std::string title) { mTitle = std::move(title); }

    void addTitleInfo(std::string const &info);

    glm::vec2 size() const;
    void      size(int32_t w, int32_t h);

    inline bool focus() { return mFocus; }
    inline void focus(bool f) { mFocus = f; }

    void destroy();
    bool isMarkedToClose() const;

    static void pollEvents();
    static void waitEvents();

    static void terminate();

private:
    bool mDelete = true;

    GLFWwindow *mHandle = nullptr;

    int32_t     mW     = 1280;
    int32_t     mH     = 720;
    std::string mTitle = "";

    bool mFocus = false;

    static inline std::vector<char const *> sExtensions                 = {};
    static inline bool                      sIsWindowContextInitialized = false;
};

}  // namespace btm
