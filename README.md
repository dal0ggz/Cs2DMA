# Cs2DMA

C++ Windows overlay project built around Direct3D 11, ImGui, generated CS2
offset data, and a vendored DMA library.

The repository is mainly a personal C++/Win32 learning project. It keeps the
source layout, UI work, configuration system, and Visual Studio setup in one
place so the project can be reviewed without digging through old notes.

## Features

- Direct3D 11 overlay window.
- ImGui menu and custom UI styling.
- Config load/save system for local settings.
- Separate modules for overlay, UI, config, and feature code.
- Generated SDK/offset headers kept under `CS2_DMA_ESP/sdk`.
- Visual Studio project files for a Windows C++ build.

## Tech Stack

- C++20
- Win32 API
- Direct3D 11 / DXGI
- Dear ImGui
- Visual Studio
- DMALibrary

## Project Structure

```text
.
|-- CS2_DMA_ESP/
|   |-- main.cpp
|   |-- overlay.cpp / overlay.h
|   |-- ui_style.cpp / ui_style.h
|   |-- config.cpp / config.h
|   |-- globals.h
|   |-- features/
|   |   |-- esp.cpp / esp.h
|   |   |-- aimbot.cpp / aimbot.h
|   |   `-- makcu.cpp / makcu.h
|   |-- sdk/
|   |   |-- offsets.h
|   |   |-- structs.h
|   |   `-- generated/
|   |-- imgui/
|   `-- CS2_DMA_ESP/
|       |-- CS2_DMA_ESP.slnx
|       `-- CS2_DMA_ESP/
|           `-- CS2_DMA_ESP.vcxproj
|-- DMALibrary-Master/
|-- .gitignore
`-- README.md
```

## Main Files

| Path | Description |
| --- | --- |
| `CS2_DMA_ESP/main.cpp` | Application startup and high-level initialization. |
| `CS2_DMA_ESP/overlay.cpp` | Win32 window creation, D3D11 setup, ImGui frame loop, and cleanup. |
| `CS2_DMA_ESP/ui_style.cpp` | ImGui styling and menu rendering. |
| `CS2_DMA_ESP/config.cpp` | Local config read/write helpers. |
| `CS2_DMA_ESP/globals.h` | Shared runtime settings and UI state. |
| `CS2_DMA_ESP/features/` | Feature modules split away from the overlay and UI code. |
| `CS2_DMA_ESP/sdk/generated/` | Generated SDK files and offset data. |
| `DMALibrary-Master/` | Vendored DMA library used by the Visual Studio project. |

## Build Notes

This is a Visual Studio C++ project for Windows.

General build setup:

1. Open `CS2_DMA_ESP/CS2_DMA_ESP/CS2_DMA_ESP.slnx` in Visual Studio.
2. Use an x64 configuration.
3. Make sure the Windows SDK and C++ desktop workload are installed.
4. Keep required third-party DLLs and libraries outside source control unless
   they are intentionally vendored.

The project file currently targets the Visual Studio `v145` platform toolset and
uses C++20 settings.

## Repository Notes

- Build output, Visual Studio cache files, debug symbols, and local editor files
  are ignored through `.gitignore`.
- Generated files are committed because the source code references them
  directly.
- ImGui and DMALibrary are vendored instead of pulled through a package manager.
- Runtime binaries and local machine files should stay out of the repository.

## Screenshots

No screenshots are included yet.

## TODO

- Document exact dependency versions.
- Add a short note for regenerating SDK/offset files.
- Clean up mixed Italian/English comments.
- Add formatting rules for the C++ files.
- Add a license if the project is meant to be reused by other people.
