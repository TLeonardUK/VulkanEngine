#pragma once
#include "Pch.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/String.h"

enum class InputKey
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,
	Num0,

    Return,
    Escape,
    Backspace,
    Tab,
    Space,
    Minus,
    Equals,
    LeftBracket,
    RrightBracket,
    Backslash,

    Semicolon,
    Apostrophe,
    Grave,
    Comma,
    Period,
    Slash,

    CapsLock,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,

    PrintScreen,
    ScrollLock,
    Pause,
    Insert,
    Home,
    PageUp,
    Delete,
    End,
    PageDown,

    Right,
    Left,
    Down,
    Up,

	KP_Divide,
    KP_Multiply,
    KP_Minus,
    KP_Plus,
    KP_Enter,
    KP_1,
    KP_2,
    KP_3,
    KP_4,
    KP_5,
    KP_6,
    KP_7,
    KP_8,
    KP_9,
    KP_0,
    KP_Period,

    LCtrl,
    LShift,
    LAlt,
    LGui,
    RCtrl,
    RShift,
    RAlt,
    RGui,

	Mouse_0,
	Mouse_1,
	Mouse_2,
	Mouse_3,
	Mouse_4,
	Mouse_5,

	COUNT
};

enum class InputModifier
{
	Shift,
	Ctrl,
	Alt,
	GUI,

	COUNT
};

enum class InputCursor
{
	None,

	Arrow,
	IBeam,
	Wait,
	Crosshair,
	WaitArrow,
	SizeNWSE,
	SizeNESW,
	SizeWE,
	SizeNS,
	SizeAll,
	No,
	Hand,

	COUNT
};

class IInput
{
protected:
	IInput() { };

public:
	virtual ~IInput() { };
	virtual void Dispose() = 0;

	virtual void PollInput() = 0;

	virtual bool IsKeyDown(InputKey key) = 0;
	virtual bool WasKeyPressed(InputKey key) = 0;
	virtual bool WasKeyReleased(InputKey key) = 0;

	virtual bool IsModifierActive(InputModifier modifier) = 0;

	virtual String GetClipboardText() = 0;
	virtual void SetClipboardText(const String& value) = 0;

	virtual String GetInput() = 0;

	virtual Vector2 GetMousePosition() = 0;
	virtual void SetMousePosition(Vector2 position) = 0;
	virtual int GetMouseWheel(bool horizontal) = 0;
	virtual void SetMouseCursor(InputCursor cursor) = 0;
	virtual void SetMouseCapture(bool capture) = 0;
	virtual void SetMouseHidden(bool hidden) = 0;
};
