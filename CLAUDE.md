# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Configure and Build
```bash
# Configure with Debug preset (recommended)
cmake --preset Debug

# Build the firmware
cmake --build build/Debug

# Alternative: use ninja directly after configuration
cd build/Debug && ninja
```

### Build Configurations
- **Debug**: No optimization (-O0), full debug symbols (-g3)
- **Release**: Size optimization (-Os), no debug symbols
- **RelWithDebInfo**: Size optimization with debug info
- **MinSizeRel**: Minimum size optimization

### Post-Build Commands
```bash
# Generate binary for flashing
arm-none-eabi-objcopy -O binary build/Debug/stm32.elf stm32.bin

# Generate Intel HEX file
arm-none-eabi-objcopy -O ihex build/Debug/stm32.elf stm32.hex

# Check firmware size
arm-none-eabi-size build/Debug/stm32.elf
```

## Architecture Overview

### MCU Configuration
- **Target**: STM32F103ZET6 (Cortex-M3)
- **Flash**: 512KB at 0x08000000
- **RAM**: 64KB at 0x20000000
- **Clock**: 72MHz (8MHz HSE × 9 PLL)
- **Stack**: 1KB (0x400)
- **Heap**: 512B (0x200)

### Project Structure
- **Core/**: Application code with STM32CubeMX-generated structure
  - User code goes between `/* USER CODE BEGIN */` and `/* USER CODE END */` markers
  - Main application loop in `main.c`
- **Drivers/**: STM32 HAL and CMSIS libraries (do not modify)
- **cmake/stm32cubemx/**: Auto-generated CMake configuration from STM32CubeMX
- **stm32.ioc**: STM32CubeMX project file - regenerate code with STM32CubeMX if peripheral configuration changes

### Build System
The project uses CMake with a custom ARM toolchain file (`cmake/gcc-arm-none-eabi.cmake`). The main CMakeLists.txt includes the STM32CubeMX-generated sources through a subdirectory. All source files, includes, and definitions are managed in `cmake/stm32cubemx/CMakeLists.txt`.

### Key Files
- **STM32F103XX_FLASH.ld**: Linker script defining memory layout
- **startup_stm32f103xe.s**: MCU startup code and vector table
- **main.h**: Pin definitions (LED0 on PB5)
- **stm32f1xx_hal_conf.h**: HAL module enable/disable configuration

## CLion IDE Configuration

### Debug Configuration

#### Option 1: ST-Link GDB Server (Recommended for CLion 2024.2+)
**Type**: Embedded GDB Server
- GDB Server: `/opt/homebrew/bin/st-util` (install with `brew install stlink`)
- GDB Server args: `--listen_port 4242`
- GDB: `/opt/ST/STM32CubeCLT_*/GNU-tools-for-STM32/bin/arm-none-eabi-gdb`
- Target remote: `localhost:4242`
- Download: ✅ (auto-download to chip)
- Reset commands: `monitor reset init`

**Advantages**:
- Native support in CLion 2024.2+
- Live variable watches during debugging
- Better integration with CLion's debugging features
- No need for OpenOCD configuration files

#### Option 2: OpenOCD (Legacy)
**Type**: OpenOCD Download & Run
- Board config file: `$ProjectFileDir$/stm32f103ze_stlink.cfg`
- Download: ✅ (auto-download to chip)
- Reset after download: ✅ (reset after download)
- GDB: `/opt/ST/STM32CubeCLT_*/GNU-tools-for-STM32/bin/arm-none-eabi-gdb`

### IDE Support
This project supports both **CLion** and **VS Code**:

**CLion Users**:
- Use configurations in `.idea/runConfigurations/`
- Debug with "OpenOCD Download & Run" 
- Flash with "External Tool"

#### Fixing CLion Red Marks/Errors
If CLion shows red marks for standard types like `uint32_t`:

1. **Update Toolchain** (Settings → Build, Execution, Deployment → Toolchains):
   - C Compiler: `/opt/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin/arm-none-eabi-gcc`
   - C++ Compiler: `/opt/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin/arm-none-eabi-g++`

2. **Enable Clangd** (Settings → Languages & Frameworks → C/C++ → Clangd):
   - Enable "Use clangd-based language engine"
   - Project includes `.clangd` configuration for ARM target

3. **Reload CMake Project**: Tools → CMake → Reload CMake Project

4. **Restart CLion** to apply all changes

**VS Code Users**:  
- Install "Cortex-Debug" extension
- Use configurations in `.vscode/launch.json`
- Requires manual OpenOCD installation

### Prerequisites for CLion
```bash
# Install OpenOCD
brew install openocd

# Verify paths
which openocd  # Should show /opt/homebrew/bin/openocd
find /opt -name "arm-none-eabi-gdb" 2>/dev/null
```

## Development Workflow

### Adding User Code
1. Place code between designated USER CODE markers to preserve it during STM32CubeMX regeneration
2. Add new source files to `CMakeLists.txt` in the `target_sources()` section
3. Add include paths to `target_include_directories()` if needed

### Modifying Peripherals
1. Open `stm32.ioc` in STM32CubeMX
2. Make peripheral changes
3. Generate code (Project Manager → Project → Generate Code)
4. Rebuild the project

### Debugging with CLion
1. Connect ST-Link to STM32 board and Mac
2. Click debug button (🐛) or press `Shift+F9`
3. Use run button (▶️) for flash-only operations

### Required Tools
- **arm-none-eabi-gcc**: ARM GCC toolchain - **MUST use ST-provided toolchain from STM32CubeCLT**
  - Located at: `/opt/ST/STM32CubeCLT_*/GNU-tools-for-STM32/bin/`
  - Add to PATH: `export PATH="/opt/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin:$PATH"`
  - **DO NOT use homebrew ARM GCC** - it has incompatible headers for macOS
- **CMake**: Version 3.22+
- **Ninja**: Build system (or use Make)
- **OpenOCD**: For ST-Link communication (`brew install openocd`)
- **STM32CubeMX**: For peripheral configuration
- **ST-Link**: Hardware debugger/programmer