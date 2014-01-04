include ../Makefile.inc

DEBUGGING_OBJS=./debugging/DebugUtil.o ./debugging/Visualizer.o ./debugging/Logger.o ./debugging/LinearRegression.o ./debugging/AutoRegression.o
DS_OBJS=./ds/AdaptiblePQueue.o ./ds/ThreadQueue.o
EVENT_OBJS=./events/MethodEventsBehaviour.o ./events/ThreadEventsBehaviour.o
MODELS_OBJS=./models/MethodPerformanceModel.o ./models/VexInfiniteServerNode.o ./models/VexQueueingNode.o ./models/PostActionNode.o ./models/VexLocalInfiniteServerNode.o ./models/VexLocalQueueingNode.o ./models/VexSchedSimQueueingNetwork.o ./models/VexSchedSimCustomer.o ./models/VexQueueingNetwork.o ./models/VexThreadCustomer.o ./models/parser/JmtNetworkParser.o ./models/parser/JmtNodeParser.o ./models/parser/JmtParser.o ./models/parser/JmtProbabilitiesParser.o ./models/parser/JmtQueueParser.o ./models/parser/tinyxml/tinystr.o ./models/parser/tinyxml/tinyxml.o ./models/parser/tinyxml/tinyxmlerror.o ./models/parser/tinyxml/tinyxmlparser.o
IO_OBJS=./io/IoProtocolClient.o ./io/IoParameters.o ./io/PredictionMethodFactory.o ./io/IoProtocol.o ./io/IoSimulator.o ./io/IoLogger.o ./io/IoPredictor.o ./io/IoRecognizer.o ./io/PredictionMethod.o ./io/IoState.o
PROFILER_OBJS=./profiler/MethodCallInfo.o ./profiler/MethodData.o ./profiler/EventLogger.o ./profiler/PerformanceMeasure.o ./profiler/ProfilingInvalidationPolicy.o
REGISTRY_OBJS=./registries/ObjectRegistry.o ./registries/LockRegistry.o ./registries/ThreadRegistry.o ./registries/ThreadManagerRegistry.o
SCHED_NO_DISTR_SERVER_OBJS=./scheduler/PassiveManager.o ./scheduler/ThreadManager.o ./scheduler/StrictParallelThreadManager.o ./scheduler/LooseParallelThreadManager.o ./scheduler/distributed/ThreadManagerClient.o
SCHED_OBJS=./scheduler/distributed/ThreadManagerServer.o $(SCHED_NO_DISTR_SERVER_OBJS)
THREAD_OBJS=./threads/ThreadState.o ./threads/AggregateStateCounters.o ./threads/ModelHandler.o ./threads/IoHandler.o ./threads/MethodLog.o ./threads/Scheduling.o ./threads/State.o ./threads/Statistics.o ./threads/Timers.o ./threads/NativeWaitingCriteria.o
TIMER_OBJS=./timer/Time.o ./timer/VirtualTimeline.o ./timer/TimeLogger.o

OBJECTS=$(PROFILER_OBJS) $(DEBUGGING_OBJS) $(DS_OBJS) $(IO_OBJS) $(SCHED_OBJS) $(THREAD_OBJS) $(TIMER_OBJS) $(REGISTRY_OBJS) $(EVENT_OBJS) $(MODELS_OBJS) VTF.o

VTFSERVER=vtfServer
NO_DISTR_SERVER_OBJECTS=$(PROFILER_OBJS) $(DEBUGGING_OBJS) $(DS_OBJS) $(IO_OBJS) $(SCHED_NO_DISTR_SERVER_OBJS) $(THREAD_OBJS) $(TIMER_OBJS) $(EVENT_OBJS) $(MODELS_OBJS) $(REGISTRY_OBJS) VTF.o

OBJCAT=debugging.o ds.o models.o events.o io.o profiler.o registries.o scheduler.o threads.o timer.o

all: $(OBJCAT) $(OBJECTS) $(VEX_LIBRARY_NAME) 

server: $(VTFSERVER)

$(VEX_LIBRARY_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(VEX_LIBRARY_NAME) $(LDDIRS) $(LDLIBS) 
	cp $(VEX_LIBRARY_NAME) $(OUTPUT_LIB_DIR)

$(VTFSERVER): $(OBJECTS)
	$(CC) $(LDFLAGS) $(INCLUDE_DIRS) $(OBJECTS) $(NO_DISTR_SERVER_OBJECTS) -o $(VTFSERVER) ./scheduler/distributed/ThreadManagerServer.cpp  $(LDDIRS) $(LDLIBS)


VTF.o: VTF.cpp VTF.h
	$(CC) $(CFLAGS) -c -fPIC $(CPPFLAGS) $(INCLUDE_DIRS) $(INCLUDE_MODEL_DIRS) $(INCLUDE_PAPIPROFILER_DIR) $(INCLUDE_CINQS_DIRS) $< -o $@

debugging.o: 
	cd debugging/ ; $(MAKE) $(MAKECMDGOALS)

ds.o: 
	cd ds/ ; $(MAKE) $(MAKECMDGOALS)

models.o: 
	cd models/ ; $(MAKE) $(MAKECMDGOALS)

events.o:
	cd events/ ; $(MAKE) $(MAKECMDGOALS)

io.o: 
	cd io/ ; $(MAKE) $(MAKECMDGOALS)

profiler.o: 
	cd profiler/ ; $(MAKE) $(MAKECMDGOALS)

registries.o: 
	cd registries/ ; $(MAKE) $(MAKECMDGOALS)

scheduler.o: 
	cd scheduler/ ; $(MAKE) $(MAKECMDGOALS)

threads.o: 
	cd threads/ ; $(MAKE) $(MAKECMDGOALS)

timer.o:
	cd timer/ ; $(MAKE) $(MAKECMDGOALS)

clean:
	rm -rf *.o $(VEX_LIBRARY_NAME)
	cd debugging/ ; $(MAKE) clean
	cd events/ ; $(MAKE) clean
	cd models/ ; $(MAKE) clean
	cd ds/ ; $(MAKE) clean
	cd io/ ; $(MAKE) clean
	cd profiler/ ; $(MAKE) clean
	cd registries/ ; $(MAKE) clean
	cd scheduler/ ; $(MAKE) clean
	cd threads/ ; $(MAKE) clean
	cd timer/ ; $(MAKE) clean
