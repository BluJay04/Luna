/**
 * @file main.cpp
 * @brief Luna - Win32 System Tray Utility for Power Management.
 * @author Antigravity
 * @date 2025-11-22
 *
 * A headless Windows application that resides in the system tray and prevents
 * the computer from going to sleep. It uses SetThreadExecutionState to toggle
 * power availability.
 *
 * Features:
 * - System Tray Icon with Context Menu.
 * - Prevents Sleep/Display Idle.
 * - Auto-detects System Theme (Light/Dark).
 * - Continuous Animation for both Sleep and Awake states.
 *
 * Compilation Instructions:
 * cl.exe /std:c++17 /EHsc /W4 main.cpp Luna.res user32.lib shell32.lib kernel32.lib advapi32.lib /link /SUBSYSTEM:WINDOWS
 */

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>
#include "resource.h"

// Global Constants
const UINT WM_TRAYICON = WM_APP + 1; ///< Custom message ID for tray icon events.
const UINT_PTR IDT_ANIMATION = 1;    ///< Timer ID for icon animation.
const UINT ANIMATION_INTERVAL = 200; ///< Animation interval in milliseconds (Faster for smooth cat animation).

const int FRAME_COUNT_AWAKE = 5;
const int FRAME_COUNT_SLEEP = 4;

const wchar_t* CLASS_NAME = L"Luna";
const wchar_t* APP_TITLE = L"Luna";

// --- Dark Mode Support (Undocumented) ---
enum PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max };
using fnSetPreferredAppMode = PreferredAppMode (WINAPI *)(PreferredAppMode appMode);

/**
 * @brief Attempts to enable Dark Mode for the application's windows and menus.
 * 
 * Uses the undocumented SetPreferredAppMode (Ordinal 135) from uxtheme.dll.
 * This allows the context menu (TrackPopupMenu) to render in dark mode on Windows 10/11.
 */
void EnableDarkMode() {
    HMODULE hUxtheme = LoadLibraryEx(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxtheme) {
        auto ord135 = (fnSetPreferredAppMode)GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        if (ord135) {
            ord135(AllowDark);
        }
        FreeLibrary(hUxtheme);
    }
}


// Global Variables
NOTIFYICONDATA g_nid = {};      ///< Structure containing information about the tray icon.
bool g_isAwake = false;         ///< Current state: true = Awake, false = Asleep.
int g_animFrame = 0;            ///< Current animation frame index.
HINSTANCE g_hInst = nullptr;    ///< Handle to the current application instance.

/**
 * @brief Checks if the system is currently using Dark Mode for apps.
 *
 * Reads the registry key:
 * HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme
 *
 * @return true if Dark Mode is active (AppsUseLightTheme == 0), false otherwise.
 */
bool IsSystemDarkTheme() {
    DWORD value = 1; // Default to Light (1) if key missing
    DWORD size = sizeof(value);
    LSTATUS status = RegGetValueW(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme",
        RRF_RT_REG_DWORD,
        NULL,
        &value,
        &size
    );
    
    // If AppsUseLightTheme is 0, it's Dark Mode.
    return (status == ERROR_SUCCESS && value == 0);
}

/**
 * @brief Retrieves the appropriate icon resource ID based on state and theme.
 *
 * @return int Resource ID of the icon to display.
 */
int GetCurrentIconId() {
    bool isDark = IsSystemDarkTheme();
    int baseId = 0;
    int frame = 0;

    if (g_isAwake) {
        // Awake Mode (5 Frames)
        baseId = isDark ? IDI_DARK_AWAKE_0 : IDI_LIGHT_AWAKE_0;
        frame = g_animFrame % FRAME_COUNT_AWAKE;
    } else {
        // Sleep Mode (4 Frames)
        baseId = isDark ? IDI_DARK_SLEEP_0 : IDI_LIGHT_SLEEP_0;
        frame = g_animFrame % FRAME_COUNT_SLEEP;
    }

    return baseId + frame;
}

