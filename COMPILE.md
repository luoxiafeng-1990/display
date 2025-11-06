# Display Package - 编译指南

Display 现在是一个独立的 Buildroot package，编译非常简单！

## 🚀 快速编译

### 方法1：使用 menuconfig（推荐）

```bash
cd /home/rlk/intchains/ai_sdk/release_version

# 1. 打开配置菜单
make menuconfig

# 2. 勾选 display package
#    导航到：[*] display - Display Framework
#    （应该在 tps-test 附近）

# 3. 保存退出，然后编译
make display
```

### 方法2：直接编译（最快）

```bash
cd /home/rlk/intchains/ai_sdk/release_version

# 1. 启用 display（如果还没启用）
echo "BR2_PACKAGE_DISPLAY=y" >> .config

# 2. 编译 display
make display
```

---

## 📦 编译结果位置

### 编译目录（您期望的位置）

```
output/current/build/display-1.0/
├── display_test          ← 可执行文件
├── test.o
└── source/
    ├── LinuxFramebufferDevice.o
    ├── VideoFile.o
    └── PerformanceMonitor.o
```

### 安装目录

```
output/current/target/usr/local/bin/
└── display_test          ← 安装到目标系统
```

---

## 🔍 验证编译

```bash
# 查看编译产物
ls -lh output/current/build/display-1.0/display_test

# 查看 .o 文件
ls -lh output/current/build/display-1.0/source/*.o

# 运行测试
output/current/build/display-1.0/display_test --help
```

---

## 🛠️ 重新编译

```bash
# 清理并重编译
make display-dirclean
make display

# 仅重新编译（不清理）
make display-rebuild
```

---

## 📁 目录结构

```
packages/display/              ← 独立 package（已从 tps-test 移出）
├── include/                   # 头文件
│   ├── Buffer.hpp
│   ├── IDisplayDevice.hpp
│   ├── LinuxFramebufferDevice.hpp
│   ├── VideoFile.hpp
│   └── PerformanceMonitor.hpp
├── source/                    # 源文件
│   ├── LinuxFramebufferDevice.cpp
│   ├── VideoFile.cpp
│   └── PerformanceMonitor.cpp
├── test.cpp                   # 测试程序（带 main 函数）
├── display.mk                 # Buildroot 编译配置
├── Config.in                  # Buildroot 菜单配置
├── configure.ac               # Autotools 配置
├── Makefile.am                # Autotools Makefile
├── BUILD.md                   # 原编译文档
├── README.md                  # 使用文档
└── COMPILE.md                 # 本文档（简化编译指南）
```

---

## ✅ 优势

相比之前的复杂配置，现在：

✅ **独立 package**：display 和 tps-test 完全分离  
✅ **简单编译**：只需 `make display` 即可  
✅ **独立目录**：编译结果在 `output/current/build/display-1.0/`  
✅ **无冲突**：不会影响 tps-test 的编译  
✅ **标准化**：遵循 Buildroot package 规范  

---

## 🎯 一条命令编译

```bash
cd /home/rlk/intchains/ai_sdk/release_version && \
echo "BR2_PACKAGE_DISPLAY=y" >> .config && \
make display
```

就这么简单！🎉

