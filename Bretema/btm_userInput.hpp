#pragma once

#include "btm_base.hpp"

namespace btm
{

enum struct State
{
    Release,
    Press,
    Hold,
};

enum struct Mouse
{
    _1 = 0,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,

    L      = _1,
    Left   = _1,
    R      = _2,
    Right  = _2,
    M      = _3,
    Middle = _3,
};

enum struct Key
{
    Unknown = -1,

    // Keyboard
    Space        = 32,
    Apostrophe   = 39,     /* ' */
    Comma        = 44,     /* , */
    Minus        = 45,     /* - */
    Period       = 46,     /* . */
    Dot          = Period, /* . */
    Slash        = 47,     /* / */
    Num0         = 48,
    Num1         = 49,
    Num2         = 50,
    Num3         = 51,
    Num4         = 52,
    Num5         = 53,
    Num6         = 54,
    Num7         = 55,
    Num8         = 56,
    Num9         = 57,
    Semicolon    = 59, /* ; */
    Equal        = 61, /* = */
    A            = 65,
    B            = 66,
    C            = 67,
    D            = 68,
    E            = 69,
    F            = 70,
    G            = 71,
    H            = 72,
    I            = 73,
    J            = 74,
    K            = 75,
    L            = 76,
    M            = 77,
    N            = 78,
    O            = 79,
    P            = 80,
    Q            = 81,
    R            = 82,
    S            = 83,
    T            = 84,
    U            = 85,
    V            = 86,
    W            = 87,
    X            = 88,
    Y            = 89,
    Z            = 90,
    LeftBracket  = 91, /* [ */
    BackSlash    = 92, /* \ */
    RightBracket = 93, /* ] */
    GraveAccent  = 96, /* ` */

    // Navigation
    Escape    = 256,
    Enter     = 257,
    Tab       = 258,
    Backspace = 259,
    Insert    = 260,
    Delete    = 261,
    Right     = 262,
    Left      = 263,
    Down      = 264,
    Up        = 265,
    PageUp    = 266,
    PageDown  = 267,
    Home      = 268,
    End       = 269,

    // Locks and more
    CapsLock    = 280,
    ScrollLock  = 281,
    NumLock     = 282,
    PrintScreen = 283,
    Pause       = 284,
    Menu        = 348,

    // Function keys
    F1  = 290,
    F2  = 291,
    F3  = 292,
    F4  = 293,
    F5  = 294,
    F6  = 295,
    F7  = 296,
    F8  = 297,
    F9  = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,

    // KeyPad (numeric keyboard)
    Kp0         = 320,
    Kp1         = 321,
    Kp2         = 322,
    Kp3         = 323,
    Kp4         = 324,
    Kp5         = 325,
    Kp6         = 326,
    Kp7         = 327,
    Kp8         = 328,
    Kp9         = 329,
    KpDecimal   = 330,
    KpDivide    = 331,
    KpMultiply  = 332,
    KpSubstract = 333,
    KpAdd       = 334,
    KpEnter     = 335,
    KpEqual     = 336,

    // Mods
    LeftShift  = 340,
    LeftCtrl   = 341,
    LeftAlt    = 342,
    LeftSuper  = 343,
    LeftMeta   = LeftSuper,
    RightShift = 344,
    RightCtrl  = 345,
    RightAlt   = 346,
    RightSuper = 347,
    RightMeta  = RightSuper,
};

using MouseState = umap<Mouse, State>;
using KeyState   = umap<Key, State>;

class UserInput
{
public:
    UserInput(std::function<void(UserInput *)> onInputChanged) : mOnInputChanged(onInputChanged) {}

    // Cursor

    glm::vec2 displ() const { return mDispl; }

    glm::vec2 cursor() const { return mCursor; }

    void cursor(glm::vec2 cursor)
    {
        glm::vec2 const displ = cursor - mCursor;
        mCursor               = std::move(cursor);
        mDispl                = displ;

        bool const isClicked = clicked(Mouse::L) or clicked(Mouse::R) or clicked(Mouse::M);
        mClickedDispl        = displ * float(isClicked);

        inputChanged();
    }

    // Wheel

    inline glm::vec2 wheel() const { return mWheel; }

    void wheel(glm::vec2 wheel)
    {
        mWheel = std::move(wheel);
        inputChanged();
    }

    // Click - mouse -

    void click(Mouse m, State s)
    {
        mMouse[m] = s;
        inputChanged();
    }

    void click(i32 m, i32 s) { click((Mouse)m, (State)s); }

    bool clicked(Mouse m, bool ignoreHold = false) const
    {
        if (mMouse.count(m) > 0)
            return false;

        State const &s = mMouse.at(m);
        return s == State::Press or (!ignoreHold and s == State::Hold);
    }

    // Press - key -

    void press(Key k, State s)
    {
        mKeys[k] = s;
        inputChanged();
    }

    void press(i32 k, i32 s) { press((Key)k, (State)s); }

    bool pressed(Key k, bool ignoreHold = false) const
    {
        if (mKeys.count(k) > 0)
            return false;

        State const &s = mKeys.at(k);
        return s == State::Press or (!ignoreHold and s == State::Hold);
    }

private:
    void inputChanged()
    {
        BTM_INFO("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!!");
        if (mOnInputChanged)
        {
            BTM_INFO("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb!!");
            mOnInputChanged(this);
        }

        mClickedDispl = mDispl = mWheel = ZERO2;
    }

    glm::vec2  mDispl        = ZERO2;
    glm::vec2  mClickedDispl = ZERO2;
    glm::vec2  mWheel        = ZERO2;
    glm::vec2  mCursor       = ZERO2;
    KeyState   mKeys         = {};
    MouseState mMouse        = {};

    std::function<void(UserInput *)> mOnInputChanged = nullptr;
};

}  // namespace btm