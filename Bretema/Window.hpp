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
#include "Base.hpp"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace btm
{

class App;

class Window
{
public:
    Window(int32_t w, int32_t h, std::string const &title);

    void *handle() const;  // @dani : should be private? (check implications)

    std::vector<char const *> extensions() const;

    std::string title() const { return mTitle; }
    void        title(std::string title) { mTitle = std::move(title); }
    void        titleInfo(std::string const &info);

    glm::vec2 size() const;
    void      size(int32_t w, int32_t h);

    void destroy();
    bool shouldClose() const;

    static void pollEvents();
    static void waitEvents();

    static void terminate();

private:
    bool mDelete = true;

    GLFWwindow *mHandle = nullptr;
    int32_t     mW      = 1280;
    int32_t     mH      = 720;
    std::string mTitle  = "";

    static inline std::vector<char const *> sExtensions = {};

    static inline bool sIsWindowContextInitialized = false;

    static inline std::string const sLogTag = "BTM_WINDOW => ";

    friend class btm::App;
};

}  // namespace btm
