# Pointer Forge

Pointer Forge is a small Windows C++ learning project focused on understanding how processes expose data in memory and how pointer arithmetic can be used to locate that data.

The project started as an experiment with a game-specific memory layout. It identifies a running process, determines the base address of its main module, adds known offsets, and then reads or updates selected `uint32_t` values. The current prototype uses *Stronghold Crusader* as its practical test case.

This project was created primarily as a hands-on way to learn. Building it helped me develop a stronger understanding of:

- pointer arithmetic and address calculations;
- module base addresses and process memory layouts;
- Windows process and module APIs;
- safe ownership of native handles through RAII;
- `std::optional`, strong types, and error handling in modern C++;
- configuring and building a small CMake-based project.

## Features

- Finds the target process by its window and module name.
- Retrieves the module base address with the Tool Help API.
- Uses an RAII-style wrapper to manage Windows handles.
- Reads values from calculated process-memory addresses.
- Writes user-selected values back to the target process.
- Provides a simple interactive console interface.

## Requirements

- Windows
- A C++23-compatible compiler, such as MSVC
- CMake 3.28 or newer
- A running, locally accessible copy of the target application for experimentation

The current implementation depends on Windows APIs and is therefore not cross-platform.

## Building

From a Visual Studio Developer PowerShell or another environment with a configured C++ toolchain:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

With a multi-configuration generator, the executable is usually located at:

```text
build/Release/app.exe
```

With a single-configuration generator, it is usually located directly in `build/`.

## Usage

1. Start the target application locally.
2. Launch Pointer Forge.
3. Select one of the displayed values.
4. Enter the amount to add.

The offsets in the source code are specific to the current test environment and may change with a different application version, executable, or memory layout.

## Learning Project Disclaimer

Pointer Forge is intended for educational experimentation with applications that you own or are authorized to inspect. It is not designed to bypass protections, interfere with online services, or provide an advantage in multiplayer environments. Always respect the terms of service and applicable laws for the software being inspected.

## Project Status

This is a small prototype and learning project rather than a general-purpose memory-inspection library. The code may evolve as I continue exploring Windows internals, modern C++, and safer abstractions for process-memory access.

## License

No license has been selected yet. Until a license is added, all rights are reserved by the copyright holder.
