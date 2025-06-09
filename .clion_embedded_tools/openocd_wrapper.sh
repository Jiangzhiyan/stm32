#!/bin/bash
# CLion OpenOCD wrapper script

# 确保使用正确的 OpenOCD 路径
OPENOCD="/opt/homebrew/bin/openocd"

# 运行 OpenOCD with CLion parameters
exec $OPENOCD "$@"