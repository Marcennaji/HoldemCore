# HoldemCore Hybrid Development Workflow

This guide explains the recommended workflow for using both VS Code and Qt Creator effectively.

## 🏗️ Setup Summary

### ✅ VS Code (Primary Development)
- **Compiler**: Clang 20.1.4 (reliable, fast)
- **Qt Support**: Via CMAKE_PREFIX_PATH environment variable
- **Launcher**: `./launch_vscode_with_qt.bat`
- **Status**: ✅ Fully working for Widgets + QML

### ✅ Qt Creator (Mobile & QML)
- **Compiler**: MSVC 2022 (Qt-native)
- **Environment**: Initialized via vcvars64.bat
- **Launcher**: `./launch_qtcreator_with_msvc.bat`  
- **Status**: ✅ Configured with launcher script

### ✅ Visual Studio (Refactoring & Analysis)
- **Compiler**: MSVC 2022 (native integration)
- **Environment**: Initialized via vcvars64.bat
- **Launcher**: `./launch_visual_studio_with_qt.bat`
- **Status**: ✅ Configured with launcher script

## 🔄 Recommended Workflow

### Daily Development → **Use VS Code**

```bash
# 1. Launch VS Code with Qt environment
./launch_vscode_with_qt.bat

# 2. Select CMake preset in VS Code
# - vscode-debug-widgets (for Qt Widgets development)
# - vscode-debug-qml (for QML development)

# 3. Build and test
Ctrl+Shift+P → "CMake: Build"
```

**Why VS Code for daily work:**
- ✅ **Reliable builds** - No environment issues
- ✅ **Fast compilation** - Clang is efficient
- ✅ **Excellent debugging** - GDB integration
- ✅ **Superior editing** - IntelliSense, refactoring
- ✅ **Git integration** - Best-in-class source control

### Mobile Deployment → **Use Qt Creator**

```bash
# 1. Launch Qt Creator with MSVC environment
./launch_qtcreator_with_msvc.bat

# 2. Open project with Qt Creator preset
# - qtcreator-debug-widgets
# - qtcreator-debug-qml

# 3. Configure mobile kit and deploy
Tools → Options → Devices → Android
```

**Why Qt Creator for mobile:**
- ✅ **Android deployment** - Built-in APK building
- ✅ **Device management** - Easy device selection
- ✅ **Mobile debugging** - On-device debugging
- ✅ **QML profiling** - Performance analysis

### Large Refactoring → **Use Visual Studio**

```bash
# 1. Launch Visual Studio with MSVC environment  
./launch_visual_studio_with_qt.bat

# 2. Open CMake project
File → Open → CMake... → Select root CMakeLists.txt

# 3. Choose Visual Studio preset
# - visualstudio-debug-widgets
# - visualstudio-debug-qml
```

**Why Visual Studio for refactoring:**
- ✅ **Advanced refactoring** - Extract method, rename across solution
- ✅ **IntelliCode** - AI-powered code suggestions
- ✅ **Code analysis** - Built-in static analysis  
- ✅ **Call hierarchy** - Visualize function relationships

### UI Design → **Use Qt Creator**

```bash
# Launch Qt Creator for visual design work
./launch_qtcreator_with_msvc.bat

# Use Qt Designer, QML Designer, etc.
```

**Why Qt Creator for UI:**
- ✅ **Visual QML editor** - Drag-and-drop design
- ✅ **Qt Designer** - Form designer
- ✅ **QML debugger** - Step through QML code
- ✅ **Qt Quick Designer** - Live preview

## 📋 Task Matrix

| Task | Recommended Tool | Alternative | Notes |
|------|------------------|-------------|-------|
| **C++ Development** | VS Code | Visual Studio | VS Code faster, VS more features |
| **Code Editing** | VS Code | Visual Studio | VS Code faster startup |
| **Building Desktop Apps** | VS Code | Visual Studio | Both work, VS Code faster |
| **Debugging C++** | Visual Studio | VS Code | VS has advanced debugging tools |
| **Large Refactoring** | Visual Studio | VS Code | VS has advanced refactoring |
| **Code Analysis** | Visual Studio | VS Code | VS has built-in static analysis |
| **QML Development** | VS Code | Qt Creator | Both work well |
| **QML Debugging** | Qt Creator | VS Code | Qt Creator has better QML tools |
| **UI Design** | Qt Creator | - | Visual designers only in Qt Creator |
| **Android Deployment** | Qt Creator | - | Mobile deployment Qt Creator only |
| **Git Operations** | VS Code | Visual Studio | VS Code has superior Git support |
| **Performance Profiling** | Visual Studio | VS Code | VS has built-in profilers |

## 🔧 Environment Variables

### For VS Code:
```bash
# Set before launching (or use launch script)
CMAKE_PREFIX_PATH=C:/Qt/6.9.2/msvc2022_64
```

### For Qt Creator:
```bash
# MSVC environment (handled by launch script)
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
CMAKE_PREFIX_PATH=C:/Qt/6.9.2/msvc2022_64
```

## 🚀 Quick Start Commands

```bash
# Daily development (fast builds)
./launch_vscode_with_qt.bat

# Large refactoring tasks
./launch_visual_studio_with_qt.bat

# Mobile deployment  
./launch_qtcreator_with_msvc.bat

# Build examples
cd build/vscode/clang/debug-widgets && ninja           # VS Code
cd build/visualstudio/msvc/debug-widgets && ninja     # Visual Studio  
cd build/qtcreator/msvc/debug-widgets && ninja        # Qt Creator
```

## 🐛 Troubleshooting

### VS Code: "Qt6 not found"
```bash
# Solution: Use the launcher script
./launch_vscode_with_qt.bat
```

### Qt Creator: "Standard headers missing"
```bash
# Solution: Use the launcher script  
./launch_qtcreator_with_msvc.bat
```

### Both: Build conflicts
```bash
# Solution: Each IDE uses separate build directories
build/vscode/debug-widgets/     # VS Code widgets
build/vscode/debug-qml/         # VS Code QML
build/qtcreator/debug-widgets/  # Qt Creator widgets  
build/qtcreator/debug-qml/      # Qt Creator QML
```

## 🎯 Best Practices

1. **Start with VS Code** for all development work
2. **Switch to Qt Creator** only for deployment or UI design
3. **Use launcher scripts** to avoid environment issues
4. **Keep builds separate** - don't mix IDE build directories
5. **Commit only source code** - exclude build directories from Git
6. **Test on both toolchains** before major releases