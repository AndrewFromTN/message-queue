CXX = g++
CPPFLAGS = -g -std=c++11
LDFLAGS = -lzmq

all: MessageQueue StatisticsConnector MonitorConnector

StatisticsConnector: StatisticsConnector.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

MessageQueue: MessageQueue.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

MonitorConnector: MonitorConnector.o
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)

StatisticsConnector.o: StatisticsConnector.cpp
MessageQueue.o: MessageQueue.cpp
MonitorConnector.o: MonitorConnector.cpp

.PHONY: clean
clean:
	rm -rf MessageQueue StatisticsConnector MonitorConnector *.o
