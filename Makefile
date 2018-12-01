CP_PATH=$(abspath ../../..)/CrossCompiler
CROSS_COMPILE=$(CP_PATH)/gcc-linaro-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ROOTFS=$(CP_PATH)/sysroots/cortexa15t2hf-vfp-neon-linux-gnueabi

ARCH=arm
CC=$(CROSS_COMPILE)gcc
CXX=$(CROSS_COMPILE)g++

INC += -Iinc
INC += -I protocol
INC += -I$(ROOTFS)/include
INC += -I$(ROOTFS)/usr/include
INC += -I$(ROOTFS)/usr/include/omap
INC += -I$(ROOTFS)/usr/include/libdrm
INC += -I$(ROOTFS)/usr/include/gbm
LIBDIR := $(ROOTFS)/usr/lib

CFLAGS := -O1 -g -Wall -fPIC -mfloat-abi=hard -mfpu=neon -Wl,-rpath,$(ROOTFS)/lib -Wl,-rpath,$(ROOTFS)/usr/lib $(INC)
CXXFLAGS = -Wall -ansi -g -fPIC -mfloat-abi=hard -mfpu=neon -std=c++11 $(INC) -I$(ROOTFS)/include/c++/4.7.3/

LDFLAGS = -lm -lpthread -L$(LIBDIR) -lrt -ldrm -lmtdev -ldrm_omap -lstdc++
TARGET = project

all: $(TARGET)

clean:
	rm -f *.a *.o $(TARGET) *.lo

$(TARGET): main.cpp v4l2.lo display-kms.lo util.lo vpe-common.lo input_cmd.lo car_lib.lo system_management.lo image_processing.lo cv.lo calibration.lo car_control.lo
	$(CXX) $(CXXFLAGS) -o $@ main.cpp v4l2.lo display-kms.lo util.lo vpe-common.lo input_cmd.lo car_lib.lo system_management.lo image_processing.lo cv.lo calibration.lo car_control.lo $(LDFLAGS)

v4l2.lo: v4l2.c v4l2.h
	$(CC) -c $(CFLAGS) -o $@ v4l2.c

display-kms.lo: display-kms.c display-kms.h
	$(CC) -c $(CFLAGS) -o $@ display-kms.c

util.lo: util.c util.h
	$(CC) -c $(CFLAGS) -o $@ util.c

vpe-common.lo: vpe-common.c vpe-common.h
	$(CC) -c $(CFLAGS) -o $@ vpe-common.c

input_cmd.lo: input_cmd.cpp input_cmd.h
	$(CXX) -c $(CXXFLAGS) -o $@ input_cmd.cpp

car_lib.lo: car_lib.c car_lib.h
	$(CC) -c $(CFLAGS) -o $@ car_lib.c

system_management.lo: system_management.cpp system_management.h
	$(CXX) -c $(CXXFLAGS) -o $@ system_management.cpp

car_control.lo: car_control.cpp car_control.h system_management.lo
	$(CXX) -c $(CXXFLAGS) -o $@ car_control.cpp system_management.lo

image_processing.lo: image_processing.cpp image_processing.h system_management.lo
	$(CXX) -c $(CXXFLAGS) -o $@ image_processing.cpp system_management.lo

cv.lo: cv.cpp cv.h system_management.lo
	$(CXX) -c $(CXXFLAGS) -o $@ cv.cpp system_management.lo

calibration.lo: calibration.cpp calibration.h system_management.lo
	$(CXX) -c $(CXXFLAGS) -o $@ calibration.cpp system_management.lo
