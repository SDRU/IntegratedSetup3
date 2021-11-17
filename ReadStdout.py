# -*- coding: utf-8 -*-
"""
Read output from cmd FLIR ONE camera
"""

import subprocess
import os


try:
    p=subprocess.Popen(['sudo','-s','/home/pi/Downloads/flir8i/flir8i'], stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE,text=True,bufsize=0)

          
    while True:
        line = p.stdout.readline()
        p.stdout.flush()
        if not(line==""):
            T = float(line.rstrip())
            if T>34:
                print(T)
            else:
                print('Cold cold')
            
    
except KeyboardInterrupt:
    id1 = p.pid
    id2 = id1+1
    os.system("sudo kill -9 {pid}".format(pid=id1))
    os.system("sudo kill -9 {pid}".format(pid=id2))

