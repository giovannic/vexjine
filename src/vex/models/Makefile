include ../../Makefile.inc

all: MethodPerformanceModel.o VexInfiniteServerNode.o VexQueueingNetwork.o VexSchedSimQueueingNetwork.o VexSchedSimCustomer.o VexThreadCustomer.o VexQueueingNode.o VexLocalQueueingNode.o VexLocalInfiniteServerNode.o PostActionNode.o 
	cd parser; $(MAKE) $(MAKECMDGOALS)

CFLAGS+=-c -fPIC $(CPPFLAGS)
INCLUDE_DIRS+=$(INCLUDE_MODEL_DIRS) $(INCLUDE_TINYXML_DIR) $(INCLUDE_CINQS_DIRS)

MethodPerformanceModel.o: MethodPerformanceModel.cpp  MethodPerformanceModel.h
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 

VexInfiniteServerNode.o: VexInfiniteServerNode.cpp  VexInfiniteServerNode.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 

VexSchedSimCustomer.o: VexSchedSimCustomer.cpp VexSchedSimCustomer.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 
	
VexQueueingNode.o: VexQueueingNode.cpp VexQueueingNode.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 

VexLocalInfiniteServerNode.o: VexLocalInfiniteServerNode.cpp  VexLocalInfiniteServerNode.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 

VexLocalQueueingNode.o: VexLocalQueueingNode.cpp VexLocalQueueingNode.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@ 

VexQueueingNetwork.o: VexQueueingNetwork.cpp  VexQueueingNetwork.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

VexSchedSimQueueingNetwork.o: VexSchedSimQueueingNetwork.cpp  VexSchedSimQueueingNetwork.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

VexThreadCustomer.o: VexThreadCustomer.cpp  VexThreadCustomer.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

PostActionNode.o: PostActionNode.cpp  PostActionNode.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o
	cd parser ; $(MAKE) clean

