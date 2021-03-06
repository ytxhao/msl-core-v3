#!/usr/bin/env python

import argparse
import os
import shutil
import logging
import subprocess
import sys
import time
import platform
import filecmp


SRC_DIR = SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_SRC_DIR = os.path.normpath(os.path.join(SCRIPT_DIR))
PROJECT_ROOT_DIR = os.path.normpath(os.path.join(PROJECT_SRC_DIR, os.pardir))

MSL_DIR = os.path.join(SRC_DIR, 'sdk', 'msl-core')
MSL_SRC_DIR = os.path.join(MSL_DIR, 'src')
OUTPUT_DIR = os.path.join(SRC_DIR, 'out')
ENABLED_ARCHS = ['arm64', 'arm', 'x64']
DEFAULT_ARCHS = ['arm64', 'arm']
MAC_SDK_BUILD_SCRIPT = SRC_DIR + "/tools_msl/mac/build_mac_libs.sh"
IOS_SDK_BUILD_SCRIPT = SRC_DIR + "/tools_msl/ios/build_ios_libs.sh"
ANDROID_SDK_BUILD_SCRIPT = SRC_DIR + "/tools_msl/android/build_aar.sh"
LINUX_SDK_BUILD_SCRIPT = SRC_DIR + "/tools_msl/linux/build_linux_libs.sh"

class GitRepo:
    def __init__(self, name, direcotry):
        self.name = name
        self.dir = direcotry

    def info(self):
        branch_list = subprocess.check_output(["git", "branch"]).split('\n')
        for branch in branch_list:
            if branch[0] == "*":
                curr_branch = branch.lstrip("* ")
                break
        print_title(self.name + " @ " + curr_branch)

    def is_git_repo(self):
        git_dir = os.path.normpath(os.path.join(self.dir, '.git'))
        if os.path.exists(git_dir):
            return True
        else:
            print("\033[31m" + self.dir + " is NOT a git root!\033[0m")
            return False


def print_title(title):
    title = " " + title + " "
    print("\033[32m" + "{0:=^90}".format(title) + "\033[0m")


def clean_artifacts(output_dir):
    if os.path.isdir(output_dir):
        print('Deleting ' + output_dir)
        shutil.rmtree(output_dir)

def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '-o',
        '--os',
        choices=['a', 'i', 'l', 'm'],
        help='The os config can be IGNORED, and decided by os info, or can be "a (Android)", '
             '"i (iOS)", "l (linux)",  "m (Mac)". Defaults to "a (Android)".')
    parser.add_argument(
        '-c',
        '--clean',
        action='store_true',
        default=False,
        help='Removes the previously generated build output, if any.')
    parser.add_argument(
        '--arch',
        nargs='+',
        default=DEFAULT_ARCHS,
        choices=ENABLED_ARCHS,
        help='Architectures to build. Defaults to %(default)s. iOS suppports x64; '
             'Linux only supports x64, will ignore this argument.')
    parser.add_argument(
        '--build',
        default='d',
        choices=['d', 'r'],
        help='The build config. Can be "d (debug)" or "r (release)". '
             'Defaults to "d (debug)".')
    parser.add_argument(
        '--symbol-level', type=str,
        help='Symbol level: 0, 1, 2.')
    parser.add_argument('--verbose',
                    action='store_true',
                    default=False,
                    help='Debug logging.')
    return parser.parse_args()

def chmod(directory):
    cmd = "chmod 777 " + directory
    subprocess.call(cmd, shell=True)

def get_build_args(args):
    build_arg_list = []
    if args.os == "a":    # Android
        # build_arg_list.append('clang_use_chrome_plugins=false')
        build_arg_list.append('is_debug=false' if args.build == 'r' else 'is_debug=true')
    elif args.os == "i":  # iOS
        build_arg_list.append('rtc_use_metal_rendering=false')
        if args.symbol_level:
            build_arg_list.append('symbol_level=' + args.symbol_level)
        else:
            build_arg_list.append('symbol_level=0' if args.build == 'r' else 'symbol_level=2')
    elif args.os == "l":  # Linux
        build_arg_list.append('is_debug=false' if args.build == 'r' else 'is_debug=true')

    build_args = " ".join(i for i in build_arg_list)
    return build_args

def gen_version_header(output_dir):
    repos = [GitRepo("msl_sdk", PROJECT_ROOT_DIR)]
    version_out = None
    for repo in repos:
        if os.path.exists(repo.dir):
            if repo.is_git_repo():
                os.chdir(repo.dir)
                repo.info()
                dev_null = open(os.devnull, 'w')
                out = subprocess.call("git describe HEAD", shell=True, stdout=dev_null, stderr=dev_null)
                if out == 0:
                    out = subprocess.check_output("git describe HEAD", shell=True)
                    if version_out:
                        version_out = version_out + "-" + out.rstrip()
                    else:
                        version_out = out.rstrip()
                else:
                    branch = subprocess.check_output("git rev-parse --abbrev-ref HEAD", shell=True)
                    branch = branch.rstrip()
                    version = subprocess.check_output("git rev-parse --short HEAD", shell=True)
                    version = version.rstrip()
                    out = branch + '-' + version
                    if version_out:
                        version_out = version_out + "-" + out
                    else:
                        version_out = out
        else:
            print(repo.dir + "NOT exist.")

    os.chdir(output_dir)
    version_file_content = '#ifndef MSL_VERSION_H_\n#define MSL_VERSION_H_\n\n#define MSL_VERSION "'\
                           + version_out + '"\n\n#endif'
    tmp_version_file = "tmp_version.h"
    version_file = "version.h"
    fp = open(tmp_version_file, 'w')
    fp.write(version_file_content)
    fp.close() 
    if os.path.exists(version_file):
        if not filecmp.cmp(tmp_version_file, version_file, False):
            os.remove(version_file)
            os.rename(tmp_version_file, version_file)
        else:
            os.remove(tmp_version_file)
            # print("Version header already up-to-update: " + version_out)
    else:
        os.rename(tmp_version_file, version_file)

    print("\033[32mVersion: \033[0m" + version_out)

