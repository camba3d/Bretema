#include "Window.hpp"

#include <vector>

namespace btm
{

using w_IS = Input::State;
using w_IK = Input::Key;
using w_IM = Input::Mouse;

// ===== CTOR =====

Window::Window(int32_t w, int32_t h, std::string const &title) : mW(w), mH(h), mTitle(title)
{
    if (!sReady)
    {
        BTM_ABORT_IF(!glfwInit(), sDebugTag + "Initializing Window-Manager");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // Avoid OpenGL context creation
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);      // Resize windows takes special care
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);  // Focus the window when its opened

        uint32_t     extsCount = 0;
        char const **exts      = glfwGetRequiredInstanceExtensions(&extsCount);
        sExts                  = exts ? decltype(sExts){exts, exts + extsCount} : decltype(sExts){};

        sReady = true;
    }

    // mHandle = glfwCreateWindow(mW, mH, (BTM_APP->mName + " :: " + mTitle).c_str(), nullptr, nullptr);
    mHandle = glfwCreateWindow(mW, mH, ("Bretema :: " + mTitle).c_str(), nullptr, nullptr);
    BTM_ABORT_IF(!mHandle, sDebugTag + "Creating a new Window");

    // // Window Dependent Events
    // // Resize
    // glfwSetFramebufferSizeCallback(mHandle, [](GLFWwindow *p, int w, int h) { BTM_APP->size(p, w, h); });
    // // Focus
    // glfwSetCursorEnterCallback(mHandle, [](GLFWwindow *p, int focus) { focus ? BTM_APP->focus(p) : []() {}(); });

    // // Window Independent Events
    // // Keyboard
    // glfwSetKeyCallback(mHandle, [](GLFWwindow *, int k, int, int a, int) { BTM_APP->key((w_IK)k, (w_IS)a); });
    // // Mouse
    // glfwSetMouseButtonCallback(mHandle, [](GLFWwindow *, int b, int a, int) { BTM_APP->mouse((w_IM)b, (w_IS)a); });
    // // Cursor
    // glfwSetCursorPosCallback(mHandle, [](GLFWwindow *, double x, double y) { BTM_APP->cursor(x, y); });
}

// ===== ACTIONs =====

bool Window::shouldClose() const
{
    return mHandle ? glfwWindowShouldClose(mHandle) : true;
}
void Window::destroy()
{
    if (mHandle)
    {
        glfwDestroyWindow(mHandle);
        mHandle = nullptr;
    }
}

void Window::pollEvents()
{
    glfwPollEvents();
}
void Window::waitEvents()
{
    glfwWaitEvents();
}

void Window::terminate()
{
    glfwTerminate();
}

// ===== GETTERs =====

glm::vec2 Window::size() const
{
    return {mW, mH};
}
std::vector<char const *> Window::extensions() const
{
    return sExtensions;
}
void *Window::handle() const
{
    return mHandle;
}

// ===== SETTERs =====

void Window::size(int32_t w, int32_t h)
{
    mW = w;
    mH = h;
    // BTM_INFOF("Window '{}' SIZE => {}, {}", mTitle, mW, mH);
}

void Window::titleInfo(std::string const &info)
{
    if (mHandle)
        glfwSetWindowTitle(mHandle, ("Bretema :: " + mTitle + " ::" + info).c_str());
    // glfwSetWindowTitle(mHandle, (BTM_APP->mName + " :: " + mTitle + " ::" + info).c_str());
}

}  // namespace btm
