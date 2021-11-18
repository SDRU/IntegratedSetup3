CC = gcc -I/usr/include/libusb-1.0
GXX = g++
CXXFLAGS      = -pipe -Wall -W -D_REENTRANT -lusb-1.0 -lm
INCPATH = -I. -I/usr/include/libusb-1.0

all:  Run_Camera.o Run_Camera


Run_Camera.o: Run_Camera.c

Run_Camera: Run_Camera.o
	${CC} -o Run_Camera Run_Camera.o -lusb-1.0 -ljpeg -lm -Wall


clean:
	rm -f  Run_Camera.o Run_Camera
