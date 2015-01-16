OMAX_BASENAMES = omax_util omax_class omax_doc omax_dict omax_realtime
OMAX_OBJECTS = $(foreach OBJ, $(OMAX_BASENAMES), $(OBJ).o) 
OMAX_CFILES = $(foreach F, $(OMAX_BASENAMES), $(F).c)
OMAX_HFILES = $(foreach F, $(OMAX_BASENAMES), $(F).h)

OPD_BASENAMES = omax_util omax_doc omax_pd_proxy 
OPD_OBJECTS = $(foreach OBJ, $(OPD_BASENAMES), $(OBJ)_libopd.o) 
OPD_CFILES = $(foreach F, $(OPD_BASENAMES), $(F).c)
OPD_HFILES = $(foreach F, $(OPD_BASENAMES), $(F).h)

C74SUPPORT = ../max6-sdk/c74support
MAX_INCLUDES = $(C74SUPPORT)/max-includes 
MSP_INCLUDES = $(C74SUPPORT)/msp-includes
PD_INCLUDES = ../pure-data/src

MAC_SYSROOT = MacOSX10.8.sdk 
#MAC-CFLAGS = -arch i386 -arch x86_64 -O3 -funroll-loops -isysroot /Users/r/Documents/dev-lib/SDKs/$(MAC_SYSROOT) -mmacosx-version-min=10.5 -std=c99 
MAC_PATH_TO_SDK = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
MAC-CFLAGS = -arch i386 -arch x86_64 -O3 -funroll-loops -isysroot $(MAC_PATH_TO_SDK)/$(MAC_SYSROOT) -mmacosx-version-min=10.5 -std=c99 
MAC-PD-CFLAGS = $(MAC-CFLAGS) -DOMAX_PD_VERSION
WIN-CFLAGS = -O3 -funroll-loops -mno-cygwin -DWIN_VERSION -DWIN_EXT_VERSION -U__STRICT_ANSI__ -U__ANSI_SOURCE -std=c99

MAC-INCLUDES = -I$(MAX_INCLUDES) -I$(MSP_INCLUDES) -I$(PD_INCLUDES) -I../libo -F/System/Library/Frameworks -I/usr/include
WIN-INCLUDES = -I$(MAX_INCLUDES) -I$(MSP_INCLUDES) -I../libo -I/usr/include

all: CFLAGS += $(MAC-CFLAGS)
all: CFLAGS_PD += $(MAC-PD-CFLAGS)
all: CC = clang
all: I = $(MAC-INCLUDES)
all: libomax.a libopd.a
all: LIBTOOL = libtool -static -o libomax.a $(OMAX_OBJECTS)
all: LIBTOOL_PD = libtool -static -o libopd.a $(OPD_OBJECTS)

win: CFLAGS += $(WIN-CFLAGS)
win: CC = i686-w64-mingw32-gcc
#win: CC = gcc
win: I = $(WIN-INCLUDES)
win: libomax.a #libopd.a
win: LIBTOOL = ar cru libomax.a $(OMAX_OBJECTS)

linux: CC = clang
linux: CFLAGS_PD += -DOMAX_PD_VERSION -funroll-loops -std=c99
linux: I = -I$(PD_INCLUDES) -I../libo
linux: libopd.a
#linux: LIBTOOL_PD = libtool --tag=CC --mode=link clang -all-static -o libopd.a $(OPD_OBJECTS)
linux: LIBTOOL_PD = ar scru libopd.a $(OPD_OBJECTS)

.phony: printomax printopd
printomax:
	$(info )
	$(info ******************************************************************)
	$(info *****                    BUILDING LIBOMAX                    *****)
	$(info ******************************************************************)
printopd:
	$(info )
	$(info ******************************************************************)
	$(info *****                    BUILDING LIBOPD                     *****)
	$(info ******************************************************************)

libomax.a: printomax $(OMAX_OBJECTS) $(OMAX_CFILES) $(OMAX_HFILES)
	$(LIBTOOL)

libopd.a: printopd $(OPD_OBJECTS) $(OPD_CFILES) $(OPD_HFILES)
	$(LIBTOOL_PD)

#%.o: %.c %.h 
#	$(CC) $(CFLAGS) $(I) -o $(notdir $(basename $@)).o -c $(notdir $(basename $@)).c

#$(OMAX_OBJECTS):
%.o: %.c %.h 
	$(CC) $(CFLAGS) $(I) -o $(notdir $(basename $@)).o -c $(notdir $(basename $@)).c

#$(OPD_OBJECTS): 
%_libopd.o: %.c %.h 
	$(CC) $(CFLAGS_PD) $(I) -o $(notdir $(basename $@)).o -c $(subst _libopd,,$(notdir $(basename $@))).c

.PHONY: clean
clean:
	rm -f *.o *.a

.PHONY: doc
doc:
	cd doc && doxygen Doxyfile
