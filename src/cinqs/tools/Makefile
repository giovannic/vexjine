include ../../Makefile.inc

CFLAGS+=-fPIC -I../network/ -I..

OBJECTS=Geometric.o AreaHistogram.o Cauchy.o Check.o ContEmpirical.o DistributionSampler.o CustomerMeasure.o Deterministic.o Diary.o DiscEmpirical.o DiscreteSampler.o SimulationTime.o EmptyListException.o Erlang.o Event.o Exp.o Gamma.o Histogram.o List.o Logger.o Measure.o Normal.o OrderedList.o Pareto.o RejectionMethod.o ReplicatedSim.o Resource.o Sim.o StudentstTable.o SystemMeasure.o Uniform.o Weibull.o

all: $(OBJECTS)

Geometric.o: Geometric.cpp Geometric.h
	$(CC) $(CFLAGS) -c Geometric.cpp

AreaHistogram.o: AreaHistogram.cpp AreaHistogram.h
	$(CC) $(CFLAGS) -c AreaHistogram.cpp

SimulationTime.o: SimulationTime.cpp SimulationTime.h
	$(CC) $(CFLAGS) -c SimulationTime.cpp
	
Cauchy.o: Cauchy.cpp Cauchy.h
	$(CC) $(CFLAGS) -c Cauchy.cpp

Check.o: Check.cpp Check.h
	$(CC) $(CFLAGS) -c Check.cpp

ContEmpirical.o: ContEmpirical.cpp ContEmpirical.h
	$(CC) $(CFLAGS) -c ContEmpirical.cpp

CustomerMeasure.o: CustomerMeasure.cpp CustomerMeasure.h
	$(CC) $(CFLAGS) -c CustomerMeasure.cpp

Deterministic.o: Deterministic.cpp Deterministic.h
	$(CC) $(CFLAGS) -c Deterministic.cpp

Diary.o: Diary.cpp Diary.h
	$(CC) $(CFLAGS) -c Diary.cpp

DiscEmpirical.o: DiscEmpirical.cpp DiscEmpirical.h
	$(CC) $(CFLAGS) -c DiscEmpirical.cpp

DiscreteSampler.o: DiscreteSampler.cpp DiscreteSampler.h
	$(CC) $(CFLAGS) -c DiscreteSampler.cpp

DistributionSampler.o: DistributionSampler.cpp DistributionSampler.h
	$(CC) $(CFLAGS) -c DistributionSampler.cpp

EmptyListException.o: EmptyListException.cpp EmptyListException.h
	$(CC) $(CFLAGS) -c EmptyListException.cpp

Erlang.o: Erlang.cpp Erlang.h
	$(CC) $(CFLAGS) -c Erlang.cpp

Event.o: Event.cpp Event.h
	$(CC) $(CFLAGS) -c Event.cpp

Exp.o: Exp.cpp Exp.h
	$(CC) $(CFLAGS) -c Exp.cpp

Gamma.o: Gamma.cpp Gamma.h
	$(CC) $(CFLAGS) -c Gamma.cpp

Histogram.o: Histogram.cpp Histogram.h
	$(CC) $(CFLAGS) -c Histogram.cpp

List.o: List.cpp List.h
	$(CC) $(CFLAGS) -c List.cpp

Logger.o: Logger.cpp Logger.h
	$(CC) $(CFLAGS) -c Logger.cpp

Measure.o: Measure.cpp Measure.h
	$(CC) $(CFLAGS) -c Measure.cpp

Normal.o: Normal.cpp Normal.h
	$(CC) $(CFLAGS) -c Normal.cpp

OrderedList.o: OrderedList.cpp OrderedList.h
	$(CC) $(CFLAGS) -c OrderedList.cpp

Pareto.o: Pareto.cpp Pareto.h
	$(CC) $(CFLAGS) -c Pareto.cpp

RejectionMethod.o: RejectionMethod.cpp RejectionMethod.h
	$(CC) $(CFLAGS) -c RejectionMethod.cpp

ReplicatedSim.o: ReplicatedSim.cpp ReplicatedSim.h
	$(CC) $(CFLAGS) -c ReplicatedSim.cpp

Resource.o: Resource.cpp Resource.h
	$(CC) $(CFLAGS) -c Resource.cpp

Sim.o: Sim.cpp Sim.h
	$(CC) $(CFLAGS) -c Sim.cpp

StudentstTable.o: StudentstTable.cpp StudentstTable.h
	$(CC) $(CFLAGS) -c StudentstTable.cpp

SystemMeasure.o: SystemMeasure.cpp SystemMeasure.h
	$(CC) $(CFLAGS) -c SystemMeasure.cpp

Uniform.o: Uniform.cpp Uniform.h
	$(CC) $(CFLAGS) -c Uniform.cpp

Weibull.o: Weibull.cpp Weibull.h
	$(CC) $(CFLAGS) -c Weibull.cpp

clean:
	rm -f *.o
