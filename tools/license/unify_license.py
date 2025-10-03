#!/usr/bin/env python3
"""
统一仓库下的开源许可证脚本

该脚本会：
1. 扫描指定目录下的所有代码文件
2. 检测现有的版权声明
3. 替换为统一的SPDX格式许可证头
4. 移除文件末尾的旧版权声明
5. 根据文件的git历史确定版权年份范围

使用方法:
    python unify_license.py [目录路径]
"""

import os
import re
import subprocess
import argparse
import sys
from datetime import datetime
from pathlib import Path


class LicenseUnifier:
    def __init__(self, root_path=None, dry_run=False):
        self.root_path = Path(root_path) if root_path else Path.cwd()
        self.current_year = datetime.now().year
        self.dry_run = dry_run
        
        # SPDX许可证模板
        self.spdx_template = """/*
 * SPDX-FileCopyrightText: {year_range} SiFli Technologies(Nanjing) Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */"""
        
        # 支持的文件扩展名
        self.supported_extensions = {'.c', '.h', '.cpp', '.hpp', '.cc', '.cxx'}
        
        # 需要排除的目录
        self.excluded_dirs = {
            '.git', '__pycache__', '.pytest_cache', 'build', 'dist',
            'external', '.vscode', '.idea', 'node_modules'
        }
        
        # 旧版权声明的匹配模式
        self.old_copyright_patterns = [
            # 匹配SPDX格式的许可证头
            re.compile(r'^/\*\s*\n\s*\*\s*SPDX-.*?\*/\s*\n', re.MULTILINE | re.DOTALL),
            # 匹配完整的许可证文本块 (如BSD, Apache等)
            re.compile(r'^/\*\*?\s*\n.*?(?:Copyright|SPDX-).*?(?:OF THE USE OF THIS SOFTWARE|SUCH DAMAGE|License-Identifier).*?\*/\s*\n', re.MULTILINE | re.DOTALL | re.IGNORECASE),
            # 匹配简单的版权声明
            re.compile(r'^/\*\*?\s*\n.*?Copyright.*?\*/\s*\n', re.MULTILINE | re.DOTALL | re.IGNORECASE),
            # 匹配单行版权声明
            re.compile(r'^//.*?(?:Copyright|SPDX-).*?\n', re.MULTILINE | re.IGNORECASE),
        ]
        
        # 文件末尾版权声明的匹配模式
        self.end_copyright_pattern = re.compile(
            r'/\*{5,}.*?\(C\).*?COPYRIGHT.*?Sifli.*?Technology.*?\*{5,}END.*?OF.*?FILE\*{4,}/',
            re.IGNORECASE
        )

    def get_file_years(self, file_path):
        """获取文件的创建年份和最后修改年份"""
        try:
            # 获取文件的git历史
            cmd = ['git', 'log', '--format=%ad', '--date=format:%Y', '--', str(file_path)]
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.root_path)
            
            if result.returncode == 0 and result.stdout.strip():
                years = [int(year) for year in result.stdout.strip().split('\n') if year.isdigit()]
                if years:
                    min_year = min(years)
                    max_year = max(years)
                    return min_year, max_year
        except Exception as e:
            print(f"警告: 无法获取文件 {file_path} 的git历史: {e}")
        
        # 如果无法获取git历史，使用文件系统时间
        try:
            stat = file_path.stat()
            # 使用当前年份作为创建年份（保守估计）
            return self.current_year, self.current_year
        except Exception:
            return self.current_year, self.current_year

    def format_year_range(self, start_year, end_year):
        """格式化年份范围"""
        if start_year == end_year:
            return str(start_year)
        else:
            return f"{start_year}-{end_year}"

    def remove_old_copyright(self, content):
        """移除旧的版权声明"""
        original_content = content
        print(f"开始处理，原始内容长度: {len(original_content)}")
        
        # 1. 移除已有的SPDX块（为了重新添加新的）
        spdx_pattern = re.compile(
            r'^/\*\s*\n'
            r'\s*\*\s*SPDX-.*?\n'
            r'(?:\s*\*.*?\n)*'
            r'\s*\*/'
            r'\s*\n?',
            re.MULTILINE | re.DOTALL
        )
        before_length = len(content)
        content = spdx_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了SPDX块，减少 {before_length - len(content)} 个字符")
        
        # 2. 根据用户指导：移除包含星号分隔线和@file的文件头注释块
        file_header_pattern = re.compile(
            r'/\*\*\s*\n'  # 注释开始
            r'(?=(?:(?!\*/).)*?\*{20,})'  # 必须包含至少20个连续的*
            r'(?=(?:(?!\*/).)*?@file)'  # 必须包含@file标记
            r'(?:(?!\*/).)*?'  # 匹配到*/之前的所有内容
            r'\*/',  # 注释结束
            re.MULTILINE | re.DOTALL | re.IGNORECASE
        )
        before_length = len(content)
        content = file_header_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了包含@file的文件头注释块，减少 {before_length - len(content)} 个字符")
        
        # 3. 专门针对@attention许可证块的模式
        attention_license_pattern = re.compile(
            r'/\*\*\s*\n'
            r'\s*\*\s*@attention\s*\n'
            r'(?:(?!\*/).)*?'  # 匹配到*/之前的所有内容
            r'\*/',
            re.MULTILINE | re.DOTALL | re.IGNORECASE
        )
        before_length = len(content)
        content = attention_license_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了@attention许可证块，减少 {before_length - len(content)} 个字符")
        
        # 4. 移除通用的完整许可证条款块（不包含@attention或@file的）
        generic_license_pattern = re.compile(
            r'/\*\*\s*\n'  # 注释开始
            r'(?=(?:(?!\*/).)*?Copyright.*?\d{4})'  # 必须包含版权年份
            r'(?=(?:(?!\*/).)*?(?:THIS SOFTWARE IS PROVIDED BY SIFLI TECHNOLOGY))'  # 包含特定的许可证声明
            r'(?=(?:(?!\*/).)*?(?:OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE))'  # 包含许可证结束标志
            r'(?:(?!\*/).)*?'  # 匹配到*/之前的所有内容
            r'\*/',  # 注释结束
            re.MULTILINE | re.DOTALL | re.IGNORECASE
        )
        before_length = len(content)
        content = generic_license_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了通用许可证条款块，减少 {before_length - len(content)} 个字符")
        
        # 5. 移除更宽泛的许可证块（处理不同格式的许可证块）
        broad_license_pattern = re.compile(
            r'/\*\*\s*\n'  # 注释开始
            r'\s*\*\s*\n'  # 可能有空行
            r'(?=(?:(?!\*/).)*?Copyright.*?Sifli.*?Technology)'  # 包含Sifli Technology版权
            r'(?=(?:(?!\*/).)*?(?:THIS SOFTWARE IS PROVIDED|All rights reserved))'  # 包含许可证开始标志
            r'(?=(?:(?!\*/).)*?(?:SUCH DAMAGE|POSSIBILITY OF SUCH DAMAGE))'  # 包含许可证结束标志
            r'(?:(?!\*/).)*?'  # 匹配到*/之前的所有内容
            r'\*/',  # 注释结束
            re.MULTILINE | re.DOTALL | re.IGNORECASE
        )
        before_length = len(content)
        content = broad_license_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了宽泛许可证条款块，减少 {before_length - len(content)} 个字符")
        before_length = len(content)
        content = generic_license_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了通用许可证条款块，减少 {before_length - len(content)} 个字符")
        
        # 4. 移除文件末尾的版权声明  
        end_patterns = [
            # 精确模式 - 匹配具体的文件末尾版权格式
            re.compile(
                r'/\*{5,}.*?\(C\).*?COPYRIGHT.*?Sifli.*?Technology.*?\*{5,}END.*?OF.*?FILE\*{4,}/',
                re.IGNORECASE
            )
        ]
        
        for i, pattern in enumerate(end_patterns):
            before_length = len(content)
            content = pattern.sub('', content)
            if len(content) != before_length:
                print(f"移除了文件末尾版权声明（模式{i+1}），减少 {before_length - len(content)} 个字符")
        
        # 5. 移除单行版权声明
        single_line_pattern = re.compile(r'^//.*?(?:Copyright|SPDX-).*?\n', re.MULTILINE | re.IGNORECASE)
        before_length = len(content)
        content = single_line_pattern.sub('', content)
        if len(content) != before_length:
            print(f"移除了单行版权声明，减少 {before_length - len(content)} 个字符")
        
        # 6. 只在文件开头清理连续的空行
        content = re.sub(r'^\n+', '', content)
        
        print(f"处理后内容长度: {len(content)}")
        
        # 7. 安全检查：如果内容被过度删除，则保留原内容
        content_ratio = len(content.strip()) / len(original_content.strip()) if original_content.strip() else 1
        if content_ratio < 0.7:  # 调整到70%阈值
            print(f"警告: 检测到可能的过度删除（保留率: {content_ratio:.1%}），保留原始内容")
            return original_content.strip()
        
        print(f"内容保留率: {content_ratio:.1%}")
        return content.strip()
    
    def _is_copyright_block_start(self, lines, start_index):
        """检查指定位置是否是版权声明块的开始"""
        # 在接下来的几行中查找版权相关关键词
        search_range = min(start_index + 10, len(lines))
        block_content = '\n'.join(lines[start_index:search_range])
        
        copyright_keywords = ['copyright', 'spdx-', 'license-identifier', 'redistribution', 'all rights reserved']
        return any(keyword in block_content.lower() for keyword in copyright_keywords)
    
    def _find_copyright_block_end(self, lines, start_index):
        """找到版权声明块的结束位置"""
        i = start_index
        in_comment = False
        
        while i < len(lines):
            line = lines[i].strip()
            
            if line.startswith('/*'):
                in_comment = True
            
            if in_comment and line.endswith('*/'):
                # 检查这个注释块是否包含版权信息
                block_content = '\n'.join(lines[start_index:i+1])
                if any(keyword in block_content.lower() for keyword in ['copyright', 'spdx-', 'license']):
                    return i
                else:
                    # 如果这个块不包含版权信息，停止搜索
                    return start_index - 1
            
            i += 1
        
        return start_index - 1

    def add_spdx_header(self, content, year_range):
        """添加SPDX许可证头"""
        header = self.spdx_template.format(year_range=year_range)
        
        # 如果文件内容不为空，在header后添加空行
        if content.strip():
            return header + '\n\n' + content
        else:
            return header + '\n'

    def should_skip_file(self, file_path):
        """判断是否应该跳过该文件"""
        # 检查文件扩展名
        if file_path.suffix not in self.supported_extensions:
            return True
        
        # 检查是否在排除的目录中
        for part in file_path.parts:
            if part in self.excluded_dirs:
                return True
        
        return False

    def process_file(self, file_path):
        """处理单个文件"""
        try:
            # 读取文件内容
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                original_content = f.read()
            
            # 移除旧的版权声明
            cleaned_content = self.remove_old_copyright(original_content)
            
            # 获取文件年份
            start_year, end_year = self.get_file_years(file_path)
            year_range = self.format_year_range(start_year, end_year)
            
            # 添加新的SPDX头
            new_content = self.add_spdx_header(cleaned_content, year_range)
            
            # 如果内容有变化，写回文件（除非是dry-run模式）
            if new_content != original_content:
                if not self.dry_run:
                    with open(file_path, 'w', encoding='utf-8') as f:
                        f.write(new_content)
                    print(f"已更新: {file_path.relative_to(self.root_path)}")
                else:
                    print(f"[DRY-RUN] 将更新: {file_path.relative_to(self.root_path)}")
                return True
            else:
                print(f"无需更新: {file_path.relative_to(self.root_path)}")
                return False
                
        except Exception as e:
            print(f"错误: 处理文件 {file_path} 时出错: {e}")
            return False

    def scan_and_process(self, target_path=None):
        """扫描并处理所有文件"""
        if target_path:
            scan_path = Path(target_path)
            if not scan_path.is_absolute():
                scan_path = self.root_path / scan_path
        else:
            scan_path = self.root_path
        
        if not scan_path.exists():
            print(f"错误: 路径 {scan_path} 不存在")
            return
        
        # 如果指定的是单个文件
        if scan_path.is_file():
            print(f"处理单个文件: {scan_path}")
            print(f"支持的文件类型: {', '.join(self.supported_extensions)}")
            print("-" * 60)
            
            if not self.should_skip_file(scan_path):
                updated = self.process_file(scan_path)
                print("-" * 60)
                print(f"处理完成! 文件{'已更新' if updated else '无需更新'}")
            else:
                print(f"跳过文件: {scan_path} (不支持的文件类型或在排除目录中)")
            return
        
        print(f"开始扫描目录: {scan_path}")
        print(f"支持的文件类型: {', '.join(self.supported_extensions)}")
        print(f"排除的目录: {', '.join(self.excluded_dirs)}")
        print("-" * 60)
        
        processed_count = 0
        updated_count = 0
        
        # 递归扫描所有文件
        for file_path in scan_path.rglob('*'):
            if file_path.is_file() and not self.should_skip_file(file_path):
                processed_count += 1
                if self.process_file(file_path):
                    updated_count += 1
        
        print("-" * 60)
        print(f"扫描完成!")
        print(f"处理文件数: {processed_count}")
        print(f"更新文件数: {updated_count}")


def main():
    parser = argparse.ArgumentParser(
        description="统一仓库下的开源许可证为SPDX格式",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例用法:
  python unify_license.py                    # 处理当前目录
  python unify_license.py drivers/          # 处理drivers目录
  python unify_license.py /path/to/project  # 处理指定路径
        """
    )
    
    parser.add_argument(
        'path',
        nargs='?',
        default=None,
        help='要处理的目录路径 (默认为当前目录)'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='仅显示将要修改的文件，不实际修改'
    )
    
    args = parser.parse_args()
    
    # 获取脚本所在目录的上两级目录作为项目根目录
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    
    # 创建统一器实例
    unifier = LicenseUnifier(project_root, dry_run=args.dry_run)
    
    if args.dry_run:
        print("*** DRY RUN 模式 - 不会实际修改文件 ***")
    
    # 处理文件
    unifier.scan_and_process(args.path)


if __name__ == '__main__':
    main()