/**
 * @brief Updates the tray icon and tooltip based on the current state.
 *
 * Switches the icon based on state, theme, and animation frame.
 */
void UpdateTrayIconState() {
    // Determine which icon to load
    int iconId = GetCurrentIconId();
    g_nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(iconId));
    
    // Update tooltip text
    const wchar_t* tipText = g_isAwake ? L"Awake" : L"Asleep";
    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), tipText);

    // Modify the existing icon
    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

/**
 * @brief Toggles the system power state to prevent or allow sleep.
 *
 * Uses the Win32 API SetThreadExecutionState to inform the system of the application's requirements.
 *
 * @param enable If true, prevents the system from sleeping (Insomnia mode).
 *               If false, restores normal power management behavior.
 */
void ToggleInsomnia(bool enable) {
    if (enable) {
        // Enable Insomnia
        SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    } else {
        // Disable Insomnia
        SetThreadExecutionState(ES_CONTINUOUS);
    }
    g_isAwake = enable;
    g_animFrame = 0; // Reset animation frame on state switch
    UpdateTrayIconState();
}

/**
 * @brief Initializes the system tray icon.
 *
 * Sets up the NOTIFYICONDATA structure and calls Shell_NotifyIcon to add the icon.
 *
 * @param hwnd Handle to the main window (message-only window in this case).
 * @return TRUE if the icon was successfully added, FALSE otherwise.
 */
BOOL InitTrayIcon(HWND hwnd) {
    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1; // ID of the icon
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    
    // Load initial icon
    int iconId = GetCurrentIconId();
    g_nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(iconId));
    
    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"Asleep");

    return Shell_NotifyIcon(NIM_ADD, &g_nid);
}

/**
 * @brief Removes the icon from the system tray.
 * Should be called upon application exit to clean up resources.
 */
void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

// --- Startup Logic ---

/**
 * @brief Adds the application to the Windows Startup Registry Key.
 * 
 * Key: HKCU\Software\Microsoft\Windows\CurrentVersion\Run
 * Value: "Luna" -> [Path to Executable]
 */
void AddToStartup() {
    wchar_t path[MAX_PATH];
    if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
        return;
    }

    HKEY hKey;
    LSTATUS status = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hKey
    );

    if (status == ERROR_SUCCESS) {
        RegSetValueEx(
            hKey,
            L"Luna",
            0,
            REG_SZ,
            (const BYTE*)path,
            (DWORD)(lstrlen(path) + 1) * sizeof(wchar_t)
        );
        RegCloseKey(hKey);
    }
}

/**
 * @brief Main window procedure to handle application messages.
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        if (!InitTrayIcon(hwnd)) {
            return -1;
        }
        // Start the animation timer immediately and keep it running
        SetTimer(hwnd, IDT_ANIMATION, ANIMATION_INTERVAL, NULL);
        return 0;

    case WM_TRAYICON:
        switch (LOWORD(lParam)) {
        case WM_LBUTTONUP:
            ToggleInsomnia(!g_isAwake);
            break;
        case WM_RBUTTONUP:
            // Right-click now exits immediately
            DestroyWindow(hwnd);
            break;
        }
        return 0;

    case WM_TIMER:
        if (wParam == IDT_ANIMATION) {
            g_animFrame++;
            UpdateTrayIconState();
        }
        return 0;

    case WM_SETTINGCHANGE:
        // Detect system theme change
        if (lParam && lstrcmpi(L"ImmersiveColorSet", (LPCWSTR)lParam) == 0) {
            UpdateTrayIconState();
        } else {
            UpdateTrayIconState();
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, IDT_ANIMATION);
        RemoveTrayIcon();
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}


/**
 * @brief Application Entry Point.
 */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
    g_hInst = hInstance;

    // Enable Dark Mode for menus (Optional now, but good for future)
    EnableDarkMode();

    // Ensure app runs at startup
    AddToStartup();

    WNDCLASSEX wc = {};


    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassEx(&wc)) return 0;

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, APP_TITLE, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) return 0;

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
