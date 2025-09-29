# Qt Creator Kit Configuration Guide

This guide explains how to properly configure Qt Creator to avoid "standard headers missing" and linker errors.

## Quick Fix: Launch Script
Use the provided launcher script:
```bash
./launch_qtcreator_with_msvc.bat
```

## Manual Configuration

### 1. Configure MSVC Kit Environment

1. **Open Qt Creator**
2. **Tools → Options → Kits**
3. **Select your MSVC kit** (or create new)
4. **Environment section → Change...**
5. **Add the following environment setup:**

```batch
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

### 2. Verify Kit Settings

Ensure your kit has:
- **Compiler**: Microsoft Visual C++ Compiler (from Visual Studio Build Tools 2022)
- **Qt Version**: Qt 6.9.2 MSVC2022 64-bit  
- **CMake Tool**: CMake 4.0+
- **Generator**: Ninja

### 3. Environment Variables

The kit should have these environment variables set:
```
CMAKE_PREFIX_PATH=C:\Qt\6.9.2\msvc2022_64
```

### 4. Test Configuration

1. **Create a simple test project**
2. **Build → Configure Project**
3. **Verify no "standard headers missing" errors**

## Troubleshooting

### Problem: "Cannot find <string>, <memory>, etc."

**Solution 1**: Use the launch script
```bash
./launch_qtcreator_with_msvc.bat
```

**Solution 2**: Manual environment setup
1. Tools → Options → Kits
2. Environment → Change
3. Add: `call "C:\...\vcvars64.bat"`

**Solution 3**: Restart Qt Creator
- Close Qt Creator completely
- Reopen and reconfigure project

### Problem: Linker errors (LNK2019)

**Solution**: Check that all source files compile successfully first
1. Build → Clean All
2. Build → Configure Project 
3. Build → Build All
4. Check compilation output for errors

### Problem: Kit not found

**Solution**: Create new kit
1. Tools → Options → Kits
2. Add → Manual kit
3. Set compiler to MSVC 2022
4. Set Qt version to 6.9.2 MSVC2022

## Environment Verification

To verify your environment is working:

1. **Open Qt Creator terminal**
2. **Run these commands:**
```bash
echo %INCLUDE%
echo %LIB%  
echo %CMAKE_PREFIX_PATH%
cl.exe /?
```

3. **Expected output:**
- INCLUDE should list MSVC header paths
- LIB should list MSVC library paths  
- CMAKE_PREFIX_PATH should point to Qt
- cl.exe should show MSVC compiler version

## Alternative: Use VS Code for Development

If Qt Creator continues to have issues:
1. Use **VS Code for all development work** (reliable)
2. Use **Qt Creator only for deployment and mobile** (when needed)
3. Switch between tools as needed for different tasks