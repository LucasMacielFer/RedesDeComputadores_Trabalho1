CXXFLAGS = -std=c++17 -Iinclude
LDLIBS = -lz

COMMON_SRC = src/network/udpSocket.cpp src/protocol/serializer.cpp src/connection/connection.cpp src/connection/connectionManager.cpp

all: build/server build/client

build/server: $(COMMON_SRC) src/server/helloWorldServer.cpp
	g++ $(CXXFLAGS) $^ -o $@ $(LDLIBS)

build/client: $(COMMON_SRC) src/client/helloWorldClient.cpp
	g++ $(CXXFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f build/server build/client
