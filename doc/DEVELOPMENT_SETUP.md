# HoldemCore Development Environment Setup

This document explains how to set up both VS Code and Qt Creator for HoldemCore development.

## Quick Start

### Option 1: Launch Scripts (Recommended)
```bash
# VS Code (Clang compiler)
cp launch_vscode_with_qt.bat.template launch_vscode_with_qt.bat
./launch_vscode_with_qt.bat

# Qt Creator (MSVC compiler)  
cp launch_qtcreator_with_msvc.bat.template launch_qtcreator_with_msvc.bat
./launch_qtcreator_with_msvc.bat

# Visual Studio (MSVC compiler)
cp launch_visual_studio_with_qt.bat.template launch_visual_studio_with_qt.bat
./launch_visual_studio_with_qt.bat
```

### Option 2: System Environment Variables
Set the following system environment variable:
```
CMAKE_PREFIX_PATH=C:\Qt\6.9.2\msvc2022_64
```

### Option 3: VS Code Terminal Setup
In VS Code terminal, run before building:
```bash
export CMAKE_PREFIX_PATH="C:/Qt/6.9.2/msvc2022_64"
# or for PowerShell:
$env:CMAKE_PREFIX_PATH="C:/Qt/6.9.2/msvc2022_64"
```

## Detailed Setup

### Qt Installation Structure
```
C:\Qt\6.9.2\
├── msvc2022_64\     # MSVC 2022 (recommended for Qt Creator)
├── llvm-mingw_64\   # LLVM/Clang + MinGW
├── mingw_64\        # Pure MinGW
└── ...
```

### VS Code Configuration
- **Compiler**: Clang 20.1.4 (from LLVM installation)
- **Qt Version**: Use `msvc2022_64` (compatible with Clang)
- **Build System**: Ninja + CMake presets
- **Presets**: `vscode-debug-widgets`, `vscode-debug-qml`, etc.

### Qt Creator Configuration  
- **Compiler**: MSVC 2022 (Visual Studio Build Tools)
- **Qt Version**: Use `msvc2022_64` (native compatibility)
- **Build System**: Ninja + CMake presets  
- **Presets**: `qtcreator-debug-widgets`, `qtcreator-debug-qml`, etc.

### Visual Studio Configuration
- **Compiler**: MSVC 2022 (Visual Studio 2022)
- **Qt Version**: Use `msvc2022_64` (native compatibility)
- **Build System**: Ninja + CMake presets
- **Presets**: `visualstudio-debug-widgets`, `visualstudio-debug-qml`, etc.

## Environment Variables Reference

| Variable | Purpose | Example Value |
|----------|---------|---------------|
| `CMAKE_PREFIX_PATH` | Qt installation path | `C:/Qt/6.9.2/msvc2022_64` |
| `Qt6_DIR` | Alternative Qt path | `C:/Qt/6.9.2/msvc2022_64/lib/cmake/Qt6` |

## Troubleshooting

### "Qt6 not found" in VS Code
1. Verify CMAKE_PREFIX_PATH is set: `echo $CMAKE_PREFIX_PATH`
2. Check Qt installation exists: `ls "$CMAKE_PREFIX_PATH/lib/cmake/Qt6"`
3. Use the launch script: `./launch_vscode_with_qt.bat`

### "Standard headers missing" in Qt Creator  
1. Verify MSVC environment in Qt Creator kit settings
2. Add environment initialization: `call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"`
3. Restart Qt Creator after kit changes

### Linker errors in Qt Creator
1. Ensure MSVC environment is properly initialized
2. Check that all source files compile successfully  
3. Verify library dependencies in CMakeLists.txt

## Hybrid Workflow

### Use VS Code for:
- ✅ Daily C++ development (reliable Clang builds)
- ✅ Code editing, refactoring, debugging
- ✅ Desktop application testing
- ✅ Git operations and version control

### Use Qt Creator for:
- ✅ QML development and visual design
- ✅ Android/mobile deployment
- ✅ Qt-specific debugging (QML debugger, profiler)
- ✅ UI/UX design work