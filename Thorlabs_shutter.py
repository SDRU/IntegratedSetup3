##from struct import pack,unpack
##import serial
##import time

Channel = 1 #Channel is always 1 for a K Cube/T Cube
destination = 0x50 #Destination byte; 0x50 for T Cube/K Cube, USB controllers
source = 0x01 #Source Byte
    
def initialize():
    print("Hello world")
##    global Shutter
##    #Port Settings
##    baud_rate = 115200
##    data_bits = 8
##    stop_bits = 1
##    Parity = serial.PARITY_NONE
##    #Controller's Port and Channel
##    COM_Port = '/dev/ttyUSB0' #Change to preferred
##
##
##    #Create Serial Object
##    Shutter = serial.Serial(port = COM_Port, baudrate = baud_rate, bytesize=data_bits, parity=Parity, stopbits=stop_bits,timeout=0.1)
##
##    #Get HW info; MGMSG_HW_REQ_INFO; may be require by a K Cube to allow confirmation Rx messages
##    #Shutter.write(pack('<HBBBB', 0x0005, 0x00, 0x00, 0x50, 0x01))
##    Shutter.flushInput()
##    Shutter.flushOutput()
##
##    #Enable Stage; MGMSG_MOD_SET_CHANENABLESTATE 
##    Shutter.write(pack('<HBBBB',0x0210,Channel,0x01,destination,source))
##    print('Stage Enabled')
##    time.sleep(0.1)
##
##    # Set mode to single, opens and closes when button is pressed
##    mode = 0x01
##    Shutter.write(pack('<HBBBB',0x04C0,Channel,mode,destination,source))


def unblock():
    global Shutter
    # Open shutter
    state = 0x01
    Shutter.write(pack('<HBBBB',0x04CB,Channel,state,destination,source))

def block():
    global Shutter
    # Open shutter
    state = 0x02
    Shutter.write(pack('<HBBBB',0x04CB,Channel,state,destination,source))

def close():
    global Shutter
    Shutter.close()
    del Shutter
