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

const UINT WM_TRAYICON = WM_APP + 1;
const UINT_PTR IDT_ANIMATION = 1;
const UINT ANIMATION_INTERVAL = 200;

const int FRAME_COUNT_AWAKE = 5;
const int FRAME_COUNT_SLEEP = 4;

const wchar_t* CLASS_NAME = L"Luna";
const wchar_t* APP_TITLE = L"Luna";

enum PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max };
using fnSetPreferredAppMode = PreferredAppMode (WINAPI *)(PreferredAppMode appMode);

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

NOTIFYICONDATA g_nid = {};
bool g_isAwake = false;
int g_animFrame = 0;
HINSTANCE g_hInst = nullptr;

bool IsSystemDarkTheme() {
    DWORD value = 1;
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
    
    return (status == ERROR_SUCCESS && value == 0);
}

int GetCurrentIconId() {
    bool isDark = IsSystemDarkTheme();
    int baseId = 0;
    int frame = 0;

    if (g_isAwake) {
        baseId = isDark ? IDI_DARK_AWAKE_0 : IDI_LIGHT_AWAKE_0;
        frame = g_animFrame % FRAME_COUNT_AWAKE;
    } else {
        baseId = isDark ? IDI_DARK_SLEEP_0 : IDI_LIGHT_SLEEP_0;
        frame = g_animFrame % FRAME_COUNT_SLEEP;
    }

    return baseId + frame;
}

void UpdateTrayIconState() {
    int iconId = GetCurrentIconId();
    g_nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(iconId));
    
    const wchar_t* tipText = g_isAwake ? L"Awake" : L"Asleep";
    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), tipText);

    Shell_NotifyIcon(NIM_MODIFY, &g_nid);
}

void ToggleInsomnia(bool enable) {
    if (enable) {
        SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
    } else {
        SetThreadExecutionState(ES_CONTINUOUS);
    }
    g_isAwake = enable;
    g_animFrame = 0;
    UpdateTrayIconState();
}

BOOL InitTrayIcon(HWND hwnd) {
    ZeroMemory(&g_nid, sizeof(g_nid));
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    
    int iconId = GetCurrentIconId();
    g_nid.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(iconId));
    
    StringCchCopy(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"Asleep");

    return Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

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

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        if (!InitTrayIcon(hwnd)) {
            return -1;
        }
        SetTimer(hwnd, IDT_ANIMATION, ANIMATION_INTERVAL, NULL);
        return 0;

    case WM_TRAYICON:
        switch (LOWORD(lParam)) {
        case WM_LBUTTONUP:
            ToggleInsomnia(!g_isAwake);
            break;
        case WM_RBUTTONUP:
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
    g_hInst = hInstance;

    EnableDarkMode();

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
