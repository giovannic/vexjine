include ../../Makefile.inc

all: MethodEventsBehaviour.o ThreadEventsBehaviour.o

CFLAGS+=-c -fPIC $(CPPFLAGS)

MethodEventsBehaviour.o: MethodEventsBehaviour.cpp  MethodEventsBehaviour.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(INCLUDE_MODEL_DIRS) $< -o $@

ThreadEventsBehaviour.o: ThreadEventsBehaviour.cpp  ThreadEventsBehaviour.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(INCLUDE_PAPIPROFILER_DIR) $< -o $@

clean:
	rm -rf *.o
