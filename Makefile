CXXFLAGS = -std=c++17 -Iinclude
LDLIBS = -lz

COMMON_SRC = src/network/udpSocket.cpp src/protocol/serializer.cpp src/connection/connection.cpp src/connection/connectionManager.cpp
FILE_SRC = $(COMMON_SRC) src/file/fileSerializer.cpp src/file/fileAssembler.cpp src/server/server.cpp

all: build/fileServer build/fileClient

build/fileServer: $(FILE_SRC) src/server/fileServer.cpp
	g++ $(CXXFLAGS) $^ -o $@ $(LDLIBS)

build/fileClient: $(COMMON_SRC) src/file/fileAssembler.cpp src/client/client.cpp src/client/fileClient.cpp
	g++ $(CXXFLAGS) $^ -o $@ $(LDLIBS)

clean:
	rm -f build/fileServer build/fileClient