def main():
    
    print("\033[32m SRC_DIR: \033[0m" + SRC_DIR)
    print("\033[32m PROJECT_SRC_DIR: \033[0m" + PROJECT_SRC_DIR)
    print("\033[32m PROJECT_ROOT_DIR: \033[0m" + PROJECT_ROOT_DIR)
    print("\033[32m MSL_DIR: \033[0m" + MSL_DIR)
    print("\033[32m OUTPUT_DIR: \033[0m" + OUTPUT_DIR)
    start_sec = time.time()
    args = parse_args()
    print(args)

    logging.basicConfig(level=logging.DEBUG if args.verbose else logging.INFO)
    level1=logging.DEBUG if args.verbose else logging.INFO
    print(level1)
    if args.clean:
        clean_artifacts(OUTPUT_DIR)
    
    gen_version_header(MSL_SRC_DIR)

    if args.build == "r":
        build_config = "release"
    else:
        build_config = "debug"
    build_dir = os.path.join(OUTPUT_DIR, build_config)
    logging.info('args.os:%s',args.os)
    if not args.os:
        if platform.system() == "Darwin":
            args.os = "i"
        elif platform.system() == "Linux":
            args.os = "a"
    elif args.os == 'l':
        if platform.system() != "Linux":
            print("\033[31mCompiled in a wrong system.\n\033[0m")
            return
    elif args.os == 'm':
        if platform.system() != "Darwin":
            print("\033[31mCompiled in a wrong system.\n\033[0m")
            return
    logging.info('args.os:%s',args.os)
    build_args = get_build_args(args)
    if args.os == "a":
        archs = list(args.arch)
        architectures = ""
        for arch in archs:
            if arch == "arm":
                arch = "armeabi-v7a"
            if arch == "arm64":
                arch = "arm64-v8a"
            architectures = architectures + " " + arch
        
        out_sdk = os.path.join(build_dir, "ms-core.aar")
        logging.info('architectures:%s out_sdk:%s', architectures, out_sdk)
        cmd = "{0} --output {1} --build-dir {2} --extra-gn-args {3} --arch {4}".format(
            ANDROID_SDK_BUILD_SCRIPT,
            out_sdk,
            build_dir,
            build_args,
            architectures)
        logging.info('msl build cmd:%s', cmd)
        subprocess.call(cmd, shell=True)

        # chmod(OUTPUT_DIR)
        # chmod(build_dir)
        arch_str = architectures.split()
        for arch in arch_str:
            arch_dir = os.path.join(build_dir, arch)
            # chmod(arch_dir)
            dir_list = os.listdir(arch_dir)
            for i in range(0, len(dir_list)):
                path = os.path.join(arch_dir, dir_list[i])
                if os.path.isdir(path):
                    print("path:" + path)
                    # chmod(path)

    if args.os == "i":
        architectures = ' '.join(list(args.arch))
        cmd = "{0} --build_config {1} --output-dir {2} --extra-gn-args {3} --arch {4}".format(
            IOS_SDK_BUILD_SCRIPT,
            build_config,
            build_dir,
            build_args,
            architectures)
        print("=============ios cmd:"+cmd)
        subprocess.call(cmd, shell=True)
    
    if args.os == "m":
        architectures = ' '.join(list(args.arch))
        cmd = "{0} --build_config {1} --output-dir {2} --extra-gn-args {3} --arch {4}".format(
            MAC_SDK_BUILD_SCRIPT,
            build_config,
            build_dir,
            build_args,
            architectures)
        print("=============mac cmd:"+cmd)
        subprocess.call(cmd, shell=True) 

    if args.os == "l":
        archs = list(args.arch)
        architectures = ""
        for arch in archs:
            if arch == "arm":
                arch = "armeabi-v7a"
            if arch == "arm64":
                arch = "arm64-v8a"
            if arch == "x64":
                arch =  "x86_64"
            architectures = architectures + " " + arch
        # architectures = "x86_64"
        architectures = architectures.strip()
        if architectures != "x86_64":
            print("\033[31mArchitecture only supports x64 for linux library\n\033[0m")
            return

        cmd = "{0} --build-dir {1} --extra-gn-args {2} --arch {3}".format(
            LINUX_SDK_BUILD_SCRIPT,
            build_dir,
            build_args,
            architectures)
        subprocess.call(cmd, shell=True)

    end_sec = time.time()
    consume_sec = end_sec - start_sec
    print("\033[32m\nDone! Consumed " + str(round(consume_sec, 2)) + " seconds.\n\033[0m")


if __name__ == '__main__':
    sys.exit(main())