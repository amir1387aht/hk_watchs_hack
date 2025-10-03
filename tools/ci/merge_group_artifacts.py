#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
合并分组artifacts的脚本
用于将多个中间收集job的artifacts合并为最终结果
"""

import os
import sys
import shutil
import time
import json
from pathlib import Path
import glob


class GroupArtifactsMerger:
    def __init__(self):
        self.final_dir = Path('final_merged_artifacts')
        self.final_dir.mkdir(exist_ok=True)
    
    def merge_group_artifacts(self):
        """合并所有组的artifacts"""
        print("🔄 开始合并分组artifacts...")
        
        # 查找所有组artifacts目录
        group_dirs = glob.glob('group_*_artifacts')
        group_dirs.sort()
        
        if not group_dirs:
            print("❌ 没有找到任何组artifacts目录")
            return False
        
        print(f"📁 发现 {len(group_dirs)} 个组artifacts目录")
        
        merged_jobs = {}
        total_jobs = 0
        total_logs = 0
        total_artifacts = 0
        
        # 合并每个组的内容
        for group_dir in group_dirs:
            group_path = Path(group_dir)
            merged_artifacts_path = group_path / 'merged_artifacts'
            
            if not merged_artifacts_path.exists():
                print(f"⚠️  跳过 {group_dir}：没有merged_artifacts目录")
                continue
            
            print(f"📦 处理 {group_dir}...")
            
            # 复制所有job目录到最终目录
            for job_dir in merged_artifacts_path.iterdir():
                if job_dir.is_dir() and not job_dir.name.endswith('.txt') and not job_dir.name.endswith('.json'):
                    dest_job_dir = self.final_dir / job_dir.name
                    
                    if dest_job_dir.exists():
                        print(f"⚠️  目录 {job_dir.name} 已存在，跳过")
                        continue
                    
                    shutil.copytree(job_dir, dest_job_dir)
                    print(f"  📂 复制: {job_dir.name}")
                    
                    # 统计这个job的文件
                    job_stats = self._count_job_files(dest_job_dir)
                    merged_jobs[job_dir.name] = job_stats
                    total_jobs += 1
                    total_logs += job_stats['logs']
                    total_artifacts += job_stats['artifacts']
        
        # 创建最终的汇总报告
        self._create_final_summary(merged_jobs, total_jobs, total_logs, total_artifacts)
        
        # 创建最终的文件索引
        self._create_final_index(merged_jobs)
        
        print(f"✅ 合并完成: {total_jobs} 个job, {total_logs} 个日志, {total_artifacts} 个构建产物")
        return True
    
    def _count_job_files(self, job_dir):
        """统计单个job目录中的文件数量"""
        logs_count = 0
        artifacts_count = 0
        
        # 统计日志文件
        logs_dir = job_dir / 'ci_build_logs'
        if logs_dir.exists():
            logs_count = len(list(logs_dir.glob('*.log')))
        
        # 统计构建产物
        artifacts_dir = job_dir / 'artifacts'
        if artifacts_dir.exists():
            artifact_files = [f for f in artifacts_dir.rglob('*') if f.is_file()]
            artifacts_count = len(artifact_files)
        
        return {'logs': logs_count, 'artifacts': artifacts_count}
    
    def _create_final_summary(self, merged_jobs, total_jobs, total_logs, total_artifacts):
        """创建最终汇总报告"""
        print("📊 创建最终汇总报告...")
        
        report_path = self.final_dir / 'build_summary.txt'
        
        with open(report_path, 'w', encoding='utf-8') as f:
            f.write("SDK构建最终汇总报告\n")
            f.write("=" * 50 + "\n")
            f.write(f"Pipeline ID: {os.getenv('CI_PIPELINE_ID', 'N/A')}\n")
            f.write(f"项目ID: {os.getenv('CI_PROJECT_ID', 'N/A')}\n")
            f.write(f"合并时间: {time.strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            
            f.write(f"总体统计:\n")
            f.write(f"  - 构建任务数: {total_jobs} 个\n")
            f.write(f"  - 构建日志: {total_logs} 个\n")
            f.write(f"  - 构建产物: {total_artifacts} 个\n\n")
            
            f.write("各Job详细统计:\n")
            f.write("-" * 50 + "\n")
            
            for job_name, stats in sorted(merged_jobs.items()):
                f.write(f"{job_name}:\n")
                f.write(f"  - 日志文件: {stats['logs']} 个\n")
                f.write(f"  - 构建产物: {stats['artifacts']} 个\n")
                
                # 列出具体文件
                job_dir = self.final_dir / job_name
                if job_dir.exists():
                    logs_dir = job_dir / 'ci_build_logs'
                    if logs_dir.exists():
                        log_files = list(logs_dir.glob('*.log'))
                        if log_files:
                            f.write(f"  日志文件列表:\n")
                            for log_file in sorted(log_files):
                                size = log_file.stat().st_size
                                f.write(f"    - {log_file.name} ({size:,} bytes)\n")
                    
                    artifacts_dir = job_dir / 'artifacts'
                    if artifacts_dir.exists():
                        artifact_files = [f for f in artifacts_dir.rglob('*') if f.is_file()]
                        if artifact_files:
                            f.write(f"  构建产物列表:\n")
                            for artifact_file in sorted(artifact_files):
                                size = artifact_file.stat().st_size
                                rel_path = artifact_file.relative_to(artifacts_dir)
                                f.write(f"    - {rel_path} ({size:,} bytes)\n")
                f.write("\n")
            
            # 计算总大小
            total_size = sum(f.stat().st_size for f in self.final_dir.rglob('*') if f.is_file())
            f.write(f"总大小: {total_size:,} bytes ({total_size / (1024*1024):.2f} MB)\n")
        
        print(f"📄 最终汇总报告已生成: {report_path}")
    
    def _create_final_index(self, merged_jobs):
        """创建最终文件索引"""
        index_path = self.final_dir / 'file_index.json'
        
        index_data = {
            "generated_at": time.strftime('%Y-%m-%d %H:%M:%S'),
            "pipeline_id": os.getenv('CI_PIPELINE_ID', 'N/A'),
            "project_id": os.getenv('CI_PROJECT_ID', 'N/A'),
            "merge_strategy": "group_based",
            "total_jobs": len(merged_jobs),
            "jobs": {}
        }
        
        # 为每个job创建索引
        for job_name in merged_jobs.keys():
            job_dir = self.final_dir / job_name
            if not job_dir.exists():
                continue
                
            job_index = {
                "logs": [],
                "artifacts": []
            }
            
            # 索引日志文件
            logs_dir = job_dir / 'ci_build_logs'
            if logs_dir.exists():
                for log_file in sorted(logs_dir.glob('*.log')):
                    job_index["logs"].append({
                        "name": log_file.name,
                        "size": log_file.stat().st_size,
                        "path": f"{job_name}/ci_build_logs/{log_file.name}"
                    })
            
            # 索引构建产物
            artifacts_dir = job_dir / 'artifacts'
            if artifacts_dir.exists():
                for artifact_file in sorted(artifacts_dir.rglob('*')):
                    if artifact_file.is_file():
                        rel_path = artifact_file.relative_to(artifacts_dir)
                        job_index["artifacts"].append({
                            "name": artifact_file.name,
                            "size": artifact_file.stat().st_size,
                            "path": f"{job_name}/artifacts/{rel_path}",
                            "type": artifact_file.suffix.lstrip('.')
                        })
            
            index_data["jobs"][job_name] = job_index
        
        with open(index_path, 'w', encoding='utf-8') as f:
            json.dump(index_data, f, indent=2, ensure_ascii=False)
        
        print(f"📋 最终文件索引已生成: {index_path}")


def main():
    """主函数"""
    try:
        merger = GroupArtifactsMerger()
        success = merger.merge_group_artifacts()
        
        if success:
            print("\n🎉 分组artifacts合并成功完成！")
        else:
            print("\n❌ 分组artifacts合并失败")
            sys.exit(1)
            
    except Exception as e:
        print(f"❌ 合并分组artifacts失败: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
