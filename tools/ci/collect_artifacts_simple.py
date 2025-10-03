#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import shutil
import time
from pathlib import Path
import glob


class SimpleArtifactsCollector:
    def __init__(self):
        self.merged_dir = Path('merged_artifacts')
        
        # 确保根目录存在
        self.merged_dir.mkdir(parents=True, exist_ok=True)
    
    def collect_local_artifacts(self):
        collected_jobs = {}
        
        # 收集日志文件并映射到对应的job
        log_pattern = "ci_build_logs/*.log"
        log_files = glob.glob(log_pattern, recursive=True)
        
        print(f"📄 发现 {len(log_files)} 个日志文件")
        for log_file in log_files:
            log_path = Path(log_file)
            job_name = log_path.stem  # 去掉.log后缀
            
            # 为每个job创建目录
            job_dir = self.merged_dir / job_name
            job_dir.mkdir(exist_ok=True)
            
            # 创建ci_build_logs子目录
            logs_subdir = job_dir / 'ci_build_logs'
            logs_subdir.mkdir(exist_ok=True)
            
            # 复制日志文件
            dest_file = logs_subdir / log_path.name
            shutil.copy2(log_file, dest_file)
            print(f"  📄 复制: {job_name}/ci_build_logs/{log_path.name}")
            
            if job_name not in collected_jobs:
                collected_jobs[job_name] = {'logs': 0, 'artifacts': 0}
            collected_jobs[job_name]['logs'] += 1
        
        # 收集artifacts目录中的文件
        artifacts_base = Path('artifacts')
        if artifacts_base.exists():
            for job_artifacts_dir in artifacts_base.iterdir():
                if job_artifacts_dir.is_dir():
                    job_name = job_artifacts_dir.name
                    
                    # 为每个job创建目录
                    job_dir = self.merged_dir / job_name
                    job_dir.mkdir(exist_ok=True)
                    
                    # 创建artifacts子目录
                    artifacts_subdir = job_dir / 'artifacts'
                    artifacts_subdir.mkdir(exist_ok=True)
                    
                    # 复制整个artifacts子目录内容
                    dest_job_artifacts = artifacts_subdir / job_name
                    if dest_job_artifacts.exists():
                        shutil.rmtree(dest_job_artifacts)
                    shutil.copytree(job_artifacts_dir, dest_job_artifacts)
                    
                    # 统计文件数量
                    file_count = len(list(dest_job_artifacts.rglob('*')))
                    print(f"  📦 复制: {job_name}/artifacts/{job_name}/ ({file_count} 个文件)")
                    
                    if job_name not in collected_jobs:
                        collected_jobs[job_name] = {'logs': 0, 'artifacts': 0}
                    collected_jobs[job_name]['artifacts'] = file_count
        
        return collected_jobs
    
    def create_summary_report(self, collected_jobs):
        report_path = self.merged_dir / 'build_summary.txt'
        
        total_logs = sum(job['logs'] for job in collected_jobs.values())
        total_artifacts = sum(job['artifacts'] for job in collected_jobs.values())
        
        with open(report_path, 'w', encoding='utf-8') as f:
            f.write("SDK构建汇总报告\n")
            f.write("=" * 50 + "\n")
            f.write(f"Pipeline ID: {os.getenv('CI_PIPELINE_ID', 'N/A')}\n")
            f.write(f"项目ID: {os.getenv('CI_PROJECT_ID', 'N/A')}\n")
            f.write(f"构建时间: {time.strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            
            f.write(f"汇总统计:\n")
            f.write(f"  - 构建任务数: {len(collected_jobs)} 个\n")
            f.write(f"  - 构建日志: {total_logs} 个\n")
            f.write(f"  - 构建产物: {total_artifacts} 个\n\n")
            
            f.write("各Job详细统计:\n")
            f.write("-" * 50 + "\n")
            for job_name, stats in sorted(collected_jobs.items()):
                f.write(f"{job_name}:\n")
                f.write(f"  - 日志文件: {stats['logs']} 个\n")
                f.write(f"  - 构建产物: {stats['artifacts']} 个\n")
                
                # 列出具体文件
                job_dir = self.merged_dir / job_name
                if job_dir.exists():
                    log_files = list((job_dir / 'ci_build_logs').glob('*.log')) if (job_dir / 'ci_build_logs').exists() else []
                    if log_files:
                        f.write(f"  日志文件列表:\n")
                        for log_file in sorted(log_files):
                            size = log_file.stat().st_size
                            f.write(f"    - {log_file.name} ({size:,} bytes)\n")
                    
                    artifacts_dir = job_dir / 'artifacts' / job_name
                    if artifacts_dir.exists():
                        artifact_files = list(artifacts_dir.rglob('*'))
                        artifact_files = [f for f in artifact_files if f.is_file()]
                        if artifact_files:
                            f.write(f"  构建产物列表:\n")
                            for artifact_file in sorted(artifact_files):
                                size = artifact_file.stat().st_size
                                rel_path = artifact_file.relative_to(artifacts_dir)
                                f.write(f"    - {rel_path} ({size:,} bytes)\n")
                f.write("\n")
            
            # 计算总大小
            total_size = sum(f.stat().st_size for f in self.merged_dir.rglob('*') if f.is_file())
            f.write(f"总大小: {total_size:,} bytes ({total_size / (1024*1024):.2f} MB)\n")
        
        print(f"📄 汇总报告已生成: {report_path}")
    
    def create_file_index(self, collected_jobs):
        index_path = self.merged_dir / 'file_index.json'
        
        import json
        
        index_data = {
            "generated_at": time.strftime('%Y-%m-%d %H:%M:%S'),
            "pipeline_id": os.getenv('CI_PIPELINE_ID', 'N/A'),
            "project_id": os.getenv('CI_PROJECT_ID', 'N/A'),
            "jobs": {}
        }
        
        # 为每个job创建索引
        for job_name in collected_jobs.keys():
            job_dir = self.merged_dir / job_name
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
            artifacts_dir = job_dir / 'artifacts' / job_name
            if artifacts_dir.exists():
                for artifact_file in sorted(artifacts_dir.rglob('*')):
                    if artifact_file.is_file():
                        rel_path = artifact_file.relative_to(artifacts_dir)
                        job_index["artifacts"].append({
                            "name": artifact_file.name,
                            "size": artifact_file.stat().st_size,
                            "path": f"{job_name}/artifacts/{job_name}/{rel_path}",
                            "type": artifact_file.suffix.lstrip('.')
                        })
            
            index_data["jobs"][job_name] = job_index
        
        with open(index_path, 'w', encoding='utf-8') as f:
            json.dump(index_data, f, indent=2, ensure_ascii=False)
        
        print(f"📋 文件索引已生成: {index_path}")
    
    def collect_artifacts(self):
        # 显示当前目录结构（用于调试）
        print("📁 当前目录结构:")
        for item in sorted(Path('.').rglob('*')):
            if item.is_file() and any(str(item).startswith(prefix) for prefix in ['ci_build_logs', 'artifacts']):
                print(f"  {item}")
        
        # 收集本地artifacts并保持原始目录结构
        collected_jobs = self.collect_local_artifacts()
        
        # 创建报告和索引
        self.create_summary_report(collected_jobs)
        self.create_file_index(collected_jobs)
        
        total_jobs = len(collected_jobs)
        total_logs = sum(job['logs'] for job in collected_jobs.values())
        total_artifacts = sum(job['artifacts'] for job in collected_jobs.values())
        
        print(f"\n✅ artifacts收集完成!")
        print(f"📊 统计: {total_jobs} 个job, {total_logs} 个日志文件, {total_artifacts} 个构建产物")
        
        # 显示合并后的目录结构
        print(f"\n📁 合并后的artifacts结构:")
        for job_dir in sorted(self.merged_dir.iterdir()):
            if job_dir.is_dir() and job_dir.name not in ['build_summary.txt', 'file_index.json']:
                print(f"  📂 {job_dir.name}/")
                
                # 显示ci_build_logs目录
                logs_dir = job_dir / 'ci_build_logs'
                if logs_dir.exists():
                    log_files = list(logs_dir.glob('*.log'))
                    if log_files:
                        print(f"    📂 ci_build_logs/ ({len(log_files)} 个日志文件)")
                
                # 显示artifacts目录
                artifacts_dir = job_dir / 'artifacts'
                if artifacts_dir.exists():
                    artifact_files = list(artifacts_dir.rglob('*'))
                    artifact_files = [f for f in artifact_files if f.is_file()]
                    if artifact_files:
                        print(f"    📂 artifacts/ ({len(artifact_files)} 个构建产物)")
        
        # 显示汇总文件
        for summary_file in ['build_summary.txt', 'file_index.json']:
            summary_path = self.merged_dir / summary_file
            if summary_path.exists():
                size = summary_path.stat().st_size
                print(f"  📄 {summary_file} ({size:,} bytes)")


def main():
    try:
        collector = SimpleArtifactsCollector()
        collector.collect_artifacts()
        print("\n🎉 Artifacts收集成功完成！")
    except Exception as e:
        print(f"❌ 收集artifacts失败: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
