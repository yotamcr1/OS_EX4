#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <string.h>
#include "client.h"
#include <winsock2.h>
#include "Massage.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library

int main(int argc, char* argv[]) {
	
	printf("welcom to main within client project\n");
	unsigned long serverIP_Address;
	int serverPort;
	char username[MAX_USER_NAME];

	serverIP_Address = inet_addr(argv[1]);
	serverPort = atoi(argv[2]);
	int res = strcpy_s(username, 20, argv[3]);

	//TBD: CHECKS INPUTS AND ETC

	ClientMain(username, serverPort, serverIP_Address);

}