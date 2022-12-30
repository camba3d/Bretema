#include "btm_window.hpp"

#include "btm_app.hpp"

#include <vector>

namespace btm
{

using w_IS = Input::State;
using w_IK = Input::Key;
using w_IM = Input::Mouse;

static void *sMainWindow = nullptr;

static umap<void *, btm::Window *> sHandleToWindow {};

App &BTM_APP(GLFWwindow *handle)
{
    App *ptApp = ((App *)(glfwGetWindowUserPointer(handle)));
    BTM_ASSERT(ptApp);
    return *ptApp;
}
btm::Window &WIN_SELF(GLFWwindow *handle)
{
    BTM_ASSERT(sHandleToWindow.count(handle) > 0);
    btm::Window *ptWin = sHandleToWindow[handle];
    BTM_ASSERT(ptWin);
    return *ptWin;
}

Window::Window(int32_t w, int32_t h, std::string const &title, App *app) : mW(w), mH(h), mTitle(title)
{
    BTM_ABORT_IF(!app, "Window requires a valid App pointer");

    bool const wasWindowContextInitialized = sIsWindowContextInitialized;

    if (!sIsWindowContextInitialized)
    {
        BTM_ABORT_IF(!glfwInit(), "Couldn't initialize Window-Manager");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // Avoid OpenGL context creation
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);      // Resize windows takes special care
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);  // Focus the window when its opened

        uint32_t     extsCount = 0;
        char const **exts      = glfwGetRequiredInstanceExtensions(&extsCount);
        sExtensions            = exts ? decltype(sExtensions) { exts, exts + extsCount } : decltype(sExtensions) {};

        sIsWindowContextInitialized = true;
    }

    // Create window object
    mHandle = glfwCreateWindow(mW, mH, (app->name() + " :: " + mTitle).c_str(), nullptr, nullptr);
    BTM_ABORT_IF(!mHandle, "Couldn't create a new Window");

    // Store app pointer into window object
    glfwSetWindowUserPointer(mHandle, app);

    // Store window handle to object relation
    sHandleToWindow[mHandle] = this;

    // Store main window object
    if (!wasWindowContextInitialized)
        sMainWindow = mHandle;

    // clang-format off

    // Window Dependent Events
    // . Resize
    glfwSetFramebufferSizeCallback(mHandle, [](GLFWwindow *p, int w, int h) { WIN_SELF(p).size(w, h); });
    // . Focus
    glfwSetCursorEnterCallback(mHandle, [](GLFWwindow *p, int focus) { WIN_SELF(p).focus((bool)focus); });
    // . On Close
    glfwSetWindowCloseCallback(mHandle, [](GLFWwindow *p) { if (p == sMainWindow) BTM_APP(p).markToClose(); });
    
    // Window Independent Events
    // . Cursor
    glfwSetCursorPosCallback(mHandle, [](GLFWwindow *p, double x, double y) { BTM_APP(p).cursor({x, y}); });
    // . Keyboard
    glfwSetKeyCallback(mHandle, [](GLFWwindow *p, int k, int, int a, int) { BTM_APP(p).key((w_IK)k, (w_IS)a); });
    // . Mouse
    glfwSetMouseButtonCallback(mHandle, [](GLFWwindow *p, int b, int a, int) { BTM_APP(p).mouse((w_IM)b, (w_IS)a); });

    // clang-format on
}

bool Window::isMarkedToClose() const
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

glm::vec2 Window::size() const
{
    return { mW, mH };
}
void Window::size(int32_t w, int32_t h)
{
    mW = w;
    mH = h;
}

void *Window::handle() const
{
    return mHandle;
}

std::vector<char const *> Window::extensions() const
{
    return sExtensions;
}

void Window::addTitleInfo(std::string const &info)
{
    if (mHandle)
        glfwSetWindowTitle(mHandle, (BTM_APP(mHandle).name() + " :: " + mTitle + " ::" + info).c_str());
}

}  // namespace btm
