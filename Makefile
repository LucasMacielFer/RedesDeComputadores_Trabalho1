all:
	g++ -std=c++17 -Iinclude src/udpSocket.cpp src/server/helloWorldServer.cpp -o build/server
	g++ -std=c++17 -Iinclude src/udpSocket.cpp src/client/helloWorldClient.cpp -o build/client
