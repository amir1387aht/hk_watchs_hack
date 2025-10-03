# -*- coding: utf-8 -*-
from __future__ import print_function
import argparse
import datetime
import os
import subprocess
import sys
import yaml
import shutil
import codecs
import tempfile


class SDKBuilder(object):
    def __init__(self, config_file, compiler, target):
        self.config = self._load_config(config_file)
        self.compiler = compiler
        self.root_dir = os.getcwd()
        self.target = target
        self.failed_projects = []
        self.build_log_dir = os.path.join(self.root_dir, self.config['common']['log_dir'])
        if not os.path.exists(self.build_log_dir):
            os.makedirs(self.build_log_dir)

    def _load_config(self, config_file):
        with codecs.open(config_file, 'r', encoding='utf-8') as f:
            return yaml.safe_load(f)

    def copy_directory_with_structure(self, common_path, destination_path):
        # relative_path = os.path.relpath(common_path, start=".")
        # full_destination_path = os.path.join(destination_path, relative_path)
        full_destination_path = os.path.join(destination_path, common_path)

        # destination_path = stand-stlone
        # common_path = example/uart/project/common
        print("full_destination_path:{}".format(full_destination_path))
        # stand-stlone\example/uart/project/common


        try:
            shutil.copytree(common_path, full_destination_path)
        except WindowsError:
                for item in os.listdir(common_path):
                    src_path1 = os.path.join(common_path, item)
                    dest_path1 = os.path.join(full_destination_path, item)
                    
                    
                    if os.path.isfile(src_path1):
                        if os.path.exists(dest_path1):
                            os.remove (dest_path1)
                            print("Deleted existing:{}".format(dest_path1))
                            try:
                                shutil.copy2(src_path1, dest_path1) 
                                print("src_path1 to dest_path1:{}".format(src_path1))
                            except shutil.Error as e:
                                print("Replication failure:{}".format(e))

        #find src
        src_path = None
        parent_directory = os.path.dirname(common_path)

        same_level_src = os.path.join(parent_directory, 'src')
        if os.path.exists(same_level_src):
            src_path = same_level_src
        else:
            one_level_up_src = os.path.dirname(parent_directory)
            if os.path.exists(os.path.join(one_level_up_src, 'src')):
                src_path = os.path.join(one_level_up_src, 'src')
   
            else:
                two_levels_up_src = os.path.dirname(one_level_up_src)
                if os.path.exists(os.path.join(two_levels_up_src, 'src')):
                    src_path = os.path.join(two_levels_up_src, 'src')


        if src_path:
            # relative_src_path = os.path.relpath(src_path, start=".")
            # full_src_destination_path = os.path.join(destination_path, relative_src_path)
            full_src_destination_path = os.path.join(destination_path, src_path)
            try:
                shutil.copytree(src_path, full_src_destination_path)
            except WindowsError:
                for i in os.listdir(src_path):
                    src_path2 = os.path.join(src_path, i)
                    dest_path2 = os.path.join(full_src_destination_path, i)   

                    if os.path.isfile(src_path2):
                        if os.path.exists(dest_path2):
                            os.remove (dest_path2)
                            print("Deleted existing2:{}".format(dest_path2))  
                            try:
                                shutil.copy2(src_path2, dest_path2)  
                                print("src_path2 to dest_path2:{}".format(src_path2))
                            except shutil.Error as e:
                                print("Replication failure:{}".format(e))

        else:                   
            print("not find src dictory")

    def copy_to_target(self):
        build_config = self.config
        target = self.target
        target = r"{}".format(target)
        if not os.path.exists(target):
            os.makedirs(target)
        if 'common_projects' in build_config:
            for item in build_config['common_projects']:
                if 'path' in item:
                    src_path = item['path']
                    self.copy_directory_with_structure(src_path, target)

    def _create_build_batch(self, commands, work_dir=None, log_file=None):
        fd, path = tempfile.mkstemp(suffix='.bat')
        try:
            with os.fdopen(fd, 'w') as f:
                f.write('@echo off\n')
                if log_file:
                    log_dir = os.path.dirname(log_file)
                    if log_dir:
                        f.write('if not exist "{0}" mkdir "{0}"\n'.format(log_dir))

                f.write('call "{0}" {1}\n'.format(os.path.join(self.root_dir, "set_env.bat"), self.compiler))

                if work_dir:
                    f.write('cd /d "{0}"\n'.format(work_dir))

                for cmd in commands:
                    if log_file:
                        f.write('echo [%date% %time%] Executing: {0} >> "{1}"\n'.format(cmd, log_file))
                        f.write('{0} >> "{1}" 2>&1\n'.format(cmd, log_file))
                        f.write('set BUILD_ERROR=%ERRORLEVEL%\n')
                        f.write('if %BUILD_ERROR% neq 0 (\n')
                        f.write(
                            '    echo [%date% %time%] Command failed with error code %BUILD_ERROR% >> "{0}"\n'.format(
                                log_file))
                        f.write('    exit /b %BUILD_ERROR%\n')
                        f.write(')\n')
                    else:
                        f.write('{0}\n'.format(cmd))
            return path
        except:
            os.remove(path)
            raise

    def _run_commands(self, commands, work_dir=None, log_file=None):
        batch_file = self._create_build_batch(commands, work_dir, log_file)
        try:
            process = subprocess.Popen(
                'cmd.exe /c "{0}"'.format(batch_file),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                shell=True
            )
            stdout, stderr = process.communicate()

            if log_file and os.path.exists(log_file):
                with open(log_file, 'a') as f:
                    f.write('\n=== Additional Build Output ===\n')
                    if stdout:
                        f.write('=== STDOUT ===\n{0}\n'.format(stdout))
                    if stderr:
                        f.write('=== STDERR ===\n{0}\n'.format(stderr))

            return process.returncode, stdout, stderr
        finally:
            try:
                os.remove(batch_file)
            except:
                pass

    def build_project(self, project_path, board=None):
        try:
            cmd = 'scons --board={0} -j8'.format(board)
            # print("project_path: {0}".format(project_path))
            # timestamp = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
            project_path_normalized = project_path.replace('\\', '/')
            # print("yaml_Build_path_change: {0}".format(project_path_normalized))

            project_name = project_path_normalized.replace('/', '_').replace(':', '')
            if board:
                log_file = os.path.join(self.build_log_dir, '{0}_{1}.log'.format(
                    project_name, board))
            else:
                log_file = os.path.join(self.build_log_dir, '{0}.log'.format(
                    project_name))

            print("\nBuilding {0} {1}".format(project_path_normalized, "for " + board if board else ""))
            print("Build log will be saved to: {0}".format(project_name))

            returncode, stdout, stderr = self._run_commands(
                [cmd],
                work_dir=os.path.abspath(project_path_normalized),
                log_file=log_file
            )

            if returncode != 0:
                self.failed_projects.append((project_path, board if board else "", log_file))
                print("xxxxx Build failed for {0} {1} ".format(project_path, "(" + board + ")" if board else ""))
                print("xxxxx Error details can be found in: {0} ".format(project_name))
                return False

            print("Successfully built {0} {1}".format(project_path, "for " + board if board else ""))
            return True

        except Exception as e:
            print("Error building {0}: {1}".format(project_path, str(e)))
            self.failed_projects.append(
                (project_path, board if board else "", log_file if 'log_file' in locals() else None))
            return False

    def build_all(self):
        print("\n--------- Building common projects ---------")
        if "common_projects" in self.config:
            for common_project in self.config['common_projects']:
                for board in common_project['boards']:
                    self.build_project(
                        os.path.join(self.target, common_project['path']),
                        board=board,
                    )

    def generate_report(self):
        log_dir = os.path.join(self.config['common']['report_dir'])
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
        report_path = os.path.join(
            self.config['common']['report_dir'],
            'build_report.txt'
        )
        with codecs.open(report_path, 'w', encoding='utf-8') as f:
            f.write(u"SDK Build Report\n")
            f.write(u"=" * 50 + u"\n")
            f.write(u"Build Time: {0}\n\n".format(datetime.datetime.now()))
            f.write(u"=" * 50 + u"\n")
            if self.failed_projects:
                f.write(u"Failed Projects:\n")
                for project, board, log_file in self.failed_projects:
                    f.write(u"- {0} {1}\n".format(project, "(" + board + ")" if board else ""))
                    # if log_file:
                    #     f.write(u"  Log file: {0}\n".format(log_file))
                f.write(u"=" * 50 + u"\n\n")
            else:
                f.write(u"All projects built successfully!\n")

            # f.write(u"\nBuild logs directory: {0}\n".format(self.build_log_dir))

            f.write(u"All build projects:\n\n")

            f.write(u"\n---------  common projects ---------\n")
            if "common_projects" in self.config:
                for common_project in self.config['common_projects']:
                    f.write(u"\n{0}\n".format(common_project['path']))
                    for board in common_project['boards']:
                        f.write(u"- {0}\n".format(board))
            f.write(u"\n")
            f.write(u"=" * 50 + u"\n")
        print("Build report generated at {0}".format(report_path))

    def backup_failed_logs(self):
        if not self.failed_projects:
            return

        backup_dir = os.path.join(self.root_dir, self.config['common']['report_dir'])
        if not os.path.exists(backup_dir):
            os.makedirs(backup_dir)

        for project, board, log_file in self.failed_projects:
            if log_file and os.path.exists(log_file):
                backup_file = os.path.join(backup_dir, os.path.basename(log_file))
                shutil.copy2(log_file, backup_file)


def main():
    parser = argparse.ArgumentParser(description='SDK Builder')
    parser.add_argument('-c', '--config', default='build_config.yaml',
                        help='Path to the build configuration file')
    parser.add_argument('-p', '--compiler', default='keil',
                        help='Compiler')
    parser.add_argument('-t', '--target', default='stand-alone', help='Target directory')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Enable verbose output')
    args = parser.parse_args()

    try:
        builder = SDKBuilder(args.config, args.compiler, args.target)
        builder.copy_to_target()
        builder.build_all()
        builder.generate_report()
        builder.backup_failed_logs()

        if builder.failed_projects:
            print("\n--------- Build completed with failures. See report for details. ---------")
            sys.exit(1)
        else:
            print("\n--------- Build completed successfully! ---------")
            sys.exit(0)

    except Exception as e:
        print("Error during build process: {0}".format(str(e)))
        sys.exit(1)


if __name__ == '__main__':
    main()
