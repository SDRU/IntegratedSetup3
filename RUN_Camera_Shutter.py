#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov 16 15:30:49 2021
@author: sandora

This code controls:
    - thermal camera (FLIR ONE PRO)
    - shutter (Thorlabs SH1)

When temperature registered by the camera surpasses Tthresh, shutter will close
"""

import subprocess
import os
import numpy as np
import psutil
from MyFunctions import *

Tthresh = 31
temperatures = []
n=1 # temperature points to average

try:
    Shutter = ShutterObject()
    Shutter.unblock()
    isopen = 1
    subprocess.Popen(['sudo', '-s', 'modprobe', 'v4l2loopback', 'video_nr=0'])
    p=subprocess.Popen(['sudo','-s','/home/pi/Downloads/Integration/Run_Camera'], stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,text=True,bufsize=0)
    
    while True:
        line = p.stdout.readline()
        p.stdout.flush()
        if not(line==""):
            T = float(line.rstrip())
            temperatures.append(T)
            if len(temperatures) > n:
                Tmean = np.mean(temperatures[-n:])
                if Tmean>Tthresh:
                    print(T)
                    if isopen == 1:
                        Shutter.block()
                        isopen = 0
                else:
                    if isopen == 0:
                        Shutter.unblock()
                        isopen = 1
               
except:
    id1 = p.pid
    os.system("sudo kill -9 {pid}".format(pid=id1))
 
    # kill also a subprocess of sudo
    for i in range(1,10):
        id2 = id1+i
        if psutil.pid_exists(id2):
            os.system("sudo kill -9 {pid}".format(pid=id2))
            break
    Shutter.block()
    Shutter.close()
    
    