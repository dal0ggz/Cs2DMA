# Cs2DMA

## Repository Status

This repository is preserved as an archival C++ / Windows systems programming
experiment. It is not recommended as a professional portfolio highlight in its
current form because it combines overlay rendering, external memory access, and
game-specific logic.

Do not use this project to gain an unfair advantage in online games, bypass
anti-cheat systems, or interfere with software you do not own or have permission
to inspect. The useful portfolio value here is limited to the transferable
engineering topics: Windows application structure, Direct3D rendering, ImGui UI
composition, configuration handling, and repository cleanup.

## Overview

`Cs2DMA` is a Visual Studio C++ project that explores a Windows overlay
application using Direct3D 11 and ImGui. The source is organized around a small
runtime entry point, an overlay/rendering layer, configuration persistence, UI
styling, feature modules, generated SDK data, and a vendored DMA library.

The repository has been cleaned up so that generated build output and local IDE
state are ignored instead of tracked. It should be treated as a learning archive,
not as a maintained or supported application.

## Technical Highlights

- Win32 application entry point with console diagnostics.
- Direct3D 11 overlay setup and render loop.
- ImGui-based menu styling and interaction patterns.
- Runtime configuration saved through a local INI-style file.
- Modular source layout for rendering, UI, configuration, and feature logic.
- Visual Studio project configuration targeting C++20.
- Vendored third-party code and generated SDK files, which are documented as
  repository maintenance concerns.

## What This Project Demonstrates

From a safe engineering perspective, this repository demonstrates:

- how a C++ desktop project can be structured around separate runtime,
  rendering, configuration, and UI modules;
- how Direct3D and ImGui can be wired into a Win32 application;
- how global application settings can be centralized and persisted;
- how generated code and vendored dependencies can quickly make a repository
  harder to maintain;
- why build outputs, IDE caches, and user-specific files should not be committed.

It should not be used as an example of professional product scope, security
review, dependency management, or responsible public portfolio positioning.

## Tech Stack

| Area | Technology |
| --- | --- |
| Language | C++20 |
| Platform | Windows |
| IDE / build files | Visual Studio project files |
| Graphics | Direct3D 11, DXGI |
| UI | Dear ImGui |
| Configuration | INI-style local configuration |
| Dependencies | Vendored ImGui files, vendored DMA library, generated SDK files |

## Project Structure

```text
.
|-- CS2_DMA_ESP/
|   |-- main.cpp                 # Application startup and high-level lifecycle
|   |-- overlay.cpp/.h           # Win32 window, Direct3D 11, ImGui render loop
|   |-- ui_style.cpp/.h          # ImGui styling and menu composition
|   |-- config.cpp/.h            # Local configuration load/save helpers
|   |-- globals.h                # Shared runtime settings and UI state
|   |-- features/
|   |   |-- esp.cpp/.h           # Rendering/data feature module
|   |   |-- aimbot.cpp/.h        # Input automation experiment
|   |   `-- makcu.cpp/.h         # Serial-device integration experiment
|   |-- sdk/
|   |   |-- offsets.h
|   |   |-- structs.h
|   |   `-- generated/           # Generated SDK data
|   |-- imgui/                   # Vendored Dear ImGui source
|   `-- CS2_DMA_ESP/
|       |-- CS2_DMA_ESP.slnx
|       `-- CS2_DMA_ESP/
|           `-- CS2_DMA_ESP.vcxproj
|-- DMALibrary-Master/           # Vendored DMA-related library
|-- .gitignore                   # Visual Studio and build artifact rules
`-- README.md
```

## Source Map

| File or directory | Purpose |
| --- | --- |
| `CS2_DMA_ESP/main.cpp` | Starts the application, initializes configuration, and controls the main lifecycle. |
| `CS2_DMA_ESP/overlay.cpp` | Owns the overlay window, Direct3D 11 device objects, ImGui frame setup, render loop, and cleanup. |
| `CS2_DMA_ESP/ui_style.cpp` | Applies UI theme settings and builds the main menu surface. |
| `CS2_DMA_ESP/config.cpp` | Reads and writes persistent settings for UI, screen, and feature state. |
| `CS2_DMA_ESP/globals.h` | Stores shared runtime flags, colors, dimensions, and feature toggles. |
| `CS2_DMA_ESP/features/` | Contains feature modules that should be treated as archival experiments. |
| `CS2_DMA_ESP/sdk/generated/` | Contains generated data; it should have a documented generation source if the repo remains public. |
| `DMALibrary-Master/` | Vendored external code; it should be replaced with a documented dependency strategy in a portfolio project. |

## Installation

This repository does not provide supported installation instructions.

The project interacts with game-specific process data and external memory access
concepts, so operational setup instructions are intentionally not included. If
you want to showcase the same skills professionally, create a separate benign
Direct3D / ImGui demo that renders synthetic data only.

## Usage

No supported usage workflow is provided.

For portfolio purposes, use this repository only as a record of learning. A
public, recruiter-friendly version should remove game-specific behavior and
focus on harmless UI rendering, configuration management, and graphics
programming.

## Screenshots / Demo

No screenshots or demos are included. A public demo would not be appropriate for
the current project scope.

## Repository Hygiene

The repository has been updated with a Visual Studio-oriented `.gitignore` so
future commits should avoid:

- `.vs/` IDE cache files;
- `x64/`, `x86/`, `Debug/`, and `Release/` build output folders;
- compiled binaries and debug symbols such as `.exe`, `.obj`, `.pdb`, `.iobj`,
  and `.ipdb`;
- local editor settings.

Previously tracked local build artifacts were removed from the working tree as
part of cleanup.

## Quality Notes

- The project has no automated tests.
- There is no CI workflow.
- Generated files are committed without a documented regeneration process.
- Vendored dependencies are committed without version notes.
- Some comments and strings mix Italian and English.
- The current architecture depends heavily on global state.
- The repository is larger and more specialized than ideal for a public
  portfolio project.

## Known Issues

- The source is tightly coupled to a game-specific runtime context.
- External dependencies are not documented with reproducible version metadata.
- Error handling is not consistently strong across all Windows and rendering
  initialization paths.
- No static analysis, formatting, or linting configuration is included.
- No license file is present.

## Future Improvements

Recommended direction if the goal is a professional portfolio:

- Archive or make this repository private.
- Create a separate harmless project that uses Direct3D 11 and ImGui to render
  mock telemetry, charts, or UI panels.
- Replace generated game-specific data with synthetic demo data.
- Document dependency versions and update procedures.
- Add a formatting configuration and a simple CI build.
- Add screenshots for the benign demo project.
- Add an explicit license to any repository intended for reuse.

## Portfolio Recommendation

Do not feature this repository as a highlighted project. If it remains public,
keep this README clear about its archival status and avoid presenting it as a
ready-to-use application.

Better portfolio alternatives:

- a Direct3D / ImGui dashboard with mock data;
- a Windows desktop utility with settings persistence;
- a C++ rendering sandbox with documented architecture;
- a small systems programming project with tests and CI.

## License

No license file is currently present. Without a license, the code should be
treated as all-rights-reserved by default. Add an explicit license before reuse,
redistribution, or collaboration.
