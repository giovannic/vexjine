include ../../../../Makefile.inc

OBJECTS=tinystr.o tinyxmlerror.o tinyxml.o tinyxmlparser.o

all: $(OBJECTS)

CFLAGS+=-c -fPIC
INCLUDE_DIRS=

tinystr.o: tinystr.cpp tinystr.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) tinystr.cpp

tinyxmlerror.o: tinyxmlerror.cpp 
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) tinyxmlerror.cpp

tinyxml.o: tinyxml.cpp tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) tinyxml.cpp

tinyxmlparser.o: tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) tinyxmlparser.cpp

clean:
	rm -rf *.o

