# Luna

**Luna** is a lightweight, headless Win32 utility that resides in the system tray. It allows users to prevent their computer from going to sleep with a single click.

## Usage Instructions

- Copy `Luna.exe` to your desired location.
- Run `Luna.exe`.
- Luna will automatically detect the system theme and start the icon animation.
- Left-click the icon to toggle Awake/Asleep.
- Right-click the icon to exit.

## API Explanations

### `RegGetValueW`
Used to detect the system theme by reading `HKCU\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize\AppsUseLightTheme`.

### `SetTimer` / `WM_TIMER`
Used to drive the icon animation. The timer runs continuously to animate both the "Awake" and "Asleep" states.

### `SetThreadExecutionState`
Prevents system sleep (`ES_SYSTEM_REQUIRED`) and display idle (`ES_DISPLAY_REQUIRED`).

## Author
Arun Mathew
2025-11-24
