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

from tkinter.messagebox import YES  # 导入threading模块

# class _InterTimer 

SCRIPT_DIR = os.path.dirname(os.path.realpath(sys.argv[0]))
SRC_DIR = os.path.normpath(os.path.join(SCRIPT_DIR, os.pardir, os.pardir))
CMAKE_ROOT_DIR = os.path.normpath(os.path.join(SRC_DIR, 'tools', 'cmake'))
CMAKE_BUILD_SCRIPT = os.path.normpath(os.path.join(CMAKE_ROOT_DIR, 'source', 'cmake-3.23.1', 'bootstrap'))
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
    t1 = threading.Timer(1, function=run)  # 创建定时器
    print("====================1start")
    t1.start()  # 开始执行线程
    cmake_tar_file = os.path.join(CMAKE_ROOT_DIR, 'source', 'cmake-3.23.1.tar.gz')
    if os.path.exists(os.path.join(CMAKE_ROOT_DIR, 'source', 'cmake-3.23.1')):
        pass
    else:
        print("depot tools cmake_tar_file:"+cmake_tar_file)
        tar = tarfile.open(cmake_tar_file)
        # names = tar.getnames()
        # for name in names:
        #     # print(name)
        #     tar.extract(name, path=os.path.join(CMAKE_ROOT_DIR, 'source'))
        tar.extractall(os.path.join(CMAKE_ROOT_DIR, 'source'))
        tar.close()
    
    global isStop
    isStop = YES
    t1.cancel()
    t1.join()
    print("====================2end")
    # tar.extractall(os.path.join(CMAKE_ROOT_DIR, 'source'))
    # 解压到上级目录


    os.chdir(os.path.join(CMAKE_ROOT_DIR, 'source', 'cmake-3.23.1'))
    os.makedirs("cmake-build")
    os.chdir("cmake-build")
    
    cmd = "{0} --prefix={1}".format(
        CMAKE_BUILD_SCRIPT,
        CMAKE_ROOT_DIR)
    logging.info('cmd:%s', cmd)
    subprocess.call(cmd, shell=True)

    subprocess.call("make -j4 && make install", shell=True)

def main():
    print("depot tools main")
    return 0


if __name__ == '__main__':
  sys.exit(main())