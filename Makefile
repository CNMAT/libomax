OMAX_BASENAMES = omax_util omax_class omax_doc omax_dict omax_pd_proxy
OMAX_OBJECTS = $(foreach OBJ, $(OMAX_BASENAMES), $(OBJ).o) 
OMAX_CFILES = $(foreach F, $(OMAX_BASENAMES), $(F).c)
OMAX_HFILES = $(foreach F, $(OMAX_BASENAMES), $(F).h)

C74SUPPORT = ../max6-sdk/c74support
MAX_INCLUDES = $(C74SUPPORT)/max-includes
PD_INCLUDES = ../pd-src/src

MAC_SYSROOT = MacOSX10.8.sdk 
MAC-CFLAGS = -arch i386 -arch x86_64 -O3 -funroll-loops -isysroot /Developer/SDKs/$(MAC_SYSROOT) -mmacosx-version-min=10.5 -std=c99 -DOMAX_PD_VERSION
WIN-CFLAGS = -O3 -funroll-loops -mno-cygwin -DWIN_VERSION -DWIN_EXT_VERSION -U__STRICT_ANSI__ -U__ANSI_SOURCE -std=c99

MAC-INCLUDES = -I$(MAX_INCLUDES) -I$(PD_INCLUDES) -I../libo -F/System/Library/Frameworks -I/usr/include
WIN-INCLUDES = -I$(MAX_INCLUDES) -I../libo -I/usr/include

all: CFLAGS += $(MAC-CFLAGS)
all: CC = gcc
all: I = $(MAC-INCLUDES)
all: $(LIBO_CFILES) $(LIBO_HFILES) libomax.a
all: LIBTOOL = libtool -static -o libomax.a $(OMAX_OBJECTS)

win: CFLAGS += $(WIN-CFLAGS)
win: CC = i686-w64-mingw32-gcc
#win: CC = gcc
win: I = $(WIN-INCLUDES)
win: $(LIBO_CFILES) $(LIBO_HFILES) libomax.a
win: LIBTOOL = ar cru libomax.a $(OMAX_OBJECTS)

libomax.a: $(OMAX_OBJECTS) $(OMAX_CFILES) $(OMAX_HFILES)
	$(LIBTOOL)

%.o: %.c %.h 
	$(CC) $(CFLAGS) $(I) -o $(notdir $(basename $@)).o -c $(notdir $(basename $@)).c


.PHONY: clean
clean:
	rm -f *.o *.a

.PHONY: doc
doc:
	cd doc && doxygen Doxyfile
