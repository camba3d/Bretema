#pragma once

#if __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#elif _WIN64 || _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif __linux || __unix || __posix
#define GLFW_EXPOSE_NATIVE_X11
// #  define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "Base.hpp"

namespace btm
{

class App;

class Window
{
public:
    Window(int32_t w, int32_t h, std::string const &title);

    void *handle() const; // @dani : should be private (check implications)

    std::vector<char const *> exts() const;

    glm::vec2 size() const;
    void      size(int32_t w, int32_t h);

    bool        isMarkedToClose() const;
    static void pollEvents();
    static void waitEvents();

    void        destroy();
    static void terminate();

    void titleInfo(std::string const &info);

private:
    GLFWwindow *mHandle = nullptr;
    int32_t     mW      = 1280;
    int32_t     mH      = 720;
    std::string mTitle  = "";

    bool mDelete = true;

    static inline std::vector<char const *> sExts  = {};
    static inline bool                      sReady = false;

    static inline std::string const sDebugTag = "BTM_WINDOW => ";

    friend class btm::App;
};

} // namespace btm
