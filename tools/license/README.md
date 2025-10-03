# 许可证统一工具

这个工具用于统一仓库下所有代码文件的开源许可证格式，使用SPDX风格进行注释。

## 功能特性

- ✅ 自动检测并替换现有的版权声明
- ✅ 添加统一的SPDX格式许可证头  
- ✅ 移除文件末尾的旧版COPYRIGHT声明
- ✅ 根据Git历史自动确定版权年份范围
- ✅ 支持多种编程语言文件 (.c, .h, .cpp, .hpp, .cc, .cxx)
- ✅ 智能排除不相关的目录 (如external, .git等)
- ✅ 智能识别复杂的许可证文本块（如完整的BSD许可证）
- ✅ 保留重要的文档注释（@file, @author, @brief等）
- ✅ 安全处理，避免误删代码内容

## 使用方法

### 处理整个项目
```bash
cd /path/to/SiFli-SDK
python3 tools/license/unify_license.py
```

### 处理指定目录
```bash
cd /path/to/SiFli-SDK
python3 tools/license/unify_license.py drivers/
python3 tools/license/unify_license.py middleware/
```

### 处理单个文件
```bash
cd /path/to/SiFli-SDK
python3 tools/license/unify_license.py drivers/hal/bf0_hal_audcodec.c
```

### 查看帮助
```bash
python3 tools/license/unify_license.py --help
```

## 许可证模板

脚本会将所有文件的许可证头统一替换为以下格式：

```c
/*
 * SPDX-FileCopyrightText: <年份范围> SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */
```

其中年份范围的确定规则：
- 如果文件只有一年的修改历史：`2025`
- 如果文件有多年的修改历史：`2020-2025`

## 处理规则

### 支持的文件类型
- C/C++源文件：`.c`, `.cpp`, `.cc`, `.cxx`
- C/C++头文件：`.h`, `.hpp`

### 排除的目录
- `.git`, `__pycache__`, `.pytest_cache`
- `build`, `dist`, `node_modules`
- `external` (第三方库)
- `.vscode`, `.idea` (IDE配置)

### 处理逻辑
1. **移除现有版权声明**：自动检测并移除文件开头的各种格式的版权声明
2. **移除文件末尾声明**：移除形如 `/************************ (C) COPYRIGHT Sifli Technology *******END OF FILE****/` 的声明
3. **添加SPDX头**：在文件开头添加统一的SPDX格式许可证头
4. **年份确定**：通过Git历史确定文件的创建和最后修改年份

## 安全性

- 脚本会在修改前读取原文件内容，只有在内容确实需要更新时才会写入
- 支持处理UTF-8编码的文件，对于编码错误会忽略而不是崩溃
- 每个处理的文件都会显示处理状态，便于追踪

## 示例输出

```
开始扫描目录: /Users/halfsweet/Documents/OpenSiFli/SiFli-SDK/drivers
支持的文件类型: .cpp, .c, .hpp, .cc, .cxx, .h
排除的目录: .git, __pycache__, .pytest_cache, build, dist, external, .vscode, .idea, node_modules
------------------------------------------------------------
已更新: drivers/hal/bf0_hal_audcodec.c
已更新: drivers/hal/bf0_hal_uart.c
无需更新: drivers/hal/bf0_hal_gpio.c
------------------------------------------------------------
扫描完成!
处理文件数: 156
更新文件数: 98
```

## 注意事项

- 建议在运行脚本前先提交当前的代码更改，以便在需要时可以回滚
- 脚本会自动跳过external目录下的第三方库文件
- 如果Git历史不可用，脚本会使用当前年份作为版权年份

## 故障排除

### 权限问题
如果遇到权限问题，请确保脚本有可执行权限：
```bash
chmod +x tools/license/unify_license.py
```

### Python版本
请确保使用Python 3.6+版本：
```bash
python3 --version
```

### Git仓库
脚本依赖Git历史来确定文件年份，请确保在Git仓库中运行。
