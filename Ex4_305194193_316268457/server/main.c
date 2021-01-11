#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "server.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc, char* argv[]) {
	printf("welcom to server main\n");
	printf("this is the argument: %s", argv[1]);
	int serverport = atoi(argv[1]);
	MainServer(serverport);
}

