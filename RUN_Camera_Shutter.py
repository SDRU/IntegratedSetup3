#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov 16 15:30:49 2021

@author: sandora
"""

from struct import pack,unpack
import serial
import time
import subprocess
import os
from MyFunctions import *

Tthresh = 34

try:
    Shutter = ShutterObject()
    Shutter.unblock()
    isopen = 1
    p=subprocess.Popen(['sudo','-s','/home/pi/Downloads/flir8i/flir8i'], stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,text=True,bufsize=0)
    
    while True:
        line = p.stdout.readline()
        p.stdout.flush()
        if not(line==""):
            T = float(line.rstrip())
            if T>Tthresh:
                if isopen == 1:
                    print(T)
                    Shutter.block()
                    isopen = 0
            else:
                if isopen == 0:
                    Shutter.unblock()
                    isopen = 1
                
except KeyboardInterrupt:
    id1 = p.pid
    id2 = id1+1
    os.system("sudo kill -9 {pid}".format(pid=id1))
    os.system("sudo kill -9 {pid}".format(pid=id2))
    Shutter.close()