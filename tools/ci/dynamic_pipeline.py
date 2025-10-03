#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import sys
import yaml
import json
import subprocess
import tempfile
from pathlib import Path

class DynamicPipelineGenerator:
    def __init__(self, config_file='build_config_gcc.yaml'):
        self.config_file = config_file
        self.config = self._load_config()
        self.pipeline_config = {
            'stages': ['build', 'collect'],
            'variables': {
                'GIT_SUBMODULE_STRATEGY': 'recursive',
                'COMPILER': 'gcc',
                'PARALLEL_JOBS': '8'
            }
        }
    
    def _load_config(self):
        try:
            with open(self.config_file, 'r', encoding='utf-8') as f:
                return yaml.safe_load(f)
        except Exception as e:
            print(f"❌ 加载配置文件失败: {e}")
            sys.exit(1)
    
    def _create_build_job(self, project_path, board=None, is_common=False):
        """创建单个构建Job配置"""
        # 生成Job名称
        job_name = self._generate_job_name(project_path, board)
        
        # 生成日志文件名
        log_name = job_name.replace('build_', '')
        
        # 构建参数
        # 处理board参数：scons命令需要去掉_hcpu后缀
        scons_board = board.replace('_hcpu', '') if board and '_hcpu' in board else board
        build_args = f"--board={scons_board}" if (is_common and scons_board) else ""
        project_type = "common" if is_common else "normal"
        
        # 简化的Job配置
        job_config = {
            'stage': 'build',
            'tags': ['build'],
            'variables': {
                'PROJECT_PATH': project_path,
                'BOARD': board or "",  # 保留原始board名称用于目录名
                'SCONS_BOARD': scons_board or "",  # scons命令使用的board名称
                'BUILD_ARGS': build_args,
                'LOG_NAME': log_name,
                'PROJECT_TYPE': project_type
            },
            'script': [
                'source ./export.sh',
                'bash tools/ci/build_job.sh'
            ],
            'artifacts': {
                'paths': [f'ci_build_logs/{log_name}.log', f'artifacts/{log_name}/'],
                'expire_in': '1 day',
                'when': 'always'
            },
            'rules': [
                {'if': '$CI_PIPELINE_SOURCE == "parent_pipeline"', 'when': 'always'},
                {'when': 'manual', 'allow_failure': True}
            ]
        }
        return job_name, job_config
    
    def _create_collect_job(self, build_job_names):
        """创建收集artifacts的Job - 使用分层策略避免needs限制"""
        MAX_NEEDS_PER_JOB = 50  # GitLab CI的needs限制
        
        collect_jobs = {}
        
        if len(build_job_names) <= MAX_NEEDS_PER_JOB:
            # 如果job数量不超过限制，直接创建单个收集job
            collect_jobs['collect_all_artifacts'] = {
                'stage': 'collect',
                'tags': ['build'],
                'script': [
                    'echo "🔍 开始收集所有构建artifacts..."',
                    'python3 tools/ci/collect_artifacts_simple.py',
                ],
                'artifacts': {
                    'paths': ['merged_artifacts/'],
                    'expire_in': '1 week',
                    'when': 'always'
                },
                'needs': [{'job': job_name, 'artifacts': True} for job_name in build_job_names],
                'rules': [
                    {'if': '$CI_PIPELINE_SOURCE == "parent_pipeline"', 'when': 'always'}
                ]
            }
        else:
            # 如果job数量超过限制，使用分层收集策略
            print(f"⚠️  构建job数量({len(build_job_names)})超过GitLab CI needs限制({MAX_NEEDS_PER_JOB})，使用分层收集策略")
            
            # 将构建job分组
            job_groups = []
            for i in range(0, len(build_job_names), MAX_NEEDS_PER_JOB):
                group = build_job_names[i:i + MAX_NEEDS_PER_JOB]
                job_groups.append(group)
            
            intermediate_jobs = []
            
            # 为每组创建中间收集job
            for group_idx, job_group in enumerate(job_groups):
                intermediate_job_name = f'collect_group_{group_idx + 1}'
                intermediate_jobs.append(intermediate_job_name)
                
                collect_jobs[intermediate_job_name] = {
                    'stage': 'collect',
                    'tags': ['build'],
                    'script': [
                        f'echo "🔍 收集第{group_idx + 1}组artifacts (共{len(job_groups)}组)..."',
                        f'mkdir -p group_{group_idx + 1}_artifacts',
                        'python3 tools/ci/collect_artifacts_simple.py',
                        f'mv merged_artifacts group_{group_idx + 1}_artifacts/',
                        f'echo "✅ 第{group_idx + 1}组收集完成"'
                    ],
                    'artifacts': {
                        'paths': [f'group_{group_idx + 1}_artifacts/'],
                        'expire_in': '1 day',
                        'when': 'always'
                    },
                    'needs': [{'job': job_name, 'artifacts': True} for job_name in job_group],
                    'rules': [
                        {'if': '$CI_PIPELINE_SOURCE == "parent_pipeline"', 'when': 'always'}
                    ]
                }
            
            # 创建最终合并job
            collect_jobs['merge_all_artifacts'] = {
                'stage': 'collect',
                'tags': ['build'],
                'script': [
                    'echo "🔄 合并所有组的artifacts..."',
                    'mkdir -p final_merged_artifacts',
                    'python3 tools/ci/merge_group_artifacts.py',
                ],
                'artifacts': {
                    'paths': [
                        'final_merged_artifacts/'
                    ],
                    'expire_in': '1 week',
                    'when': 'always'
                },
                'needs': [{'job': job_name, 'artifacts': True} for job_name in intermediate_jobs],
                'rules': [
                    {'if': '$CI_PIPELINE_SOURCE == "parent_pipeline"', 'when': 'always'}
                ]
            }
        
        return collect_jobs
    
    def _generate_job_name(self, project_path, board=None):
        """生成Job名称"""
        name = project_path.replace('/', '_').replace('-', '_').replace('.', '_')
        if board:
            name += f"_{board.replace('-', '_')}"
        return f"build_{name}"
    
    def generate_child_pipeline(self):
        """生成子Pipeline配置"""
        print("🚀 动态生成构建Pipeline...")
        
        jobs = {}
        job_count = 0
        
        # 处理普通项目
        if 'projects' in self.config:
            for project_group in self.config['projects']:
                for project_path in project_group['path']:
                    job_name, job_config = self._create_build_job(project_path, is_common=False)
                    jobs[job_name] = job_config
                    job_count += 1
        
        # 处理通用项目
        if 'common_projects' in self.config:
            for common_project in self.config['common_projects']:
                project_path = common_project['path']
                for board in common_project['boards']:
                    job_name, job_config = self._create_build_job(project_path, board, is_common=True)
                    jobs[job_name] = job_config
                    job_count += 1
        
        # 合并配置
        self.pipeline_config.update(jobs)
        
        # 添加收集artifacts的job
        if job_count > 0:
            collect_jobs = self._create_collect_job(list(jobs.keys()))
            self.pipeline_config.update(collect_jobs)
            collect_job_count = len(collect_jobs)
        else:
            collect_job_count = 0
        
        print(f"📊 动态生成了 {job_count} 个构建Job + {collect_job_count}个收集Job")
        return self.pipeline_config
    
    def save_child_pipeline(self, output_file='child-pipeline.yml'):
        """保存子Pipeline配置"""
        pipeline = self.generate_child_pipeline()
        
        with open(output_file, 'w', encoding='utf-8') as f:
            yaml.dump(pipeline, f, default_flow_style=False, allow_unicode=True, indent=2)
        
        print(f"✅ 子Pipeline配置已保存: {output_file}")
        return output_file
    
    def trigger_child_pipeline(self):
        """触发子Pipeline"""
        # 生成临时配置文件
        with tempfile.NamedTemporaryFile(mode='w', suffix='.yml', delete=False) as f:
            pipeline = self.generate_child_pipeline()
            yaml.dump(pipeline, f, default_flow_style=False, allow_unicode=True, indent=2)
            temp_file = f.name
        
        print(f"📄 临时Pipeline配置: {temp_file}")
        
        # 使用GitLab CI API触发子Pipeline
        if 'CI_PROJECT_ID' in os.environ and 'CI_PIPELINE_ID' in os.environ:
            self._trigger_via_api(temp_file)
        else:
            print("💡 在本地环境中，将配置文件复制到 child-pipeline.yml")
            import shutil
            shutil.copy2(temp_file, 'child-pipeline.yml')
        
        # 清理临时文件
        os.unlink(temp_file)


def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description='动态Pipeline生成器')
    parser.add_argument('-c', '--config', default='build_config_gcc.yaml', help='配置文件路径')
    parser.add_argument('-o', '--output', default='child-pipeline.yml', help='输出文件路径')
    parser.add_argument('--trigger', action='store_true', help='直接触发子Pipeline')
    parser.add_argument('--dry-run', action='store_true', help='仅显示生成的Job数量')
    
    args = parser.parse_args()
    
    try:
        generator = DynamicPipelineGenerator(args.config)
        
        if args.dry_run:
            pipeline = generator.generate_child_pipeline()
            job_count = len([k for k in pipeline.keys() if k.startswith('build_')])
            print(f"📊 将生成 {job_count} 个构建Job")
            
        elif args.trigger:
            generator.trigger_child_pipeline()
            
        else:
            generator.save_child_pipeline(args.output)
            
    except Exception as e:
        print(f"❌ 执行失败: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
