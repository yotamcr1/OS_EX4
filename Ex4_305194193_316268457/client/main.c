#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <stdio.h>
#include <string.h>
#include "client.h"
#include <winsock2.h>
#include "Massage.h"
#pragma comment(lib,"ws2_32.lib") //Winsock Library
//Authers: Chen Katz And Yotam Carmi

int main(int argc, char* argv[]) {
	
	printf("welcom to main within client project\n");
	if (argc != 4) {
		printf("Number of arguments is not 3. exit\n");
		exit(1);
	}
	char serverIP_Address_str[MAX_ID_SIZE];
	int serverPort;
	char username[MAX_USER_NAME];
	strcpy_s(serverIP_Address_str, MAX_ID_SIZE, argv[1]);
	serverPort = atoi(argv[2]);
	int res = strcpy_s(username, 20, argv[3]);
	ClientMain(username, serverPort, serverIP_Address_str);

}