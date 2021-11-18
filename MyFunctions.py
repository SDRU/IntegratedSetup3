#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Nov 16 16:24:52 2021

@author: sandora
"""

import serial
from struct import pack,unpack


class ShutterObject:
    def __init__(self):
        #Port Settings
        baud_rate = 115200
        data_bits = 8
        stop_bits = 1
        Parity = serial.PARITY_NONE
        #Controller's Port and Channel
        COM_Port = '/dev/ttyUSB0' #Change to preferred

        self.Channel = 1 #Channel is always 1 for a K Cube/T Cube
        self.destination = 0x50 #Destination byte; 0x50 for T Cube/K Cube, USB controllers
        self.source = 0x01 #Source Byte

        #Create Serial Object
        self.Shutter = serial.Serial(port = COM_Port, baudrate = baud_rate, bytesize=data_bits, parity=Parity, stopbits=stop_bits,timeout=0.1)
    
        #Get HW info; MGMSG_HW_REQ_INFO; may be require by a K Cube to allow confirmation Rx messages
        #Shutter.write(pack('<HBBBB', 0x0005, 0x00, 0x00, 0x50, 0x01))
        self.Shutter.flushInput()
        self.Shutter.flushOutput()
    
        #Enable Stage; MGMSG_MOD_SET_CHANENABLESTATE 
        self.Shutter.write(pack('<HBBBB',0x0210,self.Channel,0x01,self.destination,self.source))
        print('Shutter enabled')
    
        # Set mode to manual, opens and closes when button is pressed
        mode = 0x01
        self.Shutter.write(pack('<HBBBB',0x04C0,self.Channel,mode,self.destination,self.source))
               
    def unblock(self):
        # Open shutter
        state = 0x01
        self.Shutter.write(pack('<HBBBB',0x04CB,self.Channel,state,self.destination,self.source))
    
    def block(self):
        # Open shutter
        state = 0x02
        self.Shutter.write(pack('<HBBBB',0x04CB,self.Channel,state,self.destination,self.source))
    
    def close(self):
        self.Shutter.close()
