CC=g++
CFLAGS=-c -Wall -include flags.h

LDFLAGS=

INCLUDE_DIR+=$(STAGING_DIR)/usr/include\
$(STAGING_DIR)/usr/local/include\
$(SDKSTAGE)

LIB_DIR=$(STAGING_DIR)/usr/lib
#       $(STAGING_DIR)/usr/local/lib

LIBS=

SOURCES=main.c

OBJ_DIR=obj
EXE_DIR=.

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=$(EXE_DIR)/FFT_DFT_01.bin

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(AR) rcs $@ $(OBJECTS)
	@ranlib $@
	@rm -f $(OBJECTS)

.cpp.o: 
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $< -o $@
	
clean:
	@rm -Rf *.o