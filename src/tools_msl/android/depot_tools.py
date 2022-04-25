#!/usr/bin/env python
# -*- coding:utf-8 -*-
from __future__ import print_function
import os
import sys
import tarfile
import time
import datetime  # 导入datetime模块
import threading
import logging
import subprocess
import platform
import shutil
import commands
import copy
import operator

# class _InterTimer 
CMAKE_VERSION = "3.23.1"
NINJA_VERSION = "1.10.2"
SCRIPT_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
SRC_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, os.pardir, os.pardir))
TOOLS_DIR = os.path.normpath(os.path.join(SRC_DIR, 'tools'))
NINJA_ROOT_DIR = os.path.normpath(os.path.join(TOOLS_DIR, platform.system(), 'ninja'))
CMAKE_ROOT_DIR = os.path.normpath(os.path.join(TOOLS_DIR, platform.system(), 'cmake'))
CMAKE_SOURCE_DIR = os.path.normpath(os.path.join(TOOLS_DIR, 'cmake', 'source'))
CMAKE_BUILD_SCRIPT = os.path.normpath(os.path.join(CMAKE_SOURCE_DIR, 'cmake-3.23.1', 'bootstrap'))
print("depot tools CMAKE_ROOT_DIR:"+CMAKE_ROOT_DIR)


class Count:

    def __init__(self):
        self.num = 0

    def add(self):
        self.num = self.num + 1

    def sub(self):
        self.num = self.num - 1

    def set(self, cnt):
        self.num = cnt

    def cnt(self):
        return self.num


