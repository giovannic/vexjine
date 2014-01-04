include ../../Makefile.inc

all: IoProtocolClient.o  IoProtocol.o IoParameters.o IoSimulator.o IoLogger.o IoPredictor.o IoRecognizer.o PredictionMethod.o PredictionMethodFactory.o IoState.o

CFLAGS+=-c -fPIC $(CPPFLAGS)

IoProtocolClient.o: IoProtocolClient.cpp  IoProtocolClient.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

IoProtocol.o: IoProtocol.cpp  IoProtocol.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

IoSimulator.o: IoSimulator.cpp  IoSimulator.h 
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $(INCLUDE_MODEL_DIRS) $< -o $@

IoParameters.o: IoParameters.cpp  IoParameters.h 
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

IoLogger.o: IoLogger.cpp  IoLogger.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

IoPredictor.o: IoPredictor.cpp  IoPredictor.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

IoState.o: IoState.cpp  IoState.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

IoRecognizer.o: IoRecognizer.cpp  IoRecognizer.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

PredictionMethod.o: PredictionMethod.cpp  PredictionMethod.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

PredictionMethodFactory.o: PredictionMethodFactory.cpp  PredictionMethodFactory.h  
	$(CC) $(CFLAGS) $(INCLUDE_DIRS) $< -o $@

clean:
	rm -rf *.o
