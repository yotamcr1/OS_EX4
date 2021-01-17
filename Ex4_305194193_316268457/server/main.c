//Authers: Chen Katz And Yotam Carmi
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//this is the main function for Server Project.
//its accept a port from the command line and call to MainServer function within the server.c File

#include "server.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc, char* argv[]) {
	printf("welcom to server main\n");
	if (argc != 2) {
		printf("Number of arguments is not 1. exit\n");
		exit(1);
	}
	//printf("this is the argument: %s", argv[1]);
	int serverport = atoi(argv[1]);
	MainServer(serverport);
}