def progressBar(progress, total):
    i = progress * 100 / total
    ir = str(i).rjust(3, " ")
    print("\r", end="")
    if i >= 100:
        print("Extract progress: {}%: ".format(ir), "▋" * (i // 2))
    else:
        print("Extract progress: {}%: ".format(ir), "▋" * (i // 2), end="")   
    sys.stdout.flush()
    # print("==progressbar", progress, total)


def showProgress(file_numbers, total_number):
    while True:
        # print("====================")
        num = file_numbers.cnt()
        if num > 0:
            time.sleep(0.5)
            # print("====================")
            progressBar(total_number - num, total_number)
        else:
            break
    progressBar(total_number, total_number)


def extract_file(file_name, path="."):
    progressBar(0, 100)
    tar = tarfile.open(file_name)
    members = tar.getmembers()
    file_count = len(members)
    file_numbers = Count()
    file_numbers.set(file_count)
    t = threading.Thread(target=showProgress, args=(file_numbers, file_count,))
    t.setDaemon(True)
    t.start()
    directories = []
    i = 0
    for tarinfo in members:
        if tarinfo.isdir():
            # Extract directories with a safe mode.
            directories.append(tarinfo)
            tarinfo = copy.copy(tarinfo)
            tarinfo.mode = 0700
        tar.extract(tarinfo, path)
        i = i + 1
        # print(i, file_count)
        if i != file_count:
            file_numbers.sub()

    # print("=============file_numbers cnt:", file_numbers.cnt())
    # Reverse sort directories.
    directories.sort(key=operator.attrgetter('name'))
    directories.reverse()

    # Set correct owner, mtime and filemode on directories.
    for tarinfo in directories:
        dirpath = os.path.join(path, tarinfo.name)
        try:
            tar.chown(tarinfo, dirpath)
            tar.utime(tarinfo, dirpath)
            tar.chmod(tarinfo, dirpath)
        except tarfile.ExtractError as e:
            if tar.errorlevel > 1:
                raise
            else:
                raise Exception(1, "tarfile: %s" % e)

    tar.close()
    file_numbers.sub()
    # print("=============file_numbers2 cnt:", file_numbers.cnt())
    t.join()


def CheckCmakeTools():
    print("depot tools CheckCmakeTools")
    cmake_version_status = [-1, -1]
    ninja_version_status = [-1, -1]
    cur_dir = os.getcwd()
    os.chdir(TOOLS_DIR)
    if os.path.exists(os.path.join(TOOLS_DIR, platform.system(), 'cmake')):
        print("depot tools 1CheckCmakeTools")
    else:
        os.makedirs(os.path.join(TOOLS_DIR, platform.system(), 'cmake'))
        print("depot tools 2CheckCmakeTools")
    os.chdir(cur_dir)
    print("depot tools 3CheckCmakeTools:"+cur_dir)
    
    cmd = "{0} --version".format(
        os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION, 'bin', 'cmake')))
    
    cmake_version_status = commands.getstatusoutput(cmd)
    if cmake_version_status[0] == 0:
        cmd = "{0} --version".format(
            os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION, 'bin', 'ninja')))
        ninja_version_status = commands.getstatusoutput(cmd)
        # print("{0} \r\ncode:{1}".format(ninja_version_status[1], ninja_version_status[0]))
        # return
        if ninja_version_status[0] == 0:
            return os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION))
        else:
            # pass
            print("{0} \r\ncode:{1}".format(ninja_version_status[1], ninja_version_status[0]))
            # raise Exception('error') 
    else:
        print("{0} \r\ncode:{1}".format(cmake_version_status[1], cmake_version_status[0]))
        # pass


    cmake_tar_file = os.path.join(CMAKE_SOURCE_DIR, 'cmake-3.23.1.tar.gz')
    if os.path.exists(os.path.join(CMAKE_SOURCE_DIR, 'cmake-3.23.1')):
        pass
    else:
        print("extract file:" + cmake_tar_file)
        extract_file(cmake_tar_file, CMAKE_SOURCE_DIR)
    
    print("====================2end cmake")
    # tar.extractall(os.path.join(CMAKE_ROOT_DIR, 'source'))
    # 解压到上级目录

    if cmake_version_status[0] != 0:
        os.chdir(os.path.join(CMAKE_SOURCE_DIR, 'cmake-3.23.1'))
        os.makedirs("cmake-build")
        os.chdir("cmake-build")

        cmd = "{0} --prefix={1}".format(
            CMAKE_BUILD_SCRIPT,
            os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION)))
        logging.info('cmd:%s', cmd)
        subprocess.call(cmd, shell=True)

        subprocess.call("make -j4 && make install", shell=True)

    # 编译并安装ninja
    cmake_tar_file = os.path.join(CMAKE_SOURCE_DIR, 'ninja-1.10.2.tar.gz')
    if os.path.exists(os.path.join(CMAKE_SOURCE_DIR, 'ninja-1.10.2')):
        pass
    else:
        print("extract file:" + cmake_tar_file)
        extract_file(cmake_tar_file, CMAKE_SOURCE_DIR)

    print("====================2end ninja")
    # 解压ninja完成，开始编译
    if ninja_version_status[0] != 0:
        os.chdir(os.path.join(CMAKE_SOURCE_DIR, 'ninja-1.10.2'))
        cmd = "cmake -Bbuild-cmake -H. --install-prefix={0}".format(
            os.path.normpath(os.path.join(NINJA_ROOT_DIR, NINJA_VERSION))
        )
        subprocess.call(cmd, shell=True)

        subprocess.call("cmake --build build-cmake", shell=True)
        os.chdir("build-cmake")
        subprocess.call("make install", shell=True)
        os.chdir(cur_dir)

        cmd = "{0} --version".format(
            os.path.normpath(os.path.join(NINJA_ROOT_DIR, NINJA_VERSION, 'bin', 'ninja')))
        
        # result = subprocess.call(cmd, shell=True)
        ninja_version_status = commands.getstatusoutput(cmd)
        print("======ninja_version_status:",ninja_version_status)
        if ninja_version_status[0] == 0:
            src_ninja_file = os.path.normpath(os.path.join(NINJA_ROOT_DIR, NINJA_VERSION, 'bin', 'ninja'))
            dist_ninja_file = os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION, 'bin', 'ninja'))
            shutil.copyfile(src_ninja_file, dist_ninja_file)
            shutil.copymode(src_ninja_file, dist_ninja_file)
        else:
            print("{0} \r\ncode:{1}".format(ninja_version_status[1], ninja_version_status[0]))
            raise Exception(1, "build ninja failure")

    cmd = "{0} --version".format(
        os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION, 'bin', 'cmake')))
    
    result = subprocess.call(cmd, shell=True)
    print("======result:",result)
    if result == 0:
        return os.path.normpath(os.path.join(CMAKE_ROOT_DIR, '3.23.1'))
    else:
        return None

def main():
    print("depot tools main")
    return 0


if __name__ == '__main__':
  sys.exit(main())