# cadllm

A cross-platform desktop CAD application built on the [OpenCASCADE (OCCT)](https://dev.opencascade.org/) geometry kernel, combining 2D sketching and 3D solid modeling with **natural-language design via LLMs**.

The distinguishing idea: instead of the LLM guessing which part of the model you mean, you select a specific **face, edge, or vertex** in the viewport and hand that as structured context to the model. The AI integration is not built yet (see [Roadmap](#roadmap)) — the current focus is a solid, well-understood geometry and viewer foundation to build it on.

> Full technical specification, architecture decisions, and open questions live in [`docs/tech-spec.md`](docs/tech-spec.md). This README is the quick-start overview; the tech spec is the source of truth for design decisions.

## Status

**M0 (toolchain + embedded viewport) is complete.** The app opens a window with a live OCCT 3D viewport: orbit/pan/zoom camera controls and per-face selection with area/normal reporting on a sample box shape. LLM integration has not started — that's phase 2, after the modeling core (M1–M5) is in place.

## Features (current)

- Embedded OCCT 3D viewport inside a Qt widget, with OCCT owning the OpenGL context directly (no `QOpenGLWidget` context-sharing issues).
- Mouse-driven camera: left-drag to orbit, middle-drag to pan, wheel to zoom.
- Face picking with hover highlight; selecting a face reports its surface area and normal vector.

## Tech stack

| Layer | Technology |
|---|---|
| Geometry kernel | OpenCASCADE (OCCT) 7.8+ |
| Language | C++20 |
| GUI | Qt 6 (Widgets) |
| 3D rendering | OCCT AIS / V3d, drawn directly into a native Qt window handle |
| Build system | CMake 3.21+ |
| Dependency management | vcpkg (manifest mode, with an overlay triplet — see below) |

See [`docs/tech-spec.md`](docs/tech-spec.md) for the full architecture, module boundaries, and the reasoning behind each choice.

## Project structure

```
cadllm/
├── src/                    # Application source
│   ├── main.cpp            # Entry point
│   ├── mainwindow.{h,cpp}  # Top-level window, wires viewer to UI
│   ├── occtviewer.{h,cpp}  # Qt widget hosting the embedded OCCT 3D view
│   └── version.{h,cpp}     # App version string
├── docs/
│   └── tech-spec.md        # Architecture, roadmap, decisions (source of truth)
├── triplets/
│   └── x64-windows.cmake   # vcpkg overlay triplet (works around a gperf/CMake 4.x issue)
├── CMakeLists.txt
├── vcpkg.json               # vcpkg manifest (declares the OpenCASCADE dependency)
├── vcpkg-configuration.json # Points vcpkg at the overlay triplet
└── .gitattributes            # Normalizes line endings to LF
```

## Building

Currently verified on **Windows with MSVC 2022**; macOS and Linux are planned for M1 (see roadmap) but untested so far.

### Prerequisites

- [Qt 6.8 LTS](https://www.qt.io/download-qt-installer) (Widgets module), installed system-wide, built with MSVC 2022.
- [vcpkg](https://github.com/microsoft/vcpkg) for the OpenCASCADE dependency (pulled in automatically via `vcpkg.json` if you use CMake's vcpkg toolchain integration).
- CMake 3.21+.
- Visual Studio 2022 (MSVC toolchain).

### Configure and build

```powershell
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Debug
```

The build step also runs `windeployqt` automatically on Windows to copy the required Qt runtime DLLs next to `cadllm.exe`.

### Run

```powershell
build\Debug\cadllm.exe
```

You should see a window with an embedded 3D viewport showing a sample box. Left-drag to orbit, middle-drag to pan, scroll to zoom, and click a face to see its area and normal reported in the status bar.

## Roadmap

| Milestone | Scope | Status |
|---|---|---|
| M0 | Toolchain, embedded viewport, camera controls, face selection | ✅ Done |
| M1 | STEP/IGES import & viewing, edge/vertex selection modes, multi-select, cross-platform CI | Next |
| M2 | Core 3D modeling: primitives, extrude/revolve, boolean ops, fillet/chamfer, undo/redo | Planned |
| M3 | 2D sketching: constraints, extrude/revolve from sketch | Planned |
| M4 | Native document format, STEP/IGES/STL export, persistent element IDs | Planned |
| M5 | Packaging for Windows/macOS/Linux, user docs, 1.0 core release | Planned |
| M6+ | LLM integration: provider abstraction, selection-based context, tool-calling | Planned |

Details, risks, and open design questions for each milestone are tracked in [`docs/tech-spec.md`](docs/tech-spec.md).

## License

Not yet finalized. The project targets an OCCT-LGPL-compatible license (dynamic linking, per LGPL 2.1 obligations) — see [`docs/tech-spec.md`](docs/tech-spec.md) §11. A `LICENSE` file will be added once this is settled.

## Contributing

This is currently a solo project developed milestone-by-milestone on feature branches merged via PR (see `docs/tech-spec.md` for the branching and commit conventions in use). Issues and discussion are welcome; a `CONTRIBUTING.md` will follow once the project opens up further.
