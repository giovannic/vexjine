include ../Makefile.inc

OBJECT=./network/BoxedQueue.o ./network/ClassDependentBranch.o ./network/ClassDependentDelay.o ./network/Customer.o ./network/Debug.o ./network/Delay.o ./network/FIFOQueue.o ./network/InfiniteServerNode.o ./network/LIFOQueue.o ./network/Link.o ./network/Network.o ./network/Node.o ./network/NullNode.o ./network/OrderedQueue.o ./network/OrderedQueueEntry.o ./network/PreemptiveRestartNode.o ./network/PreemptiveResumeNode.o ./network/PriorityQueue.o ./network/ProbabilisticBranch.o ./network/ProcessorSharingNode.o ./network/Queue.o ./network/QueueingNode.o ./network/RandomQueue.o ./network/ResourcePool.o ./network/Sink.o ./network/Source.o ./tools/AreaHistogram.o ./tools/Cauchy.o ./tools/Check.o ./tools/ContEmpirical.o ./tools/CustomerMeasure.o ./tools/DistributionSampler.o ./tools/Deterministic.o ./tools/Diary.o ./tools/DiscEmpirical.o ./tools/DiscreteSampler.o ./tools/EmptyListException.o ./tools/Erlang.o ./tools/Event.o ./tools/Exp.o ./tools/Gamma.o ./tools/Geometric.o ./tools/Histogram.o ./tools/List.o ./tools/Logger.o ./tools/Measure.o ./tools/Normal.o ./tools/OrderedList.o ./tools/Pareto.o ./tools/RejectionMethod.o ./tools/ReplicatedSim.o ./tools/Resource.o ./tools/Sim.o ./tools/StudentstTable.o ./tools/SystemMeasure.o ./tools/Uniform.o ./tools/Weibull.o ./tools/SimulationTime.o
LIBCINQ=libcinqs.so

all: objects $(LIBCINQ) examples

objects:
	cd network; $(MAKE) all
	cd tools; $(MAKE) all

$(LIBCINQ): $(OBJECT)
	$(CC) $(LDFLAGS) $(OBJECT) -o $(LIBCINQ)

examples:
	cd examples; $(MAKE) all

clean:
	cd tools; $(MAKE) clean
	cd network; $(MAKE) clean
	cd examples; $(MAKE) clean
