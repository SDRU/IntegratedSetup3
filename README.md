This code controls the FLIR ONE PRO thermal camera. If temperature measured by the camera is higher than the threshold, it will close the Thorlabs shutter.

FLIR ONE only works on Linux, as it was designed for Android. To run the camera, install FLIR ONE app on your phone.

Shutter is not controlled by official Thorlabs library like in Windows, it is controlled through serial interface.

Main code is *RUN_Camera_Shutter.py*