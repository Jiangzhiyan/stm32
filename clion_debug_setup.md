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

## 调试配置（🐛 调试按钮）

### 1. 创建 OpenOCD 下载并运行配置
1. 打开 `Run → Edit Configurations...`
2. 点击 `+` → `OpenOCD Download & Run`
3. 配置如下：

**基本信息**：
- Name: `STM32 Debug`
- Target: `stm32`
- Executable: `stm32`

**OpenOCD 配置**：
- Board config file: `/完整路径/stm32f103ze_stlink.cfg`
- Download: ✅ （勾选，这样会自动下载程序到芯片）
- Reset after download: ✅ （勾选，下载后复位芯片）

**GDB 配置**：
- GDB: `/opt/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin/arm-none-eabi-gdb`
  （路径请根据实际安装位置调整）

### 2. 调试使用方法
1. 连接 ST-Link 到电脑和开发板
2. 点击调试按钮（🐛）或按 `Shift+F9`
3. CLion 会自动：
   - 编译项目
   - 启动 OpenOCD 服务器
   - 烧录程序到 STM32
   - 在 main 函数处设置断点并停止

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
- 使用绝对路径：`/完整路径/stm32f103ze_stlink.cfg`

### 2. "连接失败"
```bash
# 测试 ST-Link 连接
openocd -f stm32f103ze_stlink.cfg -c "init; exit"
```

### 3. "OpenOCD Download & Run 找不到"
- 确保 CLion 版本支持该功能（较新版本）
- 如果没有此选项，使用 External Tool 配置

### 4. "Board config file 相对路径不工作"
- 使用绝对路径代替相对路径
- 确保 Working directory 设置正确

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

### 添加 SVD 文件支持（强烈推荐）

项目中已包含 `STM32F103.svd` 文件。在调试配置中添加：

**在 OpenOCD Download & Run 配置中**：
- SVD File: `$ProjectFileDir$/STM32F103_Custom.svd`
  
**注意**: 项目包含两个 SVD 文件：
- `STM32F103.svd` - 标准 SVD 文件  
- `STM32F103_Custom.svd` - 包含引脚标签信息的自定义 SVD 文件（推荐使用）

**SVD 文件的好处**：
1. **外设寄存器可视化**：在 Peripherals 窗口查看所有外设寄存器
2. **位字段解析**：显示寄存器位的含义，不是原始数字
3. **实时监控**：调试时实时查看外设状态变化
4. **寄存器修改**：可以直接修改寄存器值进行测试

**使用方法**：
1. 调试时打开 `View → Tool Windows → Peripherals`
2. 展开外设树形菜单（如 GPIOB, TIM2, USART1 等）
3. 实时查看寄存器值和位字段状态

**例如查看 GPIO 状态**：
```
GPIOB
├── MODER   = 0x00000400  (Pin 5: Output mode)
├── ODR     = 0x0020      (Pin 5: High)
├── IDR     = 0x0000      (Input data)
└── BSRR    = 0x00000000  (Bit set/reset)
```

## 验证配置

成功配置后，你应该能够：
1. ✅ 使用调试按钮启动调试会话
2. ✅ 使用运行按钮快速烧录程序
3. ✅ 在代码中设置断点
4. ✅ 单步执行和查看变量
5. ✅ 查看 OpenOCD 和 GDB 的控制台输出

配置完成后，建议保存这些配置到版本控制中，这样团队其他成员也能快速上手。