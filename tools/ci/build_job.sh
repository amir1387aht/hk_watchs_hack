#!/bin/bash
# 通用构建脚本 - 处理所有动态生成的构建Job

set -e

# 读取环境变量
PROJECT_PATH=${PROJECT_PATH}
BOARD=${BOARD}  # 原始board名称（用于目录名）
SCONS_BOARD=${SCONS_BOARD}  # scons命令使用的board名称
BUILD_ARGS=${BUILD_ARGS}
LOG_NAME=${LOG_NAME}
PROJECT_TYPE=${PROJECT_TYPE}
PARALLEL_JOBS=${PARALLEL_JOBS:-8}

echo "============================================"
echo "构建项目: ${PROJECT_PATH}"
echo "开发板: ${BOARD:-N/A}"
echo "Scons Board: ${SCONS_BOARD:-N/A}"
echo "类型: ${PROJECT_TYPE}"
echo "构建参数: ${BUILD_ARGS}"
echo "============================================"

# 创建必要目录
mkdir -p ci_build_logs artifacts

# 获取绝对路径
ROOT_DIR=$(pwd)
LOG_FILE="${ROOT_DIR}/ci_build_logs/${LOG_NAME}.log"

# 切换到项目目录
cd "${PROJECT_PATH}"

# 构建命令
BUILD_CMD="scons ${BUILD_ARGS} -j${PARALLEL_JOBS}"

echo "[$(date)] 开始构建: ${BUILD_CMD}" | tee "${LOG_FILE}"

# 执行构建
if ${BUILD_CMD} >> "${LOG_FILE}" 2>&1; then
    echo "[$(date)] ✅ 构建成功" | tee -a "${LOG_FILE}"
else
    echo "[$(date)] ❌ 构建失败" | tee -a "${LOG_FILE}"
    exit 1
fi

# 返回根目录
cd "${ROOT_DIR}"

# 收集构建产物
if [ "${PROJECT_TYPE}" = "common" ] && [ -n "${BOARD}" ]; then
    # 通用项目构建目录
    # 确保构建目录名有_hcpu后缀
    if [[ "${BOARD}" == *"hcpu"* ]]; then
        BUILD_DIR="${ROOT_DIR}/${PROJECT_PATH}/build_${BOARD}"
    else
        BUILD_DIR="${ROOT_DIR}/${PROJECT_PATH}/build_${BOARD}_hcpu"
    fi
else
    # 普通项目构建目录
    BUILD_DIR="${ROOT_DIR}/${PROJECT_PATH}/build"
fi

if [ -d "${BUILD_DIR}" ]; then
    ARTIFACTS_DIR="${ROOT_DIR}/artifacts/${LOG_NAME}"
    mkdir -p "${ARTIFACTS_DIR}"
    
    echo "📦 开始收集构建产物，保留目录结构..."
    echo "   源目录: ${BUILD_DIR}"
    echo "   目标目录: ${ARTIFACTS_DIR}"
    
    # 定义要收集的文件扩展名（包括用户要求的.bat, .sh, .json）
    FILE_PATTERNS=("*.bin" "*.hex" "*.elf" "*.axf" "*.map" "*.bat" "*.sh" "*.json")
    
    # 计数器
    total_files=0
    
    # 遍历每个文件模式，保留目录结构
    for pattern in "${FILE_PATTERNS[@]}"; do
        # 使用find查找文件，保持相对路径
        find "${BUILD_DIR}" -name "${pattern}" -type f | while read -r source_file; do
            # 计算相对于BUILD_DIR的路径
            relative_path="${source_file#${BUILD_DIR}/}"
            target_file="${ARTIFACTS_DIR}/${relative_path}"
            target_dir="$(dirname "${target_file}")"
            
            # 创建目标目录
            mkdir -p "${target_dir}"
            
            # 复制文件，保持时间戳
            if cp -p "${source_file}" "${target_file}" 2>/dev/null; then
                echo "  ✓ ${relative_path}"
                total_files=$((total_files + 1))
            else
                echo "  ✗ 复制失败: ${relative_path}"
            fi
        done
    done
    
    # 统计实际复制的文件数量
    actual_files=$(find "${ARTIFACTS_DIR}" -type f | wc -l | tr -d ' ')
    
    if [ "${actual_files}" -gt 0 ]; then
        echo "✅ 构建产物收集完成!"
        echo "   📊 共收集 ${actual_files} 个文件到 artifacts/${LOG_NAME}/"
        echo "   📁 目录结构已保留"
        
        # 显示收集到的文件类型统计
        echo "   📋 文件类型统计:"
        for pattern in "${FILE_PATTERNS[@]}"; do
            count=$(find "${ARTIFACTS_DIR}" -name "${pattern}" -type f | wc -l | tr -d ' ')
            if [ "${count}" -gt 0 ]; then
                echo "      ${pattern}: ${count} 个"
            fi
        done
    else
        echo "⚠️  未找到匹配的构建产物文件"
    fi
else
    echo "⚠️  构建产物目录不存在: ${BUILD_DIR}"
fi

echo "构建完成！"
