# CLion STM32 调试和烧录配置指南

## 前置条件

### 1. 安装必要工具
```bash
# 安装 OpenOCD（用于与 ST-Link 通信）
brew install openocd

# 验证安装
which openocd  # 应该显示 /opt/homebrew/bin/openocd
```

### 2. 确认工具路径
```bash
# 查找 ARM GDB 位置
find /opt -name "arm-none-eabi-gdb" 2>/dev/null
# 通常在：/opt/ST/STM32CubeCLT_*/GNU-tools-for-STM32/bin/arm-none-eabi-gdb
```

### 3. 硬件连接
- ST-Link 连接到 Mac
- ST-Link 通过 SWD 接口连接到 STM32 开发板
- 开发板正常供电

## 配置一：调试配置（🐛 调试按钮）

### 1. 创建调试配置
1. 打开 `Run → Edit Configurations...`
2. 点击 `+` → `Embedded GDB Server`
3. 配置如下：

**基本信息**：
- Name: `STM32 Debug`
- Target: `stm32`
- Executable: `stm32`

**GDB Server 配置**：
- GDB Server: `/opt/homebrew/bin/openocd`
- GDB Server args: `-f stm32f103ze_stlink.cfg`
- Working directory: `$ProjectFileDir$`

**Debugger 配置**：
- GDB: `/opt/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin/arm-none-eabi-gdb`
  （路径请根据实际安装位置调整）
- 'target remote' args: `tcp:localhost:3333`

**Startup Commands**（在 "Startup Commands" 标签页）：
```
monitor reset halt
load
monitor reset halt
break main
continue
```

### 2. 调试使用方法
1. 连接 ST-Link 到电脑和开发板
2. 点击调试按钮（🐛）或按 `Shift+F9`
3. CLion 会自动：
   - 编译项目
   - 启动 OpenOCD 服务器
   - 烧录程序到 STM32
   - 在 main 函数处设置断点并停止

## 配置二：烧录配置（▶️ 运行按钮）

### 1. 创建 External Tool 配置
1. 打开 `Run → Edit Configurations...`
2. 点击 `+` → `External Tool`
3. 配置如下：

**Tool Settings**：
- Name: `Flash STM32`
- Program: `/opt/homebrew/bin/openocd`
- Arguments: `-f stm32f103ze_stlink.cfg -c "init; reset halt; flash write_image erase build/Debug/stm32.elf; reset run; exit"`
- Working directory: `$ProjectFileDir$`

**Advanced Options**：
- ✅ Make console active on message in stdout
- ✅ Make console active on message in stderr

### 2. 烧录使用方法
1. 确保项目已编译
2. 点击运行按钮（▶️）或按 `Shift+F10`
3. 程序会烧录到 STM32 并立即开始运行（不调试）

## 配置文件说明

项目中的 `stm32f103ze_stlink.cfg` 配置文件包含：
```bash
# STM32F103ZE with ST-Link configuration
source [find interface/stlink.cfg]
source [find target/stm32f1x.cfg]
transport select hla_swd
set CHIPNAME stm32f103ze
adapter speed 4000
reset_config srst_only srst_nogate
```

## 常见问题排查

### 1. "找不到 cfg 文件"
- 确保 `stm32f103ze_stlink.cfg` 在项目根目录
- 在配置中使用绝对路径：`$ProjectFileDir$/stm32f103ze_stlink.cfg`

### 2. "连接失败"
```bash
# 测试 ST-Link 连接
openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "init; exit"
```

### 3. "GDB 无法连接"
- 检查端口 3333 是否被占用：`lsof -i :3333`
- 确认 OpenOCD 是否正常启动并监听 3333 端口

### 4. "权限问题"
```bash
# 可能需要添加 udev 规则（Linux）或重新插拔 ST-Link（macOS）
```

### 5. "编译失败"
确保 CMake 配置正确：
```cmake
# 在 CMakeLists.txt 中确认
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
```

## 调试技巧

### GDB 常用命令
- `continue` 或 `c` - 继续运行
- `next` 或 `n` - 单步执行（不进入函数）
- `step` 或 `s` - 单步执行（进入函数）
- `break function_name` - 设置断点
- `print variable` - 打印变量值
- `info registers` - 查看寄存器状态
- `monitor reset halt` - 复位并停止 MCU

### 查看内存和寄存器
- `x/10x 0x20000000` - 查看 RAM 起始地址的内容
- `info reg` - 显示所有寄存器
- `monitor stm32f1x.cpu` - STM32 特定的监控命令

## 高级配置

### 添加 SVD 文件支持（可选）
下载 STM32F103 的 SVD 文件，在调试配置中添加：
- SVD File: 指向 STM32F103xx.svd 文件路径

这样可以在调试时查看外设寄存器的详细信息。

## 验证配置

成功配置后，你应该能够：
1. ✅ 使用调试按钮启动调试会话
2. ✅ 使用运行按钮快速烧录程序
3. ✅ 在代码中设置断点
4. ✅ 单步执行和查看变量
5. ✅ 查看 OpenOCD 和 GDB 的控制台输出

配置完成后，建议保存这些配置到版本控制中，这样团队其他成员也能快速上手。