# Visual Studio Setup Guide for HoldemCore

This guide explains how to configure Visual Studio for HoldemCore development alongside VS Code and Qt Creator.

## Quick Start

### Setup Steps
1. **Copy and customize the launcher template:**
```bash
cp launch_visual_studio_with_qt.bat.template launch_visual_studio_with_qt.bat
# Edit launch_visual_studio_with_qt.bat to match your Visual Studio and Qt paths
```

2. **Launch Visual Studio with environment:**
```bash
./launch_visual_studio_with_qt.bat
```

3. **Open the project:**
   - File → Open → CMake...
   - Select the root `CMakeLists.txt`
   - Choose Visual Studio preset: `visualstudio-debug-widgets` or `visualstudio-debug-qml`

## Visual Studio Configuration

### Prerequisites
- **Visual Studio 2022** (Community, Professional, or Enterprise)
- **CMake support** (installed via Visual Studio Installer)
- **C++ development workload** with MSVC compiler
- **Qt 6.9.2** with MSVC2022 64-bit toolchain

### Environment Setup

The launcher script initializes:
```batch
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set CMAKE_PREFIX_PATH=C:\Qt\6.9.2\msvc2022_64
```

### Available Presets

| Preset | Configuration | Build Dir |
|--------|---------------|-----------|
| `visualstudio-debug-widgets` | Debug Widgets | `build/visualstudio/msvc/debug-widgets` |
| `visualstudio-release-widgets` | Release Widgets | `build/visualstudio/msvc/release-widgets` |
| `visualstudio-debug-qml` | Debug QML | `build/visualstudio/msvc/debug-qml` |
| `visualstudio-release-qml` | Release QML | `build/visualstudio/msvc/release-qml` |

## Visual Studio Workflow

### Project Configuration
1. **Open CMake project in Visual Studio**
2. **Select preset:** Right-click CMakeLists.txt → CMake Settings → Choose preset
3. **Configure:** CMake → Configure Cache
4. **Build:** Build → Build All

### Debugging
- **Set startup project:** Right-click target → Set as Startup Project  
- **Debug:** Debug → Start Debugging (F5)
- **Breakpoints:** Click left margin or F9
- **Watch variables:** Debug → Windows → Watch

### Refactoring Features
- **Rename symbol:** Right-click → Rename (Ctrl+R, R)
- **Find all references:** Right-click → Find All References (Shift+F12)
- **Go to definition:** Right-click → Go to Definition (F12)
- **Extract method:** Right-click → Quick Actions → Extract Function
- **IntelliCode:** AI-assisted code completion

## Integration with Other IDEs

### Multi-IDE Workflow
```bash
# Daily development
./launch_vscode_with_qt.bat        # VS Code (Clang) - fast builds

# Refactoring tasks  
./launch_visual_studio_with_qt.bat # Visual Studio (MSVC) - advanced refactoring

# Mobile deployment
./launch_qtcreator_with_msvc.bat   # Qt Creator (MSVC) - Android/mobile
```

### Build Directory Isolation
Each IDE uses separate build directories:
```
build/
├── vscode/clang/           # VS Code builds
├── visualstudio/msvc/      # Visual Studio builds  
└── qtcreator/msvc/         # Qt Creator builds
```

### No Build Conflicts
- ✅ **Parallel development** - All IDEs can run simultaneously
- ✅ **Separate binaries** - Different output directories
- ✅ **Shared source code** - All IDEs read the same files

## Troubleshooting

### "Qt6 not found" Error
**Solution:** Use the launcher script to set environment
```bash
./launch_visual_studio_with_qt.bat
```

### "CMake configure failed"  
**Solution:** Check Visual Studio CMake integration
1. Tools → Get Tools and Features
2. Verify "C++ CMake tools" is installed
3. Restart Visual Studio

### IntelliSense Issues
**Solution:** Regenerate CMake cache
1. Right-click CMakeLists.txt
2. Delete Cache and Reconfigure
3. Wait for IntelliSense to update

### Build Errors with MSVC
**Solution:** Verify MSVC environment
1. Check launcher script paths match your installation
2. Ensure Visual Studio Build Tools 2022 is installed
3. Use Developer Command Prompt to test: `cl /?`

## Visual Studio Advantages

### For Refactoring Tasks:
- ✅ **Advanced refactoring** - Extract method, rename across solution
- ✅ **IntelliCode** - AI-powered code suggestions  
- ✅ **Code analysis** - Built-in static analysis
- ✅ **Call hierarchy** - Visualize function call relationships
- ✅ **Code maps** - Dependency visualization

### For Debugging:
- ✅ **Advanced debugger** - Edit and continue, mixed-mode debugging
- ✅ **Diagnostic tools** - Memory usage, CPU profiling
- ✅ **Exception handling** - First-chance exception settings
- ✅ **Multi-threaded debugging** - Threads window, parallel stacks

## Task Assignment

| Task Type | Recommended IDE | Why |
|-----------|-----------------|-----|
| **Daily development** | VS Code | Fast, reliable builds |
| **Large refactoring** | Visual Studio | Advanced refactoring tools |
| **Code analysis** | Visual Studio | Built-in static analysis |
| **Mobile deployment** | Qt Creator | Android/iOS deployment |
| **QML debugging** | Qt Creator | QML-specific tools |
| **Performance debugging** | Visual Studio | Profiling and diagnostics |

## Quick Reference

### Launch Commands
```bash
# Start Visual Studio with Qt environment
./launch_visual_studio_with_qt.bat

# Configure specific preset
cmake --preset visualstudio-debug-widgets

# Build from command line  
cmake --build build/visualstudio/msvc/debug-widgets
```

### Visual Studio Shortcuts
- **Configure CMake:** Ctrl+Shift+Alt+C
- **Build Solution:** Ctrl+Shift+B
- **Start Debugging:** F5
- **Find in Files:** Ctrl+Shift+F
- **Go to Definition:** F12
- **Find All References:** Shift+F12