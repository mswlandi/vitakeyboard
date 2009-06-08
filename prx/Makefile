TARGET = hidmouse
OBJS = main.o hidmouse.o

# Use the kernel's small inbuilt libc
#USE_KERNEL_LIBC = 1
# Use only kernel libraries
#USE_KERNEL_LIBS = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall -fno-builtin-printf -fno-builtin-log
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

LIBS =  -lpspusb_driver -lpspusbbus_driver -lpspsdk -lpspnet -lpspnet_apctl -lpspnet_inet -lpspnet_resolver -lpsplibc -lpspuser -lpsputility 


PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
