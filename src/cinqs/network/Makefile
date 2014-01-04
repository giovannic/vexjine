include ../../Makefile.inc

CFLAGS+=-fPIC -I../tools/ -I..

OBJECTS=BoxedQueue.o ClassDependentBranch.o ClassDependentDelay.o Customer.o Debug.o Delay.o FIFOQueue.o InfiniteServerNode.o LIFOQueue.o Link.o Network.o Node.o NullNode.o OrderedQueue.o OrderedQueueEntry.o PreemptiveRestartNode.o PreemptiveResumeNode.o PriorityQueue.o ProbabilisticBranch.o ProcessorSharingNode.o Queue.o QueueingNode.o RandomQueue.o ResourcePool.o Sink.o Source.o

all: $(OBJECTS)

BoxedQueue.o: BoxedQueue.cpp BoxedQueue.h
	$(CC) $(CFLAGS) -c BoxedQueue.cpp

ClassDependentBranch.o: ClassDependentBranch.cpp ClassDependentBranch.h
	$(CC) $(CFLAGS) -c ClassDependentBranch.cpp

ClassDependentDelay.o: ClassDependentDelay.cpp ClassDependentDelay.h
	$(CC) $(CFLAGS) -c ClassDependentDelay.cpp

Customer.o: Customer.cpp Customer.h
	$(CC) $(CFLAGS) -c Customer.cpp

Debug.o: Debug.cpp Debug.h
	$(CC) $(CFLAGS) -c Debug.cpp

Delay.o: Delay.cpp Delay.h
	$(CC) $(CFLAGS) -c Delay.cpp

FIFOQueue.o: FIFOQueue.cpp FIFOQueue.h
	$(CC) $(CFLAGS) -c FIFOQueue.cpp

InfiniteServerNode.o: InfiniteServerNode.cpp InfiniteServerNode.h
	$(CC) $(CFLAGS) -c InfiniteServerNode.cpp

LIFOQueue.o: LIFOQueue.cpp LIFOQueue.h
	$(CC) $(CFLAGS) -c LIFOQueue.cpp

Link.o: Link.cpp Link.h
	$(CC) $(CFLAGS) -c Link.cpp

Network.o: Network.cpp Network.h
	$(CC) $(CFLAGS) -c Network.cpp

Node.o: Node.cpp Node.h
	$(CC) $(CFLAGS) -c Node.cpp

NullNode.o: NullNode.cpp NullNode.h
	$(CC) $(CFLAGS) -c NullNode.cpp

OrderedQueue.o: OrderedQueue.cpp OrderedQueue.h
	$(CC) $(CFLAGS) -c OrderedQueue.cpp

OrderedQueueEntry.o: OrderedQueueEntry.cpp OrderedQueueEntry.h
	$(CC) $(CFLAGS) -c OrderedQueueEntry.cpp

PreemptiveRestartNode.o: PreemptiveRestartNode.cpp PreemptiveRestartNode.h
	$(CC) $(CFLAGS) -c PreemptiveRestartNode.cpp

PreemptiveResumeNode.o: PreemptiveResumeNode.cpp PreemptiveResumeNode.h
	$(CC) $(CFLAGS) -c PreemptiveResumeNode.cpp

PriorityQueue.o: PriorityQueue.cpp PriorityQueue.h
	$(CC) $(CFLAGS) -c PriorityQueue.cpp

ProbabilisticBranch.o: ProbabilisticBranch.cpp ProbabilisticBranch.h
	$(CC) $(CFLAGS) -c ProbabilisticBranch.cpp

ProcessorSharingNode.o: ProcessorSharingNode.cpp ProcessorSharingNode.h
	$(CC) $(CFLAGS) -c ProcessorSharingNode.cpp

Queue.o: Queue.cpp Queue.h
	$(CC) $(CFLAGS) -c Queue.cpp

QueueingNode.o: QueueingNode.cpp QueueingNode.h
	$(CC) $(CFLAGS) -c QueueingNode.cpp

RandomQueue.o: RandomQueue.cpp RandomQueue.h
	$(CC) $(CFLAGS) -c RandomQueue.cpp

ResourcePool.o: ResourcePool.cpp ResourcePool.h
	$(CC) $(CFLAGS) -c ResourcePool.cpp

Sink.o: Sink.cpp Sink.h
	$(CC) $(CFLAGS) -c Sink.cpp

Source.o: Source.cpp Source.h
	$(CC) $(CFLAGS) -c Source.cpp

clean:
	rm -f *.o
