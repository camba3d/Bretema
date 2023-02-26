#include "btm_window.hpp"

#include "btm_app.hpp"

#include <vector>

namespace btm
{

using IS = Input::State;
using IK = Input::Key;
using IM = Input::Mouse;

static void *sMainWindow = nullptr;

static umap<void *, Window *> sHandleToWindow {};

App &BTM_APP(GLFWwindow *handle)
{
    App *ptApp = ((App *)(glfwGetWindowUserPointer(handle)));
    BTM_ASSERT(ptApp);
    return *ptApp;
}
Window &WIN_SELF(GLFWwindow *handle)
{
    BTM_ASSERT(sHandleToWindow.count(handle) > 0);
    Window *ptWin = sHandleToWindow[handle];
    BTM_ASSERT(ptWin);
    return *ptWin;
}

Window::Window(i32 w, i32 h, std::string const &title, App *app) : mW(w), mH(h), mTitle(title)
{
    BTM_ABORT_IF(!app, "Window requires a valid App pointer");

    if (!sIsWindowContextInitialized)
    {
        BTM_ABORT_IF(!glfwInit(), "Couldn't initialize Window-Manager");
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // Avoid OpenGL context creation
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);      // Resize windows takes special care
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);  // Focus the window when its opened

        u32          extsCount = 0;
        char const **exts      = glfwGetRequiredInstanceExtensions(&extsCount);
        sExtensions            = exts ? decltype(sExtensions) { exts, exts + extsCount } : decltype(sExtensions) {};

        sIsWindowContextInitialized = true;
    }

    // Create window object
    mHandle = glfwCreateWindow(mW, mH, "", nullptr, nullptr);
    BTM_ABORT_IF(!mHandle, "Couldn't create a new Window");

    // Store app pointer into window object
    glfwSetWindowUserPointer(mHandle, app);

    // Store window handle to object relation
    sHandleToWindow[mHandle] = this;

    // Store main window object
    if (!sMainWindow)
        sMainWindow = mHandle;

    // clang-format off

    // Window Dependent Events
    // . Resize
    glfwSetFramebufferSizeCallback(mHandle, [](GLFWwindow *p, i32 w, i32 h) { WIN_SELF(p).size(w, h); });
    // . Focus
    glfwSetCursorEnterCallback(mHandle, [](GLFWwindow *p, i32 focus) { WIN_SELF(p).focus((bool)focus); });
    // . On Close
    glfwSetWindowCloseCallback(mHandle, [](GLFWwindow *p) { if (p == sMainWindow) BTM_APP(p).markToClose(); });
    
    // Window Independent Events
    // . Cursor
    glfwSetCursorPosCallback(mHandle, [](GLFWwindow *p, double x, double y) { BTM_APP(p).cursor({x, y}); });
    // . Keyboard
    glfwSetKeyCallback(mHandle, [](GLFWwindow *p, i32 k, i32, i32 s, i32) { BTM_APP(p).key((IK)k, (IS)s); });
    // . Mouse
    glfwSetMouseButtonCallback(mHandle, [](GLFWwindow *p, i32 m, i32 s, i32) { BTM_APP(p).mouse((IM)m, (IS)s); });

    // clang-format on

    refreshTitle();
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
    sIsWindowContextInitialized = false;  // Ensures recreation of window-context after app/renderer 'cleanup'
    glfwTerminate();
}

glm::vec2 Window::size() const
{
    return { mW, mH };
}
void Window::size(i32 w, i32 h)
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

void Window::refreshTitle()
{
    if (mHandle)
        glfwSetWindowTitle(mHandle, (BTM_APP(mHandle).name() + " :: " + mTitle + " ::" + mTitleInfo).c_str());
}

void Window::title(std::string title)
{
    mTitle = std::move(title);
    refreshTitle();
}

void Window::titleInfo(std::string info)
{
    mTitleInfo = std::move(info);
    refreshTitle();
}

}  // namespace btm
