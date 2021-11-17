CC = gcc -I/usr/include/libusb-1.0 -I/usr/include/python3.7m
GXX = g++ -I/usr/include/libusb-1.0 -I/usr/include/python3.7m
CXXFLAGS      = -pipe -Wall -W -D_REENTRANT -lpython3.7m -fpie -lusb-1.0 -lm  -Wno-unused-result -Wsign-compare -g -fdebug-prefix-map=/build/python3.7-DSh9dq/python3.7-3.7.3=. -specs=/usr/share/dpkg/no-pie-compile.specs -fstack-protector -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3
INCPATH = -I. -I/usr/include/libusb-1.0 -I/usr/include/python3.7m

CFLAGS      = -pipe -Wall -W -D_REENTRANT -lpython3.7m -fpie -lusb-1.0 -lm  -Wno-unused-result -Wsign-compare -g -fdebug-prefix-map=/build/python3.7-DSh9dq/python3.7-3.7.3=. -specs=/usr/share/dpkg/no-pie-compile.specs -fstack-protector -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3

all:  Palettes.o flir8i.o flir8i


Palettes.o: Palettes.cpp Palettes.h
	${CXX} ${CXXFLAGS} ${INCPATH} -o Palettes.o Palettes.cpp

flir8i.o: flir8i.cpp 
	${CXX} ${CXXFLAGS} ${INCPATH} -o flir8i.o flir8i.cpp

flir8i: flir8i.o
	${CXX} -o flir8i.o Palettes.o flir8i.cpp -lusb-1.0 -ljpeg -lm -Wall ${CXXFLAGS} ${INCPATH} 


clean:
	rm -f  Palettes.o flir8i.o flir8i
