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

isStop = False
count = 0
def run():  # 定义方法
    global isStop
    global count
    while isStop == False:
        t = threading.currentThread()
        # print('Thread id : %d' % t.ident)
        # print(datetime.datetime.now())  # 输出当前时间
        print("\r", end="")
        i = 20
        print("Download progress: {}%: ".format(count), "▋" * count, end="")
        sys.stdout.flush()
        time.sleep(0.5)
        count = count + 1
    print("")
    # timer = threading.Timer(1, run)  # 每秒运行
    # timer.start()  # 执行方法
    # timer.cancel()
    

def progress_bar():
    for i in range(1, 101):
        # print("\r", end="")
        # print("Download progress: {}%: ".format(i), "▋" * (i // 2), end="")
        sys.stdout.flush()
        time.sleep(0.05)

def CheckCmakeTools():
    print("depot tools CheckCmakeTools")
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
        pass

    # result_cmaker_version = subprocess.call(cmd, shell=True)
    # print("======result:",result)
    # if result == 0:
    #     return os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION))
    # else:
    #     pass

    # return
    t1 = threading.Timer(1, function=run)  # 创建定时器
    print("====================1start cmake")
    t1.start()  # 开始执行线程
    cmake_tar_file = os.path.join(CMAKE_SOURCE_DIR, 'cmake-3.23.1.tar.gz')
    if os.path.exists(os.path.join(CMAKE_SOURCE_DIR, 'cmake-3.23.1')):
        pass
    else:
        print("depot tools cmake_tar_file:"+cmake_tar_file)
        tar = tarfile.open(cmake_tar_file)
        # names = tar.getnames()
        # for name in names:
        #     # print(name)
        #     tar.extract(name, path=os.path.join(CMAKE_ROOT_DIR, 'source'))
        tar.extractall(CMAKE_SOURCE_DIR)
        tar.close()
    
    global isStop
    isStop = True
    t1.cancel()
    t1.join()
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

    t1 = threading.Timer(1, function=run)  # 创建定时器
    print("====================1start ninja")
    isStop = False
    t1.start()  # 开始执行线程
    cmake_tar_file = os.path.join(CMAKE_SOURCE_DIR, 'ninja-1.10.2.tar.gz')
    if os.path.exists(os.path.join(CMAKE_SOURCE_DIR, 'ninja-1.10.2')):
        pass
    else:
        print("depot tools cmake_tar_file:"+cmake_tar_file)
        tar = tarfile.open(cmake_tar_file)
        # names = tar.getnames()
        # for name in names:
        #     # print(name)
        #     tar.extract(name, path=os.path.join(CMAKE_ROOT_DIR, 'source'))
        tar.extractall(CMAKE_SOURCE_DIR)
        tar.close()
    
    isStop = True
    t1.cancel()
    t1.join()
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
        
        result = subprocess.call(cmd, shell=True)
        print("======result:",result)
        if result == 0:
            src_ninja_file = os.path.normpath(os.path.join(NINJA_ROOT_DIR, NINJA_VERSION, 'bin', 'ninja'))
            dist_ninja_file = os.path.normpath(os.path.join(CMAKE_ROOT_DIR, CMAKE_VERSION, 'bin', 'ninja'))
            shutil.copyfile(src_ninja_file, dist_ninja_file)
            shutil.copymode(src_ninja_file, dist_ninja_file)

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