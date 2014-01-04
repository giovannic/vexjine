include ../../../Makefile.inc

OBJECTS=JmtNetworkParser.o  JmtNodeParser.o  JmtParser.o  JmtProbabilitiesParser.o  JmtQueueParser.o
TINYXMLOBJ=tinyxml/tinystr.o  tinyxml/tinyxmlerror.o  tinyxml/tinyxml.o  tinyxml/tinyxmlparser.o

CFLAGS+=-c -fPIC
INCLUDE_DIRS=$(INCLUDE_MODEL_DIRS) $(INCLUDE_CINQS_DIRS)

all: $(OBJECTS) $(TINYXMLOBJ)

tester: parsetest.cpp
	g++ -L../../ -L../../../cinqs/ -I.. -I/homes/nb605/VTF/src/cinqs/ -Itinyxml/ -I/homes/nb605/VTF/src/cinqs/tools/ -I/homes/nb605/VTF/src/cinqs/network/ -lcinqs -lvex -o test parsetest.cpp

$(TINYXMLOBJ): 
	cd tinyxml; $(MAKE) $(MAKECMDGOALS)

JmtParser.o: JmtParser.cpp JmtParser.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) JmtParser.cpp

JmtNodeParser.o: JmtNodeParser.cpp JmtNodeParser.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) JmtNodeParser.cpp

JmtNetworkParser.o: JmtNetworkParser.cpp JmtNetworkParser.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) JmtNetworkParser.cpp

JmtProbabilitiesParser.o: JmtProbabilitiesParser.cpp JmtProbabilitiesParser.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) JmtProbabilitiesParser.cpp

JmtQueueParser.o: JmtQueueParser.cpp JmtQueueParser.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) JmtQueueParser.cpp

clean:
	rm -rf *.o
	cd tinyxml ; $(MAKE) clean
